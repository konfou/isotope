/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Class.h"
#include "Project.h"
#include "Variable.h"
#include "Interface.h"
#include "File.h"
#include "Constant.h"
#include "InstrArray.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GBitTable.h"
#include "../Engine/GCompiler.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

COClass::COClass(int nLine, int nCol, int nWid, const char* szName, COClass* pParent, COFile* pFile, const char* szSource, COProject* pCOProject)
 : COType(nLine, nCol, nWid, szName, pFile, pCOProject)
{
	GAssert(szName, "Must have a valid name");
	GAssert(pParent || stricmp(szName, CLASS_NAME_OBJECT) == 0, "Must have a valid parent");
	GAssert(pFile || (stricmp(pParent->GetName() , CLASS_NAME_OBJECT) == 0), "Must have a valid file");
	m_pParent = pParent;
	m_pInterfaceRefs = NULL;
	m_pMembers = new GPointerArray(16);
	m_pConstants = new GPointerArray(16);
	m_pMethods = new GPointerArray(64);
	m_pProcedures = new GPointerArray(64);
	m_pVariable = NULL;
	m_szSource = NULL;
	if(szSource)
	{
		m_szSource = new char[strlen(szSource) + 1];
		strcpy(m_szSource, szSource);
	}
}

COClass::~COClass()
{
	delete(m_pInterfaceRefs);
	int nCount = GetExtendedMemberCount();
	int n;
	for(n = 0; n < nCount; n++)
		delete(GetExtendedMember(n));
	delete(m_pMembers);
	nCount = GetConstantCount();
	for(n = 0; n < nCount; n++)
		delete(GetConstant(n));
	delete(m_pConstants);
	nCount = GetMethodCount();
	for(n = 0; n < nCount; n++)
		delete(GetMethod(n));
	delete(m_pMethods);
	nCount = GetProcedureCount();
	for(n = 0; n < nCount; n++)
		delete(GetProcedure(n));
	delete(m_pProcedures);
	delete(m_pVariable);
}

int COClass::GetExtendedMemberCount()
{
	return m_pMembers->GetSize();
}

COVariable* COClass::GetExtendedMember(int n)
{
	return (COVariable*)m_pMembers->GetPointer(n);
}

void COClass::AddMember(COVariable* pMember)
{
	m_pMembers->AddPointer(pMember);
}

int COClass::GetConstantCount()
{
	return m_pConstants->GetSize();
}

COConstant* COClass::GetConstant(int n)
{
	return (COConstant*)m_pConstants->GetPointer(n);
}

void COClass::AddConstant(COConstant* pConst)
{
	m_pConstants->AddPointer(pConst);
}

int COClass::GetMethodCount()
{
	return m_pMethods->GetSize();
}

COMethod* COClass::GetMethod(int n)
{
	return (COMethod*)m_pMethods->GetPointer(n);
}

void COClass::AddMethod(COMethod* pMethod)
{
	m_pMethods->AddPointer(pMethod);
}

int COClass::GetProcedureCount()
{
	return m_pProcedures->GetSize();
}

COMethod* COClass::GetProcedure(int n)
{
	return (COMethod*)m_pProcedures->GetPointer(n);
}

void COClass::AddProcedure(COMethod* pProcedure)
{
	m_pProcedures->AddPointer(pProcedure);
}

