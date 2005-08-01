/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "ParameterViewDialog.h"
#include <qlistbox.h>
#include "../CodeObjects/CodeObjects.h"
#include "../CodeObjects/VarRef.h"
#include "../../GClasses/GnuSDK.h"
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include "GetStringDialog.h"

ParameterViewDialog::ParameterViewDialog(COProject* pCOProject, COMethod* pMethod, COInstruction* pInstruction, COExpression* pParam, QWidget* parent, const char* name, bool modal, WFlags fl)
 : ParamViewDialogBase(parent, name, modal, fl)
{
	m_pParam = pParam;
	m_pCOProject = pCOProject;
	m_pMethod = pMethod;
	m_pInstruction = pInstruction;
	m_pLocals = new GPointerArray(64);
	m_pParamStack = new GPointerArray(64);
/*	if(!pMethod->GetLocalVariablesArray(m_pLocals, pInstruction, m_pCOProject))
	{
		GAssert(false, "instruction not found in this method");
		// todo: handle this
	}*/
	m_nSelectedList = -1;
	RefreshDeclList();
	RefreshParamStack();
}

ParameterViewDialog::~ParameterViewDialog()
{
	delete(m_pLocals);

	// Delete the Param stack
	int nCount = m_pParamStack->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((COExpression*)m_pParamStack->GetPointer(n));
	m_pParamStack = new GPointerArray(64);
}

void ParameterViewDialog::SetFocus(int n)
{
	GAssert(n >= 0  && n <= 8, "Out of range");
	QListBox* pListBox;

	if(n == m_nSelectedList)
		return;

	// Clear selection in the old list
	if(m_nSelectedList >= 0 && m_nSelectedList <= 6)
	{
		pListBox = GetSelectedListBox();
		pListBox->clearSelection();
	}

	// Move the arrow to the new list
	m_nSelectedList = n;
	if(n == 7)
	{
		PixmapLabel1->move(TextLabel1->pos().x() + 100, TextLabel1->pos().y() - 30);
	}
	else if(n == 8)
	{
		PixmapLabel1->move(TextLabel2->pos().x() + 100, TextLabel2->pos().y() - 30);
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
}

void ParameterViewDialog::RefreshParamStack()
{
	ListBox5->clear();
	char szBuff[256];
	int nCount = m_pParamStack->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COExpression* pParam = (COExpression*)m_pParamStack->GetPointer(n);
		pParam->ToString(szBuff);
		ListBox5->insertItem(QString(szBuff));
	}
	RefreshFieldList();
	RefreshConstantList();
}

QListBox* ParameterViewDialog::GetSelectedListBox()
{
	switch(m_nSelectedList)
	{
		case 0: return ListBox5;
		case 1: return ListBox6;
		case 2: return ListBox5_2;
		case 3: return ListBox7;
		case 4: return ListBox8;
		case 5: return ListBox6_2;
		case 6: return ListBox7_2;
		case 7: return NULL; // This is the integer value field
		case 8: return NULL; // This is the string value field
	}
	return NULL;
}

void ParameterViewDialog::RefreshFieldList()
{
    // todo: this only supports extended members.  Either make it support inherrited members or remove it.
    ListBox5_2->clear();
	int nCount = m_pParamStack->GetSize();
	if(nCount < 1)
		return;
	COExpression* pParam = (COExpression*)m_pParamStack->GetPointer(nCount - 1);
	COType* pType =pParam->GetType(m_pCOProject);
	GAssert(pType->GetTypeType() == COType::TT_CLASS, "todo: handle interfaces");
	COClass* pClass = (COClass*)pType;
	COVariable* pMember;
	for(pMember = pClass->GetFirstMember(); pMember; pMember = pClass->GetNextMember(pMember))
		ListBox5_2->insertItem(QString(pMember->GetName()));
}

