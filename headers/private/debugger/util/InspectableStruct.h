/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef UTIL_INSPECTABLE_STRUCT_H
#define UTIL_INSPECTABLE_STRUCT_H


#include "MacroMagic.h"
#include "StructInspector.h"


#define DEFINE_INSPECTABLE_STRUCT(structName, baseType, ...)												\
	struct structName : public baseType {									\
		structName(DEFINE_INSPECTABLE_STRUCT_CONSTRUCTOR_ARGUMENTS(			\
				__VA_ARGS__))												\
			: baseType()													\
			DEFINE_INSPECTABLE_STRUCT_CONSTRUCTOR_INITIALIZER_LIST(			\
				__VA_ARGS__)												\
		{}																	\
		~structName() {}													\
		DEFINE_INSPECTABLE_STRUCT_STRUCT_ATTRIBUTES(__VA_ARGS__)			\
		DEFINE_INSPECTABLE_STRUCT_INSPECTOR_METHOD(baseType, __VA_ARGS__)	\
		DEFINE_CONST_INSPECTABLE_STRUCT_INSPECTOR_METHOD(baseType,			\
			__VA_ARGS__)													\
		static const char* StaticStructName()								\
			{ return #structName; }											\
		virtual const char* StructName() const								\
			{ return StaticStructName(); }									\
	};


#define DEFINE_INSPECTABLE_STRUCT_CONSTRUCTOR_ARGUMENTS(...)	\
	ITERATE2(DEFINE_INSPECTABLE_STRUCT_CONSTRUCTOR_ARGUMENT, COMMA, __VA_ARGS__)

#define DEFINE_INSPECTABLE_STRUCT_CONSTRUCTOR_ARGUMENT(x, y) x y = x()

#define DEFINE_INSPECTABLE_STRUCT_CONSTRUCTOR_INITIALIZER_LIST(...)	\
	ITERATE2(DEFINE_INSPECTABLE_STRUCT_CONSTRUCTOR_INITIALIZER_LIST_ELEMENT, \
		DEFINE_EMPTY, __VA_ARGS__)

#define DEFINE_INSPECTABLE_STRUCT_CONSTRUCTOR_INITIALIZER_LIST_ELEMENT(x, y) \
	, y(y)

#define DEFINE_INSPECTABLE_STRUCT_STRUCT_ATTRIBUTES(...)	\
	ITERATE2(DEFINE_INSPECTABLE_STRUCT_ATTRIBUTE, DEFINE_EMPTY, __VA_ARGS__)

#define DEFINE_INSPECTABLE_STRUCT_ATTRIBUTE(x, y) x y;

#define DEFINE_INSPECTABLE_STRUCT_INSPECTOR_METHOD(baseType, ...)	\
	virtual void AcceptStructInspector(StructInspector<baseType>& inspector) \
	{																		\
		DEFINE_INSPECTABLE_STRUCT_INSPECTOR_METHOD_CALLS(__VA_ARGS__)		\
	}

#define DEFINE_INSPECTABLE_STRUCT_INSPECTOR_METHOD_CALLS(...)	\
	ITERATE2(INSPECTOR_METHOD_CALL, DEFINE_EMPTY, __VA_ARGS__)

#define INSPECTOR_METHOD_CALL(x, y) inspector.Inspect(#y, this->y);

#define DEFINE_CONST_INSPECTABLE_STRUCT_INSPECTOR_METHOD(baseType, ...)		\
	virtual void AcceptStructInspector(										\
		ConstStructInspector<baseType>& inspector) const					\
	{																		\
		DEFINE_INSPECTABLE_STRUCT_INSPECTOR_METHOD_CALLS(__VA_ARGS__)		\
	}


#endif	// UTIL_INSPECTABLE_STRUCT_H
