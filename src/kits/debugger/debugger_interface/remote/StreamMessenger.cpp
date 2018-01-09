/*
 * Copyright 2009-2011, Axel Dörfler, axeld@pinc-software.de.
 * Copyright 2016, Rene Gollent, rene@gollent.com.
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 */


#include "debugger_interface/remote/StreamMessenger.h"

#include <assert.h>
#include <time.h>

#include <ByteOrder.h>
#include <Message.h>
#include <Messenger.h>
#include <ObjectList.h>
#include <Referenceable.h>

#include <AutoDeleter.h>
#include <AutoLocker.h>
#include <PthreadMutexLocker.h>

#include <util/DoublyLinkedList.h>
#include <util/OpenHashTable.h>

#include "debugger_interface/remote/Stream.h"
#include "Tracing.h"


namespace {


static const uint64 kIsReplyFlag		= 0x8000000000000000;
static const uint64 kSizeMask			= 0x7fffffffffffffff;
static const uint64 kMaxSaneMessageSize	= 10 * 1024 * 1024;


typedef ChannelMessenger::ChannelId	ChannelId;
typedef ChannelMessenger::MessageId	MessageId;
typedef ChannelMessenger::Envelope	Envelope;


struct MessageHeader {
	uint64	size;
	uint64	messageId;
	uint64	channelId;
};


struct Message : DoublyLinkedListLinkImpl<Message> {
	Envelope						envelope;
	BMessage						message;

	// conceptually private
	DoublyLinkedListLink<Message>	channelLink;
	Message*						hashTableLink;
};


struct MessageHashTableDefinition {
	typedef MessageId	KeyType;
	typedef Message		ValueType;

	size_t HashKey(KeyType key) const
	{
		return (size_t)key;
	}

	size_t Hash(ValueType* value) const
	{
		return HashKey(value->envelope.messageId);
	}

	bool Compare(KeyType key, ValueType* value) const
	{
		return value->envelope.messageId == key;
	}

	ValueType*& GetLink(ValueType* value) const
	{
		return value->hashTableLink;
	}
};

typedef BOpenHashTable<MessageHashTableDefinition> MessageHashTable;


template<typename ActualClass>
struct Waiter : DoublyLinkedListLinkImpl<ActualClass> {
	Waiter()
		:
		fCondition(NULL),
		fNotified(false),
		fNotifyStatus(B_OK)
	{
	}

	void Notify(status_t status)
	{
		assert(fCondition != NULL);

		if (!fNotified) {
			fNotified = true;
			fNotifyStatus = status;
			pthread_cond_broadcast(fCondition);
		}
	}

	status_t Wait(pthread_mutex_t& mutex, bigtime_t timeout)
	{
		fNotified = false;
		pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
		fCondition = &condition;

		status_t error = _Wait(mutex, timeout);

		pthread_cond_destroy(&condition);

		return error;
	}

private:
	status_t _Wait(pthread_mutex_t& mutex, bigtime_t timeout)
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
			status_t error;
			if (timeout == B_INFINITE_TIMEOUT) {
				error = pthread_cond_wait(fCondition, &mutex);
			} else {
				error = pthread_cond_timedwait(fCondition, &mutex,
					&absoluteTimeout);
			}

			// Regardless of the error. If notified, things are good.
			if (fNotified)
				return fNotifyStatus;

			if (error != B_OK) {
				if (error == B_INTERRUPTED)
					continue;
				return error;
			}
		}
	}

private:
	pthread_cond_t*	fCondition;
	bool			fNotified;
	status_t		fNotifyStatus;
};


template<typename Waiter>
struct WaiterQueue {
	WaiterQueue()
		:
		fWaiters()
	{
	}

	Waiter* NotifyOne(status_t status)
	{
		Waiter* waiter = fWaiters.RemoveHead();
		if (waiter != NULL)
			waiter->Notify(status);

		return waiter;
	}

	void NotifyAll(status_t status)
	{
		while (NotifyOne(status)) {
		}
	}

	status_t Wait(Waiter& waiter, pthread_mutex_t& mutex, bigtime_t timeout)
	{
		fWaiters.Add(&waiter);
		return waiter.Wait(mutex, timeout);
	}

private:
	typedef DoublyLinkedList<Waiter> WaiterList;

private:
	WaiterList	fWaiters;
};


