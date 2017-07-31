/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>

#include <Referenceable.h>


struct DebugEvent;
struct RemoteDebugRequest;


struct RemoteDebugServerConnection : public BReferenceable {
public:
	typedef	uint64				RequestId;

public:
								RemoteDebugServerConnection();
	virtual						~RemoteDebugServerConnection();

	virtual	status_t			Close() = 0;

	virtual	status_t			ReceiveRequest(RemoteDebugRequest*& _request,
									RequestId& _requestId) = 0;
	virtual	status_t			SendResponse(RequestId requestId,
									const RemoteDebugRequest& response) = 0;

	virtual	status_t			SendDebugEvent(const DebugEvent& event) = 0;
};
