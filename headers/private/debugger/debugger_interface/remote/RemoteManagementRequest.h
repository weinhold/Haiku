/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>

#include "util/InspectableStruct.h"
#include "util/StructInspector.h"


struct RemoteManagementRequest : InspectableStruct<RemoteManagementRequest> {};


template<>
struct StructInspector<RemoteManagementRequest>
	:
	StructInspectorBase<RemoteManagementRequest>,
// 	virtual StructMemberInspector<bool>,
	virtual StructMemberInspector<int32>,
	virtual StructMemberInspector<uint32>
//,
// 	virtual StructMemberInspector<uint64>,
// 	virtual StructMemberInspector<BString>,
// 	virtual StructMemberInspector<RawData>,
// 	virtual StructMemberInspector<TeamInfo>,
// 	virtual StructMemberInspector<ThreadInfo>,
// 	virtual StructMemberInspector<SymbolInfo>,
// 	virtual StructMemberInspector<BObjectList<ThreadInfo> >,
// 	virtual StructMemberInspector<BObjectList<ImageInfo> >,
// 	virtual StructMemberInspector<BObjectList<SymbolInfo> >,
// 	virtual StructMemberInspector<Reference<CpuState> >
{
	virtual						~StructInspector();
};

typedef StructInspector<RemoteManagementRequest> ManagementRequestInspector;


template<>
struct ConstStructInspector<RemoteManagementRequest>
	:
	ConstStructInspectorBase<RemoteManagementRequest>,
// 	virtual StructMemberInspector<const bool>,
	virtual StructMemberInspector<const int32>,
	virtual StructMemberInspector<const uint32>
//,
// 	virtual StructMemberInspector<const uint64>,
// 	virtual StructMemberInspector<const BString>,
// 	virtual StructMemberInspector<const RawData>,
// 	virtual StructMemberInspector<const TeamInfo>,
// 	virtual StructMemberInspector<const ThreadInfo>,
// 	virtual StructMemberInspector<const SymbolInfo>,
// 	virtual StructMemberInspector<const BObjectList<ThreadInfo> >,
// 	virtual StructMemberInspector<const BObjectList<ImageInfo> >,
// 	virtual StructMemberInspector<const BObjectList<SymbolInfo> >,
// 	virtual StructMemberInspector<const Reference<CpuState> >
{
	virtual						~ConstStructInspector();
};

typedef ConstStructInspector<RemoteManagementRequest>
	ConstManagementRequestInspector;
