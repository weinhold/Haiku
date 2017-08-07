/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "debugger_interface/remote/FdStream.h"

#include <errno.h>
#include <fcntl.h>


FdStream::FdStream(int input, int output, bool closeInput, bool closeOutput)
	:
	fInput(input),
	fOutput(output),
	fCloseInput(closeInput),
	fCloseOutput(closeOutput)
{
}


FdStream::~FdStream()
{
	Close();
}


ssize_t
FdStream::Read(void* buffer, size_t size)
{
	ssize_t bytesRead = read(fInput, buffer, size);
	if (bytesRead < 0)
		return errno;
	return bytesRead;
}


ssize_t
FdStream::Write(const void* buffer, size_t size)
{
	ssize_t bytesWritten = write(fOutput, buffer, size);
	if (bytesWritten < 0)
		return errno;
	return bytesWritten;
}


status_t
FdStream::Flush()
{
	return B_OK;
}


void
FdStream::Close()
{
	if (fInput >= 0 && fCloseInput) {
		close(fInput);
		fInput = -1;
	}

	if (fOutput >= 0 && fCloseOutput) {
		close(fOutput);
		fOutput = -1;
	}
}
