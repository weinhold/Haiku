/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include "PipedCommandTargetHostInterfaceSettings.h"

#include <AutoDeleter.h>

#include "Settings.h"
#include "SettingsDescription.h"


static const char* const kCommandSetting = "command";


PipedCommandTargetHostInterfaceSettings
	::PipedCommandTargetHostInterfaceSettings()
	:
	fCommandLine()
{
}


PipedCommandTargetHostInterfaceSettings
	::PipedCommandTargetHostInterfaceSettings(
		const Settings& settings)
	:
	fCommandLine()
{
	BVariant command = settings.Value(kCommandSetting);

	if (command.Type() != B_STRING_TYPE)
		return;

	fCommandLine = command.ToString();
}


void
PipedCommandTargetHostInterfaceSettings::Apply(Settings& settings)
{
	SettingsDescription* description = settings.Description();

	Setting* commandSetting = description->SettingByID(kCommandSetting);
	if (commandSetting == NULL)
		return;

	settings.SetValue(commandSetting, fCommandLine.String());
}


/*static*/ SettingsDescription*
PipedCommandTargetHostInterfaceSettings::CreateSettingsDescription()
{
	SettingsDescription* description = new(std::nothrow) SettingsDescription;
	if (description == NULL)
		return NULL;
	ObjectDeleter<SettingsDescription> descriptionDeleter(description);

	Setting* setting = new(std::nothrow) StringSettingImpl(kCommandSetting,
		"Command", "");
	if (setting == NULL)
		return NULL;
	ObjectDeleter<Setting> settingDeleter(setting);

	if (!description->AddSetting(setting))
		return NULL;

	settingDeleter.Detach();
	return descriptionDeleter.Detach();
}
