/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <ObjectList.h>

#include "InspectableStruct.h"
#include "TeamInfo.h"

#include "debugger_interface/remote/RemoteInspectableStructMacros.h"
#include "debugger_interface/remote/RemoteManagementRequest.h"
#include "debugger_interface/remote/RemoteResponse.h"

#define DEFINE_REQUEST_AND_RESPONSE_STRUCTS(...)	\
	ITERATE3(DECLARE_REQUEST_AND_RESPONSE_STRUCT, DEFINE_EMPTY, __VA_ARGS__) \
	DECLARE_VISITOR3(RemoteManagementRequestVisitor, Request, __VA_ARGS__)	\
	DECLARE_VISITOR3(RemoteManagementResponseVisitor, Response, __VA_ARGS__)	\
	ITERATE3(DEFINE_REQUEST_AND_RESPONSE_STRUCT, DEFINE_EMPTY, __VA_ARGS__)

/*!	Defines a request struct.
	Arguments are the name of the struct (without the "Request" suffix) and
	alternatingly an attribute type and its name.
*/
#define DEFINE_REQUEST_STRUCT(name, ...) \
	DEFINE_INSPECTABLE_STRUCT(name ## Request, RemoteManagementRequest, \
		virtual void AcceptVisitor(RemoteManagementRequestVisitor* visitor) { \
			visitor->Visit(this);	\
		}, \
		__VA_ARGS__)

/*!	Defines a response struct.
	Arguments are the name of the struct (without the "Response" suffix) and
	alternatingly an attribute type and its name.
*/
#define DEFINE_REPLY_STRUCT(name, ...) \
	DEFINE_INSPECTABLE_STRUCT(name ## Response, RemoteManagementResponse, \
		virtual void AcceptVisitor(RemoteManagementResponseVisitor* visitor) { \
			visitor->Visit(this);	\
		}, \
		__VA_ARGS__)


static const uint32 kRemoteManagementProtocolVersion = 1;


// define the structs
#include "debugger_interface/remote/RemoteManagementRequestDefs.h"


// undefine macros defined in RemoteInspectableStructMacros.h
#include "debugger_interface/remote/RemoteInspectableStructMacrosUndefine.h"

// undefine macros defined here
#undef DEFINE_REQUEST_AND_RESPONSE_STRUCTS
#undef DEFINE_REQUEST_STRUCT
#undef DEFINE_REPLY_STRUCT