struct MessageWaiter : Waiter<MessageWaiter> {
	MessageWaiter()
		:
		fMessage(NULL)
	{
	}

	Message* GetMessage() const
	{
		return fMessage;
	}

	void SetMessage(Message* message)
	{
		fMessage = message;
	}

private:
	Message*	fMessage;
};


struct ReplyWaiter : MessageWaiter {
	ReplyWaiter(MessageId messageId)
		:
		MessageWaiter(),
		fMessageId(messageId)
	{
	}

	MessageId GetMessageId() const
	{
		return fMessageId;
	}

	void SetMessageId(MessageId messageId)
	{
		fMessageId = messageId;
	}

	ReplyWaiter*& HashTableNext()
	{
		return fHashTableNext;
	}

private:
	MessageId		fMessageId;
	ReplyWaiter*	fHashTableNext;
};


struct ReplyWaiterHashTableDefinition {
	typedef MessageId	KeyType;
	typedef ReplyWaiter	ValueType;

	size_t HashKey(KeyType key) const
	{
		return (size_t)key;
	}

	size_t Hash(ValueType* value) const
	{
		return HashKey(value->GetMessageId());
	}

	bool Compare(KeyType key, ValueType* value) const
	{
		return value->GetMessageId() == key;
	}

	ValueType*& GetLink(ValueType* value) const
	{
		return value->HashTableNext();
	}
};

typedef BOpenHashTable<ReplyWaiterHashTableDefinition> ReplyWaiterHashTable;


template<typename GetLink>
struct BlockingMessageQueue {
	BlockingMessageQueue()
		:
		fMessages(),
		fWaiters(),
		fClosed(false)
	{
	}

	void Close()
	{
		fClosed = true;
		fWaiters.NotifyAll(B_CANCELED);
	}

	bool IsClosed() const
	{
		return fClosed;
	}

	/*!	Appends the message to the queue. Returns whether the message has been
		appended: false, if it has already been assigned to a waiter.
	*/
	bool AppendMessage(Message* message)
	{
		if (MessageWaiter* waiter = fWaiters.NotifyOne(B_OK)) {
			waiter->SetMessage(message);
			return false;
		}

		fMessages.Add(message);
		return true;
	}

	Message* PopMessage()
	{
		return fMessages.RemoveHead();
	}

	status_t PopMessage(pthread_mutex_t& mutex, bigtime_t timeout,
		Message*& _message)
	{
		if (fClosed)
			return B_CANCELED;

		if (Message* message = fMessages.RemoveHead()) {
			assert(message != NULL);
			_message = message;
			return B_OK;
		}

		MessageWaiter waiter;
		status_t error = fWaiters.Wait(waiter, mutex, timeout);
		if (error != B_OK)
			return error;
		if (fClosed)
			return B_CANCELED;

		assert(waiter.GetMessage() != NULL);
		_message = waiter.GetMessage();
		return B_OK;
	}

	void RemoveMessage(Message* message)
	{
		fMessages.Remove(message);
	}

private:
	typedef DoublyLinkedList<Message, GetLink> MessageList;

private:
	MessageList					fMessages;
	WaiterQueue<MessageWaiter>	fWaiters;
	bool						fClosed;
};


typedef BlockingMessageQueue<DoublyLinkedListStandardGetLink<Message> >
	MessageQueue;
typedef BlockingMessageQueue<
	DoublyLinkedListMemberGetLink<Message,
		&Message::channelLink> > ChannelMessageQueue;


struct Channel : BReferenceable, ChannelMessageQueue {
	Channel(ChannelId id)
		:
		fId(id),
		fHashTableNext(NULL)
	{
	}

	~Channel()
	{
	}

	ChannelId Id() const
	{
		return fId;
	}

	Channel*& HashTableNext()
	{
		return fHashTableNext;
	}

private:
	ChannelId			fId;
	ChannelMessageQueue	fMessages;
	pthread_cond_t		fMessageAvailableCondition;
	Channel*			fHashTableNext;
};


struct ChannelHashTableDefinition {
	typedef ChannelId	KeyType;
	typedef Channel		ValueType;

