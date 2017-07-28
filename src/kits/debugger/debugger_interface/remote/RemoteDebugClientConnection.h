/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>

#include <Referenceable.h>


struct DebugEvent;
struct RemoteDebugRequest;


struct RemoteDebugClientConnection : public BReferenceable {
								RemoteDebugClientConnection();
	virtual						~RemoteDebugClientConnection();

	virtual	status_t			Close() = 0;

	virtual	status_t			SendRequest(const RemoteDebugRequest& request,
									RemoteDebugRequest*& _reply) = 0;

	virtual	status_t			GetNextDebugEvent(DebugEvent*& _event) = 0;
};
