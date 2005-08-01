/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "ClassViewDialog.h"
#include "../CodeObjects/CodeObjects.h"
#include <qlistbox.h>
#include <qmessagebox.h>
#include "MethodViewDialog.h"
#include "DeclViewDialog.h"
#include <qradiobutton.h>
#include <qevent.h>
#include <qlabel.h>
#include "GetStringDialog.h"
#include <qlineedit.h>
#include "ConstantViewDialog.h"

ClassViewDialog::ClassViewDialog(COProject* pCOProject, COClass* pClass, QWidget* parent, const char* name, bool bNeedToChangeName)
: ClassViewDialogBase(parent, name, true, 0)
{
	m_bNeedToChangeName = bNeedToChangeName;
	m_bDirty = false;
	m_pClass = pClass;
	m_pCOProject = pCOProject;
	m_nSelectedList = 0;
	TextLabel2_2->setText(QString(pClass->GetName()));
	RefreshLists();
	m_nSelectedList = -1;
	SetFocus(0);
}

ClassViewDialog::~ClassViewDialog()
{

}

COVariable* ClassViewDialog::GetSelectedMember()
{
	// See which item is selected
	int nSel = ListBox1->currentItem();

	// Find the object
	COVariable* pMember;
	int nCount = 0;
	for(pMember = m_pClass->GetFirstMember(); pMember && nCount < nSel; pMember = m_pClass->GetNextMember(pMember))
		nCount++;
	if(nCount < nSel)
	{
		GAssert(false, "Internal Error--listbox doesn't match data");
		return NULL;
	}
	return pMember;
}

COConstant* ClassViewDialog::GetSelectedConstant()
{
	// See which item is selected
	int nSel = ListBox5_2->currentItem();

	// Find the object
	COConstant* pConst;
	int nCount = 0;
	for(pConst = m_pClass->GetFirstConstant(); pConst && nCount < nSel; pConst = m_pClass->GetNextConstant(pConst))
		nCount++;
	if(nCount < nSel)
	{
		GAssert(false, "Internal Error--listbox doesn't match data");
		return NULL;
	}
	return pConst;
}

COMethod* ClassViewDialog::GetSelectedMethod()
{
	// See which item is selected
	int nSel = ListBox4->currentItem();

	// Find the object
	COMethod* pMethod;
	int nCount = 0;
	for(pMethod = m_pClass->GetFirstMethod(); pMethod && nCount < nSel; pMethod = m_pClass->GetNextMethod(pMethod))
		nCount++;
	if(nCount < nSel)
	{
		GAssert(false, "Internal Error--listbox doesn't match data");
		return NULL;
	}
	return pMethod;
}

COMethod* ClassViewDialog::GetSelectedProcedure()
{
	// See which item is selected
	int nSel = ListBox5->currentItem();

	// Find the object
	COMethod* pMethod;
	int nCount = 0;
	for(pMethod = m_pClass->GetFirstProcedure(); pMethod && nCount < nSel; pMethod = m_pClass->GetNextProcedure(pMethod))
		nCount++;
	if(nCount < nSel)
	{
		GAssert(false, "Internal Error--listbox doesn't match data");
		return NULL;
	}
	return pMethod;
}

//Refresh all lists in Class view
void ClassViewDialog::RefreshLists()
{
	RefreshMemberList();
	RefreshConstantList();
	RefreshInterfaceList();
	RefreshMethodList();
	RefreshProcedureList();
}

void ClassViewDialog::RefreshMemberList()
{
    // todo: this listbox only shows extended members.  Maybe make another one to list inherrited members.
	ListBox1->clear();
	COVariable* pMember;
	int nCount = 0;
	for(pMember = m_pClass->GetFirstMember(); pMember; pMember = m_pClass->GetNextMember(pMember))
	{
		nCount++;
		QString s = "[";
		s += QString(pMember->GetName());
		s += "] ";
		s += QString(pMember->GetType()->GetName());
		ListBox1->insertItem(s);
	}
}

void ClassViewDialog::RefreshConstantList()
{
	ListBox5_2->clear();
	COConstant* pConstant;
	for(pConstant = m_pClass->GetFirstConstant(); pConstant; pConstant = m_pClass->GetNextConstant(pConstant))
		ListBox5_2->insertItem(QString(pConstant->GetName()));
}

void ClassViewDialog::RefreshInterfaceList()
{
    ListBox2->clear();
	// todo: implement this
}

void ClassViewDialog::RefreshMethodList()
{
	ListBox4->clear();
	COMethod* pMethod;
	for(pMethod = m_pClass->GetFirstMethod(); pMethod; pMethod = m_pClass->GetNextMethod(pMethod))
		ListBox4->insertItem(QString(pMethod->GetName()));
}

void ClassViewDialog::RefreshProcedureList()
{
	ListBox5->clear();
	COMethod* pMethod;
	for(pMethod = m_pClass->GetFirstProcedure(); pMethod; pMethod = m_pClass->GetNextProcedure(pMethod))
		ListBox5->insertItem(QString(pMethod->GetName()));
}

