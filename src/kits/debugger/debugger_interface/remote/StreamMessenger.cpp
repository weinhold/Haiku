/*
 * Copyright 2009-2011, Axel Dörfler, axeld@pinc-software.de.
 * Copyright 2016, Rene Gollent, rene@gollent.com.
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 */


#include "StreamMessenger.h"

#include <time.h>

#include <Message.h>
#include <Messenger.h>
#include <ObjectList.h>

#include <AutoDeleter.h>
#include <AutoLocker.h>
#include <HashMap.h>
#include <PthreadMutexLocker.h>

#include "Stream.h"


namespace {


static const uint64 kIsReplyFlag		= 0x8000000000000000;
static const uint64 kSizeMask			= 0x7fffffffffffffff;
static const uint64 kMaxSaneMessageSize	= 10 * 1024 * 1024;


struct MessageHeader {
	uint64	size;
	uint64	id;
};


struct Message {
	StreamMessenger::MessageId	fId;
	BMessage					fMessage;

	Message(StreamMessenger::MessageId id, const BMessage& message)
		:
		fId(id),
		fMessage(message)
	{
	}
};


} // anonymous namespace


// #pragma mark - StreamMessenger::Impl


struct StreamMessenger::Impl {
	typedef HashMap<HashKey64<int64>, BMessage> ReplyMessageMap;

	Impl()
		:
		fLock(),
		fMessageReceivedCondition(),
		fTerminating(false),
		fStream(NULL),
		fReplyReader(-1),
		fReceivedReplies(),
		fReceivedMessages(16, true),
		fReplyIDCounter(0)
	{
		pthread_mutex_init(&fLock, NULL);
		pthread_cond_init(&fMessageReceivedCondition, NULL);
	}

	~Impl()
	{
		Close();

		pthread_cond_destroy(&fMessageReceivedCondition);
		pthread_mutex_destroy(&fLock);
	}

	status_t Init(Stream* stream)
	{
		fStream = stream;

		fReplyReader = spawn_thread(&_MessageReaderEntry,
			"Message Reader", B_NORMAL_PRIORITY, this);
		if (fReplyReader < 0)
			return fReplyReader;

		return resume_thread(fReplyReader);
	}

	void Close()
	{
		{
			PthreadMutexLocker locker(fLock);
			if (fTerminating)
				return;
			fTerminating = true;
			pthread_cond_broadcast(&fMessageReceivedCondition);
		}

		if (fStream != NULL)
			fStream->Close();

		if (fReplyReader > 0) {
			wait_for_thread(fReplyReader, NULL);
			fReplyReader = -1;
		}

		PthreadMutexLocker locker(fLock);
		fReceivedReplies.Clear();
		for (int32 i = 0; Message* message = fReceivedMessages.ItemAt(i); i++)
			delete message;
	}

	status_t SendMessage(const BMessage& message, MessageId& _messageId)
	{
		uint64 messageId = _NextMessageId();

		status_t error = _SendMessage(message, messageId, false);
		if (error == B_OK)
			_messageId = messageId;
		return error;
	}

	status_t SendMessage(const BMessage& message, BMessage& _reply,
		bigtime_t timeout)
	{
		uint64 replyID;
		status_t error = SendMessage(message, replyID);
		if (error != B_OK)
			return error;

		return _ReadReply(replyID, _reply, timeout);
	}

	status_t SendReply(MessageId messageId, const BMessage& reply)
	{
		return _SendMessage(reply, messageId, true);
	}

	status_t ReceiveMessage(BMessage& _message, MessageId& _messageId,
		bigtime_t timeout)
	{
		PthreadMutexLocker locker(fLock);

		for (;;) {
			if (fTerminating)
				return B_CANCELED;

			if (!fReceivedMessages.IsEmpty()) {
				Message* nextMessage = fReceivedMessages.RemoveItemAt(0);
				_message = nextMessage->fMessage;
				_messageId = nextMessage->fId;
				delete nextMessage;
				return B_OK;
				break;
			}

			status_t error = _WaitForMessage(timeout);
			if (error != B_OK)
				return error;
		}
	}

private:
	typedef BObjectList<Message> MessageQueue;

private:
	uint64 _NextMessageId()
	{
		return (uint64)atomic_add64(&fReplyIDCounter, 1);
	}

	/*! Waits for a newly received message. fLock must be held. */
	status_t _WaitForMessage(bigtime_t timeout)
	{
		// compute absolute timeout
		timespec absoluteTimeout;
		if (timeout != B_INFINITE_TIMEOUT) {
			clock_gettime(CLOCK_REALTIME, &absoluteTimeout);
			absoluteTimeout.tv_sec += timeout / 1000000;
			nanotime_t nanos = (timeout % 1000000) * 1000
				+ absoluteTimeout.tv_nsec;
			if (nanos > 1000000000) {
				nanos -= 1000000000;
				absoluteTimeout.tv_sec++;
			}
			absoluteTimeout.tv_nsec = (long)nanos;
		}

		// wait until we get a message
		for (;;) {
			if (fTerminating)
				return B_CANCELED;

			status_t error;
			if (timeout == B_INFINITE_TIMEOUT) {
				error = pthread_cond_wait(&fMessageReceivedCondition, &fLock);
			} else {
				error = pthread_cond_timedwait(&fMessageReceivedCondition,
					&fLock, &absoluteTimeout);
			}
			if (error == B_INTERRUPTED)
				continue;
			return error;
		}
	}

