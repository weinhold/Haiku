/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include "debugger_interface/remote/Stream.h"


struct FdStream : Stream {
								FdStream(int input, int output,
									bool closeInput = false,
									bool closeOutput = false);
	virtual						~FdStream();

	virtual	ssize_t				Read(void* buffer, size_t size);
	virtual	ssize_t				Write(const void* buffer, size_t size);

	virtual	status_t			Flush();

	virtual	void				Close();

private:
			int					fInput;
			int					fOutput;
			bool				fCloseInput;
			bool				fCloseOutput;
};
