/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "MethodViewDialog.h"
#include "../CodeObjects/CodeObjects.h"
#include "../CodeObjects/InstrArray.h"
#include <qlistview.h>
#include "CallViewDialog.h"
#include "DeclViewDialog.h"
#include "CallBackViewDialog.h"
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include "GetStringDialog.h"

// ********************************************************

QString MakeParametersString(COCall* pCall, COCallBack* pCallBack)
{
	GAssert((pCall || pCallBack) && (!pCall || !pCallBack), "Exactly one must be valid");
//	COParam* pParam;
//	char szBuff[256];
	QString sParams = "";
/*	bool bFirst = true;
	for(pParam = pCall ? pCall->GetFirstParam() : pCallBack->GetFirstParam(); pParam; pCall ? (pParam = pCall->GetNextParam(pParam)) : (pParam = pCallBack->GetNextParam(pParam)))
	{
		if(bFirst)
			bFirst = false;
		else
			sParams += ", ";
		pParam->GetExpression(szBuff, 256);
		szBuff[255] = '\0';
		sParams += szBuff;
	}*/
	return sParams;
}

QString MyListViewItem::text(int column) const
{
	if(!m_pInstruction)
	{
		if(column == 0)
			return QString(".");
		else
			return QString(NULL);
	}
	switch(m_pInstruction->GetInstructionType())
	{
		case COInstruction::IT_CALL:
			{
				COCall* pCall = (COCall*)m_pInstruction;
				switch(column)
				{
				case 0: // Name column
					{
						QString s = pCall->GetTargetType()->GetName();
						s += ".";
						switch(pCall->GetCallType())
						{
						case COCall::CT_METHOD:
							s += ((COMethodCall*)pCall)->GetMethod()->GetName();
							break;
						case COCall::CT_INTERFACE:
							s += ((COInterfaceCall*)pCall)->GetMethodDecl()->GetName();
							break;
						case COCall::CT_CALLBACK:
							s += "*";
							s += ((COCallBack*)pCall)->GetName();
							break;
						default:
							GAssert(false, "unexpected type");
						}
						return s;
					}
				case 1: // Params column
					return MakeParametersString(pCall, NULL);
				}
			}
			break;
		case COInstruction::IT_BLOCK:
			{
				COBlock* pBlock = (COBlock*)m_pInstruction;
				switch(column)
				{
				case 0: // Name column
					return QString(NULL);
				case 1: // Params column
					return QString(pBlock->GetComment());
				}
			}
	}
	GAssert(false, "Unexpected case");
	return QString(NULL);
}

// ********************************************************

MethodViewDialog::MethodViewDialog(COProject* pCOProject, COMethod* pMethod, QWidget* parent, const char* name, bool bNeedToChangeName)
 : MethodViewDialogBase(parent, name, true, 0)
{
	m_bNeedToChangeName = bNeedToChangeName;
	m_pMethod = pMethod;
	m_pCOProject = pCOProject;
	while(ListView1->columns() > 0)
		ListView1->removeColumn(0);
	ListView1->setTreeStepSize(20);
	ListView1->addColumn("Name", 260);
	ListView1->addColumn("Params", 500);
	ListView1->setSorting(-1);
	TextLabel2->setText(QString(m_pMethod->GetName()));
    m_pOldSelectedInstruction = NULL;
	m_nSelection = 0;
	RefreshAll();
    EditInstructions();
}

MethodViewDialog::~MethodViewDialog()
{

}

void MethodViewDialog::RefreshAll()
{
	RefreshInstructions();
	RefreshParams();
	QListViewItem* pItem = ListView1->firstChild();
	if(pItem)
		ListView1->setSelected(pItem, true);
}

void MethodViewDialog::RefreshParams()
{
	ListBox1->clear();
	int n;
	int nCount = m_pMethod->GetParameterCount();
	COVariable* pVariable;
	QString s;
	for(n = 0; n < nCount; n++)
	{
		pVariable = m_pMethod->GetParameter(n);
		s = "[";
		s += pVariable->GetName();
		s += "] ";
		s += pVariable->GetType()->GetName();
		ListBox1->insertItem(s);
	}
}

