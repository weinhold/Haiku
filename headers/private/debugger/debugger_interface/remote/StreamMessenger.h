/*
 * Copyright 2011-2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include "debugger_interface/remote/ChannelMessenger.h"


struct Stream;


struct StreamMessenger : ChannelMessenger {
								StreamMessenger();
	virtual						~StreamMessenger();

			status_t			SetTo(Stream* stream);

	// Messenger
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

	virtual	status_t			ReceiveMessage(MessageId& _messageId,
									BMessage& _message,
									bigtime_t timeout = B_INFINITE_TIMEOUT);

	// ChannelMessenger
	virtual	status_t			NewChannel(ChannelId& _id);
	virtual	status_t			AddChannel(ChannelId id);
	virtual	status_t			DeleteChannel(ChannelId channelId);

	virtual	status_t			SendMessage(ChannelId channelId,
									const BMessage& message,
									MessageId& _messageId);
	virtual	status_t			SendMessage(ChannelId channelId,
									const BMessage& message, BMessage& _reply,
									bigtime_t timeout = B_INFINITE_TIMEOUT);
	virtual	status_t			SendReply(const Envelope& envelope,
									const BMessage& reply);

	virtual	status_t			ReceiveMessage(ChannelId channelId,
									MessageId& _messageId, BMessage& _message,
									bigtime_t timeout = B_INFINITE_TIMEOUT);
	virtual	status_t			ReceiveMessage(Envelope& _envelope,
									BMessage& _message,
									bigtime_t timeout = B_INFINITE_TIMEOUT);

private:
			struct Impl;

private:
			Impl*				fImpl;
};
