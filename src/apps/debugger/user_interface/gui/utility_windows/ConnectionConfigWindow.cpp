/*
 * Copyright 2016-2017, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#include "ConnectionConfigWindow.h"

#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <SeparatorView.h>
#include <TextControl.h>

#include <AutoDeleter.h>
#include <AutoLocker.h>

#include "AppMessageCodes.h"
#include "ConnectionConfigHandlerRoster.h"
#include "Settings.h"
#include "TargetHostInterfaceInfo.h"
#include "TargetHostInterfaceRoster.h"
#include "TargetHostInterface.h"


enum {
	MSG_SWITCH_CONNECTION_TYPE 	= 'swct',
	MSG_CREATE_CONNECTION 		= 'crco',
	MSG_CONNECTION_NAME_CHANGED	= 'cnch'
};


ConnectionConfigWindow::ConnectionConfigWindow()
	:
	BWindow(BRect(), "Create new connection", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	ConnectionConfigView::Listener(),
	fConnectionTypeField(NULL),
	fConnectionNameInput(NULL),
	fConfigGroupView(NULL),
	fCloseButton(NULL),
	fConnectButton(NULL),
	fCurrentSettings(NULL),
	fActiveInfo(NULL)
{
}


ConnectionConfigWindow::~ConnectionConfigWindow()
{
	if (fCurrentSettings != NULL)
		fCurrentSettings->ReleaseReference();
}


ConnectionConfigWindow*
ConnectionConfigWindow::Create()
{
	ConnectionConfigWindow* self = new ConnectionConfigWindow();

	try {
		self->_Init();
	} catch (...) {
		delete self;
		throw;
	}

	return self;

}


void
ConnectionConfigWindow::Show()
{
	CenterOnScreen();
	BWindow::Show();
}


bool
ConnectionConfigWindow::QuitRequested()
{
	return be_app_messenger.SendMessage(MSG_CONNECTION_CONFIG_WINDOW_CLOSED)
		== B_OK;
}


void
ConnectionConfigWindow::ConfigurationChanged(Settings* settings)
{
	if (fCurrentSettings != NULL)
		fCurrentSettings->ReleaseReference();
	fCurrentSettings = settings;
	fCurrentSettings->AcquireReference();

	_UpdateConnectButton();
}


void
ConnectionConfigWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_CREATE_CONNECTION:
		{
			TargetHostInterfaceRoster* roster
				= TargetHostInterfaceRoster::Default();
			AutoLocker<TargetHostInterfaceRoster> rosterLocker(roster);

			TargetHostInterface* interface;
			status_t error = roster->CreateInterface(fActiveInfo,
				fCurrentSettings, _ConnectionName(), interface);

			// TODO: notify user.
			if (error != B_OK)
				break;

			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case MSG_SWITCH_CONNECTION_TYPE:
			_SetActiveConfig(message);
			break;
		case MSG_CONNECTION_NAME_CHANGED:
			_UpdateConnectButton();
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
ConnectionConfigWindow::_Init()
{
	BMenu* menu = _BuildTypeMenu();

	BBox* configGroupBox = NULL;
	fConfigGroupView = new BGroupView(B_HORIZONTAL);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fConnectionNameInput = new BTextControl(
			"connectionName", "Name:", "",
			new BMessage(MSG_CONNECTION_NAME_CHANGED)))
		.Add(configGroupBox = new BBox("configGroup"))
		.AddGroup(B_HORIZONTAL)
			.SetInsets(B_USE_SMALL_SPACING)
			.AddGlue()
			.Add(fCloseButton = new BButton("Close",
					new BMessage(B_QUIT_REQUESTED)))
			.Add(fConnectButton = new BButton("Connect",
					new BMessage(MSG_CREATE_CONNECTION)))
		.End();

	fConfigGroupView->GroupLayout()->SetInsets(B_USE_WINDOW_SPACING,
		B_USE_WINDOW_SPACING);
	configGroupBox->AddChild(fConfigGroupView);
	configGroupBox->SetLabel(fConnectionTypeField = new BMenuField(NULL, menu));

	fConnectionTypeField->Menu()->SetLabelFromMarked(true);
	fConnectionTypeField->Menu()->SetTargetForItems(this);

	fConnectionNameInput->SetModificationMessage(
		new BMessage(MSG_CONNECTION_NAME_CHANGED));

	fCloseButton->SetTarget(this);
	fConnectButton->SetTarget(this);
	fConnectButton->SetEnabled(false);

	if (menu->CountItems() > 0) {
		BMenuItem* item = menu->ItemAt(0);
		BMessage* message = item->Message();
		_SetActiveConfig(message, item);
	}

	fConnectionTypeField->SetExplicitMaxSize(
		fConnectionTypeField->MinSize());
}


BMenu*
ConnectionConfigWindow::_BuildTypeMenu()
{
	BMenu* menu = new BMenu("Types");
	TargetHostInterfaceRoster* roster = TargetHostInterfaceRoster::Default();

	AutoLocker<TargetHostInterfaceRoster> rosterLocker(roster);

	for (int32 i = 0; i < roster->CountInterfaceInfos(); i++) {
		TargetHostInterfaceInfo* info = roster->InterfaceInfoAt(i);
		if (info->IsLocal())
			continue;

		BMenuItem* item = new BMenuItem(info->Name(), new BMessage(
				MSG_SWITCH_CONNECTION_TYPE));
		item->Message()->AddPointer("info", info);
		menu->AddItem(item);
	}

	return menu;
}


void
ConnectionConfigWindow::_SetActiveConfig(const BMessage* message,
	BMenuItem* item)
{
	// get the info
	TargetHostInterfaceInfo* info = NULL;
	if (message->FindPointer("info", reinterpret_cast<void**>(&info)) != B_OK)
		return;

	if (info == fActiveInfo)
		return;

	// remove current config view
	if (fConfigGroupView->CountChildren() > 0) {
		BView* view = fConfigGroupView->ChildAt(0);
		fConfigGroupView->RemoveChild(view);
		delete view;
	}

	fActiveInfo = NULL;

	ConnectionConfigHandlerRoster* roster
		= ConnectionConfigHandlerRoster::Default();

	if (roster->HasHandlerFor(info)) {
		ConnectionConfigView* view = NULL;
		status_t error = roster->CreateConfigView(info, this, view);
		if (error != B_OK)
			return;

		ObjectDeleter<ConnectionConfigView> viewDeleter(view);
		BLayoutItem* item = fConfigGroupView->GroupLayout()->AddView(view);
		if (item != NULL)
			viewDeleter.Detach();

		fActiveInfo = info;
	}

	fConfigGroupView->InvalidateLayout();

	// mark respective menu item
	BMenu* menu = fConnectionTypeField->Menu();
	if (item == NULL) {
		int32 index;
		if (message->FindInt32("index", &index) == B_OK)
			item = menu->ItemAt(index);
	}

	if (item != NULL)
		item->SetMarked(true);
}


void
ConnectionConfigWindow::_UpdateConnectButton()
{
	fConnectButton->SetEnabled(
		!_ConnectionName().IsEmpty()
			&& fActiveInfo != NULL && fCurrentSettings != NULL
			&& fActiveInfo->IsConfigured(fCurrentSettings));
}


BString
ConnectionConfigWindow::_ConnectionName() const
{
	BString name = fConnectionNameInput->Text();
	name.Trim();
	return name;
}
