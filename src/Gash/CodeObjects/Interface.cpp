/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Interface.h"
#include "Variable.h"
#include "Method.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GXML.h"
#include "../Engine/TagNames.h"
#include "Project.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN
#include <wchar.h>

COInterface::COInterface(int nLine, int nCol, int nWid, const char* szName, COFile* pFile, COProject* pCOProject)
 : COType(nLine, nCol, nWid, szName, pFile, pCOProject)
{
	m_pMethodDecls = new GPointerArray(8);
	m_nID = pCOProject->GetUniqueTypeID();
}

COInterface::~COInterface()
{
	int nCount = GetMethodDeclCount();
	int n;
	for(n = 0; n < nCount; n++)
		delete(GetMethodDecl(n));
	delete(m_pMethodDecls);
}

int COInterface::GetMethodDeclCount()
{
	return m_pMethodDecls->GetSize();
}

void COInterface::AddMethodDecl(COMethodDecl* pMethodDecl)
{
	m_pMethodDecls->AddPointer(pMethodDecl);
}

COMethodDecl* COInterface::GetMethodDecl(int n)
{
	return (COMethodDecl*)m_pMethodDecls->GetPointer(n);
}

void COInterface::LoadMethodDecls(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		if(stricmp(pChild->GetName(), TAG_NAME_METHOD) == 0)
		{
			int nLine = pChild->GetLineNumber();
			int nCol, nWid;
			pChild->GetOffsetAndWidth(&nCol, &nWid);

			GXMLAttribute* pNameAttr = pChild->GetAttribute(ATTR_NAME);
			if(!pNameAttr)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChild);
			char cModifier = '\0';
			const char* szName = pNameAttr->GetValue();
			if(*szName == '&' || *szName == '!')
			{
				cModifier = *szName;
				szName++;
			}
			COMethodDecl* pMethodDecl = new COMethodDecl(nLine, nCol, nWid, szName, this, false, pCOProject);
			AddMethodDecl(pMethodDecl);
			pMethodDecl->LoadAllParams(this, pChild, cModifier, pCOProject, bPartial);
		}
		else
			pCOProject->ThrowError(&Error::EXPECTED_METHODSIG, pChild);
	}
}

GXMLTag* COInterface::SaveToXML()
{
	GXMLTag* pInterfaceTag = new GXMLTag(TAG_NAME_INTERFACE);
	pInterfaceTag->AddAttribute(new GXMLAttribute(ATTR_NAME, m_szName));
	COMethodDecl* pMethodDecl;
	int nCount = GetMethodDeclCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethodDecl = GetMethodDecl(n);
		pInterfaceTag->AddChildTag(pMethodDecl->SaveToXML());
	}
	return pInterfaceTag;
}

GXMLTag* COInterface::ToXMLForImplementationInLibrary()
{
	GXMLTag* pInterfaceTag;
	if(GetTypeType() == TT_MACHINE)
		pInterfaceTag = new GXMLTag(TAG_NAME_MACHINE);
	else
		pInterfaceTag = new GXMLTag(TAG_NAME_INTERFACE);
	char szTmp[64];
	itoa(GetID(), szTmp, 10);
	pInterfaceTag->AddAttribute(new GXMLAttribute(ATTR_ID, szTmp));
	pInterfaceTag->AddAttribute(new GXMLAttribute(ATTR_NAME, m_szName));
	return pInterfaceTag;
}

GXMLTag* COInterface::ToXMLForLibrary()
{
	GXMLTag* pInterfaceTag = ToXMLForImplementationInLibrary();
	COMethodDecl* pMethDecl;
	int nCount = GetMethodDeclCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethDecl = GetMethodDecl(n);
		pInterfaceTag->AddChildTag(pMethDecl->SaveToXML());
	}
	return pInterfaceTag;
}

void COInterface::SaveToClassicSyntax(GQueue* pQ)
{
	pQ->Push("interface ");
	pQ->Push(GetName());
	pQ->Push("\n{\n");

	// Method Decls
	COMethodDecl* pMethodDecl;
	int nCount = GetMethodDeclCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethodDecl = GetMethodDecl(n);
		pMethodDecl->SaveToClassicSyntax(pQ);
	}

	pQ->Push("}\n\n");
}

int COInterface::FindMethodDecl(COMethodDecl* pMethodSig)
{
	COMethodDecl* pMethodDecl;
	int nCount = GetMethodDeclCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethodDecl = GetMethodDecl(n);
		if(pMethodDecl == pMethodSig)
			return n;
	}
	return -1;
}

int COInterface::FindMethodDecl(EMethodSignature* pSignature)
{
	COMethodDecl* pMethodDecl;
	int nCount = GetMethodDeclCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethodDecl = GetMethodDecl(n);
		if(pSignature->Compare(pMethodDecl->GetSignature()) == 0)
			return n;
	}
	return -1;
}

COMethodDecl* COInterface::FindMethodDecl(int* pnOutIndex, const char* szName, GPointerArray* pParams, ErrorStruct** ppError, int* pnErrorParam, COProject* pProject)
{
	int nNameMatchCount = 0;
	COMethodDecl* pNameMatch = NULL;
	COMethodDecl* pMatch = NULL;
	COMethodDecl* pMethod;
	ErrorStruct* pErrorMessage = NULL;
	*pnErrorParam = -1;
	int index = -1;
	int nCount = GetMethodDeclCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethod = GetMethodDecl(n);
		if(stricmp(pMethod->GetName(), szName) == 0)
		{
			pNameMatch = pMethod;
			nNameMatchCount++;
			if(pMethod->CheckParams(pParams, &pErrorMessage, pnErrorParam, pProject))
			{
				if(pMatch)
				{
					*ppError = &Error::AMBIGUOUS_CALL;
					return NULL;
				}
				pMatch = pMethod;
				index = n;
			}
		}
	}
	if(!pMatch)
	{
		if(nNameMatchCount < 1)
		{
			*ppError = &Error::METHOD_NOT_FOUND;
			return NULL;
		}
		if(nNameMatchCount == 1)
		{
			*ppError = pErrorMessage;
			return NULL;
		}
		*ppError = &Error::NO_METHOD_MATCHES_SIGNATURE;
		return NULL;
	}
	*pnOutIndex = index;
	return pMatch;
}

