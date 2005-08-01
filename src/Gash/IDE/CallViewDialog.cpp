/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "CallViewDialog.h"
#include <qlistbox.h>
#include "../CodeObjects/CodeObjects.h"
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include "ParameterViewDialog.h"
#include "ComparatorViewDialog.h"

CallViewDialog::CallViewDialog(COProject* pCOProject, COMethod* pMethod, COInstruction* pInstruction, QWidget* parent, const char* name, bool modal, WFlags fl)
 : CallViewDialogBase(parent, name, modal, fl)
{
	GAssert(pInstruction->GetInstructionType() == COInstruction::IT_CALL, "This dialog is only valid for calls");
	m_pInstruction = pInstruction;
	m_pSelectedClass = NULL;
	m_pSelectedMethod = NULL;
	m_pMethod = pMethod;
	m_pCOProject = pCOProject;
	SetInitialSelections();
	m_nSelectedList = -1;
	RefreshLibraryList();
}

CallViewDialog::~CallViewDialog()
{
	int nCount = m_pParams->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((COExpression*)m_pParams->GetPointer(n));
	delete(m_pParams);
}

void CallViewDialog::SetInitialSelections()
{
	// Find which library
	// todo: handle interfaces
	COCall* pCall = (COCall*)m_pInstruction;
	COType* pType = pCall->GetTargetType();
	COFile* pFile = pType->GetFile();
	if(pFile && pFile->IsLibrary())
	{
		COHive* pLibraries = m_pCOProject->GetLibraries();
		COFile* pLibrary;
		m_nSelLibrary = 0;
		int n = 1;
		for(pLibrary = pLibraries->GetFirstFile(); pLibrary; pLibraries->GetNextFile(pLibrary))
		{
			if(pLibrary == pFile)
			{
				m_nSelLibrary = n;
				break;
			}
			n++;
		}
		if(m_nSelLibrary == 0)
		{
			GAssert(false, "Couldn't find that library");
		}
		m_nSelClass = -1;
		if(pType->GetTypeType() == COType::TT_CLASS)
			m_nSelClass = pFile->FindClass((COClass*)pType);
		if(m_nSelClass < 0)
		{
			GAssert(false, "Couldn't find that class in that Library");
			m_nSelClass = 0;
		}
	}
	else
	{
		m_nSelLibrary = 0;
		if(pType == m_pCOProject->m_pAsmClass)
			m_nSelClass = 0;
		else
		{
			m_nSelClass = -1;
			if(pType->GetTypeType() == COType::TT_CLASS)
				m_nSelClass = m_pCOProject->GetSource()->FindClass((COClass*)pType) + 1;
			if(m_nSelClass < 1)
			{
				GAssert(false, "Couldn't find that class in this Source");
				m_nSelClass = 0;
			}
		}
	}

	// Find which method
	if(pType->GetTypeType() == COType::TT_CLASS)
	{
		COMethod* pMethod = ((COMethodCall*)pCall)->GetMethod();
		COClass* pClass = (COClass*)pType;
		int nCount = 0;
		bool bGotIt = false;
		COMethod* pTmpMethod;
		for(pTmpMethod = pClass->GetFirstProcedure(); pTmpMethod; pTmpMethod = pClass->GetNextProcedure(pTmpMethod))
		{
			if(pTmpMethod == pMethod)
			{
				bGotIt = true;
				break;
			}
			nCount++;
		}
		if(!bGotIt)
		{
			for(pTmpMethod = pClass->GetFirstMethod(); pTmpMethod; pTmpMethod = pClass->GetNextMethod(pTmpMethod))
			{
				if(pTmpMethod == pMethod)
				{
					bGotIt = true;
					break;
				}
				nCount++;
			}
		}
		if(bGotIt)
			m_nSelMethod = nCount;
		else
		{
			GAssert(false, "Failed to find that method in this Source");
			m_nSelMethod = 0;
		}
	}
	else
	{
		GAssert(false, "todo: handle interfaces");
	}

	// Now do the parameters
	BuildParamsArray();
}