	size_t HashKey(KeyType key) const
	{
		return (size_t)key;
	}

	size_t Hash(ValueType* value) const
	{
		return HashKey(value->Id());
	}

	bool Compare(KeyType key, ValueType* value) const
	{
		return value->Id() == key;
	}

	ValueType*& GetLink(ValueType* value) const
	{
		return value->HashTableNext();
	}
};


typedef BOpenHashTable<ChannelHashTableDefinition> ChannelHashTable;


} // anonymous namespace


// #pragma mark - StreamMessenger::Impl


struct StreamMessenger::Impl {
	Impl()
		:
		fLock(),
		fTerminating(false),
		fStream(),
		fDefaultChannel(NULL),
		fReplyReader(-1),
		fReplyWaiters(),
		fChannels(),
		fReceivedMessages(),
		fChannelIdCounter(kDefaultChannelId + 1),
		fReplyIdCounter(0)
	{
		pthread_mutex_init(&fLock, NULL);
	}

	~Impl()
	{
		Close();

		pthread_mutex_destroy(&fLock);
	}

	status_t Init(Stream* stream)
	{
		fStream = stream;

		status_t error = fReplyWaiters.Init();
		if (error != B_OK)
			return error;

		error = fChannels.Init();
		if (error != B_OK)
			return error;

		error = _NewChannel(kDefaultChannelId);
		if (error != B_OK)
			return error;

		fReplyReader = spawn_thread(&_MessageReaderEntry,
			"Message Reader", B_NORMAL_PRIORITY, this);
		if (fReplyReader < 0)
			return fReplyReader;

		return resume_thread(fReplyReader);
	}

	void Close()
	{
		if (!_Close())
			return;

		if (fReplyReader > 0) {
			wait_for_thread(fReplyReader, NULL);
			fReplyReader = -1;
		}

		PthreadMutexLocker locker(fLock);

		// delete all remaining messages (shouldn't be any)
		while (Message* message = fReceivedMessages.PopMessage())
			delete message;
	}

	status_t NewChannel(ChannelId& _id)
	{
		ChannelId channelId = _NextChannelId();
		status_t error = _NewChannel(channelId);
		if (error != B_OK)
			return error;

		_id = channelId;
		return B_OK;
	}

	status_t DeleteChannel(ChannelId channelId)
	{
		if (channelId == kDefaultChannelId)
			return B_NOT_ALLOWED;

		Channel* channel = _RemoveChannel(channelId);
		if (channel == NULL)
			return B_BAD_VALUE;


		channel->ReleaseReference();
		return B_OK;
	}

	status_t SendMessage(ChannelId channelId, const BMessage& message,
		MessageId& _messageId)
	{
		uint64 messageId = _NextMessageId();

		status_t error = _SendMessage(Envelope(channelId, messageId), message,
			false);
		if (error == B_OK)
			_messageId = messageId;
		return error;
	}

	status_t SendMessage(ChannelId channelId, const BMessage& message,
		BMessage& _reply, bigtime_t timeout)
	{
		uint64 messageId = _NextMessageId();

		// register the reply waiter
		PthreadMutexLocker locker(fLock);

		ReplyWaiter waiter(messageId);
		fReplyWaiters.Insert(&waiter);

		locker.Unlock();

		// send the message
		status_t error = _SendMessage(Envelope(channelId, messageId), message,
			false);

		locker.Lock();

		if (error != B_OK) {
			// sending failed
			fReplyWaiters.Remove(&waiter);
			return error;
		}

		// wait for the reply
		error = waiter.Wait(fLock, timeout);
		if (error != B_OK) {
			fReplyWaiters.Remove(&waiter);
			return error;
		}

		Message* reply = waiter.GetMessage();
		ObjectDeleter<Message> replyDeleter(reply);
		assert(reply != NULL);

		_reply = reply->message;
		return B_OK;
	}

	status_t SendReply(const Envelope& envelope, const BMessage& reply)
	{
		return _SendMessage(envelope, reply, true);
	}

