/*
 * Copyright 2016, Rene Gollent, rene@gollent.com.
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#pragma once


#include "ConnectionConfigView.h"


class BTextControl;


class PipedCommandConnectionConfigView : public ConnectionConfigView{
public:
								PipedCommandConnectionConfigView();
	virtual						~PipedCommandConnectionConfigView();

	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);

protected:
	virtual	status_t			InitSpecific();

private:
			BTextControl*		fCommandInput;
			Settings*			fSettings;
};
