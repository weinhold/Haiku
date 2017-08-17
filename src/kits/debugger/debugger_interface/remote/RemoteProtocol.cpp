/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/RemoteProtocol.h"

#include <map>
#include <string>
#include <typeinfo>

#include <Message.h>

#include <AutoDeleter.h>

#include "Architecture.h"
#include "DebugEvent.h"
#include "Register.h"
#include "debugger_interface/remote/RemoteDebugFactoryContext.h"
#include "debugger_interface/remote/RemoteDebugRequests.h"
#include "debugger_interface/remote/RemoteManagementFactoryContext.h"
#include "debugger_interface/remote/RemoteManagementEvents.h"
#include "debugger_interface/remote/RemoteManagementRequests.h"


namespace {


static const char* const kDataTypeNameFieldName = "data type name";


template<typename Enum, const Enum kMinValue, const Enum kMaxValue>
struct EnumWrapper {
	EnumWrapper(Enum value = kMinValue)
		:
		fValue(value)
	{
	}

	EnumWrapper& operator=(int32 value)
	{
		fValue = toEnum(value);
		return *this;
	}

	int32 toInt32() const
	{
		return (int32)fValue;
	}

	operator Enum() const
	{
		return fValue;
	}

private:
	static Enum toEnum(int32 value)
	{
		if (value < kMinValue || value > kMaxValue)
			throw status_t(B_BAD_VALUE);
		return Enum(value);
	}

private:
	Enum	fValue;
};


template<typename RemoteData>
struct RemoteDataFactory {
};


template<typename RemoteData>
struct RemoteDataFactoryBase;


template<typename RemoteData>
struct RemoteDataFactoryBase {
	RemoteDataFactoryBase()
	{
	}

	~RemoteDataFactoryBase()
	{
		for (typename TypeInfoMap::const_iterator it = fTypeInfos.begin();
				it != fTypeInfos.end(); ++it) {
			delete it->second;
		}
	}

	status_t CreateRequest(const char* name, RemoteData*& _request)
	{
		typename TypeInfoMap::const_iterator it = fTypeInfos.find(name);
		if (it == fTypeInfos.end())
			return B_NAME_NOT_FOUND;

		RemoteData* request = it->second->CreateInstance();
		if (request == NULL)
			return B_NO_MEMORY;

		_request = request;
		return B_OK;
	}

private:
	struct TypeInfoBase {
		virtual ~TypeInfoBase()
		{
		}

		virtual RemoteData* CreateInstance() = 0;
	};

	typedef std::map<std::string, TypeInfoBase*> TypeInfoMap;

	template<typename RequestType>
	struct TypeInfo : TypeInfoBase {
		virtual RemoteData* CreateInstance() {
			return new(std::nothrow) RequestType;
		}
	};

protected:
	template<typename RequestType>
	void RegisterInfo()
	{
		fTypeInfos[RequestType::StaticStructName()]
			= new TypeInfo<RequestType>();
	}

private:
	TypeInfoMap	fTypeInfos;
};


#define DEFINE_VALUE_ARCHIVER(type, archiveInvocation, unarchiveInvocation)	\
	template<typename Context>										\
	void archiveData(Context& context, BMessage& archive,			\
		const char* name, const type& value)						\
	{																\
		status_t error = archiveInvocation;							\
		if (error != B_OK)											\
			throw error;											\
	}																\
	template<typename Context>										\
	void unarchiveData(Context& context, const BMessage& archive,	\
		const char* name, int32 index, type& _value)				\
	{																\
		status_t error = unarchiveInvocation;						\
		if (error != B_OK)											\
			throw error;											\
	}

#define DEFINE_INTEGRAL_VALUE_ARCHIVER(type, archiveMethod, unarchiveMethod) \
	DEFINE_VALUE_ARCHIVER(type,							\
		archive.archiveMethod(name, value),				\
 		archive.unarchiveMethod(name, index, &_value))

#define DEFINE_STRUCT_VALUE_ARCHIVER(type, archiveMethod, unarchiveMethod) \
	DEFINE_VALUE_ARCHIVER(type,							\
		archive.archiveMethod(name, &value),			\
 		archive.unarchiveMethod(name, index, &_value))


template<typename Context, typename Value>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
		Value& value);


