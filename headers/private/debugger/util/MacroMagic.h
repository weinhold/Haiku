/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef UTIL_MACRO_MAGIC_H
#define UTIL_MACRO_MAGIC_H


#include "StructInspector.h"


//! Returns the number of macro arguments.
#define COUNT_MACRO_ARGS(...)	\
	COUNT_MACRO_ARGS_IMPL(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define COUNT_MACRO_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, \
	_12, N, ...) N

#define UNWRAP_MACRO_ARGS(...)	__VA_ARGS__

#define COMMA() ,
#define DEFINE_EMPTY(x)


/*!	Iterates through the 3rd to nth macro arguments applying \a macro to pairs
	of subsequent arguments. \a separator is the name of a macro that shall be
	evalutated between \a macro invocations. So
	\code ITERATE2(macro, separator, arg1, arg2,..., argn-1 argn) \endcode
	evaluates to
	\code macro(arg1, arg2) separator() macro(arg3, arg4) separator()  ...
		macro(argn-1, argn) \endcode .
	The number of the arguments must be even.
 */
#define ITERATE2(macro, separator, ...) \
	ITERATE2_N(macro, separator, COUNT_MACRO_ARGS(__VA_ARGS__), \
		__VA_ARGS__)

#define ITERATE2_N(macro, separator, count, ...) \
	ITERATE2_N_IMPL(macro, separator, count, __VA_ARGS__)

#define ITERATE2_N_IMPL(macro, separator, count, ...) \
 	ITERATE2_N_IT##count(macro, separator, __VA_ARGS__)

#define ITERATE2_N_IT1(macro, separator, dummy)
#define ITERATE2_N_IT2(macro, separator, a, b) \
	macro(a, b)
#define ITERATE2_N_IT4(macro, separator, a, b,...)	\
	ITERATE2_N_IT2(macro, separator, a, b)			\
	separator()										\
	ITERATE2_N_IT2(macro, separator, __VA_ARGS__)
#define ITERATE2_N_IT6(macro, separator, a, b,...)	\
	ITERATE2_N_IT2(macro, separator, a, b)			\
	separator()										\
	ITERATE2_N_IT4(macro, separator, __VA_ARGS__)


/*!	Iterates through the 3rd to nth macro arguments applying \a macro to triples
	of subsequent arguments. \a separator is the name of a macro that shall be
	evalutated between \a macro invocations. So
	\code ITERATE3(macro, separator, arg1, arg2, arg3,..., argn-1 argn) \endcode
	evaluates to
	\code macro(arg1, arg2, arg3) separator() macro(arg4, arg5, arg6)
		separator()  ... macro(argn-2, argn-1, argn) \endcode .
	The number of the arguments must be even.
 */
#define ITERATE3(macro, separator, ...) \
	ITERATE3_N(macro, separator, COUNT_MACRO_ARGS(__VA_ARGS__), \
		__VA_ARGS__)

#define ITERATE3_N(macro, separator, count, ...) \
	ITERATE3_N_IMPL(macro, separator, count, __VA_ARGS__)

#define ITERATE3_N_IMPL(macro, separator, count, ...) \
 	ITERATE3_N_IT##count(macro, separator, __VA_ARGS__)

#define ITERATE3_N_IT1(macro, separator, dummy)
#define ITERATE3_N_IT3(macro, separator, a, b, c)		\
	macro(a, b, c)
#define ITERATE3_N_IT6(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT3(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT9(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT6(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT12(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT9(macro, separator, __VA_ARGS__)


#endif	// UTIL_MACRO_MAGIC_H
