/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __DECLVIEWDIALOG_H__
#define __DECLVIEWDIALOG_H__

#include "DeclViewDialogBase.h"

class COProject;
class COClass;
class COFile;
class COVariable;

class DeclViewDialog : public DeclViewDialogBase
{
public:
	DeclViewDialog(COProject* pCOProject, COVariable* pVariable, QWidget* parent, const char* name, bool bNeedToChangeName);
    virtual ~DeclViewDialog();

    virtual void slot_button1();
	virtual void slot_list1SelChange();

protected:
	COProject* m_pCOProject;
	COVariable* m_pVariable;
	int m_nSelectedList;
	bool m_bNeedToChangeName;

	void RefreshLibraryList();
	void RefreshClassList();
	COClass* GetSelectedClass();
	COFile* GetSelectedLibrary();
	void SetFocus(int n);
	QListBox* GetSelectedListBox();
	void keyPressEvent(QKeyEvent* e);
	void arrowUp();
	void arrowDown();
    void EditName();
	virtual bool event(QEvent* e);
};

#endif // __DECLVIEWDIALOG_H__