DEFINE_INTEGRAL_VALUE_ARCHIVER(bool, AddBool, FindBool)
DEFINE_INTEGRAL_VALUE_ARCHIVER(int32, AddInt32, FindInt32)
DEFINE_INTEGRAL_VALUE_ARCHIVER(uint32, AddUInt32, FindUInt32)
DEFINE_INTEGRAL_VALUE_ARCHIVER(uint64, AddUInt64, FindUInt64)
DEFINE_INTEGRAL_VALUE_ARCHIVER(BString, AddString, FindString)

DEFINE_STRUCT_VALUE_ARCHIVER(BMessage, AddMessage, FindMessage)

DEFINE_VALUE_ARCHIVER(BVariant,				\
	value.AddToMessage(archive, name),		\
	_value.SetFromMessage(archive, name))


template<typename Context>
void archiveData(Context& context, BMessage& archive, const char* name,
	const RawData& value)
{
	status_t error = archive.AddData(name, B_RAW_TYPE, value.Data(),
		value.Size(), false);
	if (error != B_OK)
		throw error;
}


template<typename Context>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
	int32 index, RawData& value)
{
	const void* data;
	ssize_t size;
	status_t error = archive.FindData(name, B_RAW_TYPE, index, &data, &size);
	if (error != B_OK)
		throw error;

	value.SetTo(data, (size_t)size, RawData::Clone);
}


template<typename Context>
void archiveData(Context& context, BMessage& archive, const char* name,
	const TeamInfo& value)
{
	BMessage valueArchive;
	archiveData(context, valueArchive, "teamId", value.TeamID());
	archiveData(context, valueArchive, "arguments", value.Arguments());
	archiveData(context, archive, name, valueArchive);
}


template<typename Context>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
	int32 index, TeamInfo& value)
{
	BMessage valueArchive;
	unarchiveData(context, archive, name, index, valueArchive);

	int32 teamId;
	BString arguments;
	unarchiveData(context, valueArchive, "teamId", teamId);
	unarchiveData(context, valueArchive, "arguments", arguments);
	value.SetTo(teamId, arguments);
}


template<typename Context>
void archiveData(Context& context, BMessage& archive, const char* name,
	const ThreadInfo& value)
{
	BMessage valueArchive;
	archiveData(context, valueArchive, "teamId", value.TeamID());
	archiveData(context, valueArchive, "threadId", value.ThreadID());
	archiveData(context, valueArchive, "name", value.Name());
	archiveData(context, archive, name, valueArchive);
}


template<typename Context>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
	int32 index, ThreadInfo& value)
{
	BMessage valueArchive;
	unarchiveData(context, archive, name, index, valueArchive);

	int32 teamId;
	int32 threadId;
	BString threadName;
	unarchiveData(context, valueArchive, "teamId", teamId);
	unarchiveData(context, valueArchive, "threadId", threadId);
	unarchiveData(context, valueArchive, "name", threadName);
	value.SetTo(teamId, threadId, threadName);
}


template<typename Context>
void archiveData(Context& context, BMessage& archive, const char* name,
	const ImageInfo& value)
{
	BMessage valueArchive;
	archiveData(context, valueArchive, "teamId", value.TeamID());
	archiveData(context, valueArchive, "imageId", value.ImageID());
	archiveData(context, valueArchive, "name", value.Name());
	archiveData(context, valueArchive, "type", (int32)value.Type());
	archiveData(context, valueArchive, "textBase", value.TextBase());
	archiveData(context, valueArchive, "textSize", value.TextSize());
	archiveData(context, valueArchive, "dataBase", value.DataBase());
	archiveData(context, valueArchive, "dataSize", value.DataSize());
	archiveData(context, archive, name, valueArchive);
}


template<typename Context>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
	int32 index, ImageInfo& value)
{
	BMessage valueArchive;
	unarchiveData(context, archive, name, index, valueArchive);

	int32 teamId;
	int32 imageId;
	BString imageName;
	int32 type;
	uint64 textBase;
	uint64 textSize;
	uint64 dataBase;
	uint64 dataSize;
	unarchiveData(context, valueArchive, "teamId", teamId);
	unarchiveData(context, valueArchive, "imageId", imageId);
	unarchiveData(context, valueArchive, "name", imageName);
	unarchiveData(context, valueArchive, "type", type);
	unarchiveData(context, valueArchive, "textBase", textBase);
	unarchiveData(context, valueArchive, "textSize", textSize);
	unarchiveData(context, valueArchive, "dataBase", dataBase);
	unarchiveData(context, valueArchive, "dataSize", dataSize);

	if (type < B_APP_IMAGE || type > B_SYSTEM_IMAGE)
		throw status_t(B_BAD_VALUE);

	value.SetTo(teamId, imageId, imageName, (image_type)type, textBase,
		textSize, dataBase, dataSize);
}