void CallViewDialog::BuildParamsArray()
{
/*	m_pParams = new GPointerArray(64);
	COCall* pCall = (COCall*)m_pInstruction;
	COParam* pParam;
	for(pParam = pCall->GetFirstParam(); pParam; pParam = pCall->GetNextParam(pParam))
		m_pParams->AddPointer(new COParam(pParam));*/
}

void CallViewDialog::RefreshLibraryList()
{
	m_pSelectedClass = NULL;
	m_pSelectedMethod = NULL;
	ListBox1->clear();
	ListBox1->insertItem(QString("This Source")); // todo: un-magic-string this line
	COHive* pLibraries = m_pCOProject->GetLibraries();
	COFile* pLibrary;
	char szLibraryName[256];
	for(pLibrary = pLibraries->GetFirstFile(); pLibrary; pLibrary = pLibraries->GetNextFile(pLibrary))
	{
		pLibrary->GetFilenameNoPath(szLibraryName);
		ListBox1->insertItem(QString(szLibraryName));
	}
	if((int)ListBox1->count() > m_nSelLibrary)
		ListBox1->setCurrentItem(m_nSelLibrary);
	else
	{
		GAssert(false, "No such selected library");
		m_nSelLibrary = 0;
		ListBox1->setCurrentItem(0);
	}
	SetFocus(0);
}

COFile* CallViewDialog::GetSelectedLibrary()
{
	int nSel = ListBox1->currentItem();
	if(nSel < 1) // The first item is "This Source", which is NULL
		return NULL;
	COFile* pFile;
	COHive* pLibraries = m_pCOProject->GetLibraries();
	int n = 1;
	for(pFile = pLibraries->GetFirstFile(); pFile; pFile = pLibraries->GetNextFile(pFile))
	{
		if(n == nSel)
			return(pFile);
		n++;
	}
	GAssert(false, "Not that many libraries");
	return NULL;
}

void CallViewDialog::RefreshClassList()
{
	m_pSelectedClass = NULL;
	m_pSelectedMethod = NULL;
	ListBox2->clear();
	COFile* pLibrary = GetSelectedLibrary();
	if(pLibrary == NULL) // If the library is "This Source"
	{
		COFileSet* pSource = m_pCOProject->GetSource();
		ListBox2->insertItem(QString(CLASS_NAME_ASM));
		COFile* pFile;
		for(pFile = pSource->GetFirstFile(); pFile; pFile = pSource->GetNextFile(pFile))
		{
			COClass* pClass;
			int nCount = pFile->GetClassCount();
			int n;
			for(n = 0; n < nCount; n++)
			{
				pClass = pFile->GetClass(n);
				ListBox2->insertItem(QString(pClass->GetName()));
			}
		}
	}
	else
	{
		COClass* pClass;
		int nCount;
		int n;
		for(n = 0; n < nCount; n++)
		{
			pClass = pLibrary->GetClass(n);
			ListBox2->insertItem(QString(pClass->GetName()));
		}
	}
	if((int)ListBox2->count() > m_nSelClass)
		ListBox2->setCurrentItem(m_nSelClass);
	else
	{
		GAssert(false, "No such selected class");
		m_nSelClass = 0;
		ListBox2->setCurrentItem(0);
	}
}

COClass* CallViewDialog::GetSelectedClass()
{
	if(m_pSelectedClass)
		return m_pSelectedClass;
	int nSel = ListBox2->currentItem();
	COFile* pLibrary = GetSelectedLibrary();
	if(pLibrary == NULL) // If the library is "This Source"
	{
		if(nSel < 1)
			return m_pCOProject->m_pAsmClass;
		COFile* pFile;
		COFileSet* pSource = m_pCOProject->GetSource();
		int nCount = 1;
		for(pFile = pSource->GetFirstFile(); pFile; pFile = pSource->GetNextFile(pFile))
		{
			COClass* pClass;
			for(pClass = pFile->GetFirstClass(); pClass; pClass = pFile->GetNextClass(pClass))
			{
				if(nCount == nSel)
				{
					m_pSelectedClass = pClass;
					return pClass;
				}
				nCount++;
			}
		}
		GAssert(false, "not that many classes in this Source");
	}
	else
	{
		int nCount = 0;
		COClass* pClass;
		for(pClass = pLibrary->GetFirstClass(); pClass; pClass = pLibrary->GetNextClass(pClass))
		{
			if(nCount == nSel)
			{
				m_pSelectedClass = pClass;
				return pClass;
			}
			nCount++;
		}
		GAssert(false, "not that many classes in this library");
	}
	GAssert(false, "it should never get to here");
	return NULL;
}

