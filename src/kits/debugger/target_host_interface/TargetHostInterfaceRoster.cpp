/*
 * Copyright 2016-2017, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#include "TargetHostInterfaceRoster.h"

#include <new>

#include <AutoDeleter.h>
#include <AutoLocker.h>

#include "LocalTargetHostInterfaceInfo.h"
#include "NetworkTargetHostInterfaceInfo.h"
#include "PipedCommandTargetHostInterfaceInfo.h"
#include "TargetHostInterfaceInfo.h"


/*static*/ TargetHostInterfaceRoster*
	TargetHostInterfaceRoster::sDefaultInstance = NULL;


TargetHostInterfaceRoster::TargetHostInterfaceRoster()
	:
	TargetHostInterface::Listener(),
	fLock(),
	fRunningTeamDebuggers(0),
	fInterfaceInfos(20, false),
	fActiveInterfaces(20, false),
	fListeners(8, false)
{
}


TargetHostInterfaceRoster::~TargetHostInterfaceRoster()
{
}


/*static*/ TargetHostInterfaceRoster*
TargetHostInterfaceRoster::Default()
{
	return sDefaultInstance;
}


/*static*/ status_t
TargetHostInterfaceRoster::CreateDefault()
{
	if (sDefaultInstance != NULL)
		return B_OK;

	TargetHostInterfaceRoster* roster
		= new(std::nothrow) TargetHostInterfaceRoster;
	if (roster == NULL)
		return B_NO_MEMORY;
	ObjectDeleter<TargetHostInterfaceRoster> rosterDeleter(roster);

	status_t error = roster->Init();
	if (error != B_OK)
		return error;

	error = roster->RegisterInterfaceInfos();
	if (error != B_OK)
		return error;

	sDefaultInstance = rosterDeleter.Detach();
	return B_OK;
}


/*static*/ void
TargetHostInterfaceRoster::DeleteDefault()
{
	TargetHostInterfaceRoster* roster = sDefaultInstance;
	sDefaultInstance = NULL;
	delete roster;
}


status_t
TargetHostInterfaceRoster::Init()
{
	return fLock.InitCheck();
}


status_t
TargetHostInterfaceRoster::RegisterInterfaceInfos()
{
	TargetHostInterfaceInfo* info = NULL;
	BReference<TargetHostInterfaceInfo> interfaceReference;

	#undef REGISTER_INTERFACE_INFO
	#define REGISTER_INTERFACE_INFO(type) \
		info = new(std::nothrow) type##TargetHostInterfaceInfo; \
		if (info == NULL) \
			return B_NO_MEMORY; \
		interfaceReference.SetTo(info, true); \
		if (info->Init() != B_OK) \
			return B_NO_MEMORY; \
		if (!fInterfaceInfos.AddItem(info)) \
			return B_NO_MEMORY; \
		interfaceReference.Detach();

	REGISTER_INTERFACE_INFO(Local)
	REGISTER_INTERFACE_INFO(PipedCommand)
	REGISTER_INTERFACE_INFO(Network)

	return B_OK;
}


int32
TargetHostInterfaceRoster::CountInterfaceInfos() const
{
	return fInterfaceInfos.CountItems();
}


TargetHostInterfaceInfo*
TargetHostInterfaceRoster::InterfaceInfoAt(int32 index) const
{
	return fInterfaceInfos.ItemAt(index);
}


status_t
TargetHostInterfaceRoster::CreateInterface(TargetHostInterfaceInfo* info,
	Settings* settings, const BString& connectionName,
	TargetHostInterface*& _interface)
{
	// TODO: this should eventually verify that an active interface with
	// matching settings/type doesn't already exist, and if so, return that
	// directly rather than instantiating a new one, since i.e. the interface
	// for the local host only requires one instance.
	AutoLocker<TargetHostInterfaceRoster> locker(this);
	TargetHostInterface* interface;
	status_t error = info->CreateInterface(settings, connectionName,
		interface);
	if (error != B_OK)
		return error;

	error = interface->Run();
	if (error < B_OK || !fActiveInterfaces.AddItem(interface)) {
		delete interface;
		return B_NO_MEMORY;
	}

	interface->AddListener(this);

	// notify listeners
	ListenerList listeners(fListeners);

	locker.Unlock();

	for (int32 i = 0; Listener* listener = listeners.ItemAt(i); i++)
		listener->TargetHostInterfaceAdded(interface);

	_interface = interface;
	return B_OK;
}


int32
TargetHostInterfaceRoster::CountActiveInterfaces()
{
	AutoLocker<TargetHostInterfaceRoster> locker(this);
	return fActiveInterfaces.CountItems();
}


TargetHostInterface*
TargetHostInterfaceRoster::ActiveInterfaceAt(int32 index)
{
	AutoLocker<TargetHostInterfaceRoster> locker(this);
	return fActiveInterfaces.ItemAt(index);
}


TargetHostInterface*
TargetHostInterfaceRoster::ActiveInterfaceForId(int32 id)
{
	AutoLocker<TargetHostInterfaceRoster> locker(this);

	for (int32 i = 0; i < fActiveInterfaces.CountItems(); i++) {
		TargetHostInterface* interface = fActiveInterfaces.ItemAt(i);
		if (interface->Id() == id)
			return interface;
	}

	return NULL;
}


bool
TargetHostInterfaceRoster::AddListener(Listener* listener)
{
	AutoLocker<TargetHostInterfaceRoster> locker(this);
	return fListeners.AddItem(listener);
}


bool
TargetHostInterfaceRoster::RemoveListener(Listener* listener)
{
	AutoLocker<TargetHostInterfaceRoster> locker(this);
	return fListeners.RemoveItem(listener);
}


void
TargetHostInterfaceRoster::TeamDebuggerStarted(TeamDebugger* debugger)
{
	int32 count = atomic_add(&fRunningTeamDebuggers, 1) + 1;

	// notify listeners
	AutoLocker<TargetHostInterfaceRoster> locker(this);
	ListenerList listeners(fListeners);
	locker.Unlock();

	for (int32 i = 0; Listener* listener = listeners.ItemAt(i); i++)
		listener->TeamDebuggerCountChanged(count);
}


void
TargetHostInterfaceRoster::TeamDebuggerQuit(TeamDebugger* debugger)
{
	int32 count = atomic_add(&fRunningTeamDebuggers, -1) - 1;

	// notify listeners
	AutoLocker<TargetHostInterfaceRoster> locker(this);
	ListenerList listeners(fListeners);
	locker.Unlock();

	for (int32 i = 0; Listener* listener = listeners.ItemAt(i); i++)
		listener->TeamDebuggerCountChanged(count);
}


void
TargetHostInterfaceRoster::TargetHostInterfaceQuit(
	TargetHostInterface* interface)
{
	AutoLocker<TargetHostInterfaceRoster> locker(this);
	fActiveInterfaces.RemoveItem(interface);

	ListenerList listeners(fListeners);

	locker.Unlock();

	for (int32 i = 0; Listener* listener = listeners.ItemAt(i); i++)
		listener->TargetHostInterfaceRemoved(interface);
}


// #pragma mark - TargetHostInterfaceRoster::Listener


TargetHostInterfaceRoster::Listener::~Listener()
{
}


void
TargetHostInterfaceRoster::Listener::TeamDebuggerCountChanged(int32 count)
{
}


void
TargetHostInterfaceRoster::Listener::TargetHostInterfaceAdded(
	TargetHostInterface* interface)
{
}


void
TargetHostInterfaceRoster::Listener::TargetHostInterfaceRemoved(
	TargetHostInterface* interface)
{
}
