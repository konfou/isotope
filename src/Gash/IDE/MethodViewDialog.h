/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __METHODVIEWDIALOG_H__
#define __METHODVIEWDIALOG_H__

#include "MethodViewDialogBase.h"
#include <qlistview.h>
#include <qstring.h>
class COMethod;
class COVariable;
class COCall;
class COInstruction;
class COProject;

// ***************************************************************************

class MyListViewItem : public QListViewItem
{
protected:
	COInstruction* m_pInstruction;
	MyListViewItem* m_pPrevItem;

public:
	MyListViewItem(MyListViewItem* pParentItem, MyListViewItem* pPrev, COInstruction* pInstruction) 
		: QListViewItem(pParentItem, pPrev)
	{
		m_pInstruction = pInstruction;
		m_pPrevItem = pPrev;
	}

	MyListViewItem(QListView* pParent, MyListViewItem* pPrev, COInstruction* pInstruction) 
		: QListViewItem(pParent, pPrev)
	{
		m_pInstruction = pInstruction;
		m_pPrevItem = pPrev;
	}

	virtual ~MyListViewItem()
	{
		MyListViewItem* pNextSib = (MyListViewItem*)nextSibling();
		if(pNextSib)
			pNextSib->SetPrevSibling(m_pPrevItem);
	}

	virtual QString text(int column) const;
	COInstruction* GetInstruction() { return m_pInstruction; }
	void SetPrevSibling(MyListViewItem* pPrev) { m_pPrevItem = pPrev; }
	MyListViewItem* GetPrevSibling() { return m_pPrevItem; }
};

// ***************************************************************************

class MethodViewDialog : public MethodViewDialogBase
{ 
public:
    MethodViewDialog(COProject* pCOProject, COMethod* pMethod, QWidget* parent, const char* name, bool bNeedToChangeName);
    virtual ~MethodViewDialog();

    virtual void slot_button1();
    virtual void slot_button3();
    virtual void slot_button4();
    virtual void slot_button5();
    virtual void slot_button6();
	virtual void slot_button7_2();
	virtual void slot_list1Clicked();
	virtual void slot_listView1Clicked();

protected:
	COMethod* m_pMethod;
	COProject* m_pCOProject;
	int m_nSelection;
    MyListViewItem* m_pOldSelectedInstruction;
	bool m_bNeedToChangeName;

	void RefreshAll();
	void RefreshInstructions();
	void RefreshParams();
	MyListViewItem* AddInstruction(MyListViewItem* pParent, MyListViewItem* pPrev, COInstruction* pInstr);
	virtual void keyPressEvent(QKeyEvent* e);
	bool EditInstruction(MyListViewItem* pItem, bool bNeedToChangeName = false);
	void arrowUp();
	void arrowDown();
	void arrowLeft();
	void arrowRight();
	void EditName();
	void EditParams();
	void EditInstructions();
	bool GetSelection(MyListViewItem** ppItem, MyListViewItem** ppPrevItem, int* pnPos, COInstruction** ppParentInstruction);
	virtual bool event(QEvent* e);
};

#endif // __METHODVIEWDIALOG_H__
