/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "DeclViewDialog.h"
#include <qlistbox.h>
#include "../CodeObjects/CodeObjects.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include "GetStringDialog.h"

#define NUMBER_OF_BUILT_IN_CLASSES 2

DeclViewDialog::DeclViewDialog(COProject* pCOProject, COVariable* pVariable, QWidget* parent, const char* name, bool bNeedToChangeName)
: DeclViewDialogBase(parent, name, true, 0)
{
	m_bNeedToChangeName = bNeedToChangeName;
	m_pCOProject = pCOProject;
	m_pVariable = pVariable;
	m_nSelectedList = -1;
	TextLabel1_3->setText(QString(pVariable->GetName()));
	CheckBox1->setChecked(pVariable->IsVarReadOnly());
	CheckBox2->setChecked(pVariable->IsObjReadOnly());
	RefreshLibraryList();
}

DeclViewDialog::~DeclViewDialog()
{

}

void DeclViewDialog::RefreshLibraryList()
{
	ListBox1->clear();
	ListBox1->insertItem(QString("This Source")); // todo: un-magic-string this line
	COHive* pLibraries = m_pCOProject->GetLibraries();
	COFile* pLibrary;
	char szLibraryName[256];
	for(pLibrary = pLibraries->GetFirstFile(); pLibrary; pLibrary = pLibraries->GetNextFile(pLibrary))
	{
		pLibrary->GetFilenameNoPath(szLibraryName);
		ListBox1->insertItem(QString(szLibraryName));
	}
	ListBox1->setCurrentItem(0);
	SetFocus(0);
}

// Change Library
void DeclViewDialog::slot_list1SelChange()
{
	RefreshClassList();
}

void DeclViewDialog::RefreshClassList()
{
	ListBox2->clear();
	COFile* pLibrary = GetSelectedLibrary();
	if(pLibrary == NULL) // If the library is "This Source"
	{
		// Add Built-in classes
        ListBox2->insertItem(QString(m_pCOProject->m_pInteger->GetName()));
        ListBox2->insertItem(QString(m_pCOProject->m_pWrapper->GetName()));

        // Add Source classes
        COFileSet* pSource = m_pCOProject->GetSource();
		COFile* pFile;
		for(pFile = pSource->GetFirstFile(); pFile; pFile = pSource->GetNextFile(pFile))
		{
			COClass* pClass;
			for(pClass = pFile->GetFirstClass(); pClass; pClass = pFile->GetNextClass(pClass))
				ListBox2->insertItem(QString(pClass->GetName()));
		}
	}
	else
	{
		COClass* pClass;
		for(pClass = pLibrary->GetFirstClass(); pClass; pClass = pLibrary->GetNextClass(pClass))
			ListBox2->insertItem(QString(pClass->GetName()));
	}
	ListBox2->setCurrentItem(0);
}

COClass* DeclViewDialog::GetSelectedClass()
{
	int nSel = ListBox2->currentItem();
    if(nSel < 0)
        return NULL;
	COFile* pLibrary = GetSelectedLibrary();
	if(pLibrary == NULL) // If the library is "This Source"
	{
        // Check for build in classes
        if(nSel < NUMBER_OF_BUILT_IN_CLASSES)
        {
            if(nSel == 0)
                return m_pCOProject->m_pInteger;
            if(nSel == 1)
                return m_pCOProject->m_pWrapper;
        }

        // Find source class
		COFile* pFile;
		COFileSet* pSource = m_pCOProject->GetSource();
		int nCount = NUMBER_OF_BUILT_IN_CLASSES;
		for(pFile = pSource->GetFirstFile(); pFile; pFile = pSource->GetNextFile(pFile))
		{
			COClass* pClass;
			for(pClass = pFile->GetFirstClass(); pClass; pClass = pFile->GetNextClass(pClass))
			{
				if(nCount == nSel)
					return pClass;
				nCount++;
			}
		}
		GAssert(false, "not that many classes in this Source");
	}
	else
	{
		int nCount = 0;
		COClass* pClass;
		for(pClass = pLibrary->GetFirstClass(); pClass; pClass = pLibrary->GetNextClass(pClass))
		{
			if(nCount == nSel)
				return pClass;
			nCount++;
		}
		GAssert(false, "not that many classes in this library");
	}
	GAssert(false, "it should never get to here");
	return NULL;
}

