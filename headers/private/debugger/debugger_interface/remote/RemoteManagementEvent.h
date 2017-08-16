/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>

#include "util/InspectableStruct.h"
#include "util/StructInspector.h"


struct RemoteManagementEvent : InspectableStruct<RemoteManagementEvent> {};


template<>
struct StructInspector<RemoteManagementEvent>
	:
	StructInspectorBase<RemoteManagementEvent>,
	virtual StructMemberInspector<bool>
{
	virtual						~StructInspector();
};

typedef StructInspector<RemoteManagementEvent> ManagementEventInspector;


template<>
struct ConstStructInspector<RemoteManagementEvent>
	:
	ConstStructInspectorBase<RemoteManagementEvent>,
	virtual StructMemberInspector<const bool>
{
	virtual						~ConstStructInspector();
};

typedef ConstStructInspector<RemoteManagementEvent>
	ConstManagementEventInspector;
