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

#include "debugger_interface/remote/RemoteDebugRequest.h"
#include "debugger_interface/remote/RemoteInspectableStructMacros.h"
#include "debugger_interface/remote/RemoteResponse.h"


#define DEFINE_REQUEST_AND_RESPONSE_STRUCTS(...)	\
	ITERATE3(DECLARE_REQUEST_AND_RESPONSE_STRUCT, DEFINE_EMPTY, __VA_ARGS__) \
	DECLARE_VISITOR3(RemoteDebugRequestVisitor, Request, __VA_ARGS__)		\
	DECLARE_VISITOR3(RemoteDebugResponseVisitor, Response, __VA_ARGS__)		\
	ITERATE3(DEFINE_REQUEST_AND_RESPONSE_STRUCT, DEFINE_EMPTY, __VA_ARGS__)

/*!	Defines a request struct.
	Arguments are the name of the struct (without the "Request" suffix) and
	alternatingly an attribute type and its name.
*/
#define DEFINE_REQUEST_STRUCT(name, ...) \
	DEFINE_INSPECTABLE_STRUCT(name ## Request, RemoteDebugRequest, \
		virtual void AcceptVisitor(RemoteDebugRequestVisitor* visitor) { \
			visitor->Visit(this);	\
		}, \
		__VA_ARGS__)

/*!	Defines a response struct.
	Arguments are the name of the struct (without the "Response" suffix) and
	alternatingly an attribute type and its name.
*/
#define DEFINE_REPLY_STRUCT(name, ...) \
	DEFINE_INSPECTABLE_STRUCT(name ## Response, RemoteDebugResponse, \
		virtual void AcceptVisitor(RemoteDebugResponseVisitor* visitor) { \
			visitor->Visit(this);	\
		}, \
		__VA_ARGS__)


// define the structs
#include "debugger_interface/remote/RemoteDebugRequestDefs.h"


// undefine macros defined in RemoteInspectableStructMacros.h
#include "debugger_interface/remote/RemoteInspectableStructMacrosUndefine.h"

// undefine macros defined here
#undef DEFINE_REQUEST_AND_RESPONSE_STRUCTS
#undef DEFINE_REQUEST_STRUCT
#undef DEFINE_REPLY_STRUCT