void CallViewDialog::RefreshMethodList()
{
	m_pSelectedMethod = NULL;
	ListBox3->clear();
	COClass* pClass = GetSelectedClass();
	if(!pClass)
		return;
	COMethod* pMethod;
	for(pMethod = pClass->GetFirstProcedure(); pMethod; pMethod = pClass->GetNextProcedure(pMethod))
	{
		ListBox3->insertItem(QString(pMethod->GetName()));
	}
	for(pMethod = pClass->GetFirstMethod(); pMethod; pMethod = pClass->GetNextMethod(pMethod))
	{
		ListBox3->insertItem(QString(pMethod->GetName()));
	}
	if(ListBox3->count() > 0)
	{
		if((int)ListBox3->count() > m_nSelMethod)
			ListBox3->setCurrentItem(m_nSelMethod);
		else
		{
			m_nSelMethod = 0;
			ListBox3->setCurrentItem(0);
		}
	}
	else
	{
		ListBox4->clear(); // Clear the Param helper list
		ListBox5->clear(); // Clear the Param list
	}
}

COMethod* CallViewDialog::GetSelectedMethod()
{
	if(m_pSelectedMethod)
		return m_pSelectedMethod;
	COClass* pClass = GetSelectedClass();
	if(!pClass)
		return NULL;
	int nSel = ListBox3->currentItem();
	COMethod* pMethod;
	int nCount = 0;
	for(pMethod = pClass->GetFirstProcedure(); pMethod; pMethod = pClass->GetNextProcedure(pMethod))
	{
		if(nCount == nSel)
		{
			m_pSelectedMethod = pMethod;
			return pMethod;
		}
		nCount++;
	}
	for(pMethod = pClass->GetFirstMethod(); pMethod; pMethod = pClass->GetNextMethod(pMethod))
	{
		if(nCount == nSel)
		{
			m_pSelectedMethod = pMethod;
			return pMethod;
		}
		nCount++;
	}
	GAssert(false, "not that many methods and procedures");
	return NULL;
}

void CallViewDialog::RefreshParamList()
{
/*	int nSel = ListBox5->currentItem();
	ListBox4->clear(); // Clear the Param helper list
	ListBox5->clear(); // Clear the Param list
	COMethod* pMethod = GetSelectedMethod();
	if(!pMethod)
		return;
	int nCount = pMethod->GetParameterCount();
	if(m_pParams->GetSize() != nCount)
	{
		GAssert(false, "Parameters don't match");
		return;
	}
	COVariable* pVariable;
	COParam* pParam;
	char szTmp[512];
	int n;
	for(n = 0; n < nCount; n++)
	{
		pVariable = pMethod->GetParameter(n);
		QString s = "[";
		s += pVariable->GetName();
		s += "] ";
		s += pVariable->GetType()->GetName();
		ListBox4->insertItem(s);
		pParam = (COParam*)m_pParams->GetPointer(n);
		pParam->GetExpression(szTmp, 512);
		ListBox5->insertItem(QString(szTmp)); // todo: implment this properly
	}
	nCount = ListBox5->count();
	if(nCount > 0)
	{
		if(nSel >= 0 && nCount > nSel)
			ListBox5->setCurrentItem(nSel); // Preserve the old selection
		else
			ListBox5->setCurrentItem(0); // Just select the first one
	}*/
}

void CallViewDialog::SelectNextParamDecl()
{
	int nSel = ListBox5->currentItem();
	nSel++;
	if(nSel >= 0 && nSel < (int)ListBox5->count())
		ListBox5->setSelected(nSel, true);
}

