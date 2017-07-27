/*
 * Copyright 2009-2011, Axel Dörfler, axeld@pinc-software.de.
 * Copyright 2016, Rene Gollent, rene@gollent.com.
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 */


#include "StreamMessenger.h"

#include <Message.h>
#include <Messenger.h>
#include <ObjectList.h>

#include <AutoDeleter.h>
#include <AutoLocker.h>
#include <HashMap.h>

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
			typedef SynchronizedHashMap<HashKey64<int64>,
									BMessage> ReplyMessageMap;
	Impl()
		:
		fStream(NULL),
		fMessageWaiters(-1),
		fReplyReader(-1),
		fReceivedReplies(),
		fReceivedMessages(16, true),
		fReplyIDCounter(0)
	{
	}

	~Impl()
	{
		if (fMessageWaiters > 0)
			delete_sem(fMessageWaiters);
		if (fReplyReader > 0)
			wait_for_thread(fReplyReader, NULL);

		ClearMessages();
	}

	void ClearMessages()
	{
		fReceivedReplies.Clear();
		AutoLocker<BLocker> queueLocker(fReceivedMessageLock);
		for (int32 i = 0; Message* message = fReceivedMessages.ItemAt(i); i++)
			delete message;
	}

	status_t Init(Stream* stream)
	{
		fMessageWaiters = create_sem(0, "message waiters");
		if (fMessageWaiters < 0)
			return fMessageWaiters;

		fStream = stream;

		fReplyReader = spawn_thread(&_MessageReaderEntry,
			"Message Reader", B_NORMAL_PRIORITY, this);
		if (fReplyReader < 0)
			return fReplyReader;

		return resume_thread(fReplyReader);
	}

	void Uninit()
	{
		if (fStream != NULL)
			fStream->Close();

		if (fReplyReader >= 0) {
			wait_for_thread(fReplyReader, NULL);
			fReplyReader = -1;
		}

		ClearMessages();

// TODO: This mechanism is probably racy!
		if (fMessageWaiters >= 0)
			release_sem_etc(fMessageWaiters, 1, B_RELEASE_ALL);
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
		status_t error = B_OK;
		AutoLocker<BLocker> queueLocker(fReceivedMessageLock);
		for (;;) {
			if (!fReceivedMessages.IsEmpty()) {
				Message* nextMessage = fReceivedMessages.RemoveItemAt(0);
				_message = nextMessage->fMessage;
				_messageId = nextMessage->fId;
				delete nextMessage;
				break;
			}

			queueLocker.Unlock();
			error = _WaitForMessage(timeout);
			if (error != B_OK)
				break;
// 			if (!fSocket.IsConnected()) {
// 				error = B_CANCELED;
// 				break;
// 			}
			queueLocker.Lock();
		}

		return error;
	}

private:
	typedef BObjectList<Message> MessageQueue;

private:
	uint64 _NextMessageId()
	{
		return (uint64)atomic_add64(&fReplyIDCounter, 1);
	}

	status_t _WaitForMessage(bigtime_t timeout)
	{
		for (;;) {
			status_t error = acquire_sem_etc(fMessageWaiters, 1,
				B_RELATIVE_TIMEOUT, timeout);
			if (error == B_INTERRUPTED) {
				if (timeout != B_INFINITE_TIMEOUT)
					timeout -= system_time();
				continue;
			}
			if (error != B_OK)
				return error;
			break;
		}

		return B_OK;
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
		status_t error = B_OK;
		for (;;) {
			if (fReceivedReplies.ContainsKey(replyID)) {
				reply = fReceivedReplies.Remove(replyID);
				break;
			}

			error = _WaitForMessage(timeout);
			if (error != B_OK)
				break;
// 			if (!fSocket.IsConnected()) {
// 				error = B_CANCELED;
// 				break;
// 			}
		}

		return error;
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

				AutoLocker<BLocker> queueLocker(fReceivedMessageLock);
				if (!fReceivedMessages.AddItem(queueMessage)) {
					delete queueMessage;
					error = B_NO_MEMORY;
					break;
				}
			}

			release_sem_etc(fMessageWaiters, 1, B_RELEASE_ALL);
		}

		// if we exit our message loop, ensure everybody wakes up and knows
		// we're no longer receiving messages.
		release_sem_etc(fMessageWaiters, 1, B_RELEASE_ALL);
		return error;
	}

private:
	Stream*				fStream;
	sem_id				fMessageWaiters;
	thread_id			fReplyReader;
	ReplyMessageMap		fReceivedReplies;
	BLocker				fReceivedMessageLock;
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

	fImpl->Uninit();

	delete fImpl;
	fImpl = NULL;
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
