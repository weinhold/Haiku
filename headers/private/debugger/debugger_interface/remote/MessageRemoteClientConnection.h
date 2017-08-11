/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <OS.h>

#include "debugger_interface/remote/RemoteClientConnection.h"


struct Messenger;


template<typename Request, typename Event, typename Context>
struct MessageRemoteClientConnection
		: RemoteClientConnection<Request, Event> {
								MessageRemoteClientConnection(
									const Context& context,
									Messenger* messenger);
	virtual						~MessageRemoteClientConnection();

	virtual	status_t			Close();

	virtual	status_t			SendRequest(const Request& request,
									Request*& _reply);

	virtual	status_t			GetNextEvent(Event*& _event);

private:
			Context				fContext;
			Messenger*			fMessenger;
};