void COClass::LoadMembers(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	int nLine = pTag->GetLineNumber();
	int nCol, nWid;
	pTag->GetOffsetAndWidth(&nCol, &nWid);

	// Check Name
	GXMLAttribute* pName = pTag->GetAttribute(ATTR_NAME);
	if(!pName)
		pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pTag);
	if(stricmp(pName->GetValue(), GetName()) != 0)
		pCOProject->ThrowError(&Error::INTERNAL_ERROR, pTag);

	// Set Parent
	GXMLAttribute* pParentTag = pTag->GetAttribute(ATTR_PARENT);
	if(!pParentTag && stricmp(pName->GetValue(), CLASS_NAME_OBJECT) != 0)
		pCOProject->ThrowError(&Error::EXPECTED_PARENT_ATTRIBUTE, pTag);
	if(bPartial)
		SetParent(pCOProject->m_pObject); // bogus parent
	else if(pParentTag)
	{
		COClass* pParent = pCOProject->FindClass(pParentTag->GetValue());
		if(!pParent)
			pCOProject->ThrowError(&Error::TYPE_NOT_FOUND, pTag, pParentTag->GetValue());
		SetParent(pParent);
	}

	// Load all the Members and Constants
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		if(stricmp(pChild->GetName(), TAG_NAME_PROCEDURE) == 0)
			continue;
		else if(stricmp(pChild->GetName(), TAG_NAME_METHOD) == 0)
			continue;
		else if(stricmp(pChild->GetName(), TAG_NAME_INTERFACE) == 0)
			continue;
		else if(stricmp(pChild->GetName(), TAG_NAME_VAR) == 0)
		{
			COVariable* pNewMember = COVariable::FromXML(pChild, pCOProject, bPartial);
			AddMember(pNewMember);
		}
		else if(stricmp(pChild->GetName(), TAG_NAME_CONSTANT) == 0)
		{
			COConstant* pNewConstant = COConstant::LoadFromXML(pChild, pCOProject);
			AddConstant(pNewConstant);
		}
		else
			pCOProject->ThrowError(&Error::EXPECTED_CLASS_MEMBER_TAG, pChild);
	}
}

void COClass::LoadAllMethodDefinitions(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		int nLine = pChild->GetLineNumber();
		int nCol, nWid;
		pChild->GetOffsetAndWidth(&nCol, &nWid);
		if(stricmp(pChild->GetName(), TAG_NAME_PROCEDURE) == 0)
		{
			GXMLAttribute* pName = pChild->GetAttribute(ATTR_NAME);
			if(!pName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChild);
			const char* szProcName = pName->GetValue();
			char cModifier = '\0';
			if(*szProcName == '!' || *szProcName == '&')
			{
				cModifier = *szProcName;
				szProcName++;
			}
			Holder<COMethod*> hNewMethod(new COMethod(nLine, nCol, nWid, szProcName, this, true, pCOProject));
			hNewMethod.Get()->LoadAllParams(this, pChild, cModifier, pCOProject, bPartial);
			AddProcedure(hNewMethod.Drop());
		}
		else if(stricmp(pChild->GetName(), TAG_NAME_METHOD) == 0)
		{
			GXMLAttribute* pName = pChild->GetAttribute(ATTR_NAME);
			if(!pName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChild);
			const char* szMethodName = pName->GetValue();
			char cModifier = '\0';
			if(*szMethodName == '!' || *szMethodName == '&')
			{
				cModifier = *szMethodName;
				szMethodName++;
			}
			Holder<COMethod*> hNewMethod(new COMethod(nLine, nCol, nWid, szMethodName, this, false, pCOProject));
			hNewMethod.Get()->LoadAllParams(this, pChild, cModifier, pCOProject, bPartial);
			AddMethod(hNewMethod.Drop());
		}
		else if(stricmp(pChild->GetName(), TAG_NAME_INTERFACE) == 0)
		{
			GXMLAttribute* pName = pChild->GetAttribute(ATTR_NAME);
			if(!pName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChild);
			COInterface* pInterface = pCOProject->FindInterface(pName->GetValue());
			if(!pInterface)
				pCOProject->ThrowError(&Error::INTERFACE_NOT_FOUND, pChild);
			AddInterfaceRef(pInterface);
		}
		else if(stricmp(pChild->GetName(), TAG_NAME_VAR) == 0)
			continue;
		else if(stricmp(pChild->GetName(), TAG_NAME_CONSTANT) == 0)
			continue;
		else
			pCOProject->ThrowError(&Error::EXPECTED_CLASS_MEMBER_TAG, pChild);
	}
}

