/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>

#include "util/InspectableStruct.h"
#include "util/StructInspector.h"


struct RemoteManagementRequestVisitor;
struct RemoteManagementResponseVisitor;

struct RemoteManagementRequest : InspectableStruct<RemoteManagementRequest> {
	virtual	void				AcceptVisitor(
									RemoteManagementRequestVisitor* visitor)
									= 0;
};


struct RemoteManagementResponse : InspectableStruct<RemoteManagementResponse> {
	virtual	void				AcceptVisitor(
									RemoteManagementResponseVisitor* visitor)
									= 0;
};


template<>
struct StructInspector<RemoteManagementRequest>
	:
	StructInspectorBase<RemoteManagementRequest>,
	virtual StructMemberInspector<int32>,
	virtual StructMemberInspector<uint32>
{
	virtual						~StructInspector();
};

typedef StructInspector<RemoteManagementRequest> ManagementRequestInspector;


template<>
struct ConstStructInspector<RemoteManagementRequest>
	:
	ConstStructInspectorBase<RemoteManagementRequest>,
	virtual StructMemberInspector<const int32>,
	virtual StructMemberInspector<const uint32>
{
	virtual						~ConstStructInspector();
};

typedef ConstStructInspector<RemoteManagementRequest>
	ConstManagementRequestInspector;


template<>
struct StructInspector<RemoteManagementResponse>
	:
	StructInspectorBase<RemoteManagementResponse>,
	virtual StructMemberInspector<int32>,
	virtual StructMemberInspector<uint32>
{
	virtual						~StructInspector();
};

typedef StructInspector<RemoteManagementResponse> ManagementResponseInspector;


template<>
struct ConstStructInspector<RemoteManagementResponse>
	:
	ConstStructInspectorBase<RemoteManagementResponse>,
	virtual StructMemberInspector<const int32>,
	virtual StructMemberInspector<const uint32>
{
	virtual						~ConstStructInspector();
};

typedef ConstStructInspector<RemoteManagementResponse>
	ConstManagementResponseInspector;
