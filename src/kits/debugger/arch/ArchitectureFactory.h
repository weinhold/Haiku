/*
 * Copyright 2018, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <SupportDefs.h>


struct Architecture;


namespace ArchitectureFactory {


status_t CreateArchitectureX86(uint32 featureFlags,
	Architecture*& _architecture);

status_t CreateArchitectureX8664(uint32 featureFlags,
	Architecture*& _architecture);

status_t CreateArchitecture(const char* name, uint32 featureFlags,
	Architecture*& _architecture);


}	// namespace ArchitectureFactory
