/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __ConstantViewDialog_H__
#define __ConstantViewDialog_H__

#include "ConstantViewDialogBase.h"

class COProject;
class COConstant;
class COClass;

class ConstantViewDialog : public ConstantViewDialogBase
{
public:
    ConstantViewDialog(COProject* pCOProject, COConstant* pConstant, QWidget* parent = 0, const char* name = 0);
    ~ConstantViewDialog();

    virtual void slot_button6();
    virtual void slot_radio1toggled();
    virtual void slot_radio2toggled();

protected:
	COProject* m_pCOProject;
	COConstant* m_pConstant;
	COClass* m_pClass;
	int m_nSelected;

	void SetFocus(int n);
	void arrowUp();
	void arrowDown();
	void keyPressEvent(QKeyEvent* e);
	void EditName();
	void EditValue();
};

#endif // __ConstantViewDialog_H__
