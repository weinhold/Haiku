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
	DECLARE_VISITOR(RemoteDebugRequestVisitor, Request, __VA_ARGS__)		\
	DECLARE_VISITOR(RemoteDebugResponseVisitor, Response, __VA_ARGS__)		\
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


DEFINE_REQUEST_AND_RESPONSE_STRUCTS(
	Close,
	(
		bool,		killTeam
	),
	(
		status_t, 	error
	),

	SetTeamDebuggingFlags,
	(
		uint32,		flags
	),
	(
		status_t, 	error
	),

	ContinueThread,
	(
		int32,		threadId
	),
	(
		status_t, 	error
	),

	StopThread,
	(
		int32,		threadId
	),
	(
		status_t, 	error
	),

	SingleStepThread,
	(
		int32,		threadId
	),
	(
		status_t, 	error
	),

	InstallBreakpoint,
	(
		uint64,		address
	),
	(
		status_t, 	error
	),

	UninstallBreakpoint,
	(
		uint64,		address
	),
	(
		status_t, 	error
	),

	InstallWatchpoint,
	(
		uint64,		address,
		uint32,		type,
		int32,		length
	),
	(
		status_t, 	error
	),

	UninstallWatchpoint,
	(
		uint64,		address
	),
	(
		status_t, 	error
	),

	GetTeamInfo,
	(
	),
	(
		status_t, 	error,
		TeamInfo,	info
	),

	GetThreadInfos,
	(
	),
	(
		status_t, 					error,
		BObjectList<ThreadInfo>,	infos
	),

	GetImageInfos,
	(
	),
	(
		status_t, 				error,
		BObjectList<ImageInfo>,	infos
	),

	GetSymbolInfos,
	(
		int32,						imageId
	),
	(
		status_t, 					error,
		BObjectList<SymbolInfo>,	infos
	),

	GetSymbolInfo,
	(
		int32,			imageId,
		BString,		name,
		int32,			type
	),
	(
		status_t, 		error,
		SymbolInfo,		info
	),

	GetThreadInfo,
	(
		int32,			threadId
	),
	(
		status_t, 		error,
		ThreadInfo,		info
	),

	GetCpuState,
	(
		int32,						threadId
	),
	(
		status_t, 					error,
		Reference<CpuState>,		cpuState
	),

	SetCpuState,
	(
		int32,						threadId,
		Reference<CpuState>,		cpuState
	),
	(
		status_t, 					error
	),

	GetCpuFeatures,
	(
		uint32,						flags
	),
	(
		status_t, 					error
	),

	WriteCoreFile,
	(
		BString,					path
	),
	(
		status_t, 					error
	),

	GetMemoryProperties,
	(
		uint64,						address
	),
	(
		status_t, 					error,
		uint32,						protection,
		uint32,						locking
	),

	ReadMemory,
	(
		uint64,			address,
		uint64,			size
	),
	(
		status_t, 		error,
		RawData,		data
	),

	WriteMemory,
	(
		uint64,			address,
		RawData,		data
	),
	(
		status_t, 		error,
		size_t,			bytesWritten
	)
)

// TODO:...
// 	virtual	status_t			GetNextDebugEvent(DebugEvent*& _event) = 0;


// TODO: for debug report generator only
// 	virtual	status_t			GetSystemInfo(SystemInfo& info) = 0;
// 	virtual status_t			GetAreaInfos(BObjectList<AreaInfo>& infos)
// 									= 0;
// 	virtual status_t			GetSemaphoreInfos(
// 									BObjectList<SemaphoreInfo>& infos)
// 									= 0;


// defined in RemoteInspectableStructMacros.h
#undef DECLARE_REQUEST_AND_RESPONSE_STRUCT
#undef DECLARE_VISITOR
#undef DECLARE_VISIT_METHOD_Request
#undef DECLARE_VISIT_METHOD_Response
#undef DEFINE_REQUEST_AND_RESPONSE_STRUCT

#undef DEFINE_REQUEST_AND_RESPONSE_STRUCTS
#undef DEFINE_REQUEST_STRUCT
#undef DEFINE_REPLY_STRUCT
