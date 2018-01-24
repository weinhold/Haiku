/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "DebuggerInterfaceServer.h"

#include "Architecture.h"
#include "DebugEvent.h"
#include "debugger_interface/DebuggerInterface.h"
#include "debugger_interface/remote/RemoteDebugRequests.h"
#include "debugger_interface/remote/RemoteServerConnection.h"


typedef RemoteDebugServerConnection::RequestId RequestId;


static const uint64 kMaxMemoryTransferSize = 1024 * 1024;


struct DebuggerInterfaceServer::RequestHandler : RemoteDebugRequestVisitor {
	RequestHandler(DebuggerInterfaceServer* server, RequestId requestId)
		:
		RemoteDebugRequestVisitor(),
		fServer(server),
		fRequestId(requestId)
	{
	}

	virtual void Visit(CloseRequest* request)
	{
		fServer->fTerminating = true;

		_SendResponse(request, CloseResponse(B_OK));
	}

	virtual void Visit(SetTeamDebuggingFlagsRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->SetTeamDebuggingFlags(
			request->flags);
		_SendResponse(request, SetTeamDebuggingFlagsResponse(error));
	}

	virtual void Visit(ContinueThreadRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->ContinueThread(
			request->threadId);
		_SendResponse(request, ContinueThreadResponse(error));
	}

	virtual void Visit(StopThreadRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->StopThread(
			request->threadId);
		_SendResponse(request, StopThreadResponse(error));
	}

	virtual void Visit(SingleStepThreadRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->SingleStepThread(
			request->threadId);
		_SendResponse(request, SingleStepThreadResponse(error));
	}

	virtual void Visit(InstallBreakpointRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->InstallBreakpoint(
			request->address);
		_SendResponse(request, InstallBreakpointResponse(error));
	}

	virtual void Visit(UninstallBreakpointRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->UninstallBreakpoint(
			request->address);
		_SendResponse(request, UninstallBreakpointResponse(error));
	}

	virtual void Visit(InstallWatchpointRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->InstallWatchpoint(
			request->address, request->type, request->length);
		_SendResponse(request, InstallWatchpointResponse(error));
	}

	virtual void Visit(UninstallWatchpointRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->UninstallWatchpoint(
			request->address);
		_SendResponse(request, UninstallWatchpointResponse(error));
	}

	virtual void Visit(GetTeamInfoRequest* request)
	{
		TeamInfo info;
		status_t error = fServer->fDebuggerInterface->GetTeamInfo(info);
		_SendResponse(request, GetTeamInfoResponse(error, info));
	}

	virtual void Visit(GetThreadInfosRequest* request)
	{
		BObjectList<ThreadInfo> infos;
		status_t error = fServer->fDebuggerInterface->GetThreadInfos(infos);
		_SendResponse(request, GetThreadInfosResponse(error, infos));
	}

	virtual void Visit(GetImageInfosRequest* request)
	{
		BObjectList<ImageInfo> infos;
		status_t error = fServer->fDebuggerInterface->GetImageInfos(infos);
		_SendResponse(request, GetImageInfosResponse(error, infos));
	}

	virtual void Visit(GetSymbolInfosRequest* request)
	{
		BObjectList<SymbolInfo> infos;
		status_t error = fServer->fDebuggerInterface->GetSymbolInfos(
			request->imageId, infos);
		_SendResponse(request, GetSymbolInfosResponse(error, infos));
	}

	virtual void Visit(GetSymbolInfoRequest* request)
	{
		SymbolInfo info;
		status_t error = fServer->fDebuggerInterface->GetSymbolInfo(
			request->imageId, request->name, request->type, info);
		_SendResponse(request, GetSymbolInfoResponse(error, info));
	}

	virtual void Visit(GetThreadInfoRequest* request)
	{
		ThreadInfo info;
		status_t error = fServer->fDebuggerInterface->GetThreadInfo(
			request->threadId, info);
		_SendResponse(request, GetThreadInfoResponse(error, info));
	}

	virtual void Visit(GetCpuStateRequest* request)
	{
		CpuState* cpuState = NULL;
		status_t error = fServer->fDebuggerInterface->GetCpuState(
			request->threadId, cpuState);
		BReference<CpuState> cpuStateReference(cpuState, true);
		_SendResponse(request,
			GetCpuStateResponse(error, Reference<CpuState>(cpuState, false)));
	}

	virtual void Visit(SetCpuStateRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->SetCpuState(
			request->threadId, request->cpuState.Get());
		_SendResponse(request, SetCpuStateResponse(error));
	}

	virtual void Visit(WriteCoreFileRequest* request)
	{
		status_t error = fServer->fDebuggerInterface->WriteCoreFile(
			request->path);
		_SendResponse(request, WriteCoreFileResponse(error));
	}

	virtual void Visit(GetMemoryPropertiesRequest* request)
	{
		uint32 protection;
		uint32 locking;
		status_t error = fServer->fDebuggerInterface->GetMemoryProperties(
			request->address, protection, locking);
		_SendResponse(request,
			GetMemoryPropertiesResponse(error, protection, locking));
	}

