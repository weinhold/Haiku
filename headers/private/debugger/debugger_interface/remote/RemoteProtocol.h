/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>


class BMessage;


template<typename RemoteData, typename Context>
status_t archiveRemoteData(const Context& context, const RemoteData& data,
			BMessage& archive);

template<typename RemoteData, typename Context>
status_t unarchiveRemoteData(const Context& context, const BMessage& archive,
			RemoteData*& _data);