void MethodViewDialog::RefreshInstructions()
{
	ListView1->clear();
	int n;
	COInstrArray* pInstructions = m_pMethod->GetInstructions();
	int nCount = pInstructions->GetInstrCount();
	MyListViewItem* pPrev = NULL;
	for(n = 0; n < nCount; n++)
		pPrev = AddInstruction(NULL, pPrev, pInstructions->GetInstr(n));
	AddInstruction(NULL, pPrev, NULL);
}

MyListViewItem* MethodViewDialog::AddInstruction(MyListViewItem* pParent, MyListViewItem* pPrev, COInstruction* pInstr)
{
	// Create the node for this instruction
	MyListViewItem* pNewItem;
	if(pParent)
		pNewItem = new MyListViewItem(pParent, pPrev, pInstr);
	else
		pNewItem = new MyListViewItem(ListView1, pPrev, pInstr);

	// Set next sibling's prev item
	MyListViewItem* pNextSibling = (MyListViewItem*)pNewItem->nextSibling();
	if(pNextSibling)
		pNextSibling->SetPrevSibling(pNewItem);

	// Add children
	COInstrArray* pInstructions = pInstr ? pInstr->GetChildInstructions() : NULL;
	if(pInstructions)
	{
		int nChildren = pInstructions->GetInstrCount();
		int n;
		MyListViewItem* pPrevItem = NULL;
		for(n = 0; n < nChildren; n++)
			pPrevItem = AddInstruction(pNewItem, pPrevItem, pInstructions->GetInstr(n));
		MyListViewItem* pBlankSpace = AddInstruction(pNewItem, pPrevItem, NULL);
	}
	return pNewItem;
}

bool MethodViewDialog::EditInstruction(MyListViewItem* pItem, bool bNeedToChangeName)
{
	bool bOK = false;
	COInstruction* pInstruction = pItem->GetInstruction();
    if(!pInstruction)
    {
        QMessageBox::information(this, "Nothing selected", "First select a line with something to edit, or use one of the insert buttons");
        return false;
    }
	switch(pInstruction->GetInstructionType())
	{
		case COInstruction::IT_CALL:
			{
				CallViewDialog dialog(m_pCOProject, m_pMethod, pInstruction, this, "Instruction View", true, 0);
				if(dialog.exec() == Accepted)
				{
					if(pInstruction->GetChildInstructions())
					{
						// Add Blank Space
						if(!pItem->firstChild())
							MyListViewItem* pBlankSpace = AddInstruction(pItem, NULL, NULL);
					}
					else
					{
						if(pItem->firstChild())
						{
							GAssert(false, "todo: Sorry, change while or if instruction to another instruction not implemented yet.");
						}
					}
					bOK = true;
				}
			}
			break;
		case COInstruction::IT_BLOCK:
			{
				GetStringDialog dialog(this, "Enter a comment for this block", ((COBlock*)pInstruction)->GetComment());
				if(dialog.exec() == Accepted)
				{
					((COBlock*)pInstruction)->SetComment(dialog.LineEdit1->text().latin1());
					bOK = true;
				}
			}
		default:
			GAssert(false, "Unexpected case");		
	}
    if(bOK)
    {
		pItem->repaint();
        m_pMethod->GetClass()->GetFile()->SetModified(true);
    }
    return bOK;
}

// edit button
void MethodViewDialog::slot_button1()
{
    if(m_nSelection == 0)
    {
		int n = ListBox1->currentItem();
		if(n < 0 && n >= m_pMethod->GetParameterCount())
		{
			QMessageBox::information(this, "Nothing selected", "First select a line to edit");
			return;
		}
		DeclViewDialog dialog(m_pCOProject, m_pMethod->GetParameter(n), this, "Decl View", false);
		if(dialog.exec() == Accepted)
		{
	        m_pMethod->GetClass()->GetFile()->SetModified(true);
			RefreshParams();
		}
    }
    else
    {
		MyListViewItem* pItem = (MyListViewItem*)ListView1->selectedItem();
		if(!pItem)
		{
			QMessageBox::information(this, "Nothing selected", "First select a line to edit");
			return;
		}
		EditInstruction(pItem);
	}
}

