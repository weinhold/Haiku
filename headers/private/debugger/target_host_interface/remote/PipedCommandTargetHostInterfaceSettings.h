/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include <String.h>


struct Settings;
struct SettingsDescription;


struct PipedCommandTargetHostInterfaceSettings {
								PipedCommandTargetHostInterfaceSettings();
								PipedCommandTargetHostInterfaceSettings(
									const Settings& settings);

			const BString&		CommandLine() const	{ return fCommandLine; }
			void				SetCommandLine(const BString& commandLine)
									{fCommandLine = commandLine; }

			void				Apply(Settings& settings);

	static	SettingsDescription* CreateSettingsDescription();

private:
			BString				fCommandLine;
};
