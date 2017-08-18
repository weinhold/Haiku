/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include <typeinfo>

#include "DebugEvent.h"
#include "debugger_interface/remote/RemoteDebugFactoryContext.h"
#include "debugger_interface/remote/RemoteDebugRequests.h"

#include "debugger_interface/remote/RemoteDataArchivingBasicTypes.h"
#include "debugger_interface/remote/RemoteDataArchivingCommon.h"


namespace {


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
		#define DEFINE_REQUEST_AND_RESPONSE_STRUCTS(...)	\
			REGISTER_REQUEST_INFOS(__VA_ARGS__)
		#include "debugger_interface/remote/RemoteDebugRequestDefs.h"
		#undef DEFINE_REQUEST_AND_RESPONSE_STRUCTS
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
		#define DEFINE_REQUEST_AND_RESPONSE_STRUCTS(...)	\
			REGISTER_RESPONSE_INFOS(__VA_ARGS__)
		#include "debugger_interface/remote/RemoteDebugRequestDefs.h"
		#undef DEFINE_REQUEST_AND_RESPONSE_STRUCTS
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


} // anonymous namespace
