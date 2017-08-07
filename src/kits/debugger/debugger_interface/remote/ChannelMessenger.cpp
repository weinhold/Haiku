/*
 * Copyright 2011-2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/ChannelMessenger.h"


ChannelMessenger::ChannelMessenger()
{
}


ChannelMessenger::~ChannelMessenger()
{
}


status_t
ChannelMessenger::SendMessage(const BMessage& message, MessageId& _messageId)
{
	return SendMessage(kDefaultChannelId, message, _messageId);
}


status_t
ChannelMessenger::SendMessage(const BMessage& message, BMessage& _reply,
	bigtime_t timeout)
{
	return SendMessage(kDefaultChannelId, message, _reply, timeout);
}


status_t
ChannelMessenger::SendReply(MessageId messageId, const BMessage& reply)
{
	return SendReply(Envelope(kDefaultChannelId, messageId), reply);
}


status_t
ChannelMessenger::ReceiveMessage(MessageId& _messageId, BMessage& _message,
	bigtime_t timeout)
{
	Envelope envelope;
	status_t error = ReceiveMessage(kDefaultChannelId, _messageId, _message,
		timeout);
	if (error != B_OK)
		return error;
	_messageId = envelope.messageId;
	return B_OK;
}
