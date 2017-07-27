/*
 * Copyright 2011-2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <OS.h>


class BMessage;

class Stream;


class StreamMessenger {
public:
	typedef	uint64				MessageId;

public:
								StreamMessenger();
								~StreamMessenger();

			status_t			SetTo(Stream* stream);
			void				Unset();

			status_t			SendMessage(const BMessage& message,
									MessageId& _messageId);
			status_t			SendMessage(const BMessage& message,
									BMessage& _reply,
									bigtime_t timeout = B_INFINITE_TIMEOUT);
			status_t			SendReply(MessageId messageId,
									const BMessage& reply);

								// wait for unsolicited message on socket
			status_t			ReceiveMessage(BMessage& _message,
									MessageId& _messageId,
									bigtime_t timeout = B_INFINITE_TIMEOUT);

private:
			struct Impl;

private:
			Impl*				fImpl;
};
