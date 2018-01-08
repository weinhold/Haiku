/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include "InspectableStruct.h"
#include "TeamInfo.h"

#include "debugger_interface/remote/RemoteInspectableStructMacros.h"
#include "debugger_interface/remote/RemoteManagementEvent.h"


#define DEFINE_EVENT_STRUCTS(...)	\
	ITERATE2_OUTER(DECLARE_EVENT_STRUCT, DEFINE_EMPTY, __VA_ARGS__) 	\
	DECLARE_VISITOR2(RemoteManagementEventVisitor, Event, __VA_ARGS__)	\
	ITERATE2_OUTER(DEFINE_EVENT_STRUCT, DEFINE_EMPTY, __VA_ARGS__)


/*!	Defines an event struct.
	Arguments are the name of the struct (without the "Event" suffix) and as the
	second argument -- in parentheses -- alternatingly an attribute type and its
	name.
*/
#define DEFINE_EVENT_STRUCT(name, fields) \
		DEFINE_EVENT_STRUCT_IMPL(name, UNWRAP_MACRO_ARGS fields)

#define DEFINE_EVENT_STRUCT_IMPL(name, ...) \
	DEFINE_INSPECTABLE_STRUCT(name ## Event, RemoteManagementEvent, \
		virtual void AcceptVisitor(RemoteManagementEventVisitor* visitor) { \
			visitor->Visit(this);	\
		}, \
		__VA_ARGS__)


// define the structs
#include "debugger_interface/remote/RemoteManagementEventDefs.h"


// undefine macros defined in RemoteInspectableStructMacros.h
#include "debugger_interface/remote/RemoteInspectableStructMacrosUndefine.h"

// undefine macros defined here
#undef DEFINE_EVENT_STRUCTS
#undef DEFINE_EVENT_STRUCT
#undef DEFINE_EVENT_STRUCT_IMPL
