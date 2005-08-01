/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "EInterface.h"
#include "../Include/GashEngine.h"
#include "../../GClasses/GXML.h"
#include "../CodeObjects/Method.h"
#include "TagNames.h"
#include <wchar.h>

EInterface::EInterface(GXMLTag* pTag, int nID)
	: EType(pTag, nID)
{
	m_nMethodDeclCount = m_pTag->GetChildTagCount();
	m_pChildTags = new GXMLTag*[m_nMethodDeclCount];
	int n = 0;
	GXMLTag* pChildTag;
	for(pChildTag = m_pTag->GetFirstChildTag(); pChildTag; pChildTag = m_pTag->GetNextChildTag(pChildTag))
	{
		GAssert(stricmp(pChildTag->GetName(), TAG_NAME_METHOD) == 0, "expected a method tag--todo: throw");
		m_pChildTags[n] = pChildTag;
		n++;
	}
}

/*virtual*/ EInterface::~EInterface()
{
	delete [] m_pChildTags;
}

GXMLTag* EInterface::GetChildTag(int nMethodDecl)
{
	return m_pChildTags[nMethodDecl];
}

int EInterface::GetParamCount(int nMethodDecl)
{
	return GetChildTag(nMethodDecl)->GetChildTagCount();
}


// ------------------------------------------------------------------------

const char* EMachineClass::GetName()
{
	GXMLAttribute* pNameAttr = m_pTag->GetAttribute(ATTR_NAME);
	if(!pNameAttr)
		return "";
	return pNameAttr->GetValue();
}

EMethodPointerHolder* EMachineClass::GetMachineProc(int nMethodDeclIndex, GVM* pVM)
{
	if(!m_pMachineMethods)
	{
		int nCount = m_pTag->GetChildTagCount();
		m_pMachineMethods = new EMethodPointerHolder*[nCount];
		memset(m_pMachineMethods, '\0', nCount * sizeof(EMethodPointerHolder*));
	}
	if(!m_pMachineMethods[nMethodDeclIndex])
	{
		CallBackGetter* pCBG = pVM->GetCallBackGetter();
		EMethodSignature sig(GetChildTag(nMethodDeclIndex));
		m_pMachineMethods[nMethodDeclIndex] = pCBG->GetCallBack(GetName(), &sig);
#ifdef _DEBUG
		if(!m_pMachineMethods[nMethodDeclIndex])
		{
			ConvertUnicodeToAnsi(sig.GetString(), szSig);
			GAssert(false, "No callback found with the specified signature");
		}
#endif // _DEBUG
	}
	return m_pMachineMethods[nMethodDeclIndex];
}