void ParameterViewDialog::RefreshConstantList()
{
	ListBox7_2->clear();
	int nCount = m_pParamStack->GetSize();
	if(nCount < 1)
		return;
	COExpression* pParam = (COExpression*)m_pParamStack->GetPointer(nCount - 1);
	COType* pType = pParam->GetType(m_pCOProject);
	GAssert(pType->GetTypeType() == COType::TT_CLASS, "todo: handle interfaces");
	COClass* pClass = (COClass*)pType;
	COConstant* pConst;
	for(pConst = pClass->GetFirstConstant(); pConst; pConst = pClass->GetNextConstant(pConst))
		ListBox7_2->insertItem(QString(pConst->GetName()));
}

void ParameterViewDialog::RefreshDeclList()
{
	ListBox6->clear(); // Clear the Local Params list
	ListBox7->clear(); // Clear the Local Decls list
	ListBox8->clear(); // Clear the Object Members list
	ListBox6_2->clear(); // Clear the Local Constants list
	int nCount;
	int n;
	COVariable* pVariable;

	// Refresh Local Params List
	nCount = m_pMethod->GetParameterCount();
	for(n = 0; n < nCount; n++)
	{
		pVariable = m_pMethod->GetParameter(n);
		ListBox6->insertItem(QString(pVariable->GetName()));
	}

	// Refresh Local Decls list
	nCount = m_pLocals->GetSize();
	for(n = 0; n < nCount; n++)
	{
		pVariable = (COVariable*)m_pLocals->GetPointer(n);
		ListBox7->insertItem(QString(pVariable->GetName()));
	}

	// Refresh the Object Members List
	if(!m_pMethod->IsStatic())
	{
		COClass* pClass = m_pMethod->GetClass();
		if(pClass)
		{
            // todo: this only supports extended members.  Either make it support inherrited members or remove it.
			for(pVariable = pClass->GetFirstMember(); pVariable; pVariable = pClass->GetNextMember(pVariable))
				ListBox8->insertItem(QString(pVariable->GetName()));
		}
		else
		{
			GAssert(false, "The method must have a pointer to its class");
		}
	}

	// Refresh the Local Constants List
	if(!m_pMethod->IsStatic()) // todo: find a way to remove this condition so procedures can access named constants
	{
		COConstant* pConst;
		COClass* pClass = m_pMethod->GetClass();
		if(pClass)
		{
			for(pConst = pClass->GetFirstConstant(); pConst; pConst = pClass->GetNextConstant(pConst))
				ListBox6_2->insertItem(QString(pConst->GetName()));
		}
	}

    // Refresh the Value field
    TextLabel1->setText(QString("0"));
}

void ParameterViewDialog::slot_list5clicked()
{
	SetFocus(0);
}

void ParameterViewDialog::slot_list6clicked()
{
	SetFocus(1);
}

void ParameterViewDialog::slot_list5_2clicked()
{
	SetFocus(2);
}

void ParameterViewDialog::slot_list7clicked()
{
	SetFocus(3);
}

void ParameterViewDialog::slot_list8clicked()
{
	SetFocus(4);
}

COVariable* ParameterViewDialog::GetSelectedLocalParam()
{
	int nSel = ListBox6->currentItem();
	if(nSel < 0)
		return NULL;
	int nCount = m_pMethod->GetParameterCount();
	if(nSel >= nCount)
	{
		GAssert(false, "not that many local params");
		return NULL;
	}
	return m_pMethod->GetParameter(nSel);
}

