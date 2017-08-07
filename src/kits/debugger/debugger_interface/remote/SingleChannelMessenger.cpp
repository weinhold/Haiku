/*
 * Copyright 2011-2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/SingleChannelMessenger.h"


SingleChannelMessenger::SingleChannelMessenger(ChannelMessenger* parent,
		ChannelId channelId)
	:
	fParent(parent),
	fChannelId(channelId)
{
}

SingleChannelMessenger::~SingleChannelMessenger()
{
}


status_t
SingleChannelMessenger::SendMessage(const BMessage& message,
	MessageId& _messageId)
{
	return fParent->SendMessage(fChannelId, message, _messageId);
}


status_t
SingleChannelMessenger::SendMessage(const BMessage& message, BMessage& _reply,
	bigtime_t timeout)
{
	return fParent->SendMessage(fChannelId, message, _reply, timeout);
}


status_t
SingleChannelMessenger::SendReply(MessageId messageId, const BMessage& reply)
{
	return fParent->SendReply(ChannelMessenger::Envelope(messageId, fChannelId),
		reply);
}


status_t
SingleChannelMessenger::ReceiveMessage(MessageId& _messageId,
	BMessage& _message, bigtime_t timeout)
{
	return fParent->ReceiveMessage(fChannelId, _messageId, _message, timeout);
}