void MethodViewDialog::arrowUp()
{
    if(m_nSelection == 0)
    {
		int n = ListBox1->currentItem() - 1;
		if(n >= 0 && n < (int)ListBox1->count())
			ListBox1->setSelected(n, true);
    }
    else
    {
	    QListViewItem* pItem = ListView1->selectedItem();
	    if(!pItem)
        {
            ListView1->setSelected(ListView1->firstChild(), true);
            return;
        }
	    pItem = pItem->itemAbove();
	    if(!pItem)
        {
            ListView1->setSelected(ListView1->firstChild(), true);
            return;
        }
	    ListView1->setSelected(pItem, true);
    }
}

void MethodViewDialog::arrowDown()
{
    if(m_nSelection == 0)
    {
		int n = ListBox1->currentItem() + 1;
		if(n >= 0 && n < (int)ListBox1->count())
			ListBox1->setSelected(n, true);
    }
    else
    {
	    QListViewItem* pItem = ListView1->selectedItem();
	    if(!pItem)
        {
            ListView1->setSelected(ListView1->firstChild(), true);
            return;
        }
	    pItem = pItem->itemBelow();
	    if(pItem)
		    ListView1->setSelected(pItem, true);
    }
}

bool MethodViewDialog::GetSelection(MyListViewItem** ppItem, MyListViewItem** ppPrevItem, int* pnPos, COInstruction** ppParentInstruction)
{
	// Get selection
	*ppItem = (MyListViewItem*)ListView1->selectedItem();
    if(!*ppItem)
        return false;
    MyListViewItem* pParent = (MyListViewItem*)(*ppItem)->parent();
	*ppPrevItem = (*ppItem)->GetPrevSibling();
	if(pParent)
		*ppParentInstruction = pParent->GetInstruction();
	else
		*ppParentInstruction = NULL;
	int nPos = 0;
	MyListViewItem* pItem = *ppPrevItem;
	while(pItem != NULL)
	{
		pItem = pItem->GetPrevSibling();
		nPos++;
	}
	*pnPos = nPos;
	return true;
}

// insert call button
void MethodViewDialog::slot_button3()
{
/*	// Get Selection
	MyListViewItem* pItem;
	MyListViewItem* pPrevItem;
	int nPos;
	COInstruction* pParentInstruction;
	if(!GetSelection(&pItem, &pPrevItem, &nPos, &pParentInstruction))
    {
        QMessageBox::information(this, "Nothing selected", "First select place to insert it");
        return;
    }

	// Insert a new COCall
	COMethodCall* pNewCall = new COMethodCall(m_pCOProject->m_pAsmClass->GetFirstProcedure(), NULL, NULL);
	if(pParentInstruction)
		pParentInstruction->InsertChild(nPos, pNewCall);
	else
		m_pMethod->InsertInstruction(nPos, pNewCall);
	MyListViewItem* pNewItem;
	pNewItem = AddInstruction((MyListViewItem*)pItem->parent(), pPrevItem, pNewCall);
	ListView1->setSelected(pNewItem, true);
	bool bOK = EditInstruction(pNewItem);
	if(!bOK)
	{
		// Back out the changes
		m_pMethod->DestroyInstruction(nPos);
		delete(pNewItem);
	}*/
}

