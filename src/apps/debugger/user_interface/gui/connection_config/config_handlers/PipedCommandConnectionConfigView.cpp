/*
 * Copyright 2016-2017, Rene Gollent, rene@gollent.com.
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
 
 
#include "PipedCommandConnectionConfigView.h"

#include <stdio.h>
#include <stdlib.h>

#include <LayoutBuilder.h>
#include <TextControl.h>

#include "PipedCommandTargetHostInterfaceSettings.h"
#include "Settings.h"
#include "SettingsDescription.h"
#include "TargetHostInterfaceInfo.h"


enum {
	MSG_PIPED_COMMAND_CONFIG_INPUT_CHANGED = 'ccic'
};


PipedCommandConnectionConfigView::PipedCommandConnectionConfigView()
	:
	ConnectionConfigView("NetworkConnectionConfig"),
	fCommandInput(NULL),
	fSettings(NULL)
{
}


PipedCommandConnectionConfigView::~PipedCommandConnectionConfigView()
{
	if (fSettings != NULL)
		fSettings->ReleaseReference();
}


void
PipedCommandConnectionConfigView::AttachedToWindow()
{
	ConnectionConfigView::AttachedToWindow();

	fCommandInput->SetTarget(this);
}


void
PipedCommandConnectionConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_PIPED_COMMAND_CONFIG_INPUT_CHANGED:
		{
			PipedCommandTargetHostInterfaceSettings settings;
			settings.SetCommandLine(fCommandInput->Text());
			settings.Apply(*fSettings);
			NotifyConfigurationChanged(fSettings);
			break;
		}
		default:
		{
			ConnectionConfigView::MessageReceived(message);
			break;
		}
	}
}


status_t
PipedCommandConnectionConfigView::InitSpecific()
{
	// create Settings
	SettingsDescription* description
		= InterfaceInfo()->GetSettingsDescription();

	fSettings = new Settings(description);

	// create views
	BLayoutBuilder::Group<>(this, B_HORIZONTAL)
		.Add((fCommandInput = new BTextControl("commandLine", "Command:", "",
				new BMessage(MSG_PIPED_COMMAND_CONFIG_INPUT_CHANGED))))
	.End();

	fCommandInput->SetModificationMessage(
		new BMessage(MSG_PIPED_COMMAND_CONFIG_INPUT_CHANGED));

	return B_OK;
}