	status_t ReceiveMessage(ChannelId channelId, MessageId& _messageId,
		BMessage& _message, bigtime_t timeout)
	{
		PthreadMutexLocker locker(fLock);

		Channel* channel = fChannels.Lookup(channelId);
		if (channel == NULL)
			return B_BAD_VALUE;
		BReference<Channel> channelReference(channel);

		Message* message;
		status_t error = channel->PopMessage(fLock, timeout, message);
		if (error != B_OK)
			return error;

		fReceivedMessages.RemoveMessage(message);
		_messageId = message->envelope.messageId;
		_message = message->message;
		delete message;
		return B_OK;
	}

	status_t ReceiveMessage(Envelope& _envelope, BMessage& _message,
		bigtime_t timeout)
	{
		PthreadMutexLocker locker(fLock);

		Message* message;
		status_t error = fReceivedMessages.PopMessage(fLock, timeout, message);
		if (error != B_OK)
			return error;

		_RemoveReceivedMessageFromChannel(message);
		_envelope = message->envelope;
		_message = message->message;
		delete message;
		return B_OK;
	}

private:
	uint64 _NextChannelId()
	{
		return (uint64)atomic_add64(&fChannelIdCounter, 1);
	}

	uint64 _NextMessageId()
	{
		return (uint64)atomic_add64(&fReplyIdCounter, 1);
	}

	status_t _NewChannel(ChannelId channelId)
	{
		Channel* channel = new(std::nothrow) Channel(channelId);
		if (channel == NULL)
			return B_NO_MEMORY;

		PthreadMutexLocker locker(fLock);

		if (fTerminating) {
			delete channel;
			return B_NO_INIT;
		}

		fChannels.Insert(channel);
		return B_OK;
	}

	Channel* _RemoveChannel(ChannelId channelId)
	{
		PthreadMutexLocker locker(fLock);

		if (Channel* channel = fChannels.Lookup(channelId)) {
			fChannels.Remove(channel);
			_CloseChannelLocked(channel);
			return channel;
		}

		return NULL;
	}

	void _CloseChannelLocked(Channel* channel)
	{
		channel->Close();

		while (Message* message = channel->PopMessage()) {
			fReceivedMessages.RemoveMessage(message);
			delete message;
		}
	}

	bool _Close()
	{
		{
			PthreadMutexLocker locker(fLock);
			if (fTerminating)
				return false;
			fTerminating = true;

			fReceivedMessages.Close();

			for (ReplyWaiterHashTable::Iterator it
						= fReplyWaiters.GetIterator();
					ReplyWaiter* waiter = it.Next();) {
				waiter->Notify(B_CANCELED);
			}
			fReplyWaiters.Clear();
		}

		if (fStream != NULL)
			fStream->Close();

		PthreadMutexLocker locker(fLock);

		// close all channels
		for (Channel* channel = fChannels.Clear(true);
				channel != NULL;) {
			Channel* next = channel->HashTableNext();
			_CloseChannelLocked(channel);
			channel->ReleaseReference();
			channel = next;
		}

		return true;
	}

	status_t _SendMessage(const Envelope& envelope, const BMessage& message,
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
			flatSize | (isReply ? kIsReplyFlag : 0));
		header->messageId = B_HOST_TO_BENDIAN_INT64(envelope.messageId);
		header->channelId = B_HOST_TO_BENDIAN_INT64(envelope.channelId);

		char* messageBuffer = buffer + sizeof(MessageHeader);
		status_t error = message.Flatten(messageBuffer, flatSize);
		if (error != B_OK)
			return error;