// insert decl button
void MethodViewDialog::slot_button4()
{
	if(m_nSelection == 0) // Add to parameters
	{
		// Insert a new COVariable
		COVariable* pNewVar = new COVariable("Name", m_pCOProject->m_pInteger, false, false); // todo: unmagic the name, make it guaranteed unique, and make params default to read-only
		DeclViewDialog dialog(m_pCOProject, pNewVar, this, "Decl View", true);
		if(dialog.exec() == Accepted)
			m_pMethod->AddParameter(pNewVar);
		else
			delete(pNewVar);
		RefreshParams();
	}
	else // Add to instructions
	{
/*		// Get Selection
		MyListViewItem* pItem;
		MyListViewItem* pPrevItem;
		int nPos;
		COInstruction* pParentInstruction;
		if(!GetSelection(&pItem, &pPrevItem, &nPos, &pParentInstruction))
		{
			QMessageBox::information(this, "Nothing selected", "First select place to insert it");
			return;
		}

		// Insert a new COVariable
		COVariable* pNewVar = new COVariable("Name", m_pCOProject->m_pInteger, false, false); // todo: unmagic the name and make it guaranteed unique
		if(pParentInstruction)
			pParentInstruction->InsertChild(nPos, pNewVar);
		else
			m_pMethod->InsertInstruction(nPos, pNewVar);
		MyListViewItem* pNewItem;
		pNewItem = AddInstruction((MyListViewItem*)pItem->parent(), pPrevItem, pNewVar);
		ListView1->setSelected(pNewItem, true);
		bool bOK = EditInstruction(pNewItem, true);
		if(bOK)
        {
			// Ask if user would like to call New
			COType* pType = pNewVar->GetType();
			GAssert(pType->GetTypeType() == COType::TT_CLASS, "todo: handle interfaces");
			COClass* pClass = (COClass*)pType;
			if(pClass == m_pCOProject->m_pInteger || pClass == m_pCOProject->m_pWrapper)
				pClass = m_pCOProject->m_pAsmClass;
			COMethod* pNewProc;
			for(pNewProc = pClass->GetFirstProcedure(); pNewProc; pNewProc = pClass->GetNextProcedure(pNewProc))
			{
				if(stricmp(pNewProc->GetName(), "New") == 0) // todo: un-magic this
					break;
			}
			if(pNewProc && pNewProc->GetParameterCount() == 1)
			{
				if(QMessageBox::information( this, "Do you want to call New?", "This variables will be initialized to \"Null\".\nWould you like to also call its \"New\" procedure?", "&Yes", "&No Thanks", 0) == 0)
				{
					GLList* pParams = new GLList();
					pParams->Link(new COParam(pNewVar));
					COMethodCall* pNewCall = new COMethodCall(pNewProc, pParams, NULL);
					m_pMethod->InsertInstruction(nPos + 1, pNewCall);
					MyListViewItem* pNewNewItem;
					pNewNewItem = AddInstruction((MyListViewItem*)pNewItem->parent(), pNewItem, pNewCall);
					ListView1->setSelected(pNewNewItem, true);
				}
			}
        }
        else
		{
			// Back out the changes
			m_pMethod->DestroyInstruction(nPos);
			delete(pNewItem);
		}*/
	}
}

// insert callback button
void MethodViewDialog::slot_button6()
{
/*	// Get Selection
	MyListViewItem* pItem;
	MyListViewItem* pPrevItem;
	int nPos;
	COInstruction* pParentInstruction;
	if(!GetSelection(&pItem, &pPrevItem, &nPos, &pParentInstruction))
    {
        QMessageBox::information(this, "Nothing selected", "First select place to insert it");
        return;
    }

	// Insert a new COVariable
	COCallBack* pNewCallBack = new COCallBack("MyCallBackFunction", NULL); // todo: unmagic the name
	if(pParentInstruction)
		pParentInstruction->InsertChild(nPos, pNewCallBack);
	else
		m_pMethod->InsertInstruction(nPos, pNewCallBack);
	MyListViewItem* pNewItem;
	pNewItem = AddInstruction((MyListViewItem*)pItem->parent(), pPrevItem, pNewCallBack);
	ListView1->setSelected(pNewItem, true);
	bool bOK = EditInstruction(pNewItem);
	if(!bOK)
	{
		// Back out the changes
		m_pMethod->DestroyInstruction(nPos);
		delete(pNewItem);
	}*/
}

// insert Block button
void MethodViewDialog::slot_button7_2()
{
/*	// Get Selection
	MyListViewItem* pItem;
	MyListViewItem* pPrevItem;
	int nPos;
	COInstruction* pParentInstruction;
	if(!GetSelection(&pItem, &pPrevItem, &nPos, &pParentInstruction))
    {
        QMessageBox::information(this, "Nothing selected", "First select place to insert it");
        return;
    }

	// Get a comment for this block
	GetStringDialog dialog(this, "Enter a comment for this block", "");
	if(dialog.exec() != Accepted)
		return;

	// Insert a new Block
	COBlock* pNewBlock = new COBlock(dialog.LineEdit1->text().latin1());
	if(pParentInstruction)
		pParentInstruction->InsertChild(nPos, pNewBlock);
	else
		m_pMethod->InsertInstruction(nPos, pNewBlock);
	MyListViewItem* pNewItem;
	pNewItem = AddInstruction((MyListViewItem*)pItem->parent(), pPrevItem, pNewBlock);
	ListView1->setSelected(pNewItem, true);*/
}