COVariable* ParameterViewDialog::GetSelectedField()
{
	int nSel = ListBox5_2->currentItem();
	if(nSel < 0)
		return NULL;
	int nCount = m_pParamStack->GetSize();
	if(nCount < 1)
		return NULL;
	COExpression* pParam = (COExpression*)m_pParamStack->GetPointer(nCount - 1);
	COType* pType = pParam->GetType(m_pCOProject);
	GAssert(pType->GetTypeType() == COType::TT_CLASS, "todo: handle interfaces");
	COClass* pClass = (COClass*)pType;
    // todo: this only supports extended members.  Either make it support inherrited members or remove it.
	nCount = pClass->GetExtendedMemberCount();
	if(nSel >= nCount)
	{
		GAssert(false, "not that many fields");
		return NULL;
	}
	int n = 0;
	COVariable* pMember;
	for(pMember = pClass->GetFirstMember(); pMember; pMember = pClass->GetNextMember(pMember))
	{
		if(n == nSel)
			return pMember;
		n++;
	}
	GAssert(false, "Not found");
	return NULL;
}

COVariable* ParameterViewDialog::GetSelectedLocalDecl()
{
	int nSel = ListBox7->currentItem();
	if(nSel < 0)
		return NULL;
	int nCount = m_pLocals->GetSize();
	if(nSel >= nCount)
	{
		GAssert(false, "not that many local decls");
		return NULL;
	}
	return((COVariable*)m_pLocals->GetPointer(nSel));
}

COVariable* ParameterViewDialog::GetSelectedObjectMember()
{
	int nSel = ListBox8->currentItem();
	if(nSel < 0)
		return NULL;
	GAssert(!m_pMethod->IsStatic(), "Procedures can't access object members");
	COClass* pClass = m_pMethod->GetClass();
	if(!pClass)
	{
		GAssert(false, "The method must have a pointer to its class");
		return NULL;
	}
	int nCount = 0;
	COVariable* pVariable;
    // todo: this only supports extended members.  Either make it support inherrited members or remove it.
	for(pVariable = pClass->GetFirstMember(); pVariable; pVariable = pClass->GetNextMember(pVariable))
	{
		if(nSel == nCount)
			return pVariable;
		nCount++;
	}
	GAssert(false, "not that many object members");
	return NULL;
}

COConstant* ParameterViewDialog::GetSelectedLocalConstant()
{
	int nSel = ListBox6_2->currentItem();
	if(nSel < 0)
		return NULL;
	COClass* pClass = m_pMethod->GetClass();
	if(!pClass)
	{
		GAssert(false, "The method must have a pointer to its class");
		return NULL;
	}
	int n = 0;
	COConstant* pConstant;
	for(pConstant = pClass->GetFirstConstant(); pConstant; pConstant = pClass->GetNextConstant(pConstant))
	{
		if(n == nSel)
			return pConstant;
		n++;
	}
	GAssert(false, "Not found");
	return NULL;
}

COConstant* ParameterViewDialog::GetSelectedConstant()
{
	int nSel = ListBox7_2->currentItem();
	if(nSel < 0)
		return NULL;
	int nCount = m_pParamStack->GetSize();
	if(nCount < 1)
		return NULL;
	COExpression* pParam = (COExpression*)m_pParamStack->GetPointer(nCount - 1);
	COType* pType = pParam->GetType(m_pCOProject);
	GAssert(pType->GetTypeType() == COType::TT_CLASS, "todo: handle interfaces");
	COClass* pClass = (COClass*)pType;
	nCount = pClass->GetConstantCount();
	if(nSel >= nCount)
	{
		GAssert(false, "not that many constants");
		return NULL;
	}
	int n = 0;
	COConstant* pConstant;
	for(pConstant = pClass->GetFirstConstant(); pConstant; pConstant = pClass->GetNextConstant(pConstant))
	{
		if(n == nSel)
			return pConstant;
		n++;
	}
	GAssert(false, "Not found");
	return NULL;
}

void ParameterViewDialog::slot_list6doubleClick()
{
	COVariable* pVariable = GetSelectedLocalParam();
	if(!pVariable)
	{
        QMessageBox::information(this, "Nothing selected", "First select something to add to the stack");
		return;
	}
    COExpression* pParam = new COVarRef(pVariable);
    m_pParamStack->AddPointer(pParam);
	RefreshParamStack();
}

