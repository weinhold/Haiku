/*
 * Copyright 2011-2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include "Messenger.h"


struct Stream;


struct StreamMessenger : Messenger {
								StreamMessenger();
	virtual						~StreamMessenger();

			status_t			SetTo(Stream* stream);
	virtual	void				Unset();
									// Must not be called while SendMessage(),
									// SendReply(), ReceiveMessage() calls are
									// being made.

	virtual	void				Close();

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
			struct Impl;

private:
			Impl*				fImpl;
};
