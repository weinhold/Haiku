/*
 * Copyright 2016-2017, Rene Gollent, rene@gollent.com.
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "PipedCommandTargetHostInterfaceInfo.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#include <exception>
#include <string>

#include "RemoteTargetHostInterface.h"
#include "PipedCommandTargetHostInterfaceSettings.h"
#include "SettingsDescription.h"

#include "SettingsDescription.h"

#include "debugger_interface/remote/FdStream.h"


namespace {


struct Error : std::exception {
	Error(const std::string& message, status_t errorCode)
		:
		fErrorCode(errorCode == B_OK ? B_ERROR : errorCode),
		fMessage(message + ": " + strerror(fErrorCode))
	{
	}

	~Error() throw()
	{
	}

	virtual const char* what() const throw()
	{
		return fMessage.c_str();
	}

	status_t ErrorCode() const
	{
		return fErrorCode;
	}

private:
	status_t	fErrorCode;
	std::string	fMessage;
};


struct Pipe {
	Pipe()
		:
		fReadFd(-1),
		fWriteFd(-1)
	{
		int fds[2];
		if (pipe(fds) != 0)
			throw Error("pipe() failed", errno);

		fReadFd = fds[0];
		fWriteFd = fds[1];
	}

	~Pipe()
	{
		Close();
	}

	void Close()
	{
		if (fReadFd >= 0) {
			close(fReadFd);
			fReadFd = -1;
		}
		if (fWriteFd >= 0) {
			close(fWriteFd);
			fWriteFd = -1;
		}
	}

	int ReadFd() const
	{
		return fReadFd;
	}

	int WriteFd() const
	{
		return fWriteFd;
	}

	void DetachReadFd()
	{
		fReadFd = -1;
	}

	void DetachWriteFd()
	{
		fWriteFd = -1;
	}

private:
	int	fReadFd;
	int	fWriteFd;
};


void
spawnPipedCommandThrows(const char* commandLine,
	BReference<Stream>& streamReference)
{
	Pipe inPipe;
	Pipe outPipe;

	pid_t child = fork();
	if (child < 0)
		throw Error("fork() failed", errno);

	if (child == 0) {
		// in child process

		// prepare stdin and stdout FDs
		dup2(inPipe.ReadFd(), 0);
		dup2(outPipe.WriteFd(), 1);

		// close pipes
		inPipe.Close();
		outPipe.Close();

		// to be on the safe side, close all left-over FDs
		struct rlimit limits;
		if (getrlimit(RLIMIT_NOFILE, &limits) == 0) {
			for (int i = 3; i < (int)limits.rlim_cur; i++)
				close(i);
		}

		// exec() the specified command
		execl("/bin/sh", "/bin/sh", "-c", commandLine, NULL);

		// exec() shouldn't return, so it apparently failed
		fprintf(stderr, "execl() failed: %s\n", strerror(errno));
		exit(1);
	}

	// in parent process
	streamReference = new FdStream(outPipe.ReadFd(), inPipe.WriteFd(), true,
		true);

	outPipe.DetachReadFd();
	inPipe.DetachWriteFd();
}


status_t
spawnPipedCommand(const char* commandLine, BReference<Stream>& streamReference)
{
	try {
		spawnPipedCommandThrows(commandLine, streamReference);
		return B_OK;
	} catch (Error& exception) {
		// TODO: log!
		return exception.ErrorCode();
	} catch (std::bad_alloc&) {
		return B_NO_MEMORY;
	}
}


} // anonymous namespace


PipedCommandTargetHostInterfaceInfo::PipedCommandTargetHostInterfaceInfo()
	:
	TargetHostInterfaceInfo("Command"),
	fDescription(NULL)
{
}


PipedCommandTargetHostInterfaceInfo::~PipedCommandTargetHostInterfaceInfo()
{
	delete fDescription;
}


status_t
PipedCommandTargetHostInterfaceInfo::Init()
{
	fDescription
		= PipedCommandTargetHostInterfaceSettings::CreateSettingsDescription();
	if (fDescription == NULL)
		return B_NO_MEMORY;

	return B_OK;
}


bool
PipedCommandTargetHostInterfaceInfo::IsLocal() const
{
	return false;
}


bool
PipedCommandTargetHostInterfaceInfo::IsConfigured(Settings* settings) const
{
	PipedCommandTargetHostInterfaceSettings remoteSettings(*settings);
	return !remoteSettings.CommandLine().IsEmpty();
}


SettingsDescription*
PipedCommandTargetHostInterfaceInfo::GetSettingsDescription() const
{
	return fDescription;
}


status_t
PipedCommandTargetHostInterfaceInfo::CreateInterface(Settings* settings,
	const BString& connectionName, TargetHostInterface*& _interface) const
{
	PipedCommandTargetHostInterfaceSettings remoteSettings(*settings);
	if (remoteSettings.CommandLine().IsEmpty())
		return B_BAD_VALUE;

	BReference<Stream> stream;
	status_t error = spawnPipedCommand(remoteSettings.CommandLine(), stream);
	if (error != B_OK)
		return error;

	RemoteTargetHostInterface* interface
		= new(std::nothrow) RemoteTargetHostInterface;
	if (interface == NULL)
		return B_NO_MEMORY;

	error = interface->Init(connectionName, stream.Get());
	if (error != B_OK) {
		delete interface;
		return error;
	}

	_interface = interface;
	return B_OK;
}
