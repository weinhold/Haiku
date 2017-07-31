/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef REMOTE_DEBUG_REQUESTS_H
#define REMOTE_DEBUG_REQUESTS_H



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

#include "RemoteDebugRequest.h"


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
	DEFINE_INSPECTABLE_STRUCT(name ## Request, RemoteDebugRequest, __VA_ARGS__)

#define DEFINE_REPLY_STRUCT(name, ...) \
	DEFINE_INSPECTABLE_STRUCT(name ## Response, RemoteDebugRequest, __VA_ARGS__)


DEFINE_REQUEST_STRUCTS(
	Close,
	(
		bool,		killTeam
	),
	(
		status_t, 	error
	)
)

DEFINE_REQUEST_STRUCTS(
	SetTeamDebuggingFlags,
	(
		uint32,		flags
	),
	(
		status_t, 	error
	)
)

DEFINE_REQUEST_STRUCTS(
	ContinueThread,
	(
		int32,		threadId
	),
	(
		status_t, 	error
	)
)

DEFINE_REQUEST_STRUCTS(
	StopThread,
	(
		int32,		threadId
	),
	(
		status_t, 	error
	)
)

DEFINE_REQUEST_STRUCTS(
	SingleStepThread,
	(
		int32,		threadId
	),
	(
		status_t, 	error
	)
)

DEFINE_REQUEST_STRUCTS(
	InstallBreakpoint,
	(
		uint64,		address
	),
	(
		status_t, 	error
	)
)

DEFINE_REQUEST_STRUCTS(
	UninstallBreakpoint,
	(
		uint64,		address
	),
	(
		status_t, 	error
	)
)

DEFINE_REQUEST_STRUCTS(
	InstallWatchpoint,
	(
		uint64,		address,
		uint32,		type,
		int32,		length
	),
	(
		status_t, 	error
	)
)

DEFINE_REQUEST_STRUCTS(
	UninstallWatchpoint,
	(
		uint64,		address
	),
	(
		status_t, 	error
	)
)

DEFINE_REQUEST_STRUCTS(
	GetTeamInfo,
	(
	),
	(
		status_t, 	error,
		TeamInfo,	info
	)
)

DEFINE_REQUEST_STRUCTS(
	GetThreadInfos,
	(
	),
	(
		status_t, 					error,
		BObjectList<ThreadInfo>,	infos
	)
)

DEFINE_REQUEST_STRUCTS(
	GetImageInfos,
	(
	),
	(
		status_t, 				error,
		BObjectList<ImageInfo>,	infos
	)
)

DEFINE_REQUEST_STRUCTS(
	GetSymbolInfos,
	(
		int32,						imageId
	),
	(
		status_t, 					error,
		BObjectList<SymbolInfo>,	infos
	)
)

DEFINE_REQUEST_STRUCTS(
	GetSymbolInfo,
	(
		int32,			imageId,
		BString,		name,
		int32,			type
	),
	(
		status_t, 		error,
		SymbolInfo,		info
	)
)

DEFINE_REQUEST_STRUCTS(
	GetThreadInfo,
	(
		int32,			threadId
	),
	(
		status_t, 		error,
		ThreadInfo,		info
	)
)

DEFINE_REQUEST_STRUCTS(
	GetCpuState,
	(
		int32,						threadId
	),
	(
		status_t, 					error,
		Reference<CpuState>,		cpuState
	)
)

DEFINE_REQUEST_STRUCTS(
	SetCpuState,
	(
		int32,						threadId,
		Reference<CpuState>,		cpuState
	),
	(
		status_t, 					error
	)
)

DEFINE_REQUEST_STRUCTS(
	GetCpuFeatures,
	(
		uint32,						flags
	),
	(
		status_t, 					error
	)
)

DEFINE_REQUEST_STRUCTS(
	WriteCoreFile,
	(
		BString,					path
	),
	(
		status_t, 					error
	)
)

DEFINE_REQUEST_STRUCTS(
	GetMemoryProperties,
	(
		uint64,						address
	),
	(
		status_t, 					error,
		uint32,						protection,
		uint32,						locking
	)
)

DEFINE_REQUEST_STRUCTS(
	ReadMemory,
	(
		uint64,			address,
		uint64,			size
	),
	(
		status_t, 		error,
		RawData,		data
	)
)

DEFINE_REQUEST_STRUCTS(
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


#endif	// REMOTE_DEBUG_REQUESTS_H

