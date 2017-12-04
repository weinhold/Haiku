/*
 * Copyright 2016, Rene Gollent, rene@gollent.com.
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include "ConnectionConfigHandler.h"


template<typename ConfigView>
class GenericConnectionConfigHandler : public ConnectionConfigHandler {
public:
								GenericConnectionConfigHandler(
									const char* name);
	virtual						~GenericConnectionConfigHandler();

	virtual	status_t			CreateView(TargetHostInterfaceInfo* info,
									ConnectionConfigView::Listener* listener,
									ConnectionConfigView*& _view);
};
