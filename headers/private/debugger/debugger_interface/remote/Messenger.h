/*
 * Copyright 2011-2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <OS.h>


class BMessage;


struct Messenger {
public:
	typedef	uint64				MessageId;

public:
								Messenger();
	virtual						~Messenger();

	virtual	void				Unset() = 0;
									// Must not be called while SendMessage(),
									// SendReply(), ReceiveMessage() calls are
									// being made.

	virtual	void				Close() = 0;

	virtual	status_t			SendMessage(const BMessage& message,
									MessageId& _messageId) = 0;
	virtual	status_t			SendMessage(const BMessage& message,
									BMessage& _reply,
									bigtime_t timeout = B_INFINITE_TIMEOUT) = 0;
	virtual	status_t			SendReply(MessageId messageId,
									const BMessage& reply) = 0;

								// wait for unsolicited message on socket
	virtual	status_t			ReceiveMessage(BMessage& _message,
									MessageId& _messageId,
									bigtime_t timeout = B_INFINITE_TIMEOUT) = 0;
};
