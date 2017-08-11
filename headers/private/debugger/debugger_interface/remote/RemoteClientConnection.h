/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>

#include <Referenceable.h>


template<typename Request, typename Event>
struct RemoteClientConnection : public BReferenceable {
								RemoteClientConnection() {}
	virtual						~RemoteClientConnection() {}

	virtual	status_t			Close() = 0;

	virtual	status_t			SendRequest(const Request& request,
									Request*& _reply) = 0;

	virtual	status_t			GetNextEvent(Event*& _event) = 0;
};