template<typename Context>
void archiveData(Context& context, BMessage& archive, const char* name,
	const SymbolInfo& value)
{
	BMessage valueArchive;
	archiveData(context, valueArchive, "address", value.Address());
	archiveData(context, valueArchive, "size", value.Size());
	archiveData(context, valueArchive, "type", value.Type());
	archiveData(context, valueArchive, "name", value.Name());
	archiveData(context, archive, name, valueArchive);
}


template<typename Context>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
	int32 index, SymbolInfo& value)
{
	BMessage valueArchive;
	unarchiveData(context, archive, name, index, valueArchive);

	uint64 address;
	uint64 size;
	uint32 type;
	BString symbolName;
	unarchiveData(context, valueArchive, "address", address);
	unarchiveData(context, valueArchive, "Size", size);
	unarchiveData(context, valueArchive, "type", type);
	unarchiveData(context, valueArchive, "name", symbolName);

	value.SetTo(address, size, type, symbolName);
}


template<typename Context>
void archiveData(Context& context, BMessage& archive, const char* name,
	const Reference<CpuState>& value)
{
	BMessage valueArchive;

	const Architecture* architecture = context.GetArchitecture();
	const Register* registers = architecture->Registers();
	int32 registerCount = architecture->CountRegisters();
	for (int32 i = 0; i < registerCount; i++) {
		BVariant registerValue;
		if (!value->GetRegisterValue(registers + i, registerValue))
			throw status_t(B_BAD_DATA);
		archiveData(context, valueArchive, registers[i].Name(), registerValue);
	}

	archiveData(context, archive, name, valueArchive);
}


template<typename Context>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
	Reference<CpuState>& value)
{
	BMessage valueArchive;
	unarchiveData(context, archive, name, valueArchive);

	const Architecture* architecture = context.GetArchitecture();
	CpuState* cpuState;
	status_t error = architecture->CreateCpuState(cpuState);
	if (error != B_OK)
		throw error;
	Reference<CpuState> cpuStateReference(cpuState, true);

	const Register* registers = architecture->Registers();
	int32 registerCount = architecture->CountRegisters();
	for (int32 i = 0; i < registerCount; i++) {
		BVariant registerValue;
		unarchiveData(context, valueArchive, registers[i].Name(),
			registerValue);

		if (cpuState->SetRegisterValue(registers + i, registerValue))
			throw status_t(B_BAD_VALUE);
	}

	value.SetTo(cpuStateReference.Detach(), true);
}


template<typename Context>
void archiveData(Context& context, BMessage& archive, const char* name,
	const BReference<CpuState>& value)
{
	Reference<CpuState> referenceValue(value.Get(), false);
	archiveData(context, archive, name, referenceValue);
}


template<typename Context>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
	BReference<CpuState>& value)
{
	Reference<CpuState> referenceValue;
	unarchiveData(context, archive, name, referenceValue);
	value.SetTo(referenceValue.Detach(), true);
}


template<typename Context, typename Value>
void archiveData(Context& context, BMessage& archive, const char* name,
	const BObjectList<Value>& list)
{
	for (int32 i = 0; const Value* value = list.ItemAt(i); i++)
		archiveData(context, archive, name, *value);
}


template<typename Context, typename Value>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
	int32 index, BObjectList<Value>& list)
{
	list.MakeEmpty();

	type_code type;
	int32 count;
	status_t error = archive.GetInfo(name, &type, &count);
	if (error != B_OK)
		return;

	for (int32 i = 0; i < count; i++) {
		Value* value = new Value;
		ObjectDeleter<Value> valueDeleter;
		unarchiveData(context, archive, name, index, *value);

		if (!list.AddItem(value))
			throw B_NO_MEMORY;
		valueDeleter.Detach();
	}
}


template<typename Context, typename Enum, const Enum kMinValue,
	const Enum kMaxValue>
