/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <Referenceable.h>
#include <String.h>

#include "TargetHostInterface.h"


struct ChannelMessenger;
struct RemoteManagementEvent;
struct RemoteManagementRequest;
struct RemoteManagementResponse;
template<typename Request, typename Response, typename Event>
	struct RemoteClientConnection;
typedef RemoteClientConnection<RemoteManagementRequest,
		RemoteManagementResponse, RemoteManagementEvent>
	RemoteManagementClientConnection;
struct Stream;
struct TeamAddedEvent;
struct TeamRemovedEvent;
struct TeamRenamedEvent;


class RemoteTargetHostInterface : public TargetHostInterface {
public:
								RemoteTargetHostInterface();
	virtual						~RemoteTargetHostInterface();

			status_t			Init(const BString& connectionName,
									Stream* stream);
	virtual	void				Close();

	virtual	bool				IsLocal() const;
	virtual	bool				Connected() const;

	virtual	TargetHost*			GetTargetHost();

	virtual	status_t			Attach(team_id id, thread_id threadID,
									DebuggerInterface*& _interface) const;
	virtual	status_t			CreateTeam(int commandLineArgc,
									const char* const* arguments,
									team_id& _teamID) const;
	virtual	status_t			LoadCore(const char* coreFilePath,
									DebuggerInterface*& _interface,
									thread_id& _thread) const;

	virtual	status_t			FindTeamByThread(thread_id thread,
									team_id& _teamID) const;

private:
	static	status_t			_EventHandlerEntry(void* data);
			status_t			_EventHandler();
			status_t			_HandleTeamEvent(team_id team, int32 opcode,
									bool& addToWaiters);
			void				_HandleTeamAdded(TeamAddedEvent& event);
			void				_HandleTeamRemoved(TeamRemovedEvent& event);
			void				_HandleTeamRenamed(TeamRenamedEvent& event);

private:
			BReference<ChannelMessenger> fMessenger;
			BReference<RemoteManagementClientConnection> fManagementConnection;
			TargetHost*			fTargetHost;
			thread_id			fEventThread;
			bool				fConnected;
};