void ParameterViewDialog::slot_list5_2doubleClick()
{
	COVariable* pVariable = GetSelectedField();
	if(!pVariable)
	{
        QMessageBox::information(this, "Nothing selected", "First select something to add to the stack");
		return;
	}
    COExpression* pParam = new COVarRef(pVariable);
    m_pParamStack->AddPointer(pParam);
	RefreshParamStack();
}

void ParameterViewDialog::slot_list7doubleClick()
{
	COVariable* pVariable = GetSelectedLocalDecl();
	if(!pVariable)
	{
        QMessageBox::information(this, "Nothing selected", "First select something to add to the stack");
		return;
	}
    COExpression* pParam = new COVarRef(pVariable);
    m_pParamStack->AddPointer(pParam);
	RefreshParamStack();
}

void ParameterViewDialog::slot_list8doubleClick()
{
	COVariable* pVariable2 = GetSelectedObjectMember();
	if(!pVariable2)
	{
        QMessageBox::information(this, "Nothing selected", "First select something to add to the stack");
		return;
	}
	if(m_pMethod->GetParameterCount() < 1)
	{
		GAssert(false, "No This pointer");
		return;
	}
	COVariable* pVariable1 = m_pMethod->GetParameter(0);
	GAssert(stricmp(pVariable1->GetName(), VAL_THIS) == 0, "First parameter not named 'This'");
	COExpression* pLeft = new COVarRef(pVariable1);
	COExpression* pRight = new COVarRef(pVariable2);
    COExpression* pParam = new COOperator(pLeft, COOperator::OP_DOT, pRight);
    m_pParamStack->AddPointer(pParam);
	RefreshParamStack();
}

void ParameterViewDialog::slot_list6_2doubleclick()
{
/*	COConstant* pConstant = GetSelectedLocalConstant();
	if(!pConstant)
	{
        QMessageBox::information(this, "Nothing selected", "First select something to add to the stack");
		return;
	}
	if(m_pMethod->GetParameterCount() < 1)
	{
		GAssert(false, "No This pointer");
		return;
	}
	COVariable* pVariable1 = m_pMethod->GetParameter(0);
	GAssert(stricmp(pVariable1->GetName(), VAL_THIS) == 0, "First parameter not named 'This'");
	COParam* pLeft = new COVarRef(pVariable1);
	COParam* pRight = new COParam(pConstant);
    COParam* pParam = new COParam(pLeft, '.', pRight);
	m_pParamStack->AddPointer(pParam);
	RefreshParamStack();*/
}

void ParameterViewDialog::slot_list7_2doubleclick()
{
/*	COConstant* pConstant = GetSelectedConstant();
	if(!pConstant)
	{
        QMessageBox::information(this, "Nothing selected", "First select something to add to the stack");
		return;
	}
	COExpression* pParam = new COParam(pConstant);
	m_pParamStack->AddPointer(pParam);
	RefreshParamStack();*/
}

void ParameterViewDialog::GetValue()
{
	SetFocus(7);
	GetStringDialog dialog(this, "Enter an Integer value", "0");
	if(dialog.exec() == Accepted)
	{
		const char* szValue = dialog.LineEdit1->text().latin1();
		int n;
		for(n = 0; szValue[n] != '\0'; n++)
		{
			if(szValue[n] < '0' && szValue[n] > '9' && szValue[n] != '-')
			{
				QMessageBox::information(this, "That's not an integer", "Only integer values are allowed here");
				return;
			}
		}
        char szTmp[64];
        int nValue = atoi(szValue);
        itoa(nValue, szTmp, 10);
        TextLabel1->setText(QString(szTmp));
        AddValue();
	}
}