void archiveData(Context& context, BMessage& archive, const char* name,
	EnumWrapper<Enum, kMinValue, kMaxValue> value)
{
	archiveData(context, archive, name, value.toInt32());
}


template<typename Context, typename Enum, const Enum kMinValue,
	const Enum kMaxValue>
void unarchiveData(Context& context, const BMessage& archive, const char* name,
	int32 index, EnumWrapper<Enum, kMinValue, kMaxValue>& _value)
{
	int32 value;
	unarchiveData(context, archive, name, index, value);
	_value = value;
}


template<typename Context, typename Value>
void
unarchiveData(Context& context, const BMessage& archive, const char* name,
	Value& value)
{
	unarchiveData(context, archive, name, 0, value);
}


template<typename DerivedType, typename Context, typename Value>
struct ArchivingStructMemberInspector : virtual StructMemberInspector<Value> {
	virtual void Inspect(const char* name, Value& value)
	{
		DerivedType* self = static_cast<DerivedType*>(this);
		archiveData(self->GetContext(), self->GetArchive(), name, value);
	}
};


template<typename RemoteData, typename Context>
struct ArchivingRemoteDataInspector : ConstStructInspector<RemoteData> {
	ArchivingRemoteDataInspector(const Context& context,
		BMessage& archive)
		:
		fContext(context),
		fArchive(archive)
	{
	}

	const Context& GetContext() const
	{
		return fContext;
	}

	BMessage& GetArchive() const
	{
		return fArchive;
	}

private:
	const Context&	fContext;
	BMessage&		fArchive;
};


template<typename DerivedType, typename Context, typename Value>
struct UnarchivingStructMemberInspector : virtual StructMemberInspector<Value> {
	virtual void Inspect(const char* name, Value& value)
	{
		DerivedType* self = static_cast<DerivedType*>(this);
		unarchiveData(self->GetContext(), self->GetArchive(), name, value);
	}
};


template<typename RemoteData, typename Context>
struct UnarchivingRemoteDataInspector : StructInspector<RemoteData> {
	UnarchivingRemoteDataInspector(const Context& context,
		const BMessage& archive)
		:
		fContext(context),
		fArchive(archive)
	{
	}

	const Context& GetContext() const
	{
		return fContext;
	}

	const BMessage& GetArchive() const
	{
		return fArchive;
	}

private:
	const Context&	fContext;
	const BMessage&	fArchive;
};


// #pragma mark - RemoteDebugRequest archiving support


struct ArchivingRemoteDebugRequestInspector;

template<typename Value>
struct ArchivingRemoteDebugRequestMemberInspector
	: ArchivingStructMemberInspector<ArchivingRemoteDebugRequestInspector,
		RemoteDebugFactoryContext, const Value> {
};


struct ArchivingRemoteDebugRequestInspector
	:
	ArchivingRemoteDataInspector<RemoteDebugRequest,
		RemoteDebugFactoryContext>,
	ArchivingRemoteDebugRequestMemberInspector<bool>,
	ArchivingRemoteDebugRequestMemberInspector<int32>,
	ArchivingRemoteDebugRequestMemberInspector<uint32>,
	ArchivingRemoteDebugRequestMemberInspector<uint64>,
	ArchivingRemoteDebugRequestMemberInspector<BString>,
	ArchivingRemoteDebugRequestMemberInspector<RawData>,
	ArchivingRemoteDebugRequestMemberInspector<TeamInfo>,
	ArchivingRemoteDebugRequestMemberInspector<ThreadInfo>,
	ArchivingRemoteDebugRequestMemberInspector<SymbolInfo>,
	ArchivingRemoteDebugRequestMemberInspector<BObjectList<ThreadInfo> >,
	ArchivingRemoteDebugRequestMemberInspector<BObjectList<ImageInfo> >,
	ArchivingRemoteDebugRequestMemberInspector<BObjectList<SymbolInfo> >,
	ArchivingRemoteDebugRequestMemberInspector<Reference<CpuState> >
{
	ArchivingRemoteDebugRequestInspector(
		const RemoteDebugFactoryContext& context,
		BMessage& archive)
		:
		ArchivingRemoteDataInspector(context, archive)
	{
	}
};


struct UnarchivingRemoteDebugRequestInspector;

template<typename Value>
struct UnarchivingRemoteDebugRequestMemberInspector
	: UnarchivingStructMemberInspector<UnarchivingRemoteDebugRequestInspector,
		RemoteDebugFactoryContext, Value> {
};


