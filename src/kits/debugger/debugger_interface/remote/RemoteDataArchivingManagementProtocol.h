/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/RemoteManagementFactoryContext.h"
#include "debugger_interface/remote/RemoteManagementEvents.h"
#include "debugger_interface/remote/RemoteManagementRequests.h"

#include "debugger_interface/remote/RemoteDataArchivingCommon.h"


namespace {


// #pragma mark - RemoteManagementRequest archiving support


struct ArchivingRemoteManagementRequestInspector;

template<typename Value>
struct ArchivingRemoteManagementRequestMemberInspector
	: ArchivingStructMemberInspector<ArchivingRemoteManagementRequestInspector,
		RemoteManagementFactoryContext, const Value> {
};


struct ArchivingRemoteManagementRequestInspector
	:
	ArchivingRemoteDataInspector<RemoteManagementRequest,
		RemoteManagementFactoryContext>,
	ArchivingRemoteManagementRequestMemberInspector<int32>,
	ArchivingRemoteManagementRequestMemberInspector<uint32>,
	ArchivingRemoteManagementRequestMemberInspector<uint64>
{
	ArchivingRemoteManagementRequestInspector(
		const RemoteManagementFactoryContext& context,
		BMessage& archive)
		:
		ArchivingRemoteDataInspector(context, archive)
	{
	}
};


struct UnarchivingRemoteManagementRequestInspector;

template<typename Value>
struct UnarchivingRemoteManagementRequestMemberInspector
	: UnarchivingStructMemberInspector<
		UnarchivingRemoteManagementRequestInspector,
		RemoteManagementFactoryContext, Value> {
};


struct UnarchivingRemoteManagementRequestInspector
	:
	UnarchivingRemoteDataInspector<RemoteManagementRequest,
		RemoteManagementFactoryContext>,
	UnarchivingRemoteManagementRequestMemberInspector<int32>,
	UnarchivingRemoteManagementRequestMemberInspector<uint32>,
	UnarchivingRemoteManagementRequestMemberInspector<uint64>
{
	UnarchivingRemoteManagementRequestInspector(
		const RemoteManagementFactoryContext& context,
		const BMessage& archive)
		:
		UnarchivingRemoteDataInspector(context, archive)
	{
	}
};


template<>
struct RemoteDataFactory<RemoteManagementRequest>
		: RemoteDataFactoryBase<RemoteManagementRequest> {
	RemoteDataFactory()
	{
		#define DEFINE_REQUEST_AND_RESPONSE_STRUCTS(...)	\
			REGISTER_REQUEST_INFOS(__VA_ARGS__)
		#include "debugger_interface/remote/RemoteManagementRequestDefs.h"
		#undef DEFINE_REQUEST_AND_RESPONSE_STRUCTS
	}
};


// #pragma mark - RemoteManagementResponse archiving support


struct ArchivingRemoteManagementResponseInspector;

template<typename Value>
struct ArchivingRemoteManagementResponseMemberInspector
	: ArchivingStructMemberInspector<ArchivingRemoteManagementResponseInspector,
		RemoteManagementFactoryContext, const Value> {
};


struct ArchivingRemoteManagementResponseInspector
	:
	ArchivingRemoteDataInspector<RemoteManagementResponse,
		RemoteManagementFactoryContext>,
	ArchivingRemoteManagementResponseMemberInspector<int32>,
	ArchivingRemoteManagementResponseMemberInspector<uint32>,
	ArchivingRemoteManagementResponseMemberInspector<uint64>
{
	ArchivingRemoteManagementResponseInspector(
		const RemoteManagementFactoryContext& context,
		BMessage& archive)
		:
		ArchivingRemoteDataInspector(context, archive)
	{
	}
};


struct UnarchivingRemoteManagementResponseInspector;

template<typename Value>
struct UnarchivingRemoteManagementResponseMemberInspector
	: UnarchivingStructMemberInspector<
		UnarchivingRemoteManagementResponseInspector,
		RemoteManagementFactoryContext, Value> {
};


struct UnarchivingRemoteManagementResponseInspector
	:
	UnarchivingRemoteDataInspector<RemoteManagementResponse,
		RemoteManagementFactoryContext>,
	UnarchivingRemoteManagementResponseMemberInspector<int32>,
	UnarchivingRemoteManagementResponseMemberInspector<uint32>,
	UnarchivingRemoteManagementResponseMemberInspector<uint64>
{
	UnarchivingRemoteManagementResponseInspector(
		const RemoteManagementFactoryContext& context,
		const BMessage& archive)
		:
		UnarchivingRemoteDataInspector(context, archive)
	{
	}
};


template<>
struct RemoteDataFactory<RemoteManagementResponse>
		: RemoteDataFactoryBase<RemoteManagementResponse> {
	RemoteDataFactory()
	{
		#define DEFINE_REQUEST_AND_RESPONSE_STRUCTS(...)	\
			REGISTER_RESPONSE_INFOS(__VA_ARGS__)
		#include "debugger_interface/remote/RemoteManagementRequestDefs.h"
		#undef DEFINE_REQUEST_AND_RESPONSE_STRUCTS
	}
};


// #pragma mark - RemoteManagementEvent archiving support


struct ArchivingRemoteManagementEventInspector;

template<typename Value>
struct ArchivingRemoteManagementEventMemberInspector
	: ArchivingStructMemberInspector<ArchivingRemoteManagementEventInspector,
		RemoteManagementFactoryContext, const Value> {
};


struct ArchivingRemoteManagementEventInspector
	:
	ArchivingRemoteDataInspector<RemoteManagementEvent,
		RemoteManagementFactoryContext>,
	ArchivingRemoteManagementEventMemberInspector<bool>
{
	ArchivingRemoteManagementEventInspector(
		const RemoteManagementFactoryContext& context,
		BMessage& archive)
		:
		ArchivingRemoteDataInspector(context, archive)
	{
	}
};


struct UnarchivingRemoteManagementEventInspector;

template<typename Value>
struct UnarchivingRemoteManagementEventMemberInspector
	: UnarchivingStructMemberInspector<
		UnarchivingRemoteManagementEventInspector,
		RemoteManagementFactoryContext, Value> {
};


struct UnarchivingRemoteManagementEventInspector
	:
	UnarchivingRemoteDataInspector<RemoteManagementEvent,
		RemoteManagementFactoryContext>,
	UnarchivingRemoteManagementEventMemberInspector<bool>
{
	UnarchivingRemoteManagementEventInspector(
		const RemoteManagementFactoryContext& context,
		const BMessage& archive)
		:
		UnarchivingRemoteDataInspector(context, archive)
	{
	}
};


template<>
struct RemoteDataFactory<RemoteManagementEvent>
		: RemoteDataFactoryBase<RemoteManagementEvent> {
	RemoteDataFactory()
	{
// 		RegisterInfo<HelloRequest>();
	}
};


} // anonymous namespace
