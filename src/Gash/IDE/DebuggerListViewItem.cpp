/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include <qpixmap.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <stdlib.h>
#include "DebuggerListViewItem.h"
#include "../../GClasses/GQueue.h"
#include "../CodeObjects/Instruction.h"
#include "../CodeObjects/Call.h"
#include "../CodeObjects/Expression.h"
#include "../CodeObjects/Project.h"
#include "../Engine/InstrTable.h"

DebugListViewItem::DebugListViewItem(DebugListViewItem* pParentItem, DebugListViewItem* pPrev, COInstruction* pInstruction, QPixmap* pPixmap, COProject* pProject)
	: QListViewItem(pParentItem, pPrev)
{
	m_eType = IT_CO;
	m_pCOInstr = pInstruction;
	m_pPrevItem = pPrev;
	m_szComment = NULL;
	if(pInstruction->GetInstructionType() == COInstruction::IT_CALL &&
		((COCall*)pInstruction)->GetCallType() == COCall::CT_METHOD &&
		((COMethodCall*)pInstruction)->GetMethod() == pProject->m_pObject_else)
		m_pPixmap = NULL; // trick to make "Else" un-indented   todo: fix so only blank pixmaps vanish
	else
		m_pPixmap = pPixmap;
}

DebugListViewItem::DebugListViewItem(QListView* pParent, DebugListViewItem* pPrev, COInstruction* pInstruction, QPixmap* pPixmap)
	: QListViewItem(pParent, pPrev)
{
	m_eType = IT_CO;
	m_pCOInstr = pInstruction;
	m_pPrevItem = pPrev;
	m_szComment = NULL;
	m_pPixmap = pPixmap;
}
/*
DebugListViewItem::DebugListViewItem(DebugListViewItem* pParentItem, DebugListViewItem* pPrev, InstrNode* pAsmInstr, const char* szComment, QPixmap* pPixmap)
	: QListViewItem(pParentItem, pPrev)
{
	m_eType = IT_ASM;
	m_pAsmInstr = pAsmInstr;
	m_pPrevItem = pPrev;
	m_szComment = NULL;
	SetComment(szComment);
	m_pPixmap = pPixmap;
}

DebugListViewItem::DebugListViewItem(QListView* pParent, DebugListViewItem* pPrev, InstrNode* pAsmInstr, const char* szComment, QPixmap* pPixmap)
	: QListViewItem(pParent, pPrev)
{
	m_eType = IT_ASM;
	m_pAsmInstr = pAsmInstr;
	m_pPrevItem = pPrev;
	m_szComment = NULL;
	SetComment(szComment);
	m_pPixmap = pPixmap;
}
*/
DebugListViewItem::~DebugListViewItem()
{
	DebugListViewItem* pNextSib = (DebugListViewItem*)nextSibling();
	if(pNextSib)
		pNextSib->SetPrevSibling(m_pPrevItem);
	delete(m_szComment);
}

COInstruction* DebugListViewItem::GetCOInstr()
{
	GAssert(m_eType == IT_CO, "Wrong type");
	return m_pCOInstr;
}
/*
InstrNode* DebugListViewItem::GetAsmInstr()
{
	GAssert(m_eType == IT_ASM, "Wrong type");
	return m_pAsmInstr;
}

QString MakeCOParametersString(COCall* pCall)
{
	COExpression* pParam;
	char szBuff[256];
	QString sParams = "";
	int n;
	int nCount = pCall->GetParamCount();
	for(n = 0; n < nCount; n++)
	{
		pParam = pCall->GetParam(n);
		if(n > 0)
			sParams += ", ";
		pParam->ToString(szBuff);
		szBuff[255] = '\0';
		sParams += szBuff;
	}
	return sParams;
}
*/

QString DebugListViewItem::text(int column) const
{
	InstrType eType = GetType();
	if(eType == IT_CO)
	{
		if(!m_pCOInstr)
		{
			GAssert(false, "Node has no instruction");
			return QString(NULL);
		}
		GQueue q;
		m_pCOInstr->SaveToClassicSyntax(&q, 0, true);
		int nSize = q.GetSize();
		QString s = "";
		char c;
		while(nSize > 0)
		{
			q.Pop(&c);
			s += c;
			nSize--;
		}
		return s;
	}
/*	else if(eType == IT_ASM)
	{
		switch(column)
		{
		case 0:
			{
				struct InstructionStruct* pInstr = m_pAsmInstr->GetInstruction();
				char szTmp[32];
				itoa(m_pAsmInstr->m_nCodePos, szTmp, 10);
				QString s = szTmp;
				s += ": ";
				s += m_pAsmInstr->GetInstruction()->szName;
				return s;
			}
		case 1:
			return QString("todo: make asm command string");
		}
		return QString(NULL);
	}*/
	else
	{
		GAssert(false, "Unexpected case");
		return QString(NULL);
	}
}
