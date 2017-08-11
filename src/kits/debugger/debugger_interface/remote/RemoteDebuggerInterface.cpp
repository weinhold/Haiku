/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/RemoteDebuggerInterface.h"

#include <new>

#include <AutoDeleter.h>

#include "ArchitectureX86.h"
#include "ArchitectureX8664.h"
#include "debugger_interface/remote/RemoteClientConnection.h"
#include "debugger_interface/remote/RemoteDebugRequests.h"
#include "debugger_interface/remote/RemoteProtocol.h"


namespace {


template<typename RequestType>
status_t
sendRequest(RemoteDebugClientConnection& connection, const RequestType& request,
		ObjectDeleter<typename RemoteResponse<RequestType>::Type>& _response)
{
	typedef typename RemoteResponse<RequestType>::Type ResponseType;

	// send request with response
	RemoteDebugRequest* response;
	status_t error = connection.SendRequest(request, response);
	if (error != B_OK)
		return error;
	ObjectDeleter<RemoteDebugRequest> responseDeleter(response);

	// check response type
	ResponseType* concreteResponse = dynamic_cast<ResponseType*>(response);
	if (concreteResponse == NULL)
		return B_BAD_DATA;

	// check the response error
	if (concreteResponse->error != B_OK)
		return concreteResponse->error;

	_response = concreteResponse;
	responseDeleter.Detach();
	return B_OK;
}


template<typename RequestType>
status_t
sendRequestStatusOnlyResponse(RemoteDebugClientConnection& connection,
	const RequestType& request)
{
	ObjectDeleter<typename RemoteResponse<RequestType>::Type> response;
	return sendRequest(connection, request, response);
}


} // anonymous namespace


RemoteDebuggerInterface::RemoteDebuggerInterface(
		RemoteDebugClientConnection* connection)
	:
	fConnection(connection),
	fArchitecture(NULL)
{
	fConnection->AcquireReference();
}


RemoteDebuggerInterface::~RemoteDebuggerInterface()
{
	if (fArchitecture != NULL)
		fArchitecture->ReleaseReference();

	if (fConnection != NULL)
		fConnection->ReleaseReference();
}


status_t
RemoteDebuggerInterface::Init()
{
	// create the Architecture object
	// TODO: implement
	return B_UNSUPPORTED;
}


void
RemoteDebuggerInterface::Close(bool killTeam)
{
	// TODO: implement
}


bool
RemoteDebuggerInterface::Connected() const
{
	// TODO: implement
	return true;
}


bool
RemoteDebuggerInterface::IsPostMortem() const
{
	return false;
}


team_id
RemoteDebuggerInterface::TeamID() const
{
	// TODO: implement
	return B_UNSUPPORTED;
}


Architecture*
RemoteDebuggerInterface::GetArchitecture() const
{
	return fArchitecture;
}


status_t
RemoteDebuggerInterface::GetNextDebugEvent(DebugEvent*& _event)
{
	return fConnection->GetNextEvent(_event);
}


status_t
RemoteDebuggerInterface::SetTeamDebuggingFlags(uint32 flags)
{
	// TODO: implement
	return B_UNSUPPORTED;
}


status_t
RemoteDebuggerInterface::ContinueThread(thread_id thread)
{
	return sendRequestStatusOnlyResponse(*fConnection,
		ContinueThreadRequest(thread));
}


status_t
RemoteDebuggerInterface::StopThread(thread_id thread)
{
	return sendRequestStatusOnlyResponse(*fConnection,
		StopThreadRequest(thread));
}


status_t
RemoteDebuggerInterface::SingleStepThread(thread_id thread)
{
	return sendRequestStatusOnlyResponse(*fConnection,
		SingleStepThreadRequest(thread));
}


status_t
RemoteDebuggerInterface::InstallBreakpoint(target_addr_t address)
{
	return sendRequestStatusOnlyResponse(*fConnection,
		InstallBreakpointRequest(address));
}


status_t
RemoteDebuggerInterface::UninstallBreakpoint(target_addr_t address)
{
	return sendRequestStatusOnlyResponse(*fConnection,
		UninstallBreakpointRequest(address));
}


status_t
RemoteDebuggerInterface::InstallWatchpoint(target_addr_t address, uint32 type,
	int32 length)
{
	return sendRequestStatusOnlyResponse(*fConnection,
		InstallWatchpointRequest(address, type, length));
}


status_t
RemoteDebuggerInterface::UninstallWatchpoint(target_addr_t address)
{
	return sendRequestStatusOnlyResponse(*fConnection,
		UninstallWatchpointRequest(address));
}


status_t
RemoteDebuggerInterface::GetSystemInfo(SystemInfo& info)
{
	// TODO: implement
	return B_UNSUPPORTED;
}


status_t
RemoteDebuggerInterface::GetTeamInfo(TeamInfo& _info)
{
	ObjectDeleter<GetTeamInfoResponse> response;
	status_t error = sendRequest(*fConnection, GetTeamInfoRequest(), response);
	if (error != B_OK)
		return error;

	_info = response->info;
	return B_OK;
}