void ParameterViewDialog::GetString()
{
	SetFocus(8);
	GetStringDialog dialog(this, "Enter a String", "");
	if(dialog.exec() == Accepted)
	{
		const char* szValue = dialog.LineEdit1->text().latin1();
		int n;
		for(n = 0; szValue[n] != '\0'; n++)
		{
			if(szValue[n] == '\'')
			{
				QMessageBox::information(this, "Illegal character", "Sorry, apostrophes are not allowed in strings");
				return;
			}
		}
        TextLabel2->setText(QString(szValue));
        AddString();
	}
}

void ParameterViewDialog::AddValue()
{
    int nValue = atoi(TextLabel1->text().latin1());
    COExpression* pParam = new COConstInt(nValue);
    m_pParamStack->AddPointer(pParam);
	RefreshParamStack();
}

void ParameterViewDialog::AddString()
{
    const char* szString = TextLabel2->text().latin1();
    COExpression* pParam = new COConstString(szString);
    m_pParamStack->AddPointer(pParam);
	RefreshParamStack();
}

void ParameterViewDialog::arrowUp()
{
	QListBox* pListBox = GetSelectedListBox();
	if(pListBox)
	{
		int n = pListBox->currentItem() - 1;
		if(n >= 0 && n < (int)pListBox->count())
			pListBox->setSelected(n, true);
	}
}

void ParameterViewDialog::arrowDown()
{
	QListBox* pListBox = GetSelectedListBox();
	if(pListBox)
	{
		int n = pListBox->currentItem() + 1;
		if(n >= 0 && n < (int)pListBox->count())
			pListBox->setSelected(n, true);
	}
}

// Add Button
void ParameterViewDialog::slot_button14()
{
	if(m_nSelectedList < 1 || m_nSelectedList > 8)
	{
        QMessageBox::information(this, "Nothing Selected", "First select something to add to the stack.");
		return;
	}
	switch(m_nSelectedList)
	{
		case 1: slot_list6doubleClick(); break;
		case 2: slot_list5_2doubleClick(); break;
		case 3: slot_list7doubleClick(); break;
		case 4: slot_list8doubleClick(); break;
		case 5: slot_list6_2doubleclick(); break;
		case 6: slot_list7_2doubleclick(); break;
        case 7: AddValue(); break;
		case 8: AddString(); break;
	}
}

// Remove Button
void ParameterViewDialog::slot_button15()
{
	int nSel = ListBox5->currentItem();
	if(nSel < 0)
	{
        QMessageBox::information(this, "Nothing Selected", "First select something to remove from the stack");
		return;
	}
	int nCount = m_pParamStack->GetSize();
	if(nCount <= nSel)
	{
		GAssert(false, "stack out of sync with display");
		return;
	}
	delete((COExpression*)m_pParamStack->GetPointer(nSel));
	m_pParamStack->DeleteCell(nSel);
	RefreshParamStack();
}

void ParameterViewDialog::slot_button3() { ApplyOperator(COOperator::OP_DOT); }
void ParameterViewDialog::slot_button4() {  }
void ParameterViewDialog::slot_button5_2() { ApplyOperator(COOperator::OP_PLUS); }
void ParameterViewDialog::slot_button6_2() { ApplyOperator(COOperator::OP_MINUS); }
void ParameterViewDialog::slot_button7() { ApplyOperator(COOperator::OP_TIMES); }
void ParameterViewDialog::slot_button8() { ApplyOperator(COOperator::OP_DIVIDE); }
void ParameterViewDialog::slot_button9() { ApplyOperator(COOperator::OP_MIN); }
void ParameterViewDialog::slot_button10() { ApplyOperator(COOperator::OP_MAX); }
void ParameterViewDialog::slot_button11() { ApplyOperator(COOperator::OP_AND); }
void ParameterViewDialog::slot_button12() { ApplyOperator(COOperator::OP_OR); }
void ParameterViewDialog::slot_button13() { ApplyOperator(COOperator::OP_XOR); }

