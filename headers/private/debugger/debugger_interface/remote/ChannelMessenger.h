/*
 * Copyright 2011-2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include "debugger_interface/remote/Messenger.h"


struct ChannelMessenger : Messenger {
public:
			typedef uint64 ChannelId;
			struct Envelope;

	static	const ChannelId		kDefaultChannelId = 0;

public:
								ChannelMessenger();
	virtual						~ChannelMessenger();

	// Messenger
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
	virtual	status_t			NewChannel(ChannelId& _id) = 0;
	virtual	status_t			AddChannel(ChannelId id) = 0;
	virtual	status_t			DeleteChannel(ChannelId channelId) = 0;

	virtual	status_t			SendMessage(ChannelId channelId,
									const BMessage& message,
									MessageId& _messageId) = 0;
	virtual	status_t			SendMessage(ChannelId channelId,
									const BMessage& message, BMessage& _reply,
									bigtime_t timeout = B_INFINITE_TIMEOUT) = 0;
	virtual	status_t			SendReply(const Envelope& envelope,
									const BMessage& reply) = 0;

								// wait for unsolicited message on socket
	virtual	status_t			ReceiveMessage(ChannelId channelId,
									MessageId& _messageId, BMessage& _message,
									bigtime_t timeout = B_INFINITE_TIMEOUT) = 0;
	virtual	status_t			ReceiveMessage(Envelope& _envelope,
									BMessage& _message,
									bigtime_t timeout = B_INFINITE_TIMEOUT) = 0;
};


struct ChannelMessenger::Envelope {
	ChannelId	channelId;
	MessageId	messageId;

	Envelope(ChannelId channelId = 0, MessageId messageId = 0)
		:
		channelId(channelId),
		messageId(messageId)
	{
	}
};
