/*
 * Copyright 2011-2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include "debugger_interface/remote/ChannelMessenger.h"


struct SingleChannelMessenger : Messenger {
public:
			typedef ChannelMessenger::ChannelId ChannelId;
public:
								SingleChannelMessenger(ChannelMessenger* parent,
									ChannelId channelId);
	virtual						~SingleChannelMessenger();

	virtual	void				Unset();
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

private:
			ChannelMessenger*	fParent;
			ChannelId			fChannelId;
			bool				fClosed;
};
