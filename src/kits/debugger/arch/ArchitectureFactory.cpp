/*
 * Copyright 2018, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "ArchitectureFactory.h"

#include <string.h>

#include "ArchitectureNames.h"
#include "ArchitectureX86.h"
#include "ArchitectureX8664.h"


namespace {


template<typename ArchitectureType>
status_t
CreateArchitectureInternal(uint32 featureFlags,
	Architecture*& _architecture)
{
	Architecture* architecture = new(std::nothrow) ArchitectureType(
		featureFlags);
	if (architecture == NULL)
		return B_NO_MEMORY;

	status_t error = architecture->Init();
	if (error != B_OK) {
		delete architecture;
		return error;
	}

	_architecture = architecture;
	return B_OK;
}	



} // anonymous


namespace ArchitectureFactory {


status_t
CreateArchitectureX86(uint32 featureFlags,
	Architecture*& _architecture)
{
	return CreateArchitectureInternal<ArchitectureX86>(featureFlags,
		_architecture);
}	


status_t
CreateArchitectureX8664(uint32 featureFlags,
	Architecture*& _architecture)
{
	return CreateArchitectureInternal<ArchitectureX86>(featureFlags,
		_architecture);
}


status_t
CreateArchitecture(const char* name, uint32 featureFlags,
	Architecture*& _architecture)
{
	if (strcmp(name, kArchitectureNameX86) == 0)
		return CreateArchitectureX86(featureFlags, _architecture);
	if (strcmp(name, kArchitectureNameX8664) == 0)
		return CreateArchitectureX8664(featureFlags, _architecture);
	return B_BAD_VALUE;
}


}	// namespace ArchitectureFactory
