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


#endif	// UTIL_STRUCT_INSPECTOR_H
