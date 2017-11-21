/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef UTIL_MACRO_MAGIC_H
#define UTIL_MACRO_MAGIC_H



//! Returns the number of macro arguments.
#define COUNT_MACRO_ARGS(...)						\
	COUNT_MACRO_ARGS_IMPL(__VA_ARGS__,				\
		100, 99, 98, 97, 96, 95, 94, 93, 92, 91,	\
		90, 89, 88, 87, 86, 85, 84, 83, 82, 81,		\
		80, 79, 78, 77, 76, 75, 74, 73, 72, 71,		\
		70, 69, 68, 67, 66, 65, 64, 63, 62, 61,		\
		60, 59, 58, 57, 56, 55, 54, 53, 52, 51,		\
		50, 49, 48, 47, 46, 45, 44, 43, 42, 41,		\
		40, 39, 38, 37, 36, 35, 34, 33, 32, 31,		\
		30, 29, 28, 27, 26, 25, 24, 23, 22, 21,		\
		20, 19, 18, 17, 16, 15, 14, 13, 12, 11,		\
		10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define COUNT_MACRO_ARGS_IMPL(							\
	_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,			\
	_11, _12, _13, _14, _15, _16, _17, _18, _19, _20,	\
	_21, _22, _23, _24, _25, _26, _27, _28, _29, _30,	\
	_31, _32, _33, _34, _35, _36, _37, _38, _39, _40,	\
	_41, _42, _43, _44, _45, _46, _47, _48, _49, _50,	\
	_51, _52, _53, _54, _55, _56, _57, _58, _59, _60,	\
	_61, _62, _63, _64, _65, _66, _67, _68, _69, _70,	\
	_71, _72, _73, _74, _75, _76, _77, _78, _79, _80,	\
	_81, _82, _83, _84, _85, _86, _87, _88, _89, _90,	\
	_91, _92, _93, _94, _95, _96, _97, _98, _99, _100,	\
	N, ...) N

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
#define ITERATE3_N_IT15(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT12(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT18(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT15(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT21(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT18(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT24(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT21(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT27(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT24(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT30(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT27(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT33(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT30(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT36(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT33(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT39(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT36(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT42(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT39(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT45(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT42(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT48(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT45(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT51(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT48(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT54(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT51(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT57(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT54(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT60(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT57(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT63(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT60(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT66(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT63(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT69(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT66(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT72(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT69(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT75(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT72(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT78(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT75(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT81(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT78(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT84(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT81(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT87(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT84(macro, separator, __VA_ARGS__)
#define ITERATE3_N_IT90(macro, separator, a, b, c,...)	\
	ITERATE3_N_IT3(macro, separator, a, b, c)			\
	separator()											\
	ITERATE3_N_IT87(macro, separator, __VA_ARGS__)


#endif	// UTIL_MACRO_MAGIC_H
