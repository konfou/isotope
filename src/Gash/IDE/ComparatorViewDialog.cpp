/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "ComparatorViewDialog.h"
#include <qlistbox.h>
#include "../CodeObjects/CodeObjects.h"
#include <qmessagebox.h>
#include "GetStringDialog.h"

ComparatorViewDialog::ComparatorViewDialog(COProject* pCOProject, QWidget* parent, const char* name)
: ComparatorViewDialogBase(parent, name, true, 0)
{
    m_pCOProject = pCOProject;
    m_pSelectedComparator = NULL;
	RefreshComparatorList();
}

ComparatorViewDialog::~ComparatorViewDialog()
{

}

void ComparatorViewDialog::RefreshComparatorList()
{
	ListBox6->clear();
    COVariable* pComparator;
    int n;
    for(n = 0; n < COMPARATOR_TABLE_SIZE; n++)
    {
        pComparator = m_pCOProject->m_pComparatorDecls[n];
        ListBox6->insertItem(QString(pComparator->GetName()));
    }
	ListBox6->setCurrentItem(0);
	SetFocus(0);
}

// Change Library
void ComparatorViewDialog::slot_ListBox6SelChange()
{
    int nSel = ListBox6->currentItem();
    if(nSel < 0 || nSel >= COMPARATOR_TABLE_SIZE)
        return;
    m_pSelectedComparator = m_pCOProject->m_pComparatorDecls[nSel];
}

void ComparatorViewDialog::arrowUp()
{
	int n = ListBox6->currentItem() - 1;
	if(n >= 0 && n < (int)ListBox6->count())
		ListBox6->setSelected(n, true);
}

void ComparatorViewDialog::arrowDown()
{
	int n = ListBox6->currentItem() + 1;
	if(n >= 0 && n < (int)ListBox6->count())
		ListBox6->setSelected(n, true);
}

void ComparatorViewDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case Key_Escape: // fall through
		case 'Q':	reject(); break;
		case Key_Space: accept(); break;
		case Key_Up:	arrowUp();	break;
		case Key_Down:	arrowDown();break;
	}
}

