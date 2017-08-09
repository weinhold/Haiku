/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>

#include "util/StructInspector.h"


class BString;
template<typename Value> class BObjectList;

class CpuState;
class ImageInfo;
struct RawData;
struct RemoteDebugRequest;
class SymbolInfo;
class TeamInfo;
class ThreadInfo;
template<typename Value> struct Reference;


template<>
struct StructInspector<RemoteDebugRequest> {
	virtual						~StructInspector();

	virtual	void				Inspect(const char* name, bool& value) = 0;
	virtual	void				Inspect(const char* name, int32& value) = 0;
	virtual	void				Inspect(const char* name, uint32& value) = 0;
	virtual	void				Inspect(const char* name, uint64& value) = 0;
	virtual	void				Inspect(const char* name, BString& value) = 0;
	virtual	void				Inspect(const char* name, RawData& value) = 0;
	virtual	void				Inspect(const char* name, TeamInfo& value) = 0;
	virtual	void				Inspect(const char* name,
									ThreadInfo& value) = 0;
	virtual	void				Inspect(const char* name,
									SymbolInfo& value) = 0;
	virtual	void				Inspect(const char* name,
									BObjectList<ThreadInfo>& value) = 0;
	virtual	void				Inspect(const char* name,
									BObjectList<ImageInfo>& value) = 0;
	virtual	void				Inspect(const char* name,
									BObjectList<SymbolInfo>& value) = 0;
	virtual	void				Inspect(const char* name,
									Reference<CpuState>& value) = 0;
};
typedef StructInspector<RemoteDebugRequest> RequestInspector;


template<>
struct ConstStructInspector<RemoteDebugRequest> {
	virtual						~ConstStructInspector();

	virtual	void				Inspect(const char* name, bool value) = 0;
	virtual	void				Inspect(const char* name, int32 value) = 0;
	virtual	void				Inspect(const char* name, uint32 value) = 0;
	virtual	void				Inspect(const char* name, uint64 value) = 0;
	virtual	void				Inspect(const char* name,
									const BString& value) = 0;
	virtual	void				Inspect(const char* name,
									const RawData& value) = 0;
	virtual	void				Inspect(const char* name,
									const ThreadInfo& value) = 0;
	virtual	void				Inspect(const char* name,
									const TeamInfo& value) = 0;
	virtual	void				Inspect(const char* name,
									const SymbolInfo& value) = 0;
	virtual	void				Inspect(const char* name,
									const BObjectList<ThreadInfo>& value) = 0;
	virtual	void				Inspect(const char* name,
									const BObjectList<ImageInfo>& value) = 0;
	virtual	void				Inspect(const char* name,
									const BObjectList<SymbolInfo>& value) = 0;
	virtual	void				Inspect(const char* name,
									const Reference<CpuState>& value) = 0;
};
typedef ConstStructInspector<RemoteDebugRequest> ConstRequestInspector;


struct RemoteDebugRequest {
	virtual						~RemoteDebugRequest();

	virtual	const char*			StructName() const = 0;

	virtual	void				AcceptStructInspector(
									RequestInspector& inspector) = 0;
	virtual	void				AcceptStructInspector(
									ConstRequestInspector& inspector) const = 0;
};


// maps the request type to its corresponding response type
template<typename Request>
struct RemoteResponse {
};
