/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __CALLVIEWDIALOG_H__
#define __CALLVIEWDIALOG_H__

#include "CallViewDialogBase.h"

class COFile;
class COInstruction;
class COClass;
class COMethod;
class COVariable;
class COCall;
class COProject;
class GPointerArray;

class CallViewDialog : public CallViewDialogBase
{ 
public:
	CallViewDialog(COProject* pCOProject, COMethod* pMethod, COInstruction* pInstruction, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
	virtual ~CallViewDialog();

	// Slots
	virtual void slot_button1();
	virtual void slot_button4();
	virtual void slot_list1SelChange();
	virtual void slot_list2SelChange();
	virtual void slot_list3SelChange();
	virtual void slot_list5SelChange();
	virtual void slot_list1clicked();
	virtual void slot_list2clicked();
	virtual void slot_list3clicked();
	virtual void slot_list5clicked();

protected:
	// Members
	COInstruction* m_pInstruction;
	COMethod* m_pMethod;
	COProject* m_pCOProject;
	COClass* m_pSelectedClass;
	COMethod* m_pSelectedMethod;
	GPointerArray* m_pParams;
	int m_nSelLibrary;
	int m_nSelClass;
	int m_nSelMethod;
	int m_nSelectedList;

	// Refresh Methods
	void RefreshLibraryList();
	void RefreshClassList();
	void RefreshMethodList();
	void RefreshParamList();

	// Get Methods
	COFile* GetSelectedLibrary();
	COClass* GetSelectedClass();
	COMethod* GetSelectedMethod();
	QListBox* GetSelectedListBox();

	// Key Handling Methods
	void keyPressEvent(QKeyEvent* e);
	void arrowUp();
	void arrowDown();

	// Misc Methods
	void SetInitialSelections();
	void BuildParamsArray();
	void SelectNextParamDecl();
	void SetSelectedParameter(COVariable* pVariable);
	void SetFocus(int n);
};

#endif // __CALLVIEWDIALOG_H__