void ClassViewDialog::GetUniqueMemberName(char* szBuff)
{
	// todo: make unique with constants and methods too
	const char* szPre = "New Member ";
	int nPreLength = strlen(szPre);
	int n = 0;
	strcpy(szBuff, szPre);
	while(true)
	{
		n++;
		if(n == 1)
			szBuff[nPreLength - 1] = '\0';
		else
        {
			szBuff[nPreLength - 1] = ' ';
			itoa(n, szBuff + nPreLength, 10);
		}
		if(!m_pClass->FindMember(szBuff))
			break;
	}
}

void ClassViewDialog::GetUniqueConstantName(char* szBuff)
{
	// todo: make unique with members and methods too
	const char* szPre = "New Constant ";
	int nPreLength = strlen(szPre);
	int n = 0;
	strcpy(szBuff, szPre);
	while(true)
	{
		n++;
		if(n == 1)
			szBuff[nPreLength - 1] = '\0';
		else
        {
			szBuff[nPreLength - 1] = ' ';
			itoa(n, szBuff + nPreLength, 10);
		}
		if(!m_pClass->FindConstant(szBuff))
			break;
	}
}

void ClassViewDialog::GetUniqueMethodName(char* szBuff)
{
	strcpy(szBuff, "New Method"); // todo: fix this method
}

// add button
void ClassViewDialog::slot_button1()
{
	switch(m_nSelectedList)
	{
	case 0: // Members
		{
			char szName[64];
			GetUniqueMemberName(szName);
			COVariable* pNewVar = new COVariable(szName, m_pClass, false, false);
			DeclViewDialog dialog(m_pCOProject, pNewVar, this, "Decl View", true);
			if(dialog.exec() == Accepted)
			{
				m_pClass->AddMember(pNewVar);
				m_pClass->GetFile()->SetModified(true);
			}
			else
				delete(pNewVar);
		}
		break;
	case 1: // Interfaces
		{
			QMessageBox::information(this, "Sorry", "Not implemented yet.");
		}
		break;
	case 2: // Constants
		{
			char szName[64];
			GetUniqueConstantName(szName);
			COConstant* pNewConst = new COConstant(szName, 0);
			ConstantViewDialog dialog(m_pCOProject, pNewConst, this, "Decl View");
			if(dialog.exec() == Accepted)
			{
				m_pClass->AddConstant(pNewConst);
				m_pClass->GetFile()->SetModified(true);
			}
			else
				delete(pNewConst);
		}
		break;
	case 3: // Methods
		{
			char szName[64];
			GetUniqueMethodName(szName);
			COMethod* pNewMethod = new COMethod(szName, m_pClass, false, m_pCOProject);
			m_pClass->AddMethod(pNewMethod);
			MethodViewDialog dialog(m_pCOProject, pNewMethod, this, "Method View", true);
			dialog.exec();
		}
		break;
	case 4: // Procedure
		{
			char szName[64];
			GetUniqueMethodName(szName);
			COMethod* pNewMethod = new COMethod(szName, m_pClass, true, m_pCOProject);
			m_pClass->AddProcedure(pNewMethod);
			MethodViewDialog dialog(m_pCOProject, pNewMethod, this, "Method View", true);
			dialog.exec();
		}
		break;
	default:
		GAssert(false, "invalid selection");
	}
	RefreshLists();
}

// delete button
void ClassViewDialog::slot_button2()
{
	QMessageBox::information(this, "Sorry", "Not implemented yet--todo: implement this");
	return;

	switch(m_nSelectedList)
	{
	case 0: // Members
		{
			// todo: make sure this member is not used by any parameter of any method in this class
			GAssert(false, "Not implemented yet");
		}
		break;
	case 1: // Interfaces
		{
			GAssert(false, "Not implemented yet");
		}
		break;
	case 2: // Constants
		{
			GAssert(false, "Not implemented yet");
		}
		break;
	case 3: // Methods
		{
			// todo: make sure this method is not called from anywhere in the Source
			GAssert(false, "Not implemented yet");
		}
		break;
	case 4: // Procedure
		{
			// todo: make sure this procedure is not called from anywhere in the Source
			GAssert(false, "Not implemented yet");
		}
		break;
	default:
		GAssert(false, "invalid selection");
	}
	RefreshLists();
}

