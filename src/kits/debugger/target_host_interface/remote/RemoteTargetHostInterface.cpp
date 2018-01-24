/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "RemoteTargetHostInterface.h"

#include <stdio.h>

#include <AutoDeleter.h>
#include <AutoLocker.h>

#include "ArchitectureFactory.h"
#include "TargetHost.h"
#include "Tracing.h"

#include "debugger_interface/remote/MessageRemoteClientConnection.h"
#include "debugger_interface/remote/RemoteDebugFactoryContext.h"
#include "debugger_interface/remote/RemoteDebuggerInterface.h"
#include "debugger_interface/remote/RemoteManagementEvents.h"
#include "debugger_interface/remote/RemoteManagementFactoryContext.h"
#include "debugger_interface/remote/RemoteManagementRequests.h"
#include "debugger_interface/remote/SingleChannelMessenger.h"
#include "debugger_interface/remote/StreamMessenger.h"


namespace {


template<typename RequestType>
status_t
sendRequest(RemoteManagementClientConnection& connection,
		const RequestType& request,
		ObjectDeleter<typename RemoteResponse<RequestType>::Type>& _response)
{
	typedef typename RemoteResponse<RequestType>::Type ResponseType;

	// send request with response
	RemoteManagementResponse* response;
	status_t error = connection.SendRequest(request, response);
	if (error != B_OK)
		return error;
	ObjectDeleter<RemoteManagementResponse> responseDeleter(response);

	// check response type
	ResponseType* concreteResponse = dynamic_cast<ResponseType*>(response);
	if (concreteResponse == NULL)
		return B_BAD_DATA;

	// check the response error
	if (concreteResponse->error != B_OK)
		return concreteResponse->error;

	_response.SetTo(concreteResponse);
	responseDeleter.Detach();
	return B_OK;
}


} // anonymous namespace


RemoteTargetHostInterface::RemoteTargetHostInterface()
	:
	TargetHostInterface(),
	fManagementConnection(),
	fTargetHost(NULL),
	fEventThread(-1),
	fConnected(false)
{
	SetName("Remote");
}


RemoteTargetHostInterface::~RemoteTargetHostInterface()
{
	Close();

	if (fTargetHost != NULL)
		fTargetHost->ReleaseReference();
}


status_t
RemoteTargetHostInterface::Init(const BString& connectionName, Stream* stream)
{
	fTargetHost = new(std::nothrow) TargetHost(connectionName);
	if (fTargetHost == NULL)
		return B_NO_MEMORY;

	// create connection
	BReference<StreamMessenger> messenger = new(std::nothrow) StreamMessenger;
	if (messenger == NULL)
		return B_NO_MEMORY;

	status_t error = messenger->SetTo(stream);
	if (error != B_OK)
		return error;

	fMessenger = messenger;

	fManagementConnection = new(std::nothrow) MessageRemoteClientConnection<
			RemoteManagementRequest, RemoteManagementResponse,
			RemoteManagementEvent, RemoteManagementFactoryContext>(
			RemoteManagementFactoryContext(), messenger.Get());
	if (fManagementConnection == NULL)
		return B_NO_MEMORY;

	// send the Hello request
	{
		TRACE_REMOTE("sending hello request\n");
		HelloRequest request(kRemoteManagementProtocolVersion);
		ObjectDeleter<HelloResponse> response;
		error = sendRequest(*fManagementConnection, request, response);
		if (error != B_OK) {
			TRACE_REMOTE("server: request failed: %s\n", strerror(error));
			return error;
		}

		TRACE_REMOTE("received hello response, protocol version: %" B_PRIu32
			"\n", response->protocolVersion);
		if (response->protocolVersion != kRemoteManagementProtocolVersion)
			return B_MISMATCHED_VALUES;
	}

	// send a GetTeams request
	{
		TRACE_REMOTE("sending get teams request\n");
		ObjectDeleter<GetTeamsResponse> response;
		error = sendRequest(*fManagementConnection, GetTeamsRequest(),
			response);
		if (error != B_OK) {
			TRACE_REMOTE("server: request failed: %s\n", strerror(error));
			return error;
		}

		TRACE_REMOTE("got %" B_PRIu32 " teams\n", response->infos.CountItems());

		for (int32 i = 0; TeamInfo* info = response->infos.ItemAt(i); i++)
			fTargetHost->AddTeam(*info);
	}

	// spawn thread for receiving management events
	fEventThread = spawn_thread(_EventHandlerEntry, "Remote Target Host Loop",
		B_NORMAL_PRIORITY, this);
	if (fEventThread < 0)
		return fEventThread;

	resume_thread(fEventThread);

	return B_OK;
}


void
RemoteTargetHostInterface::Close()
{
	if (fManagementConnection != NULL)
		fManagementConnection->Close();

	if (fEventThread >= 0) {
		wait_for_thread(fEventThread, NULL);
		fEventThread = -1;
	}

	fManagementConnection = NULL;
	fMessenger = NULL;

	fConnected = false;
}


bool
RemoteTargetHostInterface::IsLocal() const
{
	return false;
}


bool
RemoteTargetHostInterface::Connected() const
{
	return fConnected;
}


