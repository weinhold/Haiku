/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef TRACING_H
#define TRACING_H


#include <stdio.h>

#include "apps_debugger_config.h"


#define WARNING(...)	fprintf(stderr, __VA_ARGS__)
#define ERROR(...)		fprintf(stderr, __VA_ARGS__)


#if APPS_DEBUGGER_TRACE_DWARF_DIE
#	define TRACE_DIE(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_DIE_ONLY(x)	x
#else
#	define TRACE_DIE(...)		(void)0
#	define TRACE_DIE_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_DWARF_LINE_INFO
#	define TRACE_LINES(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_LINES_ONLY(x)	x
#else
#	define TRACE_LINES(...)		(void)0
#	define TRACE_LINES_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_DWARF_LINE_INFO >= 2
#	define TRACE_LINES2(...)	fprintf(stderr, __VA_ARGS__)
#	define TRACE_LINES2_ONLY(x)	x
#else
#	define TRACE_LINES2(...)	(void)0
#	define TRACE_LINES2_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_DWARF_EXPRESSIONS
#	define TRACE_EXPR(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_EXPR_ONLY(x)	x
#else
#	define TRACE_EXPR(...)		(void)0
#	define TRACE_EXPR_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_DWARF_PUBLIC_TYPES
#	define TRACE_PUBTYPES(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_PUBTYPES_ONLY(x)	x
#else
#	define TRACE_PUBTYPES(...)		(void)0
#	define TRACE_PUBTYPES_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_CFI
#	define TRACE_CFI(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_CFI_ONLY(x)	x
#else
#	define TRACE_CFI(...)		(void)0
#	define TRACE_CFI_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_STACK_FRAME_LOCALS
#	define TRACE_LOCALS(...)	fprintf(stderr, __VA_ARGS__)
#	define TRACE_LOCALS_ONLY(x)	x
#else
#	define TRACE_LOCALS(...)	(void)0
#	define TRACE_LOCALS_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_IMAGES
#	define TRACE_IMAGES(...)	fprintf(stderr, __VA_ARGS__)
#	define TRACE_IMAGES_ONLY(x)	x
#else
#	define TRACE_IMAGES(...)	(void)0
#	define TRACE_IMAGES_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_CODE
#	define TRACE_CODE(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_CODE_ONLY(x)	x
#else
#	define TRACE_CODE(...)		(void)0
#	define TRACE_CODE_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_JOBS
#	define TRACE_JOBS(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_JOBS_ONLY(x)	x
#else
#	define TRACE_JOBS(...)		(void)0
#	define TRACE_JOBS_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_DEBUG_EVENTS
#	define TRACE_EVENTS(...)	fprintf(stderr, __VA_ARGS__)
#	define TRACE_EVENTS_ONLY(x)	x
#else
#	define TRACE_EVENTS(...)	(void)0
#	define TRACE_EVENTS_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_TEAM_CONTROL
#	define TRACE_CONTROL(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_CONTROL_ONLY(x)	x
#else
#	define TRACE_CONTROL(...)		(void)0
#	define TRACE_CONTROL_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_GUI
#	define TRACE_GUI(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_GUI_ONLY(x)	x
#else
#	define TRACE_GUI(...)		(void)0
#	define TRACE_GUI_ONLY(x)
#endif

#if APPS_DEBUGGER_TRACE_REMOTE
#	define TRACE_REMOTE(...)		fprintf(stderr, __VA_ARGS__)
#	define TRACE_REMOTE_ONLY(x)		x
#else
#	define TRACE_REMOTE(...)		(void)0
#	define TRACE_REMOTE_ONLY(x)
#endif


#endif	// TRACING_H
