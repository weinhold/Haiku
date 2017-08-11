/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


class Architecture;


struct RemoteDebugFactoryContext {
								explicit RemoteDebugFactoryContext(
									const Architecture* architecture);
								RemoteDebugFactoryContext(
									const RemoteDebugFactoryContext& other);
								~RemoteDebugFactoryContext();

			RemoteDebugFactoryContext& operator=(
									const RemoteDebugFactoryContext& other);

			const Architecture*	GetArchitecture() const
									{ return fArchitecture; }

private:
			Architecture*		fArchitecture;
};