void COClass::LoadAllInstructions(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	int nMethod = 0;
	int nProcedure = 0;
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		if(stricmp(pChild->GetName(), TAG_NAME_PROCEDURE) == 0)
		{
			COMethod* pProcedure = GetProcedure(nProcedure);
			GXMLTag* pInstructions = pChild->GetChildTag(TAG_NAME_INSTRUCTIONS);
			if(!pInstructions)
				pCOProject->ThrowError(&Error::EXPECTED_COMMANDS_CHILD_TAG, pChild);
			pProcedure->GetInstructions()->LoadFromXML(pInstructions, pCOProject, bPartial);
			nProcedure++;
		}
		else if(stricmp(pChild->GetName(), TAG_NAME_METHOD) == 0)
		{
			COMethod* pMethod = GetMethod(nMethod);
			GXMLTag* pInstructions = pChild->GetChildTag(TAG_NAME_INSTRUCTIONS);
			if(!pInstructions)
				pCOProject->ThrowError(&Error::EXPECTED_COMMANDS_CHILD_TAG, pChild);
			pMethod->GetInstructions()->LoadFromXML(pInstructions, pCOProject, bPartial);
			nMethod++;
		}
		else if(stricmp(pChild->GetName(), TAG_NAME_INTERFACE) == 0)
			continue;
		else if(stricmp(pChild->GetName(), TAG_NAME_VAR) == 0)
			continue;
		else if(stricmp(pChild->GetName(), TAG_NAME_CONSTANT) == 0)
			continue;
		else
			pCOProject->ThrowError(&Error::EXPECTED_CLASS_MEMBER_TAG, pChild);
	}
	GAssert(nMethod == GetMethodCount(), "Methods don't line up");
	GAssert(nProcedure == GetProcedureCount(), "Procedures don't line up");
}

GXMLTag* COClass::SaveToXML()
{
	GXMLTag* pClass = new GXMLTag(TAG_NAME_CLASS);
	pClass->AddAttribute(new GXMLAttribute(ATTR_NAME,m_szName));

	GXMLAttribute* pAttribute = new GXMLAttribute(ATTR_PARENT, CLASS_NAME_OBJECT);
	pClass->AddAttribute(pAttribute);
	if(m_pParent)
		pAttribute->SetValue(m_pParent->GetName());

	// Constants
	int nCount;
	int n;
	nCount = GetConstantCount();
	for(n = 0; n < nCount; n++)
	{
		COConstant* pConstant = GetConstant(n);
		pClass->AddChildTag(pConstant->SaveToXML());
	}

	// Members
	nCount = GetExtendedMemberCount();
	for(n = 0; n < nCount; n++)
	{
		COVariable* pMember = GetExtendedMember(n);
		pClass->AddChildTag(pMember->SaveToXML());
	}

	// Interface refs
	nCount = GetInterfaceCount();
	for(n = 0; n < nCount; n++)
		pClass->AddChildTag(GetInterface(n)->SaveToXML());
	
	// Procedures
	nCount = GetProcedureCount();
	for(n = 0; n < nCount; n++)
	{
		COMethod* pProcedure = GetProcedure(n);
		pClass->AddChildTag(pProcedure->SaveToXML());
	}

	// Methods
	nCount = GetMethodCount();
	for(n = 0; n < nCount; n++)
	{
		COMethod* pMethod = GetMethod(n);
		pClass->AddChildTag(pMethod->SaveToXML());
	}
	return pClass;
}

GXMLTag* COClass::ToXMLForLibrary()
{
	GXMLTag* pClassTag = new GXMLTag(TAG_NAME_CLASS);
	pClassTag->AddAttribute(new GXMLAttribute(ATTR_NAME, GetName()));
	char szTmp[32];
	itoa(GetID(), szTmp, 10);
	pClassTag->AddAttribute(new GXMLAttribute(ATTR_ID, szTmp));
	if(GetParent())
	{
		itoa(GetParent()->GetID(), szTmp, 10);
		pClassTag->AddAttribute(new GXMLAttribute(ATTR_PARENTID, szTmp));
	}
	if(GetParent() != NULL)
		pClassTag->AddAttribute(new GXMLAttribute(ATTR_PARENT, GetParent()->GetName()));
	else
	{
		GAssert(stricmp(GetName(), CLASS_NAME_OBJECT) == 0, "class should have a parent");
	}
	const char* szSource = GetSource();
	if(szSource)
		pClassTag->AddAttribute(new GXMLAttribute(ATTR_SOURCE, szSource));

	// Constants
	int nCount;
	int n;
	nCount = GetConstantCount();
	for(n = 0; n < nCount; n++)
	{
		COConstant* pConstant = GetConstant(n);
		pClassTag->AddChildTag(pConstant->SaveToXML());
	}

	// Members
	nCount = GetExtendedMemberCount();
	for(n = 0; n < nCount; n++)
	{
		COVariable* pMember = GetExtendedMember(n);
		pClassTag->AddChildTag(pMember->SaveToXML());
	}

	// Interface refs
	nCount = GetInterfaceCount();
	for(n = 0; n < nCount; n++)
		pClassTag->AddChildTag(GetInterface(n)->ToXMLForImplementationInLibrary());

	return pClassTag;
}