struct UnarchivingRemoteDebugRequestInspector
	:
	UnarchivingRemoteDataInspector<RemoteDebugRequest,
		RemoteDebugFactoryContext>,
	UnarchivingRemoteDebugRequestMemberInspector<bool>,
	UnarchivingRemoteDebugRequestMemberInspector<int32>,
	UnarchivingRemoteDebugRequestMemberInspector<uint32>,
	UnarchivingRemoteDebugRequestMemberInspector<uint64>,
	UnarchivingRemoteDebugRequestMemberInspector<BString>,
	UnarchivingRemoteDebugRequestMemberInspector<RawData>,
	UnarchivingRemoteDebugRequestMemberInspector<TeamInfo>,
	UnarchivingRemoteDebugRequestMemberInspector<ThreadInfo>,
	UnarchivingRemoteDebugRequestMemberInspector<SymbolInfo>,
	UnarchivingRemoteDebugRequestMemberInspector<BObjectList<ThreadInfo> >,
	UnarchivingRemoteDebugRequestMemberInspector<BObjectList<ImageInfo> >,
	UnarchivingRemoteDebugRequestMemberInspector<BObjectList<SymbolInfo> >,
	UnarchivingRemoteDebugRequestMemberInspector<Reference<CpuState> >
{
	UnarchivingRemoteDebugRequestInspector(
		const RemoteDebugFactoryContext& context,
		const BMessage& archive)
		:
		UnarchivingRemoteDataInspector(context, archive)
	{
	}
};


template<>
struct RemoteDataFactory<RemoteDebugRequest>
		: RemoteDataFactoryBase<RemoteDebugRequest> {
	RemoteDataFactory()
	{
		RegisterInfo<CloseRequest>();
		RegisterInfo<SetTeamDebuggingFlagsRequest>();
		RegisterInfo<ContinueThreadRequest>();
		RegisterInfo<StopThreadRequest>();
		RegisterInfo<SingleStepThreadRequest>();
		RegisterInfo<InstallBreakpointRequest>();
		RegisterInfo<UninstallBreakpointRequest>();
		RegisterInfo<InstallWatchpointRequest>();
		RegisterInfo<UninstallWatchpointRequest>();
		RegisterInfo<GetTeamInfoRequest>();
		RegisterInfo<GetThreadInfosRequest>();
		RegisterInfo<GetImageInfosRequest>();
		RegisterInfo<GetSymbolInfosRequest>();
		RegisterInfo<GetSymbolInfoRequest>();
		RegisterInfo<GetThreadInfoRequest>();
		RegisterInfo<GetCpuStateRequest>();
		RegisterInfo<SetCpuStateRequest>();
		RegisterInfo<GetCpuFeaturesRequest>();
		RegisterInfo<WriteCoreFileRequest>();
		RegisterInfo<GetMemoryPropertiesRequest>();
		RegisterInfo<ReadMemoryRequest>();
		RegisterInfo<WriteMemoryRequest>();
	}
};


// #pragma mark - RemoteDebugResponse archiving support


struct ArchivingRemoteDebugResponseInspector;

template<typename Value>
struct ArchivingRemoteDebugResponseMemberInspector
	: ArchivingStructMemberInspector<ArchivingRemoteDebugResponseInspector,
		RemoteDebugFactoryContext, const Value> {
};


struct ArchivingRemoteDebugResponseInspector
	:
	ArchivingRemoteDataInspector<RemoteDebugResponse,
		RemoteDebugFactoryContext>,
	ArchivingRemoteDebugResponseMemberInspector<bool>,
	ArchivingRemoteDebugResponseMemberInspector<int32>,
	ArchivingRemoteDebugResponseMemberInspector<uint32>,
	ArchivingRemoteDebugResponseMemberInspector<uint64>,
	ArchivingRemoteDebugResponseMemberInspector<BString>,
	ArchivingRemoteDebugResponseMemberInspector<RawData>,
	ArchivingRemoteDebugResponseMemberInspector<TeamInfo>,
	ArchivingRemoteDebugResponseMemberInspector<ThreadInfo>,
	ArchivingRemoteDebugResponseMemberInspector<SymbolInfo>,
	ArchivingRemoteDebugResponseMemberInspector<BObjectList<ThreadInfo> >,
	ArchivingRemoteDebugResponseMemberInspector<BObjectList<ImageInfo> >,
	ArchivingRemoteDebugResponseMemberInspector<BObjectList<SymbolInfo> >,
	ArchivingRemoteDebugResponseMemberInspector<Reference<CpuState> >
{
	ArchivingRemoteDebugResponseInspector(
		const RemoteDebugFactoryContext& context,
		BMessage& archive)
		:
		ArchivingRemoteDataInspector(context, archive)
	{
	}
};