// Change Library
void CallViewDialog::slot_list1SelChange()
{
	m_pSelectedClass = NULL;
	m_pSelectedMethod = NULL;
	RefreshClassList();
}

// Change Class
void CallViewDialog::slot_list2SelChange()
{
	m_pSelectedClass = NULL;
	m_pSelectedMethod = NULL;
	RefreshMethodList();
}

// Change Method
void CallViewDialog::slot_list3SelChange()
{
/*	m_pSelectedMethod = NULL;

	// Ajust the size of m_pParams to be the right size for the newly
	// selected method.  Reuse any current parameters that are the right
	// type, but if they aren't or there aren't enough, use "NULL".
	COMethod* pMethod = GetSelectedMethod();
	int n;
	int nParamCount;
	if(pMethod)
		nParamCount = pMethod->GetParameterCount();
	else
		nParamCount = 0;
	COParam* pParam;
	for(n = 0; n < nParamCount; n++)
	{
		if(m_pParams->GetSize() > n)
		{
			pParam = (COParam*)m_pParams->GetPointer(n);
			
			// if param types are not related
			if(!pParam->GetType(m_pCOProject)->CanCastTo(pMethod->GetParameter(n)->GetType()))
			{
				// Reset the parameter to Null
				if((pMethod == m_pCOProject->m_pIf || pMethod == m_pCOProject->m_pWhile) && n == 1)
					pParam->SetLeafVar(m_pCOProject->m_pComparatorDecls[0]);
				else
					pParam->SetLeafVar(m_pCOProject->m_pNull);
			}
		}
		else
		{
			if((pMethod == m_pCOProject->m_pIf || pMethod == m_pCOProject->m_pWhile) && n == 1)
				pParam = new COParam(m_pCOProject->m_pComparatorDecls[0]);
			else
				pParam = new COParam(m_pCOProject->m_pNull);
			m_pParams->AddPointer(pParam);
		}
	}
	while(m_pParams->GetSize() > nParamCount)
	{
		delete((COParam*)m_pParams->GetPointer(m_pParams->GetSize() - 1));
		m_pParams->DeleteCell(m_pParams->GetSize() - 1);
	}

	// Refresh the Param list
	RefreshParamList();*/
}

void CallViewDialog::slot_list5SelChange()
{
}

void CallViewDialog::SetSelectedParameter(COVariable* pVariable)
{
/*	int nSel = ListBox5->currentItem();
	if(nSel < 0)
	{
		GAssert(false, "no parameter is selected");
		return;
	}
	int nCount = m_pParams->GetSize();
	if(nSel >= nCount)
	{
		GAssert(false, "not that many parameters");
		return;
	}
	COParam* pParam = (COParam*)m_pParams->GetPointer(nSel);
	pParam->SetLeafVar(pVariable);*/
}

// OK Button
void CallViewDialog::slot_button1()
{
/*	// Get the data from the dialog
	COClass* pClass = GetSelectedClass();
	COMethod* pMethod = GetSelectedMethod();
	COCall* pCall = (COCall*)m_pInstruction;
	if(!pMethod)
	{
		QMessageBox::information(this, "Error", "No method is selected.  Cancelling.");
		reject();
	}

	// Make the parameters
	int nCount = m_pParams->GetSize();
	GAssert(nCount == pMethod->GetParameterCount(), "Parameters don't match!");
	GLList* pNewParams = new GLList();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pNewParams->Link((COParam*)m_pParams->GetPointer(n));
		m_pParams->SetPointer(n, NULL);
	}

	// Set the method
	if(pClass == m_pCOProject->m_pAsmClass)
	{
		GAssert(pMethod->IsStatic(), "All ASM instructions are static procedures");
		GAssert(pMethod->GetClass() == m_pCOProject->m_pAsmClass, "This method doesn't think it is the right class");
	}
	GAssert(pCall->GetCallType() == COCall::CT_METHOD, "todo: implement for interfaces");
	bool bRet = ((COMethodCall*)pCall)->SetMethod(pMethod, pNewParams, NULL);
	GAssert(bRet, "failed to set method");

	// Close the dialog*/
	accept();
}

