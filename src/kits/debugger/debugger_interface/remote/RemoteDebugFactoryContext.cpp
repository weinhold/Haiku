/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/RemoteDebugFactoryContext.h"

#include "Architecture.h"


RemoteDebugFactoryContext::RemoteDebugFactoryContext(
		const Architecture* architecture)
	:
	fArchitecture(const_cast<Architecture*>(architecture))
{
	fArchitecture->AcquireReference();
}


RemoteDebugFactoryContext::RemoteDebugFactoryContext(
	const RemoteDebugFactoryContext& other)
	:
	fArchitecture(other.fArchitecture)
{
	fArchitecture->AcquireReference();
}


RemoteDebugFactoryContext::~RemoteDebugFactoryContext()
{
	fArchitecture->ReleaseReference();
}


RemoteDebugFactoryContext& RemoteDebugFactoryContext::operator=(
	const RemoteDebugFactoryContext& other)
{
	other.fArchitecture->AcquireReference();
	fArchitecture->ReleaseReference();
	fArchitecture = other.fArchitecture;
	return *this;
}
