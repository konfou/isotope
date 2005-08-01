/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "SaveFormatDialog.h"
#include <qradiobutton.h>

SaveFormatDialog::SaveFormatDialog(QWidget* parent, const char* name)
: SaveFormatDialogBase( parent, name, TRUE, 0)
{
	RadioButton1->setChecked(true);
}

SaveFormatDialog::~SaveFormatDialog()
{

}

void SaveFormatDialog::slot_radio1clicked()
{
	if(!RadioButton1->isChecked())
		return;
	RadioButton2->setChecked(false);
	RadioButton3->setChecked(false);
	m_nSelection = 0;
}

void SaveFormatDialog::slot_radio2clicked()
{
	if(!RadioButton2->isChecked())
		return;
	RadioButton1->setChecked(false);
	RadioButton3->setChecked(false);
	m_nSelection = 1;
}

void SaveFormatDialog::slot_radio3clicked()
{
	if(!RadioButton3->isChecked())
		return;
	RadioButton1->setChecked(false);
	RadioButton2->setChecked(false);
	m_nSelection = 2;
}

