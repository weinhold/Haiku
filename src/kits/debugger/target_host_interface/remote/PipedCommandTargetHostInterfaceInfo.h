/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include "TargetHostInterfaceInfo.h"


class PipedCommandTargetHostInterfaceInfo : public TargetHostInterfaceInfo {
public:
								PipedCommandTargetHostInterfaceInfo();
	virtual						~PipedCommandTargetHostInterfaceInfo();

	virtual	status_t			Init();

	virtual	bool				IsLocal() const;
	virtual	bool				IsConfigured(Settings* settings) const;
	virtual	SettingsDescription* GetSettingsDescription() const;

	virtual	status_t			CreateInterface(Settings* settings,
									const BString& connectionName,
									TargetHostInterface*& _interface) const;

private:
			BString				fName;
			SettingsDescription* fDescription;
};
