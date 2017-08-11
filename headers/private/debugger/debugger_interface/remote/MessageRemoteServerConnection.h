/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <OS.h>

#include "debugger_interface/remote/RemoteServerConnection.h"


struct Messenger;


template<typename Request, typename Event, typename Context>
struct MessageRemoteServerConnection
		: RemoteServerConnection<Request, Event> {

	typedef typename RemoteServerConnection<Request, Event>::RequestId
		RequestId;

								MessageRemoteServerConnection(
									const Context& context,
									Messenger* messenger);
	virtual						~MessageRemoteServerConnection();

	virtual	status_t			Close();

	virtual	status_t			ReceiveRequest(Request*& _request,
									RequestId& _requestId);
	virtual	status_t			SendResponse(RequestId requestId,
									const Request& response);

	virtual	status_t			SendEvent(const Event& event);

private:
			Context				fContext;
			Messenger*			fMessenger;
};
