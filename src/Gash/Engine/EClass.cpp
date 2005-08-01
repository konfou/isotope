/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "../Include/GashEngine.h"
#include "../../GClasses/GXML.h"
#include "TagNames.h"
#include "EClass.h"
#include "EInterface.h"
#include "EMethod.h"

EClass::EClass(GXMLTag* pTag, int nID, int nParentID)
 : EType(pTag, nID),
	m_nMemberCount(-1),
	m_nParentClassID(nParentID),
	m_nFirstMethodID(-1),
	m_nMethodCount(-1),
	m_pVirtualTable(NULL),
	m_nFlags(0)
{
	// todo: implement a faster way of identifying "Integer"
	GXMLAttribute* pNameAttr = pTag->GetAttribute(ATTR_NAME);
	if(pNameAttr)
	{
		if(stricmp(pNameAttr->GetValue(), CLASS_NAME_INTEGER) == 0)
			SetIntegerType();
	}
	else
	{
		GAssert(false, "expected name attribute");
	}

	// Count the extended members
	m_nMemberCountExtendedOnly = 0;
	if(!IsIntegerType())
	{
		GXMLTag* pChildTag;
		for(pChildTag = pTag->GetFirstChildTag(); pChildTag; pChildTag = pTag->GetNextChildTag(pChildTag))
		{
			if(stricmp(pChildTag->GetName(), TAG_NAME_VAR) == 0)
				m_nMemberCountExtendedOnly++;
		}
	}
}

EClass::~EClass()
{
	delete(m_pVirtualTable);
}

int EClass::GetInterfaces(EInterface** pInterfaces, Library* pLibrary)
{
	int nCount = 0;
	int nParentID = GetParentID();
	if(nParentID > 0)
	{
		EClass* pParent = (EClass*)pLibrary->GetEType(nParentID);
		nCount += pParent->GetInterfaces(pInterfaces, pLibrary);
	}
	GXMLTag* pChildTag;
	for(pChildTag = m_pTag->GetFirstChildTag(); pChildTag; pChildTag = m_pTag->GetNextChildTag(pChildTag))
	{
		if(stricmp(pChildTag->GetName(), TAG_NAME_INTERFACE) != 0)
			continue;
		GXMLAttribute* pAttrID = pChildTag->GetAttribute(ATTR_ID);
		if(!pAttrID)
			return -1;
		int nInterfaceID = atoi(pAttrID->GetValue());
		EType* pType = pLibrary->GetEType(nInterfaceID);
		if(pType->GetTypeType() != EType::TT_INTERFACE)
			return -1;
		if(pInterfaces)
			pInterfaces[nCount] = (EInterface*)pType;
		nCount++;
	}
	return nCount;
}

bool EClass::CreateVirtualTable(Library* pLibrary)
{
	// Get pointers to all the interfaces in an order that puts ancestor interfaces first
	int nInterfaceCount = GetInterfaces(NULL, pLibrary);
	if(nInterfaceCount <= 0)
		return true;
	EInterface** pInterfaces = (EInterface**)alloca(nInterfaceCount * sizeof(EInterface*));
	GetInterfaces(pInterfaces, pLibrary);

	// count the virtual methods
	int nVirtualMethodCount = 0;
	int n;
	for(n = 0; n < nInterfaceCount; n++)
		nVirtualMethodCount += pInterfaces[n]->GetMethodDeclCount();

	// Second pass--fill the virtual table
	m_pVirtualTable = new int[2 * nInterfaceCount + nVirtualMethodCount];
	int nVirtualTablePos = 0;
	EInterface* pInterface;
	for(n = 0; n < nInterfaceCount; n++)
	{
		pInterface = pInterfaces[n];
		m_pVirtualTable[nVirtualTablePos] = pInterface->GetID();
		nVirtualTablePos++;
		if(n < nInterfaceCount - 1)
			m_pVirtualTable[nVirtualTablePos] = nVirtualTablePos + 1 + pInterface->GetMethodDeclCount();
		else
			m_pVirtualTable[nVirtualTablePos] = -1;
		nVirtualTablePos++;
		int i;
		for(i = 0; i < pInterface->GetMethodDeclCount(); i++)
		{
			EClass* pClass = this;
			EMethod* pMethod;
			GXMLTag* pMethodDeclTag = pInterface->GetChildTag(i);
			EMethodSignature sig(pMethodDeclTag);
			m_pVirtualTable[nVirtualTablePos] = -1;
			bool bGotIt = false;
			while(pClass && !bGotIt)
			{
				int nMethodCount = pClass->GetMethodCount();
				int nBaseMethod = pClass->GetFirstMethodID();
				int j;
				for(j = 0; j < nMethodCount && !bGotIt; j++)
				{
					pMethod = pLibrary->GetEMethod(nBaseMethod + j);
					if(sig.Compare(pMethod->GetSignature()) == 0)
					{
						bGotIt = true;
						m_pVirtualTable[nVirtualTablePos] = pMethod->GetID();
						break;
					}
				}
				pClass = (EClass*)pLibrary->GetEType(pClass->GetParentID()); // todo: check that the type is a class type
			}
			nVirtualTablePos++;
		}
	}
	return true;
}

GXMLTag* EClass::GetMemberTag(int n, Library* pLibrary)
{
	int nPos = m_nMemberCountExtendedOnly - m_nMemberCount + n;
	if(nPos < 0)
	{
		EClass* pParent = (EClass*)pLibrary->GetEType(m_nParentClassID);
		return pParent->GetMemberTag(n, pLibrary);
	}
	GXMLTag* pMemberTag;
	int i = 0;
	for(pMemberTag = m_pTag->GetFirstChildTag(); pMemberTag; pMemberTag = m_pTag->GetNextChildTag(pMemberTag))
	{
		if(stricmp(pMemberTag->GetName(), TAG_NAME_VAR) != 0)
			continue; // not a member
		if(i >= nPos)
			return pMemberTag;
		i++;
	}
	GAssert(false, "Couldn't find member tag");
	return NULL;
}

bool EClass::DoesImplement(EInterface* pInterface)
{
	GAssert(false, "todo: write me");
return false;
}

int EClass::GetTotalMemberCount()
{
	GAssert(m_nMemberCount >= 0, "Members haven't been counted yet");
	return m_nMemberCount;
}

int EClass::CountTotalMembers(Library* pLibrary)
{
	if(m_nMemberCount < 0)
	{
		int nInheritedMembers = 0;
		if(m_nParentClassID > 0)
		{
			EClass* pParent = (EClass*)pLibrary->GetEType(m_nParentClassID);
			GAssert(((EType*)pParent)->GetTypeType() == EType::TT_CLASS, "expected a class"); // todo: make a formal error
			nInheritedMembers = pParent->CountTotalMembers(pLibrary);
		}
		m_nMemberCount = nInheritedMembers + m_nMemberCountExtendedOnly;
	}
	return m_nMemberCount;
}

