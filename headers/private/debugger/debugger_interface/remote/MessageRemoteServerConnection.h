/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <OS.h>

#include "debugger_interface/remote/RemoteServerConnection.h"


struct Messenger;


template<typename Request, typename Response, typename Event, typename Context>
struct MessageRemoteServerConnection
		: RemoteServerConnection<Request, Response, Event> {

	typedef typename RemoteServerConnection<Request, Response, Event>::RequestId
		RequestId;

								MessageRemoteServerConnection(
									const Context& context,
									Messenger* messenger);
	virtual						~MessageRemoteServerConnection();

	virtual	status_t			Close();

	virtual	status_t			ReceiveRequest(Request*& _request,
									RequestId& _requestId);
	virtual	status_t			SendResponse(RequestId requestId,
									const Response& response);

	virtual	status_t			SendEvent(const Event& event);

private:
			Context				fContext;
			Messenger*			fMessenger;
};