void COClass::SaveToClassicSyntax(GQueue* pQ)
{
	pQ->Push("class ");
	pQ->Push(GetName());
	pQ->Push("(");
	pQ->Push(GetParent()->GetName());
	pQ->Push(")");
	pQ->Push("\n{\n");

	// Constants
	int nCount = GetConstantCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COConstant* pConstant = GetConstant(n);
		pConstant->SaveToClassicSyntax(pQ);
	}
	if(nCount > 0)
		pQ->Push("\n");

	// Members
	nCount = GetExtendedMemberCount();
	for(n = 0; n < nCount; n++)
	{
		COVariable* pMember = GetExtendedMember(n);
		pQ->Push("\t");
		pMember->SaveToClassicSyntax(pQ);
		pQ->Push("\n");
	}
	pQ->Push("\n");

	// Procedures
	nCount = GetProcedureCount();
	for(n = 0; n < nCount; n++)
	{
		COMethod* pProcedure = GetProcedure(n);
		pProcedure->SaveToClassicSyntax(pQ);
	}

	// Methods
	nCount = GetMethodCount();
	for(n = 0; n < nCount; n++)
	{
		COMethod* pMethod = GetMethod(n);
		pMethod->SaveToClassicSyntax(pQ);
	}

	pQ->Push("}\n\n");
}

/*static*/ void COClass::FromClassicSyntax(ClassicSyntax* pParser)
{
	if(!pParser->EatToken("class "))
	{
		pParser->SetError(&Error::EXPECTED_CLASS_INTERFACE_OR_MACHINE);
		return;
	}
	pParser->m_pCurrentClassTag = new GXMLTag(TAG_NAME_CLASS);
	pParser->m_pCurrentClassTag->SetLineNumber(pParser->m_nLineNumber);
	pParser->m_pCurrentClassTag->SetColumnAndWidth(pParser->m_nPos - pParser->m_nLineStartPos + 1, 5);
	pParser->m_pCurrentFileTag->AddChildTag(pParser->m_pCurrentClassTag);
	CSToken* pTok = pParser->GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		pParser->SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	pParser->m_pCurrentClassTag->AddAttribute(new GXMLAttribute(ATTR_NAME, pTok->GetValue(), pTok->GetLength()));
	bool b = pParser->Advance();
	GAssert(b, "unexpected state");
	if(!pParser->EatToken("("))
	{
		pParser->SetError(&Error::EXPECTED_OPEN_PAREN);
		return;
	}
	pTok = pParser->GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		pParser->SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	pParser->m_pCurrentClassTag->AddAttribute(new GXMLAttribute(ATTR_PARENT, pTok->GetValue(), pTok->GetLength()));
	b = pParser->Advance();
	GAssert(b, "unexpected state");
	if(!pParser->EatToken(")"))
	{
		pParser->SetError(&Error::EXPECTED_CLOSE_PAREN);
		return;
	}
	if(!pParser->EatToken("{"))
	{
		pParser->SetError(&Error::EXPECTED_OPEN_SQUIGGLY_BRACE);
		return;
	}
	while(true)
	{
		pTok = pParser->GetToken(0);
		if(pTok->GetLength() <= 0)
		{
			pParser->SetError(&Error::EXPECTED_CLOSE_SQUIGGLY_BRACE);
			return;
		}
		if(pTok->StartsWith("}"))
			break;
		if(pTok->StartsWith("method "))
			COMethod::FromClassicSyntax(pParser, false);
		else if(pTok->StartsWith("proc "))
			COMethod::FromClassicSyntax(pParser, true);
		else if(pTok->StartsWith("const "))
			pParser->ParseConstant(pParser->m_pCurrentClassTag);
		else if(pTok->StartsWith("interface "))
			pParser->ParseInterfaceRef();
		else
			COVariable::FromClassicSyntax(pParser, pParser->m_pCurrentClassTag);
		if(pParser->m_pErrorHolder->HaveError())
			return;
	}
	b = pParser->EatToken("}");
	GAssert(b, "unexpected state");
}
/*
int COClass::FindImplementingMethod(COMethodDecl* pMethodDecl)
{
	COMethod* pMethod;
	int nCount = GetMethodCount();
	EMethodSignature* pMethodDeclSig = pMethodDecl->GetSignature();
	EMethodSignature* pMethodSig;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethod = GetMethod(n);
		pMethodSig = pMethod->GetSignature();
		if(pMethodDeclSig->Compare(pMethodSig) == 0)
			return n;
	}
	return -1;
}
*/
COMethod* COClass::FindMethod(EMethodSignature* pSignature)
{
	COMethod* pMethod;
	int nCount = GetProcedureCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethod = GetProcedure(n);
		if(pSignature->Compare(pMethod->GetSignature()) == 0)
			return pMethod;
	}
	nCount = GetMethodCount();
	for(n = 0; n < nCount; n++)
	{
		pMethod = GetMethod(n);
		if(pSignature->Compare(pMethod->GetSignature()) == 0)
			return pMethod;
	}
	if(m_pParent)
		return m_pParent->FindMethod(pSignature);
	return NULL;
}