struct UnarchivingRemoteDebugResponseInspector;

template<typename Value>
struct UnarchivingRemoteDebugResponseMemberInspector
	: UnarchivingStructMemberInspector<UnarchivingRemoteDebugResponseInspector,
		RemoteDebugFactoryContext, Value> {
};


struct UnarchivingRemoteDebugResponseInspector
	:
	UnarchivingRemoteDataInspector<RemoteDebugResponse,
		RemoteDebugFactoryContext>,
	UnarchivingRemoteDebugResponseMemberInspector<bool>,
	UnarchivingRemoteDebugResponseMemberInspector<int32>,
	UnarchivingRemoteDebugResponseMemberInspector<uint32>,
	UnarchivingRemoteDebugResponseMemberInspector<uint64>,
	UnarchivingRemoteDebugResponseMemberInspector<BString>,
	UnarchivingRemoteDebugResponseMemberInspector<RawData>,
	UnarchivingRemoteDebugResponseMemberInspector<TeamInfo>,
	UnarchivingRemoteDebugResponseMemberInspector<ThreadInfo>,
	UnarchivingRemoteDebugResponseMemberInspector<SymbolInfo>,
	UnarchivingRemoteDebugResponseMemberInspector<BObjectList<ThreadInfo> >,
	UnarchivingRemoteDebugResponseMemberInspector<BObjectList<ImageInfo> >,
	UnarchivingRemoteDebugResponseMemberInspector<BObjectList<SymbolInfo> >,
	UnarchivingRemoteDebugResponseMemberInspector<Reference<CpuState> >
{
	UnarchivingRemoteDebugResponseInspector(
		const RemoteDebugFactoryContext& context,
		const BMessage& archive)
		:
		UnarchivingRemoteDataInspector(context, archive)
	{
	}
};


template<>
struct RemoteDataFactory<RemoteDebugResponse>
		: RemoteDataFactoryBase<RemoteDebugResponse> {
	RemoteDataFactory()
	{
		RegisterInfo<CloseResponse>();
		RegisterInfo<SetTeamDebuggingFlagsResponse>();
		RegisterInfo<ContinueThreadResponse>();
		RegisterInfo<StopThreadResponse>();
		RegisterInfo<SingleStepThreadResponse>();
		RegisterInfo<InstallBreakpointResponse>();
		RegisterInfo<UninstallBreakpointResponse>();
		RegisterInfo<InstallWatchpointResponse>();
		RegisterInfo<UninstallWatchpointResponse>();
		RegisterInfo<GetTeamInfoResponse>();
		RegisterInfo<GetThreadInfosResponse>();
		RegisterInfo<GetImageInfosResponse>();
		RegisterInfo<GetSymbolInfosResponse>();
		RegisterInfo<GetSymbolInfoResponse>();
		RegisterInfo<GetThreadInfoResponse>();
		RegisterInfo<GetCpuStateResponse>();
		RegisterInfo<SetCpuStateResponse>();
		RegisterInfo<GetCpuFeaturesResponse>();
		RegisterInfo<WriteCoreFileResponse>();
		RegisterInfo<GetMemoryPropertiesResponse>();
		RegisterInfo<ReadMemoryResponse>();
		RegisterInfo<WriteMemoryResponse>();
	}
};


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
	ArchivingRemoteManagementRequestMemberInspector<uint32>
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
	UnarchivingRemoteManagementRequestMemberInspector<uint32>
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
		RegisterInfo<HelloRequest>();
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
	ArchivingRemoteManagementResponseMemberInspector<uint32>
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
	UnarchivingRemoteManagementResponseMemberInspector<uint32>
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
		RegisterInfo<HelloResponse>();
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


// #pragma mark - DebugEvent archiving support


struct DebugEventArchivingContext {
	DebugEventArchivingContext(const Architecture* architecture)
		:
		fArchitecture(architecture)
	{
	}