// remove button
void MethodViewDialog::slot_button5()
{
/*	QMessageBox::information(this, "Sorry", "Remove is not implemented yet.\nYou'll have to do in manually by editing the source code\ntodo: implement this");
	return;

	if(m_nSelection == 0) // Add to parameters
	{
		// todo: be able to remove parameters too
		GAssert(false, "remove parameter not implemented yet");
	}
	else
	{
		// todo: make this work with nested instructions too

		// Get selection
		MyListViewItem* pItem = (MyListViewItem*)ListView1->selectedItem();
		if(!pItem || pItem->GetInstruction() == NULL)
		{
			QMessageBox::information(this, "Nothing selected", "First select a line to remove");
			return;
		}
		MyListViewItem* pNextSibling = (MyListViewItem*)pItem->nextSibling();
		COInstruction* pInstruction = pItem->GetInstruction();
		int nPos = m_pMethod->FindInstruction(pInstruction);
		GAssert(nPos >= 0, "Couldn't find instruction");

		// Remove the line
		delete(pItem);
		m_pMethod->DestroyInstruction(nPos);
		m_pMethod->GetClass()->GetFile()->SetModified(true);
		ListView1->setSelected(pNextSibling, true);
	}*/
}

void MethodViewDialog::slot_list1Clicked()
{
	ListView1->clearSelection();
	m_nSelection = 0;
}

void MethodViewDialog::slot_listView1Clicked()
{
	ListBox1->clearSelection();
	m_nSelection = 1;
}

void MethodViewDialog::EditName()
{
	// Move the arrow to the name
	PixmapLabel1->move(TextLabel1->pos().x() + 110, TextLabel1->pos().y() - 30);
	GetStringDialog dialog(this, "Enter a name for this method", m_pMethod->GetName());
	if(dialog.exec() == Accepted)
	{
		QString s = dialog.LineEdit1->text();
		m_pMethod->SetName(s.latin1());
		TextLabel2->setText(s);
        m_pMethod->GetClass()->GetFile()->SetModified(true);
	}
}

void MethodViewDialog::EditParams()
{
	PixmapLabel1->move(TextLabel3->pos().x() + 110, TextLabel3->pos().y() - 30);
	m_pOldSelectedInstruction = (MyListViewItem*)ListView1->selectedItem();
    ListView1->clearSelection();
	if(ListBox1->currentItem() < 0 && ListBox1->count() > 0)
		ListBox1->setSelected(0, true);
	m_nSelection = 0;
}

void MethodViewDialog::EditInstructions()
{
	PixmapLabel1->move(ListView1->pos().x() + 110, ListView1->pos().y() - 30);
	ListBox1->clearSelection();
	if(m_pOldSelectedInstruction != NULL)
        ListView1->setSelected(m_pOldSelectedInstruction, true);
    else if(ListView1->selectedItem() == NULL)
        ListView1->setSelected(ListView1->firstChild(), true);
    m_nSelection = 1;
}

void MethodViewDialog::arrowRight()
{
	MyListViewItem* pItem = (MyListViewItem*)ListView1->selectedItem();
	if(pItem)
		pItem->setOpen(true);
}

void MethodViewDialog::arrowLeft()
{
	MyListViewItem* pItem = (MyListViewItem*)ListView1->selectedItem();
	if(pItem)
		pItem->setOpen(false);
}

void MethodViewDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'E':		slot_button1(); break;
		case Key_Escape: // fall through
		case 'Q':		accept(); break;
		case 'C':		slot_button3(); break;
		case 'V':		slot_button4(); break;
		case 'R':		slot_button5(); break;
		case 'A':		slot_button6(); break;
		case 'B':		slot_button7_2(); break;
		case 'T':		EditName(); break;
		case 'D':		EditParams(); break;
		case 'Z':		EditInstructions(); break;
		case Key_Up:	arrowUp(); break;
		case Key_Down:	arrowDown(); break;
		case Key_Right:	arrowRight(); break;
		case Key_Left:	arrowLeft(); break;
	}
}

bool MethodViewDialog::event(QEvent* e)
{
	if(m_bNeedToChangeName && e->type() == QEvent::Show)
	{
		EditName();
		m_bNeedToChangeName = false;
	}
	return MethodViewDialogBase::event(e);
}
