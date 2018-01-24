/*
 * Copyright 2016, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef TARGET_HOST_INTERFACE_ROSTER_H
#define TARGET_HOST_INTERFACE_ROSTER_H

#include <OS.h>

#include <Locker.h>
#include <ObjectList.h>

#include "TargetHostInterface.h"


class Settings;
class TargetHostInterfaceInfo;


class TargetHostInterfaceRoster : private TargetHostInterface::Listener {
public:
	class Listener;
								TargetHostInterfaceRoster();
	virtual						~TargetHostInterfaceRoster();

	static	TargetHostInterfaceRoster* Default();
	static	status_t			CreateDefault();
	static	void				DeleteDefault();

			bool				Lock()		{ return fLock.Lock(); }
			void				Unlock()	{ fLock.Unlock(); }

			status_t			Init();
			status_t			RegisterInterfaceInfos();

			int32				CountInterfaceInfos() const;
			TargetHostInterfaceInfo*
								InterfaceInfoAt(int32 index) const;

			status_t			CreateInterface(TargetHostInterfaceInfo* info,
									Settings* settings,
									const BString& connectionName,
									TargetHostInterface*& _interface);

			int32				CountActiveInterfaces();
			TargetHostInterface* ActiveInterfaceAt(int32 index);
			TargetHostInterface* ActiveInterfaceForId(int32 id);

			int32				CountRunningTeamDebuggers() const
									{ return fRunningTeamDebuggers; }

			bool				AddListener(Listener* listener);
			bool				RemoveListener(Listener* listener);

private:
	// TargetHostInterface::Listener
	virtual	void				TeamDebuggerStarted(TeamDebugger* debugger);
	virtual	void 				TeamDebuggerQuit(TeamDebugger* debugger);
	virtual	void				TargetHostInterfaceQuit(
									TargetHostInterface* interface);

private:
			typedef BObjectList<TargetHostInterfaceInfo> InfoList;
			typedef BObjectList<TargetHostInterface> InterfaceList;
			typedef BObjectList<Listener> ListenerList;

private:
			BLocker				fLock;
	static	TargetHostInterfaceRoster* sDefaultInstance;

			int32				fRunningTeamDebuggers;
			InfoList			fInterfaceInfos;
			InterfaceList		fActiveInterfaces;
			ListenerList		fListeners;
};


class TargetHostInterfaceRoster::Listener {
public:
	virtual						~Listener();

	virtual	void				TeamDebuggerCountChanged(int32 newCount);

	virtual	void				TargetHostInterfaceAdded(
									TargetHostInterface* interface);
	virtual	void				TargetHostInterfaceRemoved(
									TargetHostInterface* interface);
};


#endif	// TARGET_HOST_INTERFACE_ROSTER_H
