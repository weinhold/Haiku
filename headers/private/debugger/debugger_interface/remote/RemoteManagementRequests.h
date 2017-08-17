/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <ObjectList.h>
#include <String.h>

#include <AutoDeleter.h>

#include "CpuState.h"
#include "ImageInfo.h"
#include "InspectableStruct.h"
#include "RawData.h"
#include "Reference.h"
#include "SymbolInfo.h"
#include "TeamInfo.h"
#include "ThreadInfo.h"

#include "debugger_interface/remote/RemoteInspectableStructMacros.h"
#include "debugger_interface/remote/RemoteManagementRequest.h"
#include "debugger_interface/remote/RemoteResponse.h"

#define DEFINE_REQUEST_AND_RESPONSE_STRUCTS(...)	\
	ITERATE3(DECLARE_REQUEST_AND_RESPONSE_STRUCT, DEFINE_EMPTY, __VA_ARGS__) \
	DECLARE_VISITOR(RemoteManagementRequestVisitor, Request, __VA_ARGS__)	\
	DECLARE_VISITOR(RemoteManagementResponseVisitor, Response, __VA_ARGS__)	\
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


DEFINE_REQUEST_AND_RESPONSE_STRUCTS(
	Hello,
	(
		uint32,		protocolVersion
	),
	(
		status_t, 	error,
		uint32,		protocolVersion
	),

	FooBar,
	(
		uint32,		xxx
	),
	(
		status_t, 	error,
		uint32,		yyy
	)
)


// defined in RemoteInspectableStructMacros.h
#undef DECLARE_REQUEST_AND_RESPONSE_STRUCT
#undef DECLARE_VISITOR
#undef DECLARE_VISIT_METHOD_Request
#undef DECLARE_VISIT_METHOD_Response
#undef DEFINE_REQUEST_AND_RESPONSE_STRUCT

#undef DEFINE_REQUEST_AND_RESPONSE_STRUCTS
#undef DEFINE_REQUEST_STRUCT
#undef DEFINE_REPLY_STRUCT
