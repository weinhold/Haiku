/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "MessageRemoteDebugClientConnection.h"

#include <Message.h>

#include "Architecture.h"
#include "Messenger.h"
#include "RemoteProtocol.h"


MessageRemoteDebugClientConnection::MessageRemoteDebugClientConnection(
	Architecture* architecture, Messenger* messenger)
	:
	fArchitecture(architecture),
	fMessenger(messenger)
{
	fArchitecture->AcquireReference();
}


MessageRemoteDebugClientConnection::~MessageRemoteDebugClientConnection()
{
	Close();

	fArchitecture->ReleaseReference();
}


status_t
MessageRemoteDebugClientConnection::Close()
{
	fMessenger->Close();
	return B_OK;
}


status_t
MessageRemoteDebugClientConnection::SendRequest(
	const RemoteDebugRequest& request, RemoteDebugRequest*& _reply)
{
	BMessage requestMessage;
	status_t error = archiveRemoteDebugRequest(fArchitecture, request,
		requestMessage);
	if (error != B_OK)
		return error;

	BMessage replyMessage;
	error = fMessenger->SendMessage(requestMessage, replyMessage);
	if (error != B_OK) {
		// TODO: A communication error might be fatal for the connection.
		return error;
	}

	return unarchiveRemoteDebugRequest(fArchitecture, replyMessage, _reply);
}


status_t
MessageRemoteDebugClientConnection::GetNextDebugEvent(DebugEvent*& _event)
{
	BMessage message;
	Messenger::MessageId messageId;
	status_t error = fMessenger->ReceiveMessage(message, messageId);
	if (error != B_OK) {
		// TODO: A communication error might be fatal for the connection.
		return error;
	}

	return unarchiveDebugEvent(fArchitecture, message, _event);
}
