/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef REMOTE_DEBUG_CONNECTION_H
#define REMOTE_DEBUG_CONNECTION_H


#include <SupportDefs.h>

#include <Referenceable.h>


struct RemoteDebugRequest;


class RemoteDebugConnection : public BReferenceable {
public:
								RemoteDebugConnection();
	virtual						~RemoteDebugConnection();

	virtual	status_t			SendRequest(const RemoteDebugRequest& request)
									= 0;
	virtual	status_t			SendRequest(const RemoteDebugRequest& request,
									RemoteDebugRequest*& _reply) = 0;
	virtual	status_t			ReceiveRequest(RemoteDebugRequest*& _request)
									= 0;

	virtual	status_t			Close() = 0;
};


#endif	// REMOTE_DEBUG_CONNECTION_H
