/*
 * Copyright 2011-2016, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <Socket.h>


class BMessage;


class StreamMessenger {
public:
	typedef	uint64				MessageId;

public:
								StreamMessenger();
								StreamMessenger(
									const BNetworkAddress& address,
									bigtime_t timeout = B_INFINITE_TIMEOUT);
								// adopt an existing already connected socket.
								StreamMessenger(const BSocket& socket);
	virtual						~StreamMessenger();

			void				Unset();
			status_t			SetTo(const BNetworkAddress& address,
									bigtime_t timeout = B_INFINITE_TIMEOUT);
			status_t			SetTo(const StreamMessenger& target,
									bigtime_t timeout = B_INFINITE_TIMEOUT);

			status_t			InitCheck() const { return fInitStatus; }

			const BNetworkAddress& Address() const { return fSocket.Peer(); }

	virtual	status_t			SendMessage(const BMessage& message,
									MessageId& _messageId);
	virtual	status_t			SendMessage(const BMessage& message,
									BMessage& _reply,
									bigtime_t timeout = B_INFINITE_TIMEOUT);
	virtual	status_t			SendReply(MessageId messageId,
									const BMessage& reply);

								// wait for unsolicited message on socket
	virtual	status_t			ReceiveMessage(BMessage& _message,
									MessageId& _messageId,
									bigtime_t timeout = B_INFINITE_TIMEOUT);

private:
			struct Private;
			struct Message;
private:
								StreamMessenger(const StreamMessenger&);
			StreamMessenger&	operator=(const StreamMessenger&);

			void				_Init();
			status_t			_WaitForMessage(bigtime_t timeout);
			status_t			_SendMessage(const BMessage& message,
									MessageId messageId, bool isReply);
			status_t			_ReadMessage(BMessage& _message,
									MessageId& _messageId, bool& _isReply);
			status_t			_ReadReply(int64 replyID,
									BMessage& _reply, bigtime_t timeout);

	static	status_t			_MessageReaderEntry(void* arg);
			status_t			_MessageReader();

private:
			Private*			fPrivateData;
			BSocket				fSocket;
			status_t			fInitStatus;
};
