/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>

#include <Referenceable.h>


struct DebugEvent;
struct RemoteDebugRequest;


template<typename Request, typename Response, typename Event>
struct RemoteServerConnection : public BReferenceable {
public:
	typedef	uint64				RequestId;

public:
								RemoteServerConnection()	{}
	virtual						~RemoteServerConnection()	{}

	virtual	status_t			Close() = 0;

	virtual	status_t			ReceiveRequest(Request*& _request,
									RequestId& _requestId) = 0;
	virtual	status_t			SendResponse(RequestId requestId,
									const Response& response) = 0;

	virtual	status_t			SendEvent(const Event& event) = 0;
};
