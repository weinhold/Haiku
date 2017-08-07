/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <OS.h>

#include "debugger_interface/remote/RemoteDebugServerConnection.h"


class Architecture;
struct Messenger;


struct MessageRemoteDebugServerConnection : RemoteDebugServerConnection {
								MessageRemoteDebugServerConnection(
									Architecture* architecture,
									Messenger* messenger);
	virtual						~MessageRemoteDebugServerConnection();

	virtual	status_t			Close();

	virtual	status_t			ReceiveRequest(RemoteDebugRequest*& _request,
									RequestId& _requestId);
	virtual	status_t			SendResponse(RequestId requestId,
									const RemoteDebugRequest& response);

	virtual	status_t			SendDebugEvent(const DebugEvent& event);

private:
			Architecture*		fArchitecture;
			Messenger*			fMessenger;
			thread_id			fReceiverThread;
};
