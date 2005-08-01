/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "CallBackViewDialog.h"
#include <qlistbox.h>
#include "../CodeObjects/CodeObjects.h"
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include "ParameterViewDialog.h"
#include "GetStringDialog.h"

CallBackViewDialog::CallBackViewDialog(COProject* pCOProject, COMethod* pMethod, COInstruction* pInstruction, QWidget* parent, const char* name, bool modal, WFlags fl)
: CallBackViewDialogBase(parent, name, modal, fl)
{
	//GAssert(pInstruction->GetInstructionType() == COInstruction::IT_CALLBACK, "This dialog is only valid for callbacks");
	m_pInstruction = pInstruction;
	m_pCOProject = pCOProject;
	m_pMethod = pMethod;
	BuildParamsArray();
	m_nSelectedList = -1;
	TextLabel1->setText(QString(((COCallBack*)pInstruction)->GetName()));
	RefreshParamList();
}

CallBackViewDialog::~CallBackViewDialog()
{
	int nCount = m_pParams->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((COExpression*)m_pParams->GetPointer(n));
	delete(m_pParams);
}

void CallBackViewDialog::SetFocus(int n)
{
	// Move the arrow to the new list
	m_nSelectedList = n;
	if(n == 0)
		PixmapLabel1->move(TextLabel1->pos().x() + 100, TextLabel1->pos().y() - 30);
	else if(n == 1)
	{
		PixmapLabel1->move(ListBox5->pos().x() + 100, ListBox5->pos().y() - 30);
		if(ListBox5->count() > 0)
		{
			if(ListBox5->currentItem() < 0 || !ListBox5->isSelected(ListBox5->currentItem()))
				ListBox5->setSelected(0, true);
		}
	}
	else
	{
		GAssert(false, "Out of range");
	}
}

void CallBackViewDialog::BuildParamsArray()
{
	m_pParams = new GPointerArray(64);
	COCallBack* pCallBack = (COCallBack*)m_pInstruction;
//	COExpression* pParam;
//	for(pParam = pCallBack->GetFirstParam(); pParam; pParam = pCallBack->GetNextParam(pParam))
//		m_pParams->AddPointer(new COExpression(pParam));
}

void CallBackViewDialog::slot_list5selChanged()
{

}

void CallBackViewDialog::slot_list5clicked()
{
	SetFocus(1);
}

// Add button
void CallBackViewDialog::slot_button11()
{
/*	COParam* pNewParam = new COParam(m_pCOProject->m_pNull);
	ParameterViewDialog dialog(m_pCOProject, m_pMethod, m_pInstruction, pNewParam, this, "Parameter View", true, 0);
	if(dialog.exec() == Accepted)
		m_pParams->AddPointer(pNewParam);
	else
		delete(pNewParam);
	RefreshParamList();*/
}

// Edit button
void CallBackViewDialog::slot_button18()
{
    int nSel = ListBox5->currentItem();
	if(nSel < 0)
    {
        QMessageBox::information(this, "Nothing selected", "First select a parameter to edit");
        return;
    }
    GAssert(nSel < m_pParams->GetSize(), "Out of range");
/*    COParam* pParam = (COParam*)m_pParams->GetPointer(nSel);
	ParameterViewDialog dialog(m_pCOProject, m_pMethod, m_pInstruction, pParam, this, "Parameter View", true, 0);
	dialog.exec();
	RefreshParamList();*/
}

// Remove button
void CallBackViewDialog::slot_button4_2()
{
    int nSel = ListBox5->currentItem();
	if(nSel < 0)
    {
        QMessageBox::information(this, "Nothing selected", "First select a parameter to remove");
        return;
    }
    GAssert(nSel < m_pParams->GetSize(), "Out of range");
    m_pParams->DeleteCell(nSel);
	RefreshParamList();
}

void CallBackViewDialog::EditName()
{
	SetFocus(0);
	GetStringDialog dialog(this, "Enter a name for this call back", ((COCallBack*)m_pInstruction)->GetName());
	if(dialog.exec() == Accepted)
	{
		QString s = dialog.LineEdit1->text();
		TextLabel1->setText(s);
	}
}

void CallBackViewDialog::arrowUp()
{
	int n = ListBox5->currentItem() - 1;
	if(n >= 0 && n < (int)ListBox5->count())
		ListBox5->setSelected(n, true);
}

void CallBackViewDialog::arrowDown()
{
	int n = ListBox5->currentItem() + 1;
	if(n >= 0 && n < (int)ListBox5->count())
		ListBox5->setSelected(n, true);
}

void CallBackViewDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'A':	slot_button11(); break;
		case 'E':	slot_button18(); break;
		case 'R':	slot_button4_2(); break;
		case 'T':	EditName(); break;
		case 'W':	SetFocus(1); break;
		case Key_Escape: // fall through
		case 'Q':	reject(); break;
		case Key_Up:	arrowUp(); break;
		case Key_Down:	arrowDown(); break;
		case Key_Space: slot_button3(); break;
	}
}

void CallBackViewDialog::RefreshParamList()
{
	ListBox5->clear(); // Clear the Param list
/*	COParam* pParam;
	char szTmp[512];
	int n;
	int nCount = m_pParams->GetSize();
	for(n = 0; n < nCount; n++)
	{
		pParam = (COParam*)m_pParams->GetPointer(n);
		pParam->GetExpression(szTmp, 512);
		ListBox5->insertItem(QString(szTmp)); // todo: implment this properly
	}
	if(ListBox5->currentItem() < 0 || !ListBox5->isSelected(ListBox5->currentItem()))
		ListBox5->setSelected(0, true);*/
}

// OK button
void CallBackViewDialog::slot_button3()
{
/*	// Set the name
	COCallBack* pCallBack = (COCallBack*)m_pInstruction;
	pCallBack->SetName(TextLabel1->text().latin1());

	// Make the parameters
	int nCount = m_pParams->GetSize();
	GLList* pNewParams = new GLList();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pNewParams->Link((COParam*)m_pParams->GetPointer(n));
		m_pParams->SetPointer(n, NULL);
	}
	pCallBack->SetParams(pNewParams);*/

	// Close the dialog
	accept();
}