// edit button
bool ClassViewDialog::EditItem()
{
	bool bOK = false;
	switch(m_nSelectedList)
	{
	case 0: // Members
		{
			COVariable* pVariable = GetSelectedMember();
			if(!pVariable)
			{
				QMessageBox::information(this, "Nothing selected", "Select something first");
				return false;
			}
			DeclViewDialog dialog(m_pCOProject, pVariable, this, "Decl View", false);
			if(dialog.exec() == Accepted)
			{
				m_pClass->GetFile()->SetModified(true);
				bOK = true;
			}
		}
		break;
	case 1: // Interfaces
		{
			QMessageBox::information(this, "Sorry", "Not implemented yet.");
		}
		break;
	case 2: // Constants
		{
			COConstant* pConstant = GetSelectedConstant();
			if(!pConstant)
			{
				QMessageBox::information(this, "Nothing selected", "Select something first");
				return false;
			}
			ConstantViewDialog dialog(m_pCOProject, pConstant, this, "Constant View");
			if(dialog.exec() == Accepted)
			{
				m_pClass->GetFile()->SetModified(true);
				bOK = true;
			}
		}
		break;
	case 3: // Methods
		{
			COMethod* pMethod = GetSelectedMethod();
			if(!pMethod)
			{
				QMessageBox::information(this, "Nothing selected", "Select something first");
				return false;
			}
			MethodViewDialog dialog(m_pCOProject, pMethod, this, "Method View", false);
			dialog.exec();
		}
		break;
	case 4: // Procedure
		{
			COMethod* pMethod = GetSelectedProcedure();
			if(!pMethod)
			{
				QMessageBox::information(this, "Nothing selected", "Select something first");
				return false;
			}
			MethodViewDialog dialog(m_pCOProject, pMethod, this, "Procedure View", false);
			dialog.exec();
		}
		break;
	default:
		GAssert(false, "invalid selection");
	}
	RefreshLists();
	return bOK;
}

void ClassViewDialog::slot_button3()
{
	EditItem();
}

QListBox* ClassViewDialog::GetSelectedListBox()
{
	switch(m_nSelectedList)
	{
		case 0: return ListBox1; // Members
		case 1: return ListBox2; // Interfaces
		case 2: return ListBox5_2; // Constants
		case 3: return ListBox4; // Methods
		case 4: return ListBox5; // Procedures
	}
	GAssert(false, "No listbox is selected");
	return NULL;
}

void ClassViewDialog::SetFocus(int n)
{
	GAssert(n >= 0 && n <= 4, "Out of range");
	QListBox* pListBox;

	if(n == m_nSelectedList)
		return;

	// Clear selection in the old list
	if(m_nSelectedList >= 0 && m_nSelectedList <= 4)
	{
		pListBox = GetSelectedListBox();
		pListBox->clearSelection();
	}

	// Move the arrow to the new list
	m_nSelectedList = n;
	pListBox = GetSelectedListBox();
	PixmapLabel1->move(pListBox->pos().x() + 110, pListBox->pos().y() - 30);
	if(pListBox->count() > 0)
		pListBox->setSelected(0, true);
}

// who uses me button
void ClassViewDialog::slot_button4()
{
    qWarning( "ClassViewDialogBase::slot_button4(): Not implemented yet!" );
}

// close button
void ClassViewDialog::slot_button5()
{
	accept();
}

void ClassViewDialog::slot_list1clicked()
{
	SetFocus(0);
}

void ClassViewDialog::slot_list2clicked()
{
	SetFocus(1);
}

void ClassViewDialog::slot_list4clicked()
{
	SetFocus(3);
}

void ClassViewDialog::slot_list5clicked()
{
	SetFocus(4);
}

void ClassViewDialog::EditName()
{
	// Move the arrow to the name
	PixmapLabel1->move(TextLabel1_2->pos().x() + 110, TextLabel1_2->pos().y() - 30);
	GetStringDialog dialog(this, "Enter a name for this class", m_pClass->GetName());
	if(dialog.exec() == Accepted)
	{
		QString s = dialog.LineEdit1->text();
		m_pClass->SetName(s.latin1());
		TextLabel2_2->setText(s);
		m_pClass->GetFile()->SetModified(true);
	}
}

void ClassViewDialog::arrowUp()
{
	QListBox* pListBox = GetSelectedListBox();
	int n = pListBox->currentItem() - 1;
	if(n >= 0 && n < (int)pListBox->count())
		pListBox->setSelected(n, true);
}

void ClassViewDialog::arrowDown()
{
	QListBox* pListBox = GetSelectedListBox();
	int n = pListBox->currentItem() + 1;
	if(n >= 0 && n < (int)pListBox->count())
		pListBox->setSelected(n, true);
}

void ClassViewDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'T':	EditName();		break;
		case 'B':	SetFocus(0);	break;
		case 'R':	SetFocus(1);	break;
		case 'C':	SetFocus(2);	break;
		case 'F':	SetFocus(3);	break;
		case 'S':	SetFocus(4);	break;
		case 'A':	slot_button1();	break;
		case 'D':	slot_button2();	break;
		case 'E':	slot_button3();	break;
		case 'W':	slot_button4();	break;
		case Key_Escape: // fall through
		case 'Q':	slot_button5();	break;
		case Key_Up:	arrowUp();	break;
		case Key_Down:	arrowDown();break;
	}
}

bool ClassViewDialog::event(QEvent* e)
{
	if(m_bNeedToChangeName && e->type() == QEvent::Show)
	{
		EditName();
		m_bNeedToChangeName = false;
	}
	return ClassViewDialogBase::event(e);
}
