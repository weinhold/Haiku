/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <Message.h>
#include <ObjectList.h>

#include <AutoDeleter.h>

#include "Architecture.h"
#include "Register.h"

#include "CpuState.h"
#include "ImageInfo.h"
#include "ImageInfo.h"
#include "RawData.h"
#include "Reference.h"
#include "SymbolInfo.h"
#include "TeamInfo.h"
#include "ThreadInfo.h"


namespace {


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


} // anonymous namespace
