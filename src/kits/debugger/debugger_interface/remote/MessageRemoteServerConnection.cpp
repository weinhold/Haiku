/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/MessageRemoteServerConnection.h"

#include <Message.h>

#include "debugger_interface/remote/Messenger.h"
#include "debugger_interface/remote/RemoteProtocol.h"


template<typename Request, typename Response, typename Event, typename Context>
MessageRemoteServerConnection<Request, Response, Event, Context>
	::MessageRemoteServerConnection(const Context& context,
		Messenger* messenger)
	:
	fContext(context),
	fMessenger(messenger)
{
	fMessenger->AcquireReference();
}


template<typename Request, typename Response, typename Event, typename Context>
MessageRemoteServerConnection<Request, Response, Event, Context>
	::~MessageRemoteServerConnection()
{
	Close();

	fMessenger->ReleaseReference();
}


template<typename Request, typename Response, typename Event, typename Context>
status_t
MessageRemoteServerConnection<Request, Response, Event, Context>::Close()
{
	fMessenger->Close();
	return B_OK;
}


template<typename Request, typename Response, typename Event, typename Context>
status_t
MessageRemoteServerConnection<Request, Response, Event, Context>
	::ReceiveRequest(Request*& _request, RequestId& _requestId)
{
	BMessage message;
	Messenger::MessageId messageId;
	status_t error = fMessenger->ReceiveMessage(messageId, message);
	if (error != B_OK) {
		// TODO: A communication error might be fatal for the connection.
		return error;
	}

	error = unarchiveRemoteData(fContext, message, _request);
	if (error != B_OK)
		return error;

	_requestId = messageId;
	return B_OK;
}


template<typename Request, typename Response, typename Event, typename Context>
status_t
MessageRemoteServerConnection<Request, Response, Event, Context>::SendResponse(
	RequestId requestId, const Response& response)
{
	BMessage message;
	status_t error = archiveRemoteData(fContext, response, message);
	if (error != B_OK)
		return error;

	error = fMessenger->SendReply(requestId, message);
	if (error != B_OK) {
		// TODO: A communication error might be fatal for the connection.
		return error;
	}

	return B_OK;
}


template<typename Request, typename Response, typename Event, typename Context>
status_t
MessageRemoteServerConnection<Request, Response, Event, Context>::SendEvent(
	const Event& event)
{
	BMessage message;
	status_t error = archiveRemoteData(fContext, event, message);
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


// explicit template instantiations

#include "debugger_interface/remote/RemoteDebugFactoryContext.h"
#include "debugger_interface/remote/RemoteManagementFactoryContext.h"


struct DebugEvent;
struct RemoteDebugRequest;
struct RemoteManagementEvent;
struct RemoteManagementRequest;
struct RemoteManagementResponse;


template struct MessageRemoteServerConnection<RemoteDebugRequest,
	RemoteDebugRequest, DebugEvent, RemoteDebugFactoryContext>;
template struct MessageRemoteServerConnection<RemoteManagementRequest,
	RemoteManagementResponse, RemoteManagementEvent,
	RemoteManagementFactoryContext>;
