/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/MessageRemoteDebugServerConnection.h"

#include <Message.h>

#include "Architecture.h"
#include "debugger_interface/remote/Messenger.h"
#include "debugger_interface/remote/RemoteProtocol.h"


MessageRemoteDebugServerConnection::MessageRemoteDebugServerConnection(
	Architecture* architecture, Messenger* messenger)
	:
	fArchitecture(architecture),
	fMessenger(messenger)
{
	fArchitecture->AcquireReference();
}


MessageRemoteDebugServerConnection::~MessageRemoteDebugServerConnection()
{
	Close();

	fArchitecture->ReleaseReference();
}


status_t
MessageRemoteDebugServerConnection::Close()
{
	fMessenger->Close();
	return B_OK;
}


status_t
MessageRemoteDebugServerConnection::ReceiveRequest(
	RemoteDebugRequest*& _request, RequestId& _requestId)
{
	BMessage message;
	Messenger::MessageId messageId;
	status_t error = fMessenger->ReceiveMessage(messageId, message);
	if (error != B_OK) {
		// TODO: A communication error might be fatal for the connection.
		return error;
	}

	error = unarchiveRemoteDebugRequest(fArchitecture, message, _request);
	if (error != B_OK)
		return error;

	_requestId = messageId;
	return B_OK;
}


status_t
MessageRemoteDebugServerConnection::SendResponse(RequestId requestId,
	const RemoteDebugRequest& response)
{
	BMessage message;
	status_t error = archiveRemoteDebugRequest(fArchitecture, response,
		message);
	if (error != B_OK)
		return error;

	error = fMessenger->SendReply(requestId, message);
	if (error != B_OK) {
		// TODO: A communication error might be fatal for the connection.
		return error;
	}

	return B_OK;
}


status_t
MessageRemoteDebugServerConnection::SendDebugEvent(const DebugEvent& event)
{
	BMessage message;
	status_t error = archiveDebugEvent(fArchitecture, event, message);
	if (error != B_OK)
		return error;

	Messenger::MessageId messageId;
	error = fMessenger->SendMessage(message, messageId);
	if (error != B_OK) {
		// TODO: A communication error might be fatal for the connection.
		return error;
	}

	return B_OK;
}
