/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef UTIL_STRUCT_INSPECTOR_H
#define UTIL_STRUCT_INSPECTOR_H


template<typename Struct>
struct StructInspector {
	virtual						~StructInspector() {}
};


template<typename Struct>
struct ConstStructInspector {
	virtual						~ConstStructInspector() {}
};


template<typename Value>
struct StructMemberInspector {
	virtual						~StructMemberInspector() {}

	virtual	void				Inspect(const char* name, Value& value) = 0;
};


template<typename StructType>
struct StructInspectorBase {
	template<typename Value>
	inline void DoInspect(const char* name, Value& value)
	{
		typedef StructInspector<StructType> DerivedType;
		DerivedType* self = static_cast<DerivedType*>(this);
		static_cast<StructMemberInspector<Value>*>(self)->Inspect(name, value);
	}
};


template<typename StructType>
struct ConstStructInspectorBase {
	template<typename Value>
	inline void DoInspect(const char* name, const Value& value)
	{
		typedef ConstStructInspector<StructType> DerivedType;
		DerivedType* self = static_cast<DerivedType*>(this);
		static_cast<StructMemberInspector<const Value>*>(self)
			->Inspect(name, value);
	}
};


#endif	// UTIL_STRUCT_INSPECTOR_H