status_t
RemoteDebuggerInterface::GetThreadInfos(BObjectList<ThreadInfo>& _infos)
{
	ObjectDeleter<GetThreadInfosResponse> response;
	status_t error = sendRequest(*fConnection, GetThreadInfosRequest(),
		response);
	if (error != B_OK)
		return error;

	// TODO: Move semantics or std::swap for BObjectList!
	_infos = response->infos;
	return B_OK;
}


status_t
RemoteDebuggerInterface::GetImageInfos(BObjectList<ImageInfo>& _infos)
{
	ObjectDeleter<GetImageInfosResponse> response;
	status_t error = sendRequest(*fConnection, GetImageInfosRequest(),
		response);
	if (error != B_OK)
		return error;

	// TODO: Move semantics or std::swap for BObjectList!
	_infos = response->infos;
	return B_OK;
}


status_t
RemoteDebuggerInterface::GetAreaInfos(BObjectList<AreaInfo>& infos)
{
	// TODO: implement
	return B_UNSUPPORTED;
}


status_t
RemoteDebuggerInterface::GetSemaphoreInfos(BObjectList<SemaphoreInfo>& infos)
{
	// TODO: implement
	return B_UNSUPPORTED;
}


status_t
RemoteDebuggerInterface::GetSymbolInfos(team_id team, image_id image,
	BObjectList<SymbolInfo>& _infos)
{
	ObjectDeleter<GetSymbolInfosResponse> response;
	status_t error = sendRequest(*fConnection, GetSymbolInfosRequest(image),
		response);
	if (error != B_OK)
		return error;

	// TODO: Move semantics or std::swap for BObjectList!
	_infos = response->infos;
	return B_OK;
}


status_t
RemoteDebuggerInterface::GetSymbolInfo(team_id team, image_id image,
	const char* name, int32 symbolType, SymbolInfo& _info)
{
	ObjectDeleter<GetSymbolInfoResponse> response;
	status_t error = sendRequest(*fConnection,
		GetSymbolInfoRequest(image, name, symbolType), response);
	if (error != B_OK)
		return error;

	_info = response->info;
	return B_OK;
}


status_t
RemoteDebuggerInterface::GetThreadInfo(thread_id thread, ThreadInfo& _info)
{
	ObjectDeleter<GetThreadInfoResponse> response;
	status_t error = sendRequest(*fConnection, GetThreadInfoRequest(thread),
		response);
	if (error != B_OK)
		return error;

	_info = response->info;
	return B_OK;
}


status_t
RemoteDebuggerInterface::GetCpuState(thread_id thread, CpuState*& _state)
{
	ObjectDeleter<GetCpuStateResponse> response;
	status_t error = sendRequest(*fConnection, GetCpuStateRequest(), response);
	if (error != B_OK)
		return error;

	_state = response->cpuState.Detach();
	return B_OK;
}


status_t
RemoteDebuggerInterface::SetCpuState(thread_id thread, const CpuState* state)
{
	return sendRequestStatusOnlyResponse(*fConnection,
		SetCpuStateRequest(thread, state));
}


status_t
RemoteDebuggerInterface::GetCpuFeatures(uint32& flags)
{
	return fArchitecture->GetCpuFeatures(flags);
}


status_t
RemoteDebuggerInterface::WriteCoreFile(const char* path)
{
	return sendRequestStatusOnlyResponse(*fConnection,
		WriteCoreFileRequest(path));
}


status_t
RemoteDebuggerInterface::GetMemoryProperties(target_addr_t address,
	uint32& _protection, uint32& _locking)
{
	ObjectDeleter<GetMemoryPropertiesResponse> response;
	status_t error = sendRequest(*fConnection,
		GetMemoryPropertiesRequest(address), response);
	if (error != B_OK)
		return error;

	_protection = response->protection;
	_locking = response->locking;
	return B_OK;
}


ssize_t
RemoteDebuggerInterface::ReadMemory(target_addr_t address, void* _buffer,
	size_t size)
{
	ObjectDeleter<ReadMemoryResponse> response;
	status_t error = sendRequest(*fConnection,
		ReadMemoryRequest(address, size), response);
	if (error != B_OK)
		return error;

	size_t actualSize = std::min(size, response->data.Size());
	memcpy(_buffer, response->data.Data(), actualSize);
	return ssize_t(actualSize);
}


ssize_t
RemoteDebuggerInterface::WriteMemory(target_addr_t address, void* buffer,
	size_t size)
{
	ObjectDeleter<WriteMemoryResponse> response;
	status_t error = sendRequest(*fConnection,
		WriteMemoryRequest(address, RawData(buffer, size, RawData::Reference)),
		response);
	if (error != B_OK)
		return error;

	size_t actualSize = std::min(size, response->bytesWritten);
	return ssize_t(actualSize);
}
