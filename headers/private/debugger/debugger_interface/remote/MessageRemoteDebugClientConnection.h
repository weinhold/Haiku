/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <OS.h>

#include "RemoteDebugClientConnection.h"


class Architecture;
struct Messenger;


struct MessageRemoteDebugClientConnection : RemoteDebugClientConnection {
								MessageRemoteDebugClientConnection(
									Architecture* architecture,
									Messenger* messenger);
	virtual						~MessageRemoteDebugClientConnection();

	virtual	status_t			Close();

	virtual	status_t			SendRequest(const RemoteDebugRequest& request,
									RemoteDebugRequest*& _reply);

	virtual	status_t			GetNextDebugEvent(DebugEvent*& _event);

private:
			Architecture*		fArchitecture;
			Messenger*			fMessenger;
			thread_id			fReceiverThread;
};
