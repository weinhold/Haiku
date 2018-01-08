/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <map>
#include <string>

#include "InspectableStruct.h"

#include "debugger_interface/remote/RemoteDataArchivingBasicTypes.h"


namespace {


static const char* const kDataTypeNameFieldName = "data type name";


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

#define REGISTER_REQUEST_INFOS(...) \
	ITERATE3(REGISTER_REQUEST_INFO, DEFINE_EMPTY, __VA_ARGS__)

#define REGISTER_REQUEST_INFO(name, requestFields,	responseFields)	\
	RegisterInfo<name ## Request>();

#define REGISTER_RESPONSE_INFOS(...) \
	ITERATE3(REGISTER_RESPONSE_INFO, DEFINE_EMPTY, __VA_ARGS__)

#define REGISTER_RESPONSE_INFO(name, requestFields,	responseFields)	\
	RegisterInfo<name ## Response>();

#define REGISTER_EVENT_INFOS(...) \
	ITERATE2_OUTER(REGISTER_EVENT_INFO, DEFINE_EMPTY, __VA_ARGS__)

#define REGISTER_EVENT_INFO(name, eventFields)	\
	RegisterInfo<name ## Event>();


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


} // anonymous namespace
