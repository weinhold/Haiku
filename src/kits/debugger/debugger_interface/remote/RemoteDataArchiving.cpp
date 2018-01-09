/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/RemoteDataArchiving.h"

#include <AutoDeleter.h>

#include "debugger_interface/remote/RemoteDataArchivingDebugProtocol.h"
#include "debugger_interface/remote/RemoteDataArchivingManagementProtocol.h"


namespace {


// #pragma mark - Archiver classes


template<typename RemoteData>
struct Types {
};


template<>
struct Types<RemoteDebugRequest> {
	typedef ArchivingRemoteDebugRequestInspector ArchivingInspector;
	typedef UnarchivingRemoteDebugRequestInspector UnarchivingInspector;
};


template<>
struct Types<RemoteDebugResponse> {
	typedef ArchivingRemoteDebugResponseInspector ArchivingInspector;
	typedef UnarchivingRemoteDebugResponseInspector UnarchivingInspector;
};


template<>
struct Types<RemoteManagementRequest> {
	typedef ArchivingRemoteManagementRequestInspector ArchivingInspector;
	typedef UnarchivingRemoteManagementRequestInspector UnarchivingInspector;
};


template<>
struct Types<RemoteManagementResponse> {
	typedef ArchivingRemoteManagementResponseInspector ArchivingInspector;
	typedef UnarchivingRemoteManagementResponseInspector UnarchivingInspector;
};


template<>
struct Types<RemoteManagementEvent> {
	typedef ArchivingRemoteManagementEventInspector ArchivingInspector;
	typedef UnarchivingRemoteManagementEventInspector UnarchivingInspector;
};


template<typename RemoteData, typename Context>
struct Archiver {
	static status_t Archive(const Context& context,
		const RemoteData& data, BMessage& archive)
	{
		try {
			status_t error = archive.AddString(kDataTypeNameFieldName,
				data.StructName());
			if (error != B_OK)
				return error;

			typename Types<RemoteData>::ArchivingInspector inspector(
				context, archive);
			data.AcceptStructInspector(inspector);
			return B_OK;
		} catch (status_t error) {
			return error;
		}
	}


	static status_t Unarchive(const Context& context,
		const BMessage& archive, RemoteData*& _data)
	{
		// TODO: should follow the usual patterns instead
		static RemoteDataFactory<RemoteData> dataFactory;

		const char* dataName;
		status_t error = archive.FindString(kDataTypeNameFieldName, &dataName);
		if (error != B_OK)
			return error;

		RemoteData* data;
		error = dataFactory.CreateRequest(dataName, data);
		if (error != B_OK)
			return error;
		ObjectDeleter<RemoteData> dataDeleter(data);

		try {
			typename Types<RemoteData>::UnarchivingInspector inspector(
				context, archive);
			data->AcceptStructInspector(inspector);
		} catch (status_t unarchivingError) {
			return unarchivingError;
		} catch (std::bad_alloc&) {
			return B_NO_MEMORY;
		}

		_data = dataDeleter.Detach();
		return B_OK;
	}
};


template<>
struct Archiver<DebugEvent, RemoteDebugFactoryContext> {
	static status_t Archive(const RemoteDebugFactoryContext& context,
		const DebugEvent& event, BMessage& archive)
	{
		try {
			DebugEventArchivingManager::Default().ArchiveEvent(
				context.GetArchitecture(), archive, event);
			return B_OK;
		} catch (status_t archivingError) {
			return archivingError;
		} catch (std::bad_alloc&) {
			return B_NO_MEMORY;
		}
	}


	static status_t Unarchive(const RemoteDebugFactoryContext& context,
		const BMessage& archive, DebugEvent*& _event)
	{
		DebugEvent* event;
		try {
			event = DebugEventArchivingManager::Default().UnarchiveEvent(
				context.GetArchitecture(), archive);
		} catch (status_t unarchivingError) {
			return unarchivingError;
		} catch (std::bad_alloc&) {
			return B_NO_MEMORY;
		}

		_event = event;
		return B_OK;
	}
};


} // anonymous namespace


template<typename RemoteData, typename Context>
status_t
archiveRemoteData(const Context& context, const RemoteData& data,
	BMessage& archive)
{
	return Archiver<RemoteData, Context>::Archive(context, data, archive);
}


template<typename RemoteData, typename Context>
status_t
unarchiveRemoteData(const Context& context, const BMessage& archive,
	RemoteData*& _data)
{
	return Archiver<RemoteData, Context>::Unarchive(context, archive, _data);
}


// explicit template instantiations

// RemoteDebugRequest
template status_t archiveRemoteData<RemoteDebugRequest,
		RemoteDebugFactoryContext>(
	const RemoteDebugFactoryContext& context, const RemoteDebugRequest& data,
	BMessage& archive);
template status_t unarchiveRemoteData<RemoteDebugRequest,
		RemoteDebugFactoryContext>(
	const RemoteDebugFactoryContext& context, const BMessage& archive,
	RemoteDebugRequest*& _data);

// RemoteDebugResponse
template status_t archiveRemoteData<RemoteDebugResponse,
		RemoteDebugFactoryContext>(
	const RemoteDebugFactoryContext& context, const RemoteDebugResponse& data,
	BMessage& archive);
template status_t unarchiveRemoteData<RemoteDebugResponse,
		RemoteDebugFactoryContext>(
	const RemoteDebugFactoryContext& context, const BMessage& archive,
	RemoteDebugResponse*& _data);

// RemoteManagementRequest
template status_t archiveRemoteData<RemoteManagementRequest,
		RemoteManagementFactoryContext>(
	const RemoteManagementFactoryContext& context,
	const RemoteManagementRequest& data,
	BMessage& archive);
template status_t unarchiveRemoteData<RemoteManagementRequest,
		RemoteManagementFactoryContext>(
	const RemoteManagementFactoryContext& context, const BMessage& archive,
	RemoteManagementRequest*& _data);

// RemoteManagementResponse
template status_t archiveRemoteData<RemoteManagementResponse,
		RemoteManagementFactoryContext>(
	const RemoteManagementFactoryContext& context,
	const RemoteManagementResponse& data,
	BMessage& archive);
template status_t unarchiveRemoteData<RemoteManagementResponse,
		RemoteManagementFactoryContext>(
	const RemoteManagementFactoryContext& context, const BMessage& archive,
	RemoteManagementResponse*& _data);

// DebugEvent
template status_t archiveRemoteData<DebugEvent, RemoteDebugFactoryContext>(
	const RemoteDebugFactoryContext& context, const DebugEvent& data,
	BMessage& archive);
template status_t unarchiveRemoteData<DebugEvent, RemoteDebugFactoryContext>(
	const RemoteDebugFactoryContext& context, const BMessage& archive,
	DebugEvent*& _data);

// RemoteManagementEvent
template status_t archiveRemoteData<RemoteManagementEvent,
		RemoteManagementFactoryContext>(
	const RemoteManagementFactoryContext& context,
	const RemoteManagementEvent& data, BMessage& archive);
template status_t unarchiveRemoteData<RemoteManagementEvent,
		RemoteManagementFactoryContext>(
	const RemoteManagementFactoryContext& context, const BMessage& archive,
	RemoteManagementEvent*& _data);
