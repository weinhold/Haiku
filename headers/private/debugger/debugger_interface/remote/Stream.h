/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <DataIO.h>
#include <Referenceable.h>


struct Stream : BDataIO, BReferenceable {
	virtual						~Stream();

	virtual	void				Close() = 0;
};