TargetHost*
RemoteTargetHostInterface::GetTargetHost()
{
	return fTargetHost;
}


status_t
RemoteTargetHostInterface::Attach(team_id teamID, thread_id threadID,
	DebuggerInterface*& _interface) const
{
	if (teamID < 0 && threadID < 0)
		return B_BAD_VALUE;

	// send the request
	ObjectDeleter<AttachToTeamResponse> response;
	status_t error = sendRequest(*fManagementConnection,
		AttachToTeamRequest(teamID, threadID), response);
	if (error != B_OK)
		return error;
	ChannelMessenger::ChannelId channelId = response->channel;

	// get the architecture for the target team
	Architecture* architecture;
	error = ArchitectureFactory::CreateArchitecture(
		response->architecture.String(), response->cpuFeatureFlags,
		architecture);
	if (error != B_OK)
		return error;

	// create the messenger for the channel
	BReference<SingleChannelMessenger> messenger(
			new(std::nothrow) SingleChannelMessenger(fMessenger.Get(),
				channelId),
			true);
	if (messenger.Get() == NULL)
		return B_NO_MEMORY;

	// create the debug connection
	typedef MessageRemoteClientConnection<RemoteDebugRequest,
			RemoteDebugResponse, DebugEvent, RemoteDebugFactoryContext>
		DebugConnection;
	BReference<DebugConnection> connection(
			new(std::nothrow) DebugConnection(
				RemoteDebugFactoryContext(architecture), messenger.Get()),
			true);
	if (messenger.Get() == NULL)
		return B_NO_MEMORY;

	// create the debugger interface
	RemoteDebuggerInterface* debuggerInterface
		= new(std::nothrow) RemoteDebuggerInterface(connection);
	if (debuggerInterface == NULL)
		return B_NO_MEMORY;
	BReference<RemoteDebuggerInterface> debuggerInterfaceReference(
		debuggerInterface, true);

	error = debuggerInterface->Init();
	if (error != B_OK)
		return error;

	_interface = debuggerInterfaceReference.Detach();
	return B_OK;
}


status_t
RemoteTargetHostInterface::CreateTeam(int commandLineArgc,
	const char* const* arguments, team_id& _teamID) const
{
// TODO:...
	return B_NOT_SUPPORTED;
}


status_t
RemoteTargetHostInterface::LoadCore(const char* coreFilePath,
	DebuggerInterface*& _interface, thread_id& _thread) const
{
	return B_NOT_SUPPORTED;
}


status_t
RemoteTargetHostInterface::FindTeamByThread(thread_id thread,
	team_id& _teamID) const
{
// TODO:...
	return B_NOT_SUPPORTED;
}


status_t
RemoteTargetHostInterface::_EventHandlerEntry(void* data)
{
	return ((RemoteTargetHostInterface*)data)->_EventHandler();
}


status_t
RemoteTargetHostInterface::_EventHandler()
{
	struct Visitor : RemoteManagementEventVisitor {
		Visitor(RemoteTargetHostInterface* self)
			:
			fSelf(self)
		{
		}

		virtual void Visit(TeamAddedEvent* event)
		{
			fSelf->_HandleTeamAdded(*event);
		}

		virtual void Visit(TeamRemovedEvent* event)
		{
			fSelf->_HandleTeamRemoved(*event);
		}

		virtual void Visit(TeamRenamedEvent* event)
		{
			fSelf->_HandleTeamRenamed(*event);
		}

	private:
		RemoteTargetHostInterface* fSelf;
	} visitor(this);

	for (;;) {
		// get next event from server
		RemoteManagementEvent* event;
		status_t error = fManagementConnection->GetNextEvent(event);
		if (error != B_OK)
			break;
		ObjectDeleter<RemoteManagementEvent> eventDeleter(event);

		TRACE_REMOTE("received event: %s\n", event->StructName());

		event->AcceptVisitor(&visitor);
	}

	fConnected = false;

	return B_OK;
}

void
RemoteTargetHostInterface::_HandleTeamAdded(TeamAddedEvent& event)
{
	TRACE_REMOTE("remote: team added: %" B_PRId32 ": %s\n", event.info.TeamID(),
		event.info.Arguments().String());

	AutoLocker<TargetHost> targetHostLocker(fTargetHost);
	if (fTargetHost->TeamInfoByID(event.info.TeamID()) != NULL)
		fTargetHost->UpdateTeam(event.info);
	else
		fTargetHost->AddTeam(event.info);
}


void
RemoteTargetHostInterface::_HandleTeamRemoved(TeamRemovedEvent& event)
{
	TRACE_REMOTE("remote: team removed: %" B_PRId32 "\n", event.teamId);

	AutoLocker<TargetHost> targetHostLocker(fTargetHost);
	fTargetHost->RemoveTeam(event.teamId);
}


void
RemoteTargetHostInterface::_HandleTeamRenamed(TeamRenamedEvent& event)
{
	TRACE_REMOTE("remote: team renamed: %" B_PRId32 ": %s\n",
		event.info.TeamID(), event.info.Arguments().String());

	AutoLocker<TargetHost> targetHostLocker(fTargetHost);
	fTargetHost->UpdateTeam(event.info);
}