	const Architecture* GetArchitecture() const
	{
		return fArchitecture;
	}

private:
	const Architecture*	fArchitecture;
};


struct DebugEventArchivingHandler {
	virtual ~DebugEventArchivingHandler()
	{
	}

	virtual const char* EventName() const = 0;

	virtual void ArchiveEvent(DebugEventArchivingContext& context,
		BMessage& archive, const DebugEvent& event) = 0;

	virtual DebugEvent* UnarchiveEvent(DebugEventArchivingContext& context,
		const BMessage& archive) = 0;
};


template<typename Event>
struct DebugEventArchivingHandlerBase : DebugEventArchivingHandler {
};


#define DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(eventName, ...)			\
template<>																	\
	struct DebugEventArchivingHandlerBase<eventName ## Event>				\
			: DebugEventArchivingHandler {									\
		static const char* StaticEventName()								\
			{ return #eventName; }											\
		virtual const char* EventName() const								\
			{ return StaticEventName(); }									\
		virtual void ArchiveEvent(DebugEventArchivingContext& context,		\
			BMessage& archive, const DebugEvent& _event)					\
		{																	\
			const eventName ## Event& event									\
				= dynamic_cast<const eventName ## Event&>(_event);			\
			DEFINE_HANDLER_ARCHIVING_CALLS(__VA_ARGS__)						\
		}																	\
		virtual DebugEvent* UnarchiveEvent(									\
			DebugEventArchivingContext& context, const BMessage& archive)	\
		{																	\
			DEFINE_HANDLER_UNARCHIVING_CALLS(__VA_ARGS__)					\
			return new eventName ## Event(									\
				DEFINE_HANDLER_UNARCHIVING_CREATE_EVENT_ARGS(__VA_ARGS__));	\
		}																	\
	};

#define DEFINE_HANDLER_ARCHIVING_CALLS(...)	\
	ITERATE3(DEFINE_HANDLER_ARCHIVING_CALL, DEFINE_EMPTY, __VA_ARGS__)
#define DEFINE_HANDLER_ARCHIVING_CALL(type, name, method) \
	archiveData(context, archive, #name, type(event.method()));

#define DEFINE_HANDLER_UNARCHIVING_CALLS(...)	\
	ITERATE3(DEFINE_HANDLER_UNARCHIVING_CALL, DEFINE_EMPTY, __VA_ARGS__)
#define DEFINE_HANDLER_UNARCHIVING_CALL(type, name, method) \
	type name;												\
	unarchiveData(context, archive, #name, name);

#define DEFINE_HANDLER_UNARCHIVING_CREATE_EVENT_ARGS(...)	\
	ITERATE3(DEFINE_HANDLER_UNARCHIVING_CREATE_EVENT_ARG, COMMA, __VA_ARGS__)
#define DEFINE_HANDLER_UNARCHIVING_CREATE_EVENT_ARG(type, name, method) name


typedef EnumWrapper<debug_exception_type, B_NON_MASKABLE_INTERRUPT,
	B_FLOATING_POINT_EXCEPTION> DebugExceptionType;


DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	ThreadDebugged,

	team_id,		teamId,		Team,
	thread_id,		threadId,	Thread
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	DebuggerCall,

	team_id,		teamId,		Team,
	thread_id,		threadId,	Thread,
	target_addr_t,	message,	Message
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	BreakpointHit,

	team_id,				teamId,		Team,
	thread_id,				threadId,	Thread,
	BReference<CpuState>,	cpuState,	GetCpuState
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	WatchpointHit,

	team_id,				teamId,		Team,
	thread_id,				threadId,	Thread,
	BReference<CpuState>,	cpuState,	GetCpuState
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	SingleStep,

	team_id,				teamId,		Team,
	thread_id,				threadId,	Thread,
	BReference<CpuState>,	cpuState,	GetCpuState
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	ExceptionOccurred,

	team_id,				teamId,			Team,
	thread_id,				threadId,		Thread,
	DebugExceptionType,		exception,		Exception,
	BString,				description,	Description
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	TeamDeleted,

	team_id,				teamId,			Team,
	thread_id,				threadId,		Thread
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	TeamExec,

	team_id,				teamId,			Team,
	thread_id,				threadId,		Thread
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	ThreadCreated,

	team_id,				teamId,			Team,
	thread_id,				threadId,		Thread,
	thread_id,				newThreadId,	NewThread
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	ThreadRenamed,

	team_id,				teamId,				Team,
	thread_id,				threadId,			Thread,
	thread_id,				renamedThreadId,	RenamedThread,
	BString,				newName,			NewName
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	ThreadPriorityChanged,

	team_id,				teamId,				Team,
	thread_id,				threadId,			Thread,
	thread_id,				changedThreadId,	ChangedThread,
	int32,					newPriority,		NewPriority
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	ThreadDeleted,

	team_id,				teamId,				Team,
	thread_id,				threadId,			Thread
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	ImageCreated,

	team_id,				teamId,				Team,
	thread_id,				threadId,			Thread,
	ImageInfo,				info,				GetImageInfo
)

DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
	ImageDeleted,

	team_id,				teamId,				Team,
	thread_id,				threadId,			Thread,
	ImageInfo,				info,				GetImageInfo
)

