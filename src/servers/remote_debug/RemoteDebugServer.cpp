/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <OS.h>
#include <Referenceable.h>

#include <AutoDeleter.h>
#include <AutoLocker.h>

#include <Architecture.h>
#include <DebuggerGlobals.h>
#include <DebuggerInterface.h>
#include <debugger_interface/remote/FdStream.h>
#include <debugger_interface/remote/MessageRemoteServerConnection.h>
#include <debugger_interface/remote/RemoteDebugFactoryContext.h>
#include <debugger_interface/remote/RemoteManagementEvents.h>
#include <debugger_interface/remote/RemoteManagementFactoryContext.h>
#include <debugger_interface/remote/RemoteManagementRequests.h>
#include <debugger_interface/remote/SingleChannelMessenger.h>
#include <debugger_interface/remote/StreamMessenger.h>
#include <TargetHost.h>
#include <TargetHostInterfaceRoster.h>
#include <Tracing.h>

#include "DebuggerInterfaceServer.h"


struct RemoteDebugResponse;

extern const char* __progname;
const char* kProgramName = __progname;


static const char* const kUsage =
	"Usage: %s [ --team <team> | --thread <thread> ] [ <options> ]\n"
	"\n"
	"Starts a remote debug server and attaches it to a team. If a specific\n"
	"thread is specified that thread is stopped.\n"
	"\n"
	"Currently the remote debug server supports only stdin plus stdout as\n"
	"transport for the remote debugging protocol. Therefore it can be used\n"
	"e.g. in combination with ssh.\n"
	"\n"
	"Options:\n"
	"  -h, --help         - Print this usage info and exit.\n"
	"  --team <team>      - Attach to the team with ID <team>.\n"
	"  --thread <thread>  - Attach to the team of the thread with ID <thread>\n"
	"                       and stop the thread.\n"
;


static void
print_usage_and_exit(bool error)
{
    fprintf(error ? stderr : stdout, kUsage, kProgramName);
    exit(error ? 1 : 0);
}


struct Options {
	int					commandLineArgc;
	const char* const*	commandLineArgv;
	team_id				team;
	thread_id			thread;

	Options()
		:
		commandLineArgc(0),
		commandLineArgv(NULL),
		team(-1),
		thread(-1)
	{
	}
};


static bool
parse_arguments(int argc, const char* const* argv, bool noOutput,
	Options& options)
{
	optind = 1;

	while (true) {
		static struct option sLongOptions[] = {
			{ "help", no_argument, 0, 'h' },
			{ "team", required_argument, 0, 't' },
			{ "thread", required_argument, 0, 'T' },
			{ 0, 0, 0, 0 }
		};

		opterr = 0; // don't print errors

		int c = getopt_long(argc, (char**)argv, "+h", sLongOptions, NULL);
		if (c == -1)
			break;

		switch (c) {
			case 'h':
				if (noOutput)
					return false;
				print_usage_and_exit(false);
				break;

			case 't':
			{
				options.team = strtol(optarg, NULL, 0);
				if (options.team <= 0) {
					if (noOutput)
						return false;
					print_usage_and_exit(true);
				}
				break;
			}

			case 'T':
			{
				options.thread = strtol(optarg, NULL, 0);
				if (options.thread <= 0) {
					if (noOutput)
						return false;
					print_usage_and_exit(true);
				}
				break;
			}

			default:
				if (noOutput)
					return false;
				print_usage_and_exit(true);
				break;
		}
	}

	if (optind < argc) {
		options.commandLineArgc = argc - optind;
		options.commandLineArgv = argv + optind;
	}

	int exclusiveParams = 0;
	if (options.team > 0)
		exclusiveParams++;
	if (options.thread > 0)
		exclusiveParams++;
	if (options.commandLineArgc > 0)
		exclusiveParams++;

	if (exclusiveParams == 0) {
		return true;
	} else if (exclusiveParams != 1) {
		if (noOutput)
			return false;
		print_usage_and_exit(true);
	}

	return true;
}


