/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include <stdlib.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <malloc.h>
#include "DebuggerVariableItem.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GString.h"
#include "../Include/GashEngine.h"
#include "../Engine/TagNames.h"
#include "../Engine/EClass.h"

DebugVariableItem::DebugVariableItem(int n, DebugVariableItem* pParentItem, DebugVariableItem* pPrev, GObject* pObject, QString sName, Library* pLibrary)
	: QListViewItem(pParentItem, pPrev)
{
	Init(n, pObject, pPrev, sName, pLibrary);
}

DebugVariableItem::DebugVariableItem(int n, QListView* pParent, DebugVariableItem* pPrev, GObject* pObject, QString sName, Library* pLibrary)
	: QListViewItem(pParent, pPrev)
{
	Init(n, pObject, pPrev, sName, pLibrary);
}

void DebugVariableItem::Init(int n, GObject* pObject, DebugVariableItem* pPrev, QString sName, Library* pLibrary)
{
	m_nStackPos = n;
	m_pObject = pObject;
	m_pPrevItem = pPrev;
	m_sName = sName;
	m_pLibrary = pLibrary;
	if(m_pObject)
	{
#ifdef _DEBUG
		m_nRefCount = pObject->nRefCount;
		GAssert(m_nRefCount > 0 && m_nRefCount < 1000000, "looks like a bad object");
#endif
		if(pObject->GetType()->GetID() < 0)
		{
			GAssert(false, "invalid class id");
			return;
		}
		EType* pType = pObject->GetType();
		if(pType->GetTypeType() != EType::TT_CLASS)
			return;
		EClass* pClass = (EClass*)pType;
		if(pClass->GetTotalMemberCount() > 0)
			setExpandable(true);
	}
}

DebugVariableItem::~DebugVariableItem()
{
	DebugVariableItem* pNextSib = (DebugVariableItem*)nextSibling();
	if(pNextSib)
		pNextSib->SetPrevSibling(m_pPrevItem);
}

// Used in DebugVariableItem::text to recognize floats
void Float_Destructor(void* pObject);

bool IsStringObject(GObject* pObject, Library* pLibrary)
{
	EType* pType = pObject->GetType();
	return (stricmp(pType->GetName(), CLASS_NAME_STRING) == 0);
}

QString DebugVariableItem::text(int column) const
{
	GAssert(!m_pObject || (unsigned int)m_nRefCount == m_pObject->nRefCount, "Object has changed!");
	switch(column)
	{
	case 0: // #
		{
			char szTmp[32];
			itoa(m_nStackPos, szTmp, 10);
			return QString(szTmp);
		}
		return "0";
	case 1: // name
		return m_sName;
	case 2: // value
		{
			if(!m_pObject)
				return QString(VAL_NULL);
			EType* pType = m_pObject->GetType();
			switch(pType->GetTypeType())
			{
			case EType::TT_CLASS:
				{
					EClass* pClass = (EClass*)pType;
					if(pClass->IsIntegerType())
					{
						char szTmp[32];
						IntObject* pInt = (IntObject*)m_pObject;
						itoa(pInt->m_value, szTmp, 10);
						return QString(szTmp);
					}
					else
					{
						return QString(pType->GetName());
					}
				}
				break;
			case EType::TT_INTERFACE:
				return QString("<Interface>");
				break;
			case EType::TT_MACHINE:
				{
					wchar_t wszTmp[256];
					WrapperObject* pMachineObj = (WrapperObject*)m_pObject;
					pMachineObj->GetDisplayValue(wszTmp, 256);
					return QString((QChar*)wszTmp, 256);
				}
				break;
			}
		}
	case 3: // class
		{
			if(!m_pObject)
				return QString("Object");
			EType* pType = m_pObject->GetType();
			return QString(pType->GetName());
		}
	case 4: // address
		{
			if(m_pObject)
			{
				char szTmp[32];
				sprintf(szTmp, "%0x8", m_pObject);
				return QString(szTmp);
			}
			else
				return QString("0");
		}
	}
	return QString(NULL);
}

int DebugVariableItem::CountChildren()
{
	int nCount = 0;
	DebugVariableItem* pChild = (DebugVariableItem*)firstChild();
	while(pChild)
	{
		nCount++;
		pChild = (DebugVariableItem*)pChild->nextSibling();
	}
	return nCount;
}

void DebugVariableItem::setOpen(bool bOpen)
{
	EType* pType = m_pObject->GetType();
	if(pType->GetTypeType() == EType::TT_CLASS) // todo: add a special case for arrays too
	{
		// Check if we have populated the children yet
		EClass* pClass = (EClass*)pType;
		int nChildCount = CountChildren();
		int nMemberCount = pClass->GetTotalMemberCount();
		if(nChildCount != nMemberCount)
		{
			// Populate the children
			GAssert(nChildCount == 0, "Wrong number of children");
			DebugVariableItem* pPrev = NULL;
			int n;
			for(n = 0; n < nMemberCount; n++)
			{
				GXMLTag* pMemberTag = pClass->GetMemberTag(n, m_pLibrary);
				GXMLAttribute* pNameAttr;
				if(pMemberTag)
					pNameAttr = pMemberTag->GetAttribute(ATTR_NAME);
				else
					pNameAttr = NULL;
				QString sName;
				if(pNameAttr)
					sName = QString(pNameAttr->GetValue());
				else
				{
					char szTmp[32];
					itoa(n, szTmp, 10);
					sName = "[Member ";
					sName += szTmp;
					sName += "]";
				}
				ObjectObject* pClassOb = (ObjectObject*)m_pObject;
				pPrev = new DebugVariableItem(n, this, pPrev, pClassOb->arrFields[n], sName, m_pLibrary);
			}
		}
	}
	QListViewItem::setOpen(bOpen);
}