COMethodDecl* COClass::FindVirtualTableIndex(int* pOutIndex, const char* szName, GPointerArray* pParams, ErrorStruct** ppError, int* pnErrorParam, COProject* pProject)
{
	COMethodDecl* pMethodDecl = NULL;
	if(m_pParent)
	{
		pMethodDecl = m_pParent->FindVirtualTableIndex(pOutIndex, szName, pParams, ppError, pnErrorParam, pProject);
		if(pMethodDecl)
			return pMethodDecl;
	}
	else
		*pOutIndex = 0;
	int nCount = GetInterfaceCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COInterface* pInterface = GetInterface(n);
		int index;
		COMethodDecl* pMethodDecl = pInterface->FindMethodDecl(&index, szName, pParams, ppError, pnErrorParam, pProject);
		(*pOutIndex) += 2; // for the interface ID and the next interface index
		if(pMethodDecl)
		{
			(*pOutIndex) += index;
			return pMethodDecl;
		}
		else
			(*pOutIndex) += pInterface->GetMethodDeclCount();
	}
	return NULL;
}

COMethod* COClass::FindMethod(const char* szName, GPointerArray* pParams, ErrorStruct** ppError, int* pnErrorParam, COProject* pProject)
{
	int nNameMatchCount = 0;
	COMethod* pNameMatch = NULL;
	COMethod* pMatch = NULL;
	COMethod* pMethod;
	*pnErrorParam = -1;
	ErrorStruct* pErrorMessage = NULL;
	int nCount = GetMethodCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethod = GetMethod(n);
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
			}
		}
	}
	nCount = GetProcedureCount();
	for(n = 0; n < nCount; n++)
	{
		pMethod = GetProcedure(n);
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
			}
		}
	}
	if(!pMatch)
	{
		// Check parent class
		ErrorStruct* pTmpError = NULL;
		int nTmpErrorParam;
		if(m_pParent)
		{
			pMatch = m_pParent->FindMethod(szName, pParams, &pTmpError, &nTmpErrorParam, pProject);
			if(pMatch)
				return pMatch;
		}

		// Return the appropriate error code
		if(pTmpError == &Error::NO_METHOD_MATCHES_SIGNATURE)
			return NULL;
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
	return pMatch;
}

int COClass::FindMember(COVariable* pMember)
{
	int nCount = GetExtendedMemberCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COVariable* pTmp = GetExtendedMember(n);
		if(pTmp == pMember)
			return m_pParent->GetTotalMemberCount() + n;
	}
	if(m_pParent)
		return m_pParent->FindMember(pMember);
	else
		return -1;
}

