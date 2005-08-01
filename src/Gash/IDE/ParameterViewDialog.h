/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __ParameterVIEWDIALOG_H__
#define __ParameterVIEWDIALOG_H__

#include "ParameterViewDialogBase.h"
#include "../CodeObjects/Operator.h"

class GPointerArray;
class COProject;
class COMethod;
class COExpression;
class COVariable;
class COInstruction;
class COConstant;

class ParameterViewDialog : public ParamViewDialogBase
{ 
public:
    ParameterViewDialog(COProject* pCOProject, COMethod* pMethod, COInstruction* pInstruction, COExpression* pParam, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~ParameterViewDialog();

	virtual void slot_list5clicked();
	virtual void slot_list6clicked();
	virtual void slot_list5_2clicked();
	virtual void slot_list7clicked();
	virtual void slot_list8clicked();
	virtual void slot_list6doubleClick();
	virtual void slot_list5_2doubleClick();
	virtual void slot_list7doubleClick();
	virtual void slot_list8doubleClick();
	virtual void slot_list6_2doubleclick();
	virtual void slot_list7_2doubleclick();
    virtual void slot_button3();
    virtual void slot_button4();
	virtual void slot_button5();
    virtual void slot_button5_2();
    virtual void slot_button6_2();
    virtual void slot_button7();
    virtual void slot_button8();
    virtual void slot_button9();
    virtual void slot_button10();
    virtual void slot_button11();
    virtual void slot_button12();
    virtual void slot_button13();
	virtual void slot_button14();
	virtual void slot_button15();

protected:
	// Members
	GPointerArray* m_pLocals;
	GPointerArray* m_pParamStack;
	COExpression* m_pParam;
	COMethod* m_pMethod;
	COInstruction* m_pInstruction;
	COProject* m_pCOProject;
	int m_nSelectedList;

	// Refreshers
	void RefreshParamStack();
	void RefreshFieldList();
    void RefreshDeclList();
	void RefreshConstantList();

	// Selection Getters
	QListBox* GetSelectedListBox();
	COVariable* GetSelectedLocalParam();
	COVariable* GetSelectedField();
	COVariable* GetSelectedLocalDecl();
	COVariable* GetSelectedObjectMember();
	COConstant* GetSelectedLocalConstant();
	COConstant* GetSelectedConstant();

	// Misc
	void SetFocus(int n);
	void arrowUp();
	void arrowDown();
	void keyPressEvent(QKeyEvent* e);
	void ApplyOperator(COOperator::Operator eOperator);
	void GetValue();
    void AddValue();
	void GetString();
	void AddString();
};

#endif // __ParameterVIEWDIALOG_H__
