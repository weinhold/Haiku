/*
 * Copyright 2016, Rene Gollent, rene@gollent.com.
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#include "GenericConnectionConfigHandler.h"

#include <AutoDeleter.h>

#include "TargetHostInterfaceInfo.h"


template<typename ConfigView>
GenericConnectionConfigHandler<ConfigView>::GenericConnectionConfigHandler(
		const char* name)
	:
	ConnectionConfigHandler(name)
{
}


template<typename ConfigView>
GenericConnectionConfigHandler<ConfigView>::~GenericConnectionConfigHandler()
{
}


template<typename ConfigView>
status_t
GenericConnectionConfigHandler<ConfigView>::CreateView(
	TargetHostInterfaceInfo* info, ConnectionConfigView::Listener* listener,
	ConnectionConfigView*& _view)
{
	ConfigView* view = NULL;
	try {
		view = new ConfigView;
		ObjectDeleter<BView> viewDeleter(view);
		status_t error = view->Init(info, listener);
		if (error != B_OK)
			return error;
		viewDeleter.Detach();
	} catch (...) {
		return B_NO_MEMORY;
	}

	_view = view;
	return B_OK;
}


// explicit template instantiation

#include "NetworkConnectionConfigView.h"
#include "PipedCommandConnectionConfigView.h"


template class GenericConnectionConfigHandler<NetworkConnectionConfigView>;
template class GenericConnectionConfigHandler<PipedCommandConnectionConfigView>;
