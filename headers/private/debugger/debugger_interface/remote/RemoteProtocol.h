/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef REMOTE_PROTOCOL_H
#define REMOTE_PROTOCOL_H


#include "debugger_interface/remote/RemoteDebugRequest.h"


class BMessage;

class Architecture;
class DebugEvent;


status_t archiveRemoteDebugRequest(const Architecture* architecture,
			const RemoteDebugRequest& request, BMessage& archive);
status_t unarchiveRemoteDebugRequest(const Architecture* architecture,
			const BMessage& archive, RemoteDebugRequest*& _request);

status_t archiveDebugEvent(const Architecture* architecture,
			const DebugEvent& event, BMessage& archive);
status_t unarchiveDebugEvent(const Architecture* architecture,
			const BMessage& archive, DebugEvent*& _event);


#endif	// REMOTE_PROTOCOL_H
