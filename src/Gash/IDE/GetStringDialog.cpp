/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "GetStringDialog.h"
#include <qlineedit.h>

GetStringDialog::GetStringDialog(QWidget* parent, const char* name, const char* szInitialText)
: GetStringDialogBase(parent, name, TRUE, 0)
{
	setCaption(QString(name));
	LineEdit1->setText(QString(szInitialText));
	LineEdit1->setSelection(0, 32767);
	LineEdit1->setFocus();
}

GetStringDialog::~GetStringDialog()
{

}

void GetStringDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case Key_Escape: reject();  break;
        case Key_Enter:  accept();  break;
	}
}