// TODO:
// DEFINE_DEBUG_EVENT_ARCHIVING_HANDLER(
// 	SignalReceived,
//
// 	team_id,				teamId,				Team,
// 	thread_id,				threadId,			Thread,
// 	SignalInfo,				info,				GetSignalInfo
// )

// Note: Unmapped events:
// * PostSyscallEvent (too slow over a remote connection)
// * HandedOverEvent (not needed)


struct DebugEventArchivingManager {
	DebugEventArchivingManager()
	{
		_RegisterInfo<ThreadDebuggedEvent>();
		_RegisterInfo<DebuggerCallEvent>();
		_RegisterInfo<BreakpointHitEvent>();
		_RegisterInfo<WatchpointHitEvent>();
		_RegisterInfo<SingleStepEvent>();
		_RegisterInfo<ExceptionOccurredEvent>();
		_RegisterInfo<TeamDeletedEvent>();
		_RegisterInfo<TeamExecEvent>();
		_RegisterInfo<ThreadCreatedEvent>();
		_RegisterInfo<ThreadRenamedEvent>();
		_RegisterInfo<ThreadPriorityChangedEvent>();
		_RegisterInfo<ThreadDeletedEvent>();
		_RegisterInfo<ImageCreatedEvent>();
		_RegisterInfo<ImageDeletedEvent>();
// TODO:
// 		_RegisterInfo<SignalReceivedEvent>();
	}

	~DebugEventArchivingManager()
	{
		for (typename HandlerMap::const_iterator it = fHandlerByName.begin();
				it != fHandlerByName.end(); ++it) {
			delete it->second;
		}
	}

	static const DebugEventArchivingManager& Default()
	{
		static const DebugEventArchivingManager manager;
		return manager;
	}

	void ArchiveEvent(const Architecture* architecture, BMessage& archive,
		const DebugEvent& event) const
	{
		HandlerMap::const_iterator it
			= fHandlerByType.find(typeid(event).name());
		if (it == fHandlerByType.end())
			throw status_t(B_NAME_NOT_FOUND);

		DebugEventArchivingHandler* handler = it->second;

		status_t error = archive.AddString(kDataTypeNameFieldName,
			handler->EventName());
		if (error != B_OK)
			throw error;

		DebugEventArchivingContext context(architecture);
		handler->ArchiveEvent(context, archive, event);
	}

	DebugEvent* UnarchiveEvent(const Architecture* architecture,
		const BMessage& archive) const
	{
		const char* eventName;
		status_t error = archive.FindString(kDataTypeNameFieldName, &eventName);
		if (error != B_OK)
			throw(error);

		typename HandlerMap::const_iterator it = fHandlerByName.find(eventName);
		if (it == fHandlerByName.end())
			throw status_t(B_NAME_NOT_FOUND);

		DebugEventArchivingContext context(architecture);
		return it->second->UnarchiveEvent(context, archive);
	}

private:
	typedef std::map<std::string, DebugEventArchivingHandler*> HandlerMap;

private:
	template<typename Event>
	void _RegisterInfo()
	{
		typedef DebugEventArchivingHandlerBase<Event> Handler;
		Handler* handler = new Handler();
		fHandlerByName[Handler::StaticEventName()] = handler;
		fHandlerByType[typeid(Event).name()] = handler;
	}

private:
	HandlerMap	fHandlerByName;
	HandlerMap	fHandlerByType;
};


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
		ObjectDeleter<RemoteData> dataDeleter;

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
