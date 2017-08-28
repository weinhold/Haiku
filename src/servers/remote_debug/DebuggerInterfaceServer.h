/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <OS.h>

#include <util/DoublyLinkedList.h>


class Architecture;
struct DebugEvent;
struct DebuggerInterface;
struct RemoteDebugRequest;
struct RemoteDebugResponse;
template<typename Request, typename Response, typename Event>
	struct RemoteServerConnection;
typedef RemoteServerConnection<RemoteDebugRequest, RemoteDebugResponse,
	DebugEvent> RemoteDebugServerConnection;


struct DebuggerInterfaceServer
		: DoublyLinkedListLinkImpl<DebuggerInterfaceServer> {
			struct Listener;

								DebuggerInterfaceServer(
									DebuggerInterfaceServer::Listener* listener,
									DebuggerInterface* debuggerInterface,
									RemoteDebugServerConnection* connection);
								~DebuggerInterfaceServer();

			status_t			Init();
			void				Run();

private:
			struct RequestHandler;

private:
	static	status_t			_RequestHandlerEntry(void* data);
			status_t			_RequestHandler();
			status_t			_RequestHandlerLoop();

	static	status_t			_DebugEventHandlerEntry(void* data);
			status_t			_DebugEventHandler();

private:
			Listener*			fListener;
			DebuggerInterface*	fDebuggerInterface;
			RemoteDebugServerConnection* fConnection;
			thread_id			fRequestHandler;
			thread_id			fDebugEventHandler;
			bool				fTerminating;
};


struct DebuggerInterfaceServer::Listener {
	virtual						~Listener();

	virtual	void				ServerFinished(DebuggerInterfaceServer* server)
									= 0;
};
