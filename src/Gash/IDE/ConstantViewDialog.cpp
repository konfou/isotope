/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "ConstantViewDialog.h"
#include "../CodeObjects/CodeObjects.h"
#include <qmessagebox.h>
#include <qlabel.h>
#include "GetStringDialog.h"
#include <qradiobutton.h>
#include <qlineedit.h>

ConstantViewDialog::ConstantViewDialog(COProject* pCOProject, COConstant* pConstant, QWidget* parent, const char* name)
: ConstantViewDialogBase(parent, name, true, 0)
{
	m_pCOProject = pCOProject;
	m_pConstant = pConstant;
	m_nSelected = -1;
	TextLabel2->setText(QString(pConstant->GetName()));
	if(pConstant->IsString())
	{
		TextLabel4->setText(QString(pConstant->GetStringValue()));
		RadioButton2->setChecked(true);
	}
	else
	{
		char szTmp[32];
		itoa(pConstant->GetIntegerValue(), szTmp, 10);
		TextLabel4->setText(QString(szTmp));
		RadioButton1->setChecked(true);
	}
}

ConstantViewDialog::~ConstantViewDialog()
{
}

void ConstantViewDialog::SetFocus(int n)
{
	GAssert(n >= 0 && n <= 2, "Out of range");
	// Move the arrow to the new list
	switch(n)
	{
	case 0:
		PixmapLabel1->move(TextLabel2->pos().x() + 100, TextLabel2->pos().y() - 30);
		break;
	case 1:
		PixmapLabel1->move(RadioButton1->pos().x() + 50, RadioButton1->pos().y() - 30);
		break;
	case 2:
		PixmapLabel1->move(TextLabel4->pos().x() + 100, TextLabel4->pos().y() - 30);
		break;
	}
	m_nSelected = n;
}

void ConstantViewDialog::EditName()
{
	SetFocus(0);
	GetStringDialog dialog(this, "Enter a name for this call back", m_pConstant->GetName());
	if(dialog.exec() == Accepted)
	{
		QString s = dialog.LineEdit1->text();
		TextLabel2->setText(s);
	}
}

void ConstantViewDialog::EditValue()
{
	SetFocus(2);
	const char* szTitle;
	QString s = TextLabel4->text();
	const char* szValue = s.latin1();
	char szTmp[32];
	if(RadioButton1->isChecked())
	{
		szTitle = "Enter an integer value";
		itoa(atoi(szValue), szTmp, 10);
		szValue = szTmp;
	}
	else
		szTitle = "Enter a string value";

	GetStringDialog dialog(this, szTitle, szValue);
	if(dialog.exec() == Accepted)
	{
		QString s = dialog.LineEdit1->text();
		const char* sTmp = s.latin1();
		if(RadioButton1->isChecked())
		{
			int n;
			for(n = 0; szTmp[n] != '\0'; n++)
			{
				if(szTmp[n] != '0' && (szTmp[n] < '0' || szTmp[n] > '9'))
				{
					QMessageBox::information(this, "invalid character", "That's not an integer value");
					return;
				}
			}
		}
		else
		{
			int n;
			for(n = 0; szTmp[n] != '\0'; n++)
			{
				if(szTmp[n] == '\'')
				{
					QMessageBox::information(this, "invalid character", "Sorry, apostrophes are not allowed in strings");
					return;
				}
			}
		}
		TextLabel4->setText(s);
	}
}

// OK button
void ConstantViewDialog::slot_button6()
{
	const char* szName = TextLabel2->text().latin1();
	if(strlen(szName) < 1)
	{
		QMessageBox::information(this, "invalid name", "The name can't be empty");
		return;
	}
	// todo: check to see if that name is in use
	const char* szValue = TextLabel4->text().latin1();
	if(RadioButton1->isChecked())
		m_pConstant->SetValue(atoi(szValue));
	else
		m_pConstant->SetValue(szValue);
	m_pConstant->SetName(szName);
	accept();
}

void ConstantViewDialog::slot_radio1toggled()
{
	if(RadioButton1->isChecked())
	{
		RadioButton2->setChecked(false);
	}
	else
	{
		if(!RadioButton2->isChecked())
		{
			RadioButton1->setChecked(true);
		}
	}
}

void ConstantViewDialog::slot_radio2toggled()
{
	if(RadioButton2->isChecked())
	{
		RadioButton1->setChecked(false);
	}
	else
	{
		if(!RadioButton1->isChecked())
		{
			RadioButton2->setChecked(true);
		}
	}
}

void ConstantViewDialog::arrowUp()
{
	RadioButton1->setChecked(true);
}

void ConstantViewDialog::arrowDown()
{
	RadioButton2->setChecked(true);
}

void ConstantViewDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'A':	EditName(); break;
		case 'V':	EditValue(); break;
		case 'W':	arrowUp(); break;
		case 'S':	arrowDown(); break;
		case Key_Escape: // fall through
		case 'Q':	reject(); break;
		case Key_Up:	arrowUp(); break;
		case Key_Down:	arrowDown(); break;
		case Key_Space: slot_button6(); break;
	}
}
