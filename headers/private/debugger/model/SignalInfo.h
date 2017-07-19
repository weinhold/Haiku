/*
 * Copyright 2015, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef SIGNAL_INFO_H
#define SIGNAL_INFO_H


#include <signal.h>

#include "Types.h"


enum {
	SIGNAL_HUP			= 1,
	SIGNAL_INT			= 2,
	SIGNAL_QUIT			= 3,
	SIGNAL_ILL			= 4,
	SIGNAL_CHLD			= 5,
	SIGNAL_ABRT			= 6,
	SIGNAL_PIPE			= 7,
	SIGNAL_FPE			= 8,
	SIGNAL_KILL			= 9,
	SIGNAL_STOP			= 10,
	SIGNAL_SEGV			= 11,
	SIGNAL_CONT			= 12,
	SIGNAL_TSTP			= 13,
	SIGNAL_ALRM			= 14,
	SIGNAL_TERM			= 15,
	SIGNAL_TTIN			= 16,
	SIGNAL_TTOU			= 17,
	SIGNAL_USR1			= 18,
	SIGNAL_USR2			= 19,
	SIGNAL_WINCH		= 20,
	SIGNAL_KILLTHR		= 21,
	SIGNAL_TRAP			= 22,
	SIGNAL_POLL			= 23,
	SIGNAL_PROF			= 24,
	SIGNAL_SYS			= 25,
	SIGNAL_URG			= 26,
	SIGNAL_VTALRM		= 27,
	SIGNAL_XCPU			= 28,
	SIGNAL_XFSZ			= 29,
	SIGNAL_BUS			= 30,

	SIGNAL_REALTIME_MIN	= 256,
	SIGNAL_REALTIME_MAX	= 511,

	SIGNAL_UNKNOWN_BASE	= 1024,
};


class SignalInfo {
public:
								SignalInfo();
								SignalInfo(const SignalInfo& other);
								SignalInfo(int signal,
									const struct sigaction& handler,
									bool deadly);

			void				SetTo(int signal,
									const struct sigaction& handler,
									bool deadly);

			int					Signal() const	{ return fSignal; }
			const struct sigaction&	Handler() const	{ return fHandler; }
			bool				Deadly() const 	{ return fDeadly; }
private:
			int 				fSignal;
			struct sigaction	fHandler;
			bool				fDeadly;
};


#endif	// SIGNAL_INFO_H
