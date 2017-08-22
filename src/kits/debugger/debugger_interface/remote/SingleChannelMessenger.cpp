/*
 * Copyright 2011-2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/SingleChannelMessenger.h"


SingleChannelMessenger::SingleChannelMessenger(ChannelMessenger* parent,
		ChannelId channelId)
	:
	fParent(parent),
	fChannelId(channelId),
	fClosed(false)
{
	fParent->AcquireReference();
}

SingleChannelMessenger::~SingleChannelMessenger()
{
	Unset();
}


void
SingleChannelMessenger::Unset()
{
	if (fParent != NULL) {
		Close();

		fParent->ReleaseReference();
		fParent = NULL;
	}
}


void
SingleChannelMessenger::Close()
{
	if (!fClosed) {
		fParent->DeleteChannel(fChannelId);
		fClosed = true;
	}
}


status_t
SingleChannelMessenger::SendMessage(const BMessage& message,
	MessageId& _messageId)
{
	if (fClosed)
		return B_CANCELED;
	return fParent->SendMessage(fChannelId, message, _messageId);
}


status_t
SingleChannelMessenger::SendMessage(const BMessage& message, BMessage& _reply,
	bigtime_t timeout)
{
	if (fClosed)
		return B_CANCELED;
	return fParent->SendMessage(fChannelId, message, _reply, timeout);
}


status_t
SingleChannelMessenger::SendReply(MessageId messageId, const BMessage& reply)
{
	if (fClosed)
		return B_CANCELED;
	return fParent->SendReply(ChannelMessenger::Envelope(messageId, fChannelId),
		reply);
}


status_t
SingleChannelMessenger::ReceiveMessage(MessageId& _messageId,
	BMessage& _message, bigtime_t timeout)
{
	if (fClosed)
		return B_CANCELED;
	return fParent->ReceiveMessage(fChannelId, _messageId, _message, timeout);
}
