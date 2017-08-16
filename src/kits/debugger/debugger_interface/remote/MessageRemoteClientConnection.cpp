/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/MessageRemoteClientConnection.h"

#include <Message.h>

#include "debugger_interface/remote/Messenger.h"
#include "debugger_interface/remote/RemoteProtocol.h"


template<typename Request, typename Event, typename Context>
MessageRemoteClientConnection<Request, Event, Context>
	::MessageRemoteClientConnection(const Context& context,
		Messenger* messenger)
	:
	fContext(context),
	fMessenger(messenger)
{
	fMessenger->AcquireReference();
}


template<typename Request, typename Event, typename Context>
MessageRemoteClientConnection<Request, Event, Context>
	::~MessageRemoteClientConnection()
{
	Close();

	fMessenger->ReleaseReference();
}


template<typename Request, typename Event, typename Context>
status_t
MessageRemoteClientConnection<Request, Event, Context>::Close()
{
	fMessenger->Close();
	return B_OK;
}


template<typename Request, typename Event, typename Context>
status_t
MessageRemoteClientConnection<Request, Event, Context>::SendRequest(
	const Request& request, Request*& _reply)
{
	BMessage requestMessage;
	status_t error = archiveRemoteData(fContext, request, requestMessage);
	if (error != B_OK)
		return error;

	BMessage replyMessage;
	error = fMessenger->SendMessage(requestMessage, replyMessage);
	if (error != B_OK) {
		// TODO: A communication error might be fatal for the connection.
		return error;
	}

	return unarchiveRemoteData(fContext, replyMessage, _reply);
}


template<typename Request, typename Event, typename Context>
status_t
MessageRemoteClientConnection<Request, Event, Context>::GetNextEvent(
	Event*& _event)
{
	BMessage message;
	Messenger::MessageId messageId;
	status_t error = fMessenger->ReceiveMessage(messageId, message);
	if (error != B_OK) {
		// TODO: A communication error might be fatal for the connection.
		return error;
	}

	return unarchiveRemoteData(fContext, message, _event);
}


// explicit template instantiations

#include "debugger_interface/remote/RemoteDebugFactoryContext.h"
#include "debugger_interface/remote/RemoteManagementFactoryContext.h"


struct DebugEvent;
struct RemoteDebugRequest;
struct RemoteManagementEvent;
struct RemoteManagementRequest;


template struct MessageRemoteClientConnection<RemoteDebugRequest, DebugEvent,
	RemoteDebugFactoryContext>;
template struct MessageRemoteClientConnection<RemoteManagementRequest,
	RemoteManagementEvent, RemoteManagementFactoryContext>;