COFile* DeclViewDialog::GetSelectedLibrary()
{
	int nSel = ListBox1->currentItem();
	if(nSel < 1) // The first item is "This Source", which is NULL
		return NULL;
	COFile* pFile;
	COHive* pLibraries = m_pCOProject->GetLibraries();
	int n = 1;
	for(pFile = pLibraries->GetFirstFile(); pFile; pFile = pLibraries->GetNextFile(pFile))
	{
		if(n == nSel)
			return(pFile);
		n++;
	}
	GAssert(false, "Not that many libraries");
	return NULL;
}

QListBox* DeclViewDialog::GetSelectedListBox()
{
	switch(m_nSelectedList)
	{
		case 0: return ListBox1;
		case 1: return ListBox2;
		case 2: return NULL; // 2 is the LineEdit for entering a value
	}
	GAssert(false, "No listbox is selected");
	return NULL;
}

void DeclViewDialog::SetFocus(int n)
{
	GAssert(n >= 0 && n <= 2, "Out of range");
	QListBox* pListBox;

	if(n == m_nSelectedList)
		return;

	// Move the arrow to the new list
	m_nSelectedList = n;
	if(n == 2)
	{
		PixmapLabel1->move(TextLabel1_3->pos().x() + 100, TextLabel1_3->pos().y() - 30);
	}
	else
	{
		pListBox = GetSelectedListBox();
		PixmapLabel1->move(pListBox->pos().x() + 100, pListBox->pos().y() - 30);
		if(pListBox->count() > 0)
		{
			if(pListBox->currentItem() < 0 || !pListBox->isSelected(pListBox->currentItem()))
				pListBox->setSelected(0, true);
		}
	}
	setFocus();
}

void DeclViewDialog::arrowUp()
{
	QListBox* pListBox = GetSelectedListBox();
	if(pListBox)
	{
		int n = pListBox->currentItem() - 1;
		if(n >= 0 && n < (int)pListBox->count())
			pListBox->setSelected(n, true);
	}
}

void DeclViewDialog::arrowDown()
{
	QListBox* pListBox = GetSelectedListBox();
	if(pListBox)
	{
		int n = pListBox->currentItem() + 1;
		if(n >= 0 && n < (int)pListBox->count())
			pListBox->setSelected(n, true);
	}
}

// OK button
void DeclViewDialog::slot_button1()
{
	// Get the data from the dialog
	COClass* pClass = GetSelectedClass();
	if(!pClass)
	{
        QMessageBox::information(this, "No class selected", "First select a class");
		return;
	}
	QString sName = TextLabel1_3->text();
	if(sName.length() < 1)
	{
        QMessageBox::information(this, "No name", "First give it a name");
		return;
	}

	// Make the changes
	m_pVariable->SetType(pClass);
	m_pVariable->SetName(sName.latin1());
	m_pVariable->SetExpReadOnly(CheckBox1->isChecked(), CheckBox2->isChecked());

	// todo: find all the places in the method that use this decl and make sure they can accept the new type (or else you can make non-compiling code!)

	// Close the dialog
	accept();
}

void DeclViewDialog::EditName()
{
	// Move the arrow to the name
    SetFocus(2);
	GetStringDialog dialog(this, "Enter a name for this variable", m_pVariable->GetName());
	if(dialog.exec() == Accepted)
	{
		QString s = dialog.LineEdit1->text();
		TextLabel1_3->setText(s);
	}
}

void DeclViewDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'A':	SetFocus(0); break;
		case 'C':	SetFocus(1); break;
		case 'N':	EditName(); break;
		case Key_Escape: // fall through
		case 'Q':	reject(); break;
		case Key_Up:	arrowUp(); break;
		case Key_Down:	arrowDown(); break;
		case Key_Space: slot_button1(); break;
	}
}

bool DeclViewDialog::event(QEvent* e)
{
	if(m_bNeedToChangeName && e->type() == QEvent::Show)
	{
		EditName();
		m_bNeedToChangeName = false;
	}
	return DeclViewDialogBase::event(e);
}
