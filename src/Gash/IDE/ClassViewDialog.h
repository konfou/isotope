/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __CLASSVIEWDIALOG_H__
#define __CLASSVIEWDIALOG_H__

#include "ClassViewDialogBase.h"

class COClass;
class COVariable;
class COMethod;
class COProject;
class QKeyEvent;
class COConstant;

class ClassViewDialog : public ClassViewDialogBase
{
public:
    ClassViewDialog(COProject* pCOProject, COClass* pClass, QWidget* parent, const char* name, bool bNeedToChangeName);
    virtual ~ClassViewDialog();

	bool m_bDirty;

    virtual void slot_button1();
    virtual void slot_button2();
    virtual void slot_button3();
    virtual void slot_button4();
    virtual void slot_button5();
    virtual void slot_list1clicked();
    virtual void slot_list2clicked();
    virtual void slot_list4clicked();
    virtual void slot_list5clicked();
	QListBox* GetSelectedListBox();
	void arrowUp();
	void arrowDown();

protected:
	COClass* m_pClass;
	COProject* m_pCOProject;
	int m_nSelectedList;
	bool m_bNeedToChangeName;

	COVariable* GetSelectedMember();
	COConstant* GetSelectedConstant();
	COMethod* GetSelectedMethod();
	COMethod* GetSelectedProcedure();
	void SetFocus(int n);
	void RefreshLists();
	void RefreshMemberList();
	void RefreshConstantList();
	void RefreshInterfaceList();
	void RefreshMethodList();
	void RefreshProcedureList();
	virtual void keyPressEvent(QKeyEvent* e);
	bool EditItem();
	void GetUniqueMemberName(char* szBuff);
	void GetUniqueConstantName(char* szBuff);
	void GetUniqueMethodName(char* szBuff);
	void EditName();
	virtual bool event(QEvent* e);
};

#endif // __CLASSVIEWDIALOG_H__
