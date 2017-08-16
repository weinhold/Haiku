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

#include "debugger_interface/remote/RemoteManagementRequest.h"
#include "debugger_interface/remote/RemoteResponse.h"


#define DEFINE_REQUEST_STRUCTS(name, requestFields, responseFields)	\
	struct name ## Response;										\
	DEFINE_REQUEST_STRUCT(name, UNWRAP_MACRO_ARGS requestFields)	\
	DEFINE_REPLY_STRUCT(name, UNWRAP_MACRO_ARGS responseFields)		\
	template<>														\
	struct RemoteResponse<name ## Request> {						\
		typedef name ## Response Type;								\
	};

/*!	Defines a request struct.
	Arguments are the name of the struct (without the "Request" suffix) and
	alternatingly an attribute type and its name.
*/
#define DEFINE_REQUEST_STRUCT(name, ...) \
	DEFINE_INSPECTABLE_STRUCT(name ## Request, RemoteManagementRequest, \
		__VA_ARGS__)

#define DEFINE_REPLY_STRUCT(name, ...) \
	DEFINE_INSPECTABLE_STRUCT(name ## Response, RemoteManagementRequest, \
		__VA_ARGS__)


DEFINE_REQUEST_STRUCTS(
	Hello,
	(
		uint32,		protocolVersion
	),
	(
		status_t, 	error,
		uint32,		protocolVersion
	)
)


#undef DEFINE_REQUEST_STRUCTS
#undef DEFINE_REQUEST_STRUCT
#undef DEFINE_REPLY_STRUCT