	virtual void Visit(ReadMemoryRequest* request)
	{
		size_t size = (size_t)std::min(request->size, kMaxMemoryTransferSize);
		void* buffer = malloc(size);
		MemoryDeleter bufferDeleter(buffer);

		status_t error = buffer != NULL ? B_OK : B_NO_MEMORY;
		RawData data;
		ssize_t bytesRead = 0;

		if (error == B_OK) {
			bytesRead = fServer->fDebuggerInterface->ReadMemory(
				request->address, buffer, size);
			if (bytesRead < 0)
				error = status_t(bytesRead);
		}
		if (error == B_OK)
			data.SetTo(buffer, bytesRead, RawData::Reference);
		_SendResponse(request, ReadMemoryResponse(error, data));
	}

	virtual void Visit(WriteMemoryRequest* request)
	{
		ssize_t bytesWritten = fServer->fDebuggerInterface->WriteMemory(
			request->address, request->data.Data(), request->data.Size());
		status_t error = bytesWritten >= 0 ? B_OK : status_t(bytesWritten);
		_SendResponse(request,
			WriteMemoryResponse(error, (uint64)bytesWritten));
	}

private:
	// Note: The request argument is not needed, but we use it to ensure the
	// correct response type.
	template<typename Request>
	void _SendResponse(const Request* /*request*/,
		const typename RemoteResponse<Request>::Type& response)
	{
		status_t error = fServer->fConnection->SendResponse(fRequestId,
			response);
		if (error != B_OK)
			throw error;
	}

private:
	DebuggerInterfaceServer*	fServer;
	RequestId					fRequestId;
};


DebuggerInterfaceServer::DebuggerInterfaceServer(
	DebuggerInterfaceServer::Listener* listener,
	DebuggerInterface* debuggerInterface,
	RemoteDebugServerConnection* connection)
	:
	fListener(listener),
	fDebuggerInterface(debuggerInterface),
	fConnection(connection),
	fRequestHandler(-1),
	fDebugEventHandler(-1)
{
	fDebuggerInterface->AcquireReference();
	fConnection->AcquireReference();
}


DebuggerInterfaceServer::~DebuggerInterfaceServer()
{
	fConnection->Close();

	if (fDebugEventHandler >= 0)
		wait_for_thread(fDebugEventHandler, NULL);
	if (fRequestHandler >= 0)
		wait_for_thread(fRequestHandler, NULL);

	fConnection->ReleaseReference();
	fDebuggerInterface->ReleaseReference();
}


status_t
DebuggerInterfaceServer::Init()
{
	fRequestHandler = spawn_thread(&_RequestHandlerEntry,
		"debug request handler", B_NORMAL_PRIORITY, this);
	if (fRequestHandler < 0)
		return fRequestHandler;

	fDebugEventHandler = spawn_thread(&_DebugEventHandlerEntry,
		"debug event handler", B_NORMAL_PRIORITY, this);
	if (fDebugEventHandler < 0)
		return fDebugEventHandler;

	return B_OK;
}


void
DebuggerInterfaceServer::Run()
{
	resume_thread(fRequestHandler);
	resume_thread(fDebugEventHandler);
}


/*static*/ status_t
DebuggerInterfaceServer::_RequestHandlerEntry(void* data)
{
	return ((DebuggerInterfaceServer*)data)->_RequestHandler();
}


status_t
DebuggerInterfaceServer::_RequestHandler()
{
	status_t error = _RequestHandlerLoop();

	fTerminating = true;
	fListener->ServerFinished(this);

	return error;
}


status_t
DebuggerInterfaceServer::_RequestHandlerLoop()
{
	while (!fTerminating) {
		// receive next request
		RemoteDebugRequest* request = NULL;
		RequestId requestId;
		status_t error = fConnection->ReceiveRequest(request, requestId);
		if (error != B_OK)
			return error;

		ObjectDeleter<RemoteDebugRequest> requestDeleter(request);

		if (fTerminating)
			break;

		// handle the request
		try {
			RequestHandler handler(this, requestId);
			request->AcceptVisitor(&handler);
		} catch (status_t error) {
			return error;
		}
	}

	return B_OK;
}


/*static*/ status_t
DebuggerInterfaceServer::_DebugEventHandlerEntry(void* data)
{
	return ((DebuggerInterfaceServer*)data)->_DebugEventHandler();
}


status_t
DebuggerInterfaceServer::_DebugEventHandler()
{
	status_t error = B_OK;

	while (!fTerminating) {
		// get the next event
		DebugEvent* event;
		error = fDebuggerInterface->GetNextDebugEvent(event);
		if (error != B_OK) {
			if (error == B_INTERRUPTED)
				continue;
			break;
		}

		ObjectDeleter<DebugEvent> eventDeleter(event);

		if (fTerminating)
			break;

		// handle the event (send it to the debugger)
		error = fConnection->SendEvent(*event);
		if (error != B_OK)
			break;
	}

	fTerminating = true;
	fListener->ServerFinished(this);

	return error;
}


// #pragma mark - DebuggerInterfaceServer::Listener {


DebuggerInterfaceServer::Listener::~Listener()
{
}