struct RemoteDebugServer : private TargetHostInterfaceRoster::Listener,
	private DebuggerInterfaceServer::Listener,
	private TargetHost::Listener,
	private RemoteManagementRequestVisitor {
	RemoteDebugServer()
		:
		TargetHostInterfaceRoster::Listener(),
		DebuggerInterfaceServer::Listener(),
		fLock("servers"),
		fHostInterface(NULL),
		fHost(NULL),
		fStreamMessenger(),
		fManagementConnection(),
		fServers(),
		fFinishedServers(),
		fCurrentRequestId()
	{
	}

	~RemoteDebugServer()
	{
		debugger_global_uninit();
	}

	bool Run()
	{
		status_t error = _Init();
		if (error != B_OK)
			return false;

		error = _InitConnection();
		if (error != B_OK)
			return false;

		_Run();
		_CleanupFinishedServers();

		return true;
	}

	virtual void TeamDebuggerCountChanged(int32 /*newCount*/)
	{
		// nothing to do
	}

private:
	status_t _Init()
	{
		status_t error = debugger_global_init();
		if (error != B_OK)
			return error;

		TargetHostInterfaceRoster::Default()->AddListener(this);

		error = fLock.InitCheck();
		if (error != B_OK)
			return error;

		fHostInterface
			= TargetHostInterfaceRoster::Default()->ActiveInterfaceAt(0);
		fHost = fHostInterface->GetTargetHost();
		fHost->AddListener(this);

		BReference<Stream> stream(new(std::nothrow) FdStream(0, 1), true);
		if (stream == NULL)
			return B_NO_MEMORY;

		fStreamMessenger.SetTo(new(std::nothrow) StreamMessenger, true);
		if (fStreamMessenger == NULL)
			return B_NO_MEMORY;

		error = fStreamMessenger->SetTo(stream);
		if (error != B_OK)
			return error;

		fManagementConnection.SetTo(
			new(std::nothrow) ManagementConnection(
				RemoteManagementFactoryContext(), fStreamMessenger),
			true);
		if (fManagementConnection == NULL)
			return B_NO_MEMORY;

		return B_OK;
	}

	status_t _InitConnection()
	{
		// handle the initial hello request
		TRACE_REMOTE("server: waiting for hello request\n");
		RemoteManagementRequest* request = NULL;
		ManagementConnection::RequestId requestId;
		status_t error = fManagementConnection->ReceiveRequest(request,
			requestId);
		if (error != B_OK) {
			TRACE_REMOTE("server: failed to receive request\n");
			return error;
		}

		ObjectDeleter<RemoteManagementRequest> requestDeleter(request);

		// check the request
		HelloRequest* helloRequest = dynamic_cast<HelloRequest*>(request);
		if (helloRequest == NULL) {
			TRACE_REMOTE("server: unexpected request: %s\n",
				request->StructName());
			return B_BAD_DATA;
		}

		status_t status = B_OK;
		TRACE_REMOTE("server: received hello request, protocol version %"
			B_PRIu32 "\n", helloRequest->protocolVersion);
		if (helloRequest->protocolVersion != kRemoteManagementProtocolVersion) {
			// protocol version mismatch
			status = B_UNSUPPORTED;
		}

		TRACE_REMOTE("server: sending hello response\n");
		HelloResponse response(status, kRemoteManagementProtocolVersion);
		error = fManagementConnection->SendResponse(requestId, response);
		return error == B_OK ? status : error;
	}

	status_t _Run()
	{
		for (;;) {
			_CleanupFinishedServers();

			RemoteManagementRequest* request = NULL;
			status_t error = fManagementConnection->ReceiveRequest(request,
				fCurrentRequestId);
			if (error != B_OK)
				return error;

			ObjectDeleter<RemoteManagementRequest> requestDeleter(request);

			try {
				request->AcceptVisitor(this);
			} catch (status_t error) {
				return error;
			}
		}
	}

	void _CleanupFinishedServers()
	{
		AutoLocker<BLocker> locker(fLock);

		ServerList servers;
		servers.MoveFrom(&fFinishedServers);

		locker.Unlock();

		while (DebuggerInterfaceServer* server = servers.RemoveHead())
			delete server;
	}

private:
	// DebuggerInterfaceServer::Listener

	virtual void ServerFinished(DebuggerInterfaceServer* server)
	{
		AutoLocker<BLocker> locker(fLock);
		fServers.Remove(server);
		fFinishedServers.Add(server);
	}

private:
	// TargetHost::Listener

	virtual void TeamAdded(const TeamInfo& info)
	{
		fManagementConnection->SendEvent(TeamAddedEvent(info));
	}

	virtual void TeamRemoved(team_id team)
	{
		fManagementConnection->SendEvent(TeamRemovedEvent(team));
	}

	virtual void TeamRenamed(const TeamInfo& info)
	{
		fManagementConnection->SendEvent(TeamRenamedEvent(info));
	}

private:
	// RemoteManagementRequestVisitor

	virtual void Visit(HelloRequest* request)
	{
		// unexpected, should only be received when the connection is initiated
		TRACE_REMOTE("server: received unexpected hello request\n");
		throw status_t(B_UNSUPPORTED);
	}

	virtual void Visit(GetTeamsRequest* request)
	{
		TRACE_REMOTE("server: received get teams request\n");

		// prepare the response
		GetTeamsResponse response;
		response.error = B_OK;

		response.infos.SetOwning(true);

		// We hold the TargetHost lock until we've sent the response. This
		// ensures that events we get after we have retrieved the list of
		// running teams won't be sent before the response is sent.
		AutoLocker<TargetHost> hostLocker(fHost);

		int32 count = fHost->CountTeams();
		for (int32 i = 0; i < count; i++) {
			TeamInfo* info = fHost->TeamInfoAt(i);
			TeamInfo* clonedInfo = new(std::nothrow) TeamInfo(*info);
			if (clonedInfo == NULL || !response.infos.AddItem(clonedInfo)) {
				delete clonedInfo;
				response.infos.MakeEmpty();
				response.error = B_NO_MEMORY;
				break;
			}
		}

		// send the response
		TRACE_REMOTE("server: sending get teams response (ID: %" B_PRIu64 ")\n",
			fCurrentRequestId);
		fManagementConnection->SendResponse(fCurrentRequestId, response);
	}

	virtual void Visit(AttachToTeamRequest* request)
	{
		TRACE_REMOTE("server: received attach to team request\n");

		// create a new channel
		ChannelId channelId;
		status_t requestError = fStreamMessenger->NewChannel(channelId);
		BString architectureName;
		uint32 cpuFeatureFlags = 0;

		// start the debugger interface server
		ObjectDeleter<DebuggerInterfaceServer> serverDeleter;
		BReference<DebuggerInterface> debuggerInterface;
		if (requestError == B_OK) {
			requestError = _AttachToTeam(channelId, request->teamId,
				request->threadId, serverDeleter, debuggerInterface);
			if (requestError == B_OK) {
				Architecture* architecture
					= debuggerInterface->GetArchitecture();
				architectureName = architecture->Name();
				cpuFeatureFlags = architecture->CpuFeatures();
			} else
				fStreamMessenger->DeleteChannel(channelId);
		}

		// send the response
		TRACE_REMOTE("server: sending attach to team response\n");
		AttachToTeamResponse response(requestError, channelId, architectureName,
			cpuFeatureFlags);
		status_t error = fManagementConnection->SendResponse(fCurrentRequestId,
			response);
		if (error != B_OK)
			return;

		if (requestError == B_OK) {
			AutoLocker<BLocker> locker(fLock);
			DebuggerInterfaceServer* server = serverDeleter.Detach();
			fServers.Add(server);
			locker.Unlock();

			server->Run();
		}
	}

private:
	typedef MessageRemoteServerConnection<RemoteManagementRequest,
			RemoteManagementResponse, RemoteManagementEvent,
			RemoteManagementFactoryContext>
		ManagementConnection;
	typedef MessageRemoteServerConnection<RemoteDebugRequest,
			RemoteDebugResponse, DebugEvent, RemoteDebugFactoryContext>
		DebugConnection;
	typedef StreamMessenger::ChannelId ChannelId;
	typedef ManagementConnection::RequestId RequestId;

	typedef DoublyLinkedList<DebuggerInterfaceServer> ServerList;

private:
	status_t _AttachToTeam(ChannelId channelId, team_id teamId,
		thread_id threadId, ObjectDeleter<DebuggerInterfaceServer>& _server,
		BReference<DebuggerInterface>& _debuggerInterface)
	{
		// create the debugger interface
		DebuggerInterface* debuggerInterface;
		status_t error = fHostInterface->Attach(teamId, threadId,
			debuggerInterface);
		if (error != B_OK)
			return error;
		BReference<DebuggerInterface> debuggerInterfaceReference(
			debuggerInterface, true);

		// create a messenger using the new channel
		BReference<SingleChannelMessenger> messenger(
			new(std::nothrow) SingleChannelMessenger(fStreamMessenger,
				channelId),
			true);
		if (messenger == NULL)
			return B_NO_MEMORY;

		// create the debug connection
		BReference<DebugConnection> connection(
			new(std::nothrow) DebugConnection(
				RemoteDebugFactoryContext(debuggerInterface->GetArchitecture()),
				messenger),
			true);
		if (connection == NULL)
			return B_NO_MEMORY;

		// create the debugger interface server
		DebuggerInterfaceServer* server
			= new(std::nothrow) DebuggerInterfaceServer(this, debuggerInterface,
				connection);
		if (server == NULL)
			return B_NO_MEMORY;
		ObjectDeleter<DebuggerInterfaceServer> serverDeleter(server);

		error = server->Init();
		if (error != B_OK)
			return error;

		_server.SetTo(serverDeleter.Detach());
		_debuggerInterface = debuggerInterfaceReference;
		return B_OK;
	}

private:
	BLocker								fLock;
	TargetHostInterface*				fHostInterface;
	TargetHost*							fHost;
	BReference<StreamMessenger>			fStreamMessenger;
	BReference<ManagementConnection>	fManagementConnection;
	ServerList							fServers;
	ServerList							fFinishedServers;
	RequestId							fCurrentRequestId;
};


int
main(int argc, const char* const* argv)
{
	Options options;
	parse_arguments(argc, argv, false, options);

	RemoteDebugServer server;
	return server.Run() ? 0 : 1;
}