void ParameterViewDialog::ApplyOperator(COOperator::Operator eOperator)
{
/*	// Check for underflow
	int nCount = m_pParamStack->GetSize();
	if(nCount < 2)
	{
        QMessageBox::information(this, "Underflow", "This operator requires at least two things on the stack");
		return;
	}

	// Check types
	COExpression* pParam1 = (COExpression*)m_pParamStack->GetPointer(nCount - 2);
	COExpression* pParam2 = (COExpression*)m_pParamStack->GetPointer(nCount - 1);
	if(eOperator == COOperator::OP_DOT)
	{
		if(pParam2->GetExpressionType() == COExpression::ET_VARREF)
		{
			COType* pType = pParam1->GetType(m_pCOProject);
			if(pType->GetTypeType() == COType::TT_CLASS)
			{
				if(((COClass*)pType)->FindMember(((COVarRef*)pParam2)->GetVar()) < 0)
				{
					QMessageBox::information(this, "Wrong type", "This is not a member of that class");
					return;
				}
			}
			else
				GAssert(false, "todo: handle interfaces");
		}
		else if(pParam2->GetExpressionType() == COExpression::ET_NAMED_CONSTANT)
		{
			COType* pType = pParam1->GetType(m_pCOProject);
			if(pType->GetTypeType() != COType::TT_CLASS ||
				((COClass*)pType)->FindConstant(((CONamedConst*)pParam2)->GetNamedConstant()) < 0)
			{
				QMessageBox::information(this, "Wrong type", "This is not a member of that class");
				return;
			}
		}
		else
		{
			QMessageBox::information(this, "Wrong type", "This is not a member of that class");
			return;
		}
	}
	else
	{
		if(pParam1->GetType(m_pCOProject) != m_pCOProject->m_pInteger ||
			pParam2->GetType(m_pCOProject) != m_pCOProject->m_pInteger)
		{
			QMessageBox::information(this, "Wrong type", "This operator can only combine two integer values");
			return;
		}
	}

	// Pop the top two items from the stack
	m_pParamStack->DeleteCell(nCount - 1);
	m_pParamStack->DeleteCell(nCount - 2);

	// Combine them and push back on the stack
	COExpression* pNewParam = new COOperator(pParam1, eOperator, pParam2);
	m_pParamStack->AddPointer(pNewParam);
	RefreshParamStack();*/
}

// OK button
void ParameterViewDialog::slot_button5()
{
/*    if(m_pParamStack->GetSize() == 0 && m_nSelectedList >= 1 && m_nSelectedList <= 8)
        slot_button14();
    if(m_pParamStack->GetSize() != 1)
	{
		QMessageBox::information(this, "Stack unresolved", "You can only push OK when there is exactly one item on the stack.");
		return;
	}
	COExpression* pParam = (COExpression*)m_pParamStack->GetPointer(0);
	m_pParam->Copy(pParam);*/
	accept();
}

void ParameterViewDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'E':	SetFocus(0);	break;
		case 'Z':	SetFocus(1);	break;
		case 'F':	SetFocus(2);	break;
		case 'D':	SetFocus(3);	break;
		case 'X':	SetFocus(4);	break;
		case 'C':	SetFocus(5);	break;
		case 'G':	SetFocus(6);	break;
		case 'V':	GetValue();		break;
		case 'S':	GetString();	break;
		case '.':	slot_button3();		break;
		case ',':	slot_button4();		break;
		case '+':	slot_button5_2();	break;
		case '-':	slot_button6_2();	break;
		case '*':	slot_button7();		break;
		case '/':	slot_button8();		break;
		case '~':	slot_button9();		break;
		case '!':	slot_button10();	break;
		case '&':	slot_button11();	break;
		case '|':	slot_button12();	break;
		case '^':	slot_button13();	break;
		case 'A':	slot_button14();	break;
		case 'R':	slot_button15();	break;
		case Key_Escape: // fall through
		case 'Q':	reject();		break;
		case Key_Up:	arrowUp();	break;
		case Key_Down:	arrowDown();break;
		case Key_Space: slot_button5(); break;
	}
}