QListBox* CallViewDialog::GetSelectedListBox()
{
	switch(m_nSelectedList)
	{
		case 0: return ListBox1;
		case 1: return ListBox2;
		case 2: return ListBox3;
		case 3: return ListBox4;
		case 4: return ListBox5;
	}
	return NULL;
}

void CallViewDialog::SetFocus(int n)
{
	GAssert(n >= 0 && n <= 4, "Out of range");
	QListBox* pListBox;

	if(n == m_nSelectedList)
		return;

	// Move the arrow to the new list
	m_nSelectedList = n;
	pListBox = GetSelectedListBox();
	PixmapLabel1->move(pListBox->pos().x() + 100, pListBox->pos().y() - 30);
	if(pListBox->count() > 0)
	{
		if(pListBox->currentItem() < 0 || !pListBox->isSelected(pListBox->currentItem()))
			pListBox->setSelected(0, true);
	}
}

void CallViewDialog::slot_list1clicked()
{
	SetFocus(0);
}

void CallViewDialog::slot_list2clicked()
{
	SetFocus(1);
}

void CallViewDialog::slot_list3clicked()
{
	SetFocus(2);
}

void CallViewDialog::slot_list5clicked()
{
	SetFocus(4);
}

// Edit button
void CallViewDialog::slot_button4()
{
/*	int nSel = ListBox5->currentItem();
	if(nSel < 0)
	{
		QMessageBox::information(this, "Nothing Selected", "First select a parameter to edit");
		return;
	}
	COMethod* pMethod = GetSelectedMethod();
	GAssert(m_pParams->GetSize() == pMethod->GetParameterCount(), "Parameter list out of sync with selected method");
	COParam* pParam = (COParam*)m_pParams->GetPointer(nSel);
	COVariable* pVariable = pMethod->GetParameter(nSel);
    if(pVariable->GetType() == m_pCOProject->m_pInternalComparator)
    {
        ComparatorViewDialog dialog(m_pCOProject, this, "Comparator View");
        if(dialog.exec() == Accepted)
        {
            GAssert(dialog.m_pSelectedComparator->GetType() == m_pCOProject->m_pInternalComparator, "Bad Comparator");
            pParam->SetLeafVar(dialog.m_pSelectedComparator);
        }
    }
    else
    {
        ParameterViewDialog dialog(m_pCOProject, m_pMethod, m_pInstruction, pParam, this, "Parameter View", true, 0);
        dialog.exec();
    }
    if(pParam->GetType(m_pCOProject) != pVariable->GetType()) // todo: check for related, not equal
	{
		// todo: QMessageBox::information(this, "Wrong type", "That parameter is not the right type--setting to NULL"); // todo: make a more useful message here
		// Note: it will be set to NULL when we call RefreshParamList
	}
	RefreshParamList();*/
}

void CallViewDialog::arrowUp()
{
	QListBox* pListBox = GetSelectedListBox();
	if(pListBox)
	{
		int n = pListBox->currentItem() - 1;
		if(n >= 0 && n < (int)pListBox->count())
			pListBox->setSelected(n, true);
	}
}

void CallViewDialog::arrowDown()
{
	QListBox* pListBox = GetSelectedListBox();
	if(pListBox)
	{
		int n = pListBox->currentItem() + 1;
		if(n >= 0 && n < (int)pListBox->count())
			pListBox->setSelected(n, true);
	}
}

void CallViewDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'B':	SetFocus(0);	break;
		case 'C':	SetFocus(1);	break;
		case 'X':	SetFocus(2);	break;
		case 'R':	SetFocus(4);	break;
		case 'E':	slot_button4();	break;
		case Key_Escape: // fall through
		case 'Q':	reject();		break;
		case Key_Up:	arrowUp();	break;
		case Key_Down:	arrowDown();break;
		case Key_Return: // fall through
		case Key_Space: slot_button1(); break;
	}
}
