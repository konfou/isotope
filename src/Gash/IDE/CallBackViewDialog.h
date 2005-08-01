/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __CALLBACKVIEWDIALOG_H__
#define __CALLBACKVIEWDIALOG_H__

#include "CallBackDialogBase.h"

class GPointerArray;
class COProject;
class COMethod;
class COInstruction;
class COVariable;

class CallBackViewDialog : public CallBackViewDialogBase
{ 
public:
    CallBackViewDialog(COProject* pCOProject, COMethod* pMethod, COInstruction* pInstruction, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~CallBackViewDialog();

	virtual void slot_button3();
	virtual void slot_list5selChanged();
	virtual void slot_list5clicked();
    virtual void slot_button4_2();
	virtual void slot_button11();
	virtual void slot_button18();

protected:
	GPointerArray* m_pParams;
	COInstruction* m_pInstruction;
	COMethod* m_pMethod;
	COProject* m_pCOProject;
	int m_nSelectedList;

	void RefreshParamList();
	void BuildParamsArray();
	void SetFocus(int n);
	void arrowUp();
	void arrowDown();
	void keyPressEvent(QKeyEvent* e);
	void EditName();
};

#endif // __CALLBACKVIEWDIALOG_H__