	status_t _SendMessage(const BMessage& message, MessageId messageId,
		bool isReply)
	{
		ssize_t flatSize = message.FlattenedSize();
		ssize_t totalSize = flatSize + sizeof(MessageHeader);

		char* buffer = new(std::nothrow) char[totalSize];
		if (buffer == NULL)
			return B_NO_MEMORY;

		ArrayDeleter<char> bufferDeleter(buffer);

		MessageHeader* header = (MessageHeader*)buffer;
		header->size = B_HOST_TO_BENDIAN_INT64(
			totalSize | (isReply ? kIsReplyFlag : 0));
		header->id = B_HOST_TO_BENDIAN_INT64(messageId);

		char* messageBuffer = buffer + sizeof(MessageHeader);
		status_t error = message.Flatten(messageBuffer, flatSize);
		if (error != B_OK)
			return error;

		return fStream->WriteExactly(buffer, totalSize);
	}

	status_t _ReadMessage(BMessage& _message, MessageId& _messageId,
		bool& _isReply)
	{
		MessageHeader header;
		status_t error = fStream->ReadExactly(&header, sizeof(header));
		if (error != B_OK)
			return error;

		header.size = B_BENDIAN_TO_HOST_INT64(header.size);
		header.id = B_BENDIAN_TO_HOST_INT64(header.id);

		if ((header.size & kSizeMask) > kMaxSaneMessageSize)
			return B_BAD_DATA;
		size_t size = size_t(header.size & kSizeMask);

		char* buffer = new(std::nothrow) char[size];
		if (buffer == NULL)
			return B_NO_MEMORY;

		ArrayDeleter<char> bufferDeleter(buffer);
		error = fStream->ReadExactly(buffer, size);
		if (error != B_OK)
			return error;

		error = _message.Unflatten(buffer);
		if (error != B_OK)
			return error;

		_messageId = header.id;
		_isReply = (header.size & kIsReplyFlag) != 0;
		return B_OK;
	}

	status_t _ReadReply(const int64 replyID, BMessage& reply,
		bigtime_t timeout)
	{
		PthreadMutexLocker locker(fLock);

		for (;;) {
			if (fTerminating)
				return B_CANCELED;

			if (fReceivedReplies.ContainsKey(replyID)) {
				reply = fReceivedReplies.Remove(replyID);
				return B_OK;
			}

			status_t error = _WaitForMessage(timeout);
			if (error != B_OK)
				return error;
		}
	}

	static status_t _MessageReaderEntry(void* arg)
	{
		return ((StreamMessenger::Impl*)arg)->_MessageReader();
	}

	status_t _MessageReader()
	{
		status_t error = B_OK;

		for (;;) {
			BMessage message;
			MessageId messageId;
			bool isReply;
			error = _ReadMessage(message, messageId, isReply);
			if (error != B_OK)
				break;

			if (isReply) {
				PthreadMutexLocker locker(fLock);
				error = fReceivedReplies.Put(messageId, message);
				if (error != B_OK)
					break;
			} else {
				Message* queueMessage
					= new(std::nothrow) Message(messageId, message);
				if (queueMessage == NULL) {
					error = B_NO_MEMORY;
					break;
				}

				PthreadMutexLocker locker(fLock);
				if (!fReceivedMessages.AddItem(queueMessage)) {
					delete queueMessage;
					error = B_NO_MEMORY;
					break;
				}
			}

			PthreadMutexLocker locker(fLock);
			pthread_cond_broadcast(&fMessageReceivedCondition);
		}

		// if we exit our message loop, ensure everybody wakes up and knows
		// we're no longer receiving messages.
		PthreadMutexLocker locker(fLock);
		fTerminating = true;
		pthread_cond_broadcast(&fMessageReceivedCondition);
		return error;
	}

private:
	pthread_mutex_t		fLock;
	pthread_cond_t		fMessageReceivedCondition;
	bool				fTerminating;
	Stream*				fStream;
	thread_id			fReplyReader;
	ReplyMessageMap		fReceivedReplies;
	MessageQueue		fReceivedMessages;
	int64				fReplyIDCounter;
};


// #pragma mark - StreamMessenger


StreamMessenger::StreamMessenger()
	:
	fImpl(NULL)
{
}


StreamMessenger::~StreamMessenger()
{
	Unset();
}


status_t
StreamMessenger::SetTo(Stream* stream)
{
	if (fImpl != NULL)
		return B_BAD_VALUE;

	fImpl = new(std::nothrow) StreamMessenger::Impl;
	if (fImpl == NULL)
		return B_NO_MEMORY;

	return fImpl->Init(stream);
}


void
StreamMessenger::Unset()
{
	if (fImpl == NULL)
		return;

	fImpl->Close();

	delete fImpl;
	fImpl = NULL;
}


void
StreamMessenger::Close()
{
	if (fImpl == NULL)
		return;

	fImpl->Close();
}


status_t
StreamMessenger::SendMessage(const BMessage& message, MessageId& _messageId)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->SendMessage(message, _messageId);
}


status_t
StreamMessenger::SendMessage(const BMessage& message, BMessage& _reply,
	bigtime_t timeout)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->SendMessage(message, _reply, timeout);
}


status_t
StreamMessenger::SendReply(MessageId messageId, const BMessage& reply)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->SendReply(messageId, reply);
}


status_t
StreamMessenger::ReceiveMessage(BMessage& _message, MessageId& _messageId,
	bigtime_t timeout)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->ReceiveMessage(_message, _messageId, timeout);
}