		return fStream->WriteExactly(buffer, totalSize);
	}

	status_t _ReadMessage(Envelope& _envelope, BMessage& _message,
		bool& _isReply)
	{
		MessageHeader header;
		status_t error = fStream->ReadExactly(&header, sizeof(header));
		if (error != B_OK)
			return error;

		header.size = B_BENDIAN_TO_HOST_INT64(header.size);
		header.messageId = B_BENDIAN_TO_HOST_INT64(header.messageId);
		header.channelId = B_BENDIAN_TO_HOST_INT64(header.channelId);

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

		_envelope.messageId = header.messageId;
		_envelope.channelId = header.channelId;
		_isReply = (header.size & kIsReplyFlag) != 0;
		return B_OK;
	}

	static status_t _MessageReaderEntry(void* arg)
	{
		return ((StreamMessenger::Impl*)arg)->_MessageReader();
	}

	status_t _MessageReader()
	{
		status_t error = B_OK;

		for (;;) {
			Message* message = new(std::nothrow) Message;
			if (message == NULL) {
				error = B_NO_MEMORY;
				break;
			}
			ObjectDeleter<Message> messageDeleter(message);

			bool isReply = false;
			error = _ReadMessage(message->envelope, message->message, isReply);
			if (error != B_OK) {
				TRACE_REMOTE("StreamMessenger::_MessageReader(): terminating,"
					" failed to read message: %s\n", strerror(error));
				break;
			}

			PthreadMutexLocker locker(fLock);

			if (isReply)
				_AddReply(messageDeleter.Detach());
			else
				_AddReceivedMessage(messageDeleter.Detach());
		}

		// if we exit our message loop, ensure everybody wakes up and knows
		// we're no longer receiving messages.
		_Close();
		return error;
	}

	// fLock must be held
	void _AddReply(Message* message)
	{
		MessageId messageId = message->envelope.messageId;

		ReplyWaiter* waiter = fReplyWaiters.Lookup(messageId);
		if (waiter == NULL) {
			// The waiter probably already timeout out.
			delete message;
			return;
		}

		waiter->SetMessage(message);
		fReplyWaiters.Remove(waiter);
		waiter->Notify(B_OK);
	}

	// fLock must be held
	void _AddReceivedMessage(Message* message)
	{
		// find the channel
		Channel* channel = fChannels.Lookup(message->envelope.channelId);
		if (channel == NULL) {
			// unknown channel
			delete message;
			return;
		}

		if (channel->AppendMessage(message))
			fReceivedMessages.AppendMessage(message);
	}

	// fLock must be held
	void _RemoveReceivedMessageFromChannel(Message* message)
	{
		if (Channel* channel = fChannels.Lookup(message->envelope.channelId))
			channel->RemoveMessage(message);
	}

private:
	pthread_mutex_t		fLock;
	bool				fTerminating;
	BReference<Stream>	fStream;
	Channel*			fDefaultChannel;
	thread_id			fReplyReader;
	ReplyWaiterHashTable fReplyWaiters;
	ChannelHashTable	fChannels;
	MessageQueue		fReceivedMessages;
	int64				fChannelIdCounter;
	int64				fReplyIdCounter;
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
	return ChannelMessenger::SendMessage(message, _messageId);
}


status_t
StreamMessenger::SendMessage(const BMessage& message, BMessage& _reply,
	bigtime_t timeout)
{
	return ChannelMessenger::SendMessage(message, _reply, timeout);
}


status_t
StreamMessenger::SendReply(MessageId messageId, const BMessage& reply)
{
	return ChannelMessenger::SendReply(messageId, reply);
}


status_t
StreamMessenger::ReceiveMessage(MessageId& _messageId, BMessage& _message,
	bigtime_t timeout)
{
	return ChannelMessenger::ReceiveMessage(_messageId, _message,  timeout);
}


status_t
StreamMessenger::NewChannel(ChannelId& _id)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->NewChannel(_id);
}


status_t
StreamMessenger::DeleteChannel(ChannelId channelId)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->DeleteChannel(channelId);
}


status_t
StreamMessenger::SendMessage(ChannelId channelId, const BMessage& message,
	MessageId& _messageId)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->SendMessage(channelId, message, _messageId);
}


status_t
StreamMessenger::SendMessage(ChannelId channelId, const BMessage& message,
	BMessage& _reply, bigtime_t timeout)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->SendMessage(channelId, message, _reply, timeout);
}


status_t
StreamMessenger::SendReply(const Envelope& envelope, const BMessage& reply)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->SendReply(envelope, reply);
}


status_t
StreamMessenger::ReceiveMessage(ChannelId channelId, MessageId& _messageId,
	BMessage& _message, bigtime_t timeout)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->ReceiveMessage(channelId, _messageId, _message, timeout);
}


status_t
StreamMessenger::ReceiveMessage(Envelope& _envelope, BMessage& _message,
	bigtime_t timeout)
{
	if (fImpl == NULL)
		return B_NO_INIT;

	return fImpl->ReceiveMessage(_envelope, _message, timeout);
}