COVariable* COClass::FindMember(const char* pMemberName, int nLength)
{
	GTEMPBUF(pBuf, nLength + 1);
	memcpy(pBuf, pMemberName, nLength);
	pBuf[nLength] = '\0';
	return FindMember(pBuf);
}

COVariable* COClass::FindMember(const char* szMemberName)
{
	int nCount = GetExtendedMemberCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COVariable* pVariable = GetExtendedMember(n);
		if(stricmp(szMemberName, pVariable->GetName()) == 0)
			return pVariable;
	}
	if(m_pParent)
		return m_pParent->FindMember(szMemberName);
	else
		return NULL;
}

int COClass::FindConstant(COConstant* pConstant)
{
	int nCount = GetConstantCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COConstant* pConst = GetConstant(n);
		if(pConst == pConstant)
			return n;
	}
	return -1;
}

COConstant* COClass::FindConstant(const char* pName, int nLen)
{
	char* szTmp = (char*)alloca(nLen + 1);
	memcpy(szTmp, pName, nLen);
	szTmp[nLen] = '\0';
	return FindConstant(szTmp);
}

COConstant* COClass::FindConstant(const char* szConstantName)
{
	int nCount = GetConstantCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COConstant* pConst = GetConstant(n);
		if(stricmp(szConstantName, pConst->GetName()) == 0)
			return pConst;
	}
	return NULL;
}

bool COClass::DoesInherritFrom(COClass* pClass)
{
	COClass* pMyLine;
	for(pMyLine = this; pMyLine; pMyLine = pMyLine->GetParent())
	{
		if(pMyLine == pClass)
			return true;
	}
	return false;
}

bool COClass::DoesImplement(COInterface* pThat)
{
	int n;
	int nCount = GetInterfaceCount();
	COInterface* pInterface;
	for(n = 0; n < nCount; n++)
	{
		pInterface = GetInterface(n);
		if(pInterface == pThat)
			return true;
	}
	return false;
}

const char* COClass::GetFilename()
{
	if(m_pFile) return m_pFile->GetFilename(); else return NULL;
}

void COClass::SetVariable(COVariable* pVariable)
{
	delete(m_pVariable);
	m_pVariable = pVariable;
}

int COClass::GetInterfaceCount()
{
	return m_pInterfaceRefs ? m_pInterfaceRefs->GetSize() : 0;
}

void COClass::AddInterfaceRef(COInterface* pInterface)
{
	if(!m_pInterfaceRefs)
		m_pInterfaceRefs = new GPointerArray(4);
	m_pInterfaceRefs->AddPointer(pInterface);
}

COInterface* COClass::GetInterface(int nIndex)
{
	GAssert(m_pInterfaceRefs, "Out of range");
	return (COInterface*)m_pInterfaceRefs->GetPointer(nIndex);
}

COVariable* COClass::GetVariable(COProject* pProject)
{
	if(m_pVariable == NULL)
	{
		int nLine = GetLineNumber();
		int nCol, nWid;
		GetColumnAndWidth(&nCol, &nWid);
		m_pVariable = new COVariable(nLine, nCol, nWid, GetName(), pProject->m_pInternalClass, true, true, false);
	}
	return m_pVariable;
}

bool COClass::Compile(GCompiler* pCompiler)
{
	// Compile Class name, parent, members, and implementations
	GAssert(GetParent(), "All compile-able classes must have a valid parent class");
	if(!pCompiler->CompileClassStart(this))
	{
		pCompiler->CheckError();
		return false;
	}

	// Compile methods
	int nCount = GetMethodCount();	
	COMethod* pMethod;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pMethod = GetMethod(n);
		if(!pMethod->Compile(pCompiler))
		{
			pCompiler->CheckError();
			return false;
		}
	}

	// Compile Procedures
	nCount = GetProcedureCount();
	for(n = 0; n < nCount; n++)
	{
		pMethod = GetProcedure(n);
		if(!pMethod->Compile(pCompiler))
		{
			pCompiler->CheckError();
			return false;
		}
	}

	return true;
}

