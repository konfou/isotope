/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Method.h"
#include "Project.h"
#include "Variable.h"
#include "Instruction.h"
#include "Class.h"
#include "Call.h"
#include "InstrArray.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#include "../Engine/ClassicSyntax.h"
#include "../Engine/GCompiler.h"
#include "../Engine/EInstrArray.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

COMethodDecl::COMethodDecl(int nLine, int nCol, int nWid, const char* szName, COType* pType, bool bStatic, COProject* pProject)
: COScope(nLine, nCol, nWid, NULL)
{
	GAssert(szName, "Must have valid name");
	m_pType = pType;
	m_bStatic = bStatic;
	m_szName = new char[strlen(szName) + 1];
	strcpy(m_szName, szName);
	m_pParamArray = NULL;
	m_pSignature = NULL;
	m_nID = pProject->GetUniqueMethodID();
}

COMethodDecl::~COMethodDecl()
{
	// Delete Parameters
	if(m_pParamArray)
	{
		COVariable* pVariable;
		int nCount = m_pParamArray->GetSize();
		int n;
		for(n = 0; n < nCount; n++)
		{
			pVariable = GetParameter(n);
			delete(pVariable);
		}
		delete(m_pParamArray);
	}

	delete(m_szName);
	delete(m_pSignature);
}

EMethodSignature* COMethodDecl::GetSignature()
{
	if(m_pSignature == NULL)
		m_pSignature = new EMethodSignature(this);
	return m_pSignature;
}

void COMethodDecl::SetSignature(EMethodSignature* pSig)
{
	delete(m_pSignature);
	m_pSignature = pSig;
}

/*virtual*/ GXMLTag* COMethodDecl::SaveToXML()
{
	// Save the method tag and name attribute
	GXMLTag* pTag;
	char* szModifier = "";
	if(m_bStatic)
		pTag = new GXMLTag(TAG_NAME_PROCEDURE);
	else
	{
		pTag = new GXMLTag(TAG_NAME_METHOD);
		COVariable* pThisVar = GetParameter(0);
		GAssert(pThisVar, "no this var");
		if(!pThisVar->IsVarReadOnly())
			szModifier = "!";
		else if(!pThisVar->IsObjReadOnly())
			szModifier = "&";
	}
	char* szMethodName = (char*)alloca(strlen(m_szName) + 2);
	strcpy(szMethodName, szModifier);
	strcat(szMethodName, m_szName);
	pTag->AddAttribute(new GXMLAttribute(ATTR_NAME, szMethodName));

	// Save all the Parameters
	COVariable* pVariable;
	int nCount = GetParameterCount();
	int n = m_bStatic ? 0 : 1;
	for( ; n < nCount; n++)
	{
		pVariable = GetParameter(n);
		pTag->AddChildTag(pVariable->SaveToXML());
	}
	return pTag;
}

COVariable* COMethodDecl::FindVariable(const char* pName, int nLength)
{
	// Try each parameter
	int n;
	int nCount = GetParameterCount();
	COVariable* pVariable;
	for(n = 0; n < nCount; n++)
	{
		pVariable = GetParameter(n);
		if(strnicmp(pVariable->GetName(), pName, nLength) == 0 && pVariable->GetName()[nLength] == '\0')
			return pVariable;
	}
	return NULL;
}

void COMethodDecl::SaveToClassicSyntax(GQueue* pQ)
{
	// determine if read only
	bool bObjIsReadOnly = true;
	bool bVarIsReadOnly = true;
	if(!IsStatic())
	{
		COVariable* pThisVar = GetParameter(0);
		bObjIsReadOnly = pThisVar->IsObjReadOnly();
		bVarIsReadOnly = pThisVar->IsVarReadOnly();
		GAssert(bVarIsReadOnly || !bObjIsReadOnly, "if you can modify the var, you can modify the obj");
	}

	// method name	
	if(IsStatic())
		pQ->Push("\tproc ");
	else
		pQ->Push("\tmethod ");
	if(!bVarIsReadOnly)
		pQ->Push("!");
	else if(!bObjIsReadOnly)
		pQ->Push("&");		
	pQ->Push(GetName());
	pQ->Push("(");

	// Save Variables
	COVariable* pVariable;
	int nCount = GetParameterCount();
	int n = IsStatic() ? 0 : 1;
	bool bFirst = true;
	for( ; n < nCount; n++)
	{
		if(bFirst)
			bFirst = false;
		else
			pQ->Push(", ");
		pVariable = GetParameter(n);
		pVariable->SaveToClassicSyntax(pQ);
	}
	pQ->Push(")\n\t");
}

bool COMethodDecl::CheckParams(GPointerArray* pParams, ErrorStruct** ppError, int* pnErrorParam, COProject* pProject)
{
	*pnErrorParam = -1;
	int nCount = pParams ? pParams->GetSize() : 0;
	if(nCount < GetParameterCount())
	{
		*ppError = &Error::NOT_ENOUGH_PARAMETERS;
		return false;
	}
	else if(nCount > GetParameterCount())
	{
		*ppError = &Error::TOO_MANY_PARAMETERS;
		return false;
	}
	int n;
	for(n = 0; n < nCount; n++)
	{
		COExpression* pParam = (COExpression*)pParams->GetPointer(n);
		COType* pParamType = pParam->GetType(pProject);
		COType* pVarType = GetParameter(n)->GetType();
		bool bNeedCast;
		if(!pParamType->CanCastTo(pVarType, &bNeedCast, ppError))
		{
			*pnErrorParam = n;
			return false;
		}
	}
	return true;
}

Library* COMethodDecl::GetLibrary()
{
	return m_pType->GetLibrary();
}

bool COMethodDecl::Compile(GCompiler* pCompiler)
{
	// Make an XML tag for the method
	Holder<GXMLTag*> hMethodTag(NULL);
	if(IsStatic())
		hMethodTag.Set(new GXMLTag(TAG_NAME_PROCEDURE));
	else
		hMethodTag.Set(new GXMLTag(TAG_NAME_METHOD));

	// Add the "Name" attribute
	char* pMethodName = (char*)alloca(1 + strlen(GetName()) + 1);
	const char* szModifier = "";
	if(!IsStatic())
	{
		COVariable* pThisVar = GetParameter(0);
		if(!pThisVar->IsVarReadOnly())
			szModifier = "!";
		else if(!pThisVar->IsObjReadOnly())
			szModifier = "&";
	}
	strcpy(pMethodName, "");
	strcat(pMethodName, szModifier);
	strcat(pMethodName, GetName());
	hMethodTag.Get()->AddAttribute(new GXMLAttribute(ATTR_NAME, pMethodName));

	// Add the "ID" attribute
	char szID[32];
	itoa(GetID(), szID, 10);
	hMethodTag.Get()->AddAttribute(new GXMLAttribute(ATTR_ID, szID));

	// Add parameters to the XML tag
	int n = 0;
	if(!IsStatic())
		n++; // Skip the "This" pointer
	int nParamCount = GetParameterCount();
	COVariable* pVariable;
	for( ; n < nParamCount; n++)
	{
		pVariable = GetParameter(n);
		GXMLTag* pVariableTag = pVariable->SaveToXML();
		hMethodTag.Get()->AddChildTag(pVariableTag);
	}

	// Compile the instructions
	if(!CompileInstructions(hMethodTag.Get(), pCompiler))
		return false;

	// Finish up
	pCompiler->m_pCurrentClassTag->AddChildTag(hMethodTag.Drop());
	return true;
}

/*virtual*/ bool COMethodDecl::CompileInstructions(GXMLTag* pTag, GCompiler* pCompiler)
{
	// Abstract methods have no instructions
	return true;
}


// --------------------------------------------------------------------






COMethod::COMethod(int nLine, int nCol, int nWid, const char* szName, COClass* pClass, bool bStatic, COProject* pProject)
 : COMethodDecl(nLine, nCol, nWid, szName, pClass, bStatic, pProject)
{
	m_pInstrArray = new COInstrArray(nLine, nCol, nWid, this);
}

COMethod::~COMethod()
{
	delete(m_pInstrArray);
}

void COMethodDecl::LoadAllParams(COType* pThisType, GXMLTag* pTag, char cModifier, COProject* pCOProject, bool bPartial)
{
	int nLine = pTag->GetLineNumber();
	int nCol, nWid;
	pTag->GetOffsetAndWidth(&nCol, &nWid);
	GAssert(!m_pParamArray, "Array for params already created!");
//	GAssert(!m_pVariable, "A Decl for this method already created!");
//	m_pVariable = new COVariable(m_szName, pCOProject->m_pInternalMethod, true, true);

	// Load all the parameters
	if(m_bStatic)
	{
		if(cModifier != '\0')
			pCOProject->ThrowError(&Error::PROCS_HAVE_NO_THIS_VAR_TO_MODIFY, pTag);
	}
	else
	{
		// Interpret modifier
		bool bThisVarIsReadOnly = true;
		bool bThisObjIsReadOnly = true;
		if(cModifier == '!')
		{
			bThisVarIsReadOnly = false;
			bThisObjIsReadOnly = false;
		}
		else if(cModifier == '&')
			bThisObjIsReadOnly = false;

		// Add the declaration for the this pointer
		COVariable* pNewParameter = new COVariable(nLine, nCol, nWid, VAL_THIS, pThisType, bThisVarIsReadOnly, bThisObjIsReadOnly, false);
		AddParameter(pNewParameter);
	}
	GXMLTag* pChild;
	if(pTag)
	{
		for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
		{
			if(stricmp(pChild->GetName(), TAG_NAME_VAR) != 0)
				break; // Params are required to come before other tags
			COVariable* pNewParameter = COVariable::FromXML(pChild, pCOProject, bPartial);
			AddParameter(pNewParameter);
		}
	}
	if(bPartial)
		SetSignature(new EMethodSignature(pTag)); // get the signature now since we won't be able to make later it with the bogus partial params
}

/*virtual*/ GXMLTag* COMethod::SaveToXML()
{
	GXMLTag* pTag = COMethodDecl::SaveToXML();

	// Save all the Instructions
	GXMLTag* pInstructionsTag = new GXMLTag(TAG_NAME_INSTRUCTIONS);
	pTag->AddChildTag(pInstructionsTag);
	m_pInstrArray->SaveToXML(pInstructionsTag);

	return pTag;
}

void COMethod::SaveToClassicSyntax(GQueue* pQ)
{
	COMethodDecl::SaveToClassicSyntax(pQ);

	// Save all the Instructions
	m_pInstrArray->SaveToClassicSyntax(pQ, 1);
	pQ->Push("\n\n");
}

/*static*/ void COMethod::FromClassicSyntax(ClassicSyntax* pParser, bool bStatic)
{
	// Method name
	int nStartCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
	bool b;
	if(bStatic)
		b = pParser->EatToken("proc ");
	else
		b = pParser->EatToken("method ");
	GAssert(b, "unexpected state");
	pParser->m_pCurrentMethodTag = new GXMLTag(bStatic ? TAG_NAME_PROCEDURE : TAG_NAME_METHOD);
	pParser->m_pCurrentMethodTag->SetLineNumber(pParser->m_nLineNumber);
	pParser->m_pCurrentClassTag->AddChildTag(pParser->m_pCurrentMethodTag);
	CSToken* pTok = pParser->GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		pParser->SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	pParser->m_pCurrentMethodTag->AddAttribute(new GXMLAttribute(ATTR_NAME, pTok->GetValue(), pTok->GetLength()));
	b = pParser->Advance();
	GAssert(b, "unexpected state");

	// Parameters
	if(!pParser->EatToken("("))
	{
		pParser->SetError(&Error::EXPECTED_OPEN_PAREN);
		return;
	}
	bool bFirst = true;
	while(true)
	{
		pTok = pParser->GetToken(0);
		if(pTok->GetLength() <= 0)
		{
			pParser->SetError(&Error::UNEXPECTED_EOF);
			return;
		}
		if(pTok->StartsWith(")"))
			break;
		if(bFirst)
			bFirst = false;
		else
		{
			if(!pParser->EatToken(","))
			{
				pParser->SetError(&Error::EXPECTED_COMMA_TOKEN);
				return;
			}
		}
		COVariable::FromClassicSyntax(pParser, pParser->m_pCurrentMethodTag);
		if(pParser->m_pErrorHolder->HaveError())
			return;
	}
	b = pParser->EatToken(")");
	int nEndCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
	pParser->m_pCurrentMethodTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);

	// Instructions
	GAssert(b, "unexpected state");
	GXMLTag* pInstrTag = new GXMLTag(TAG_NAME_INSTRUCTIONS);
	pInstrTag->SetLineNumber(pParser->m_nLineNumber);
	pParser->m_pCurrentMethodTag->AddChildTag(pInstrTag);
	COInstruction::FromClassicSyntax(pParser, pInstrTag, ClassicSyntax::IT_METH);
}

const char* COMethod::GetFilename()
{
	return ((COClass*)m_pType)->GetFilename(); 
}


/*virtual*/ bool COMethod::CompileInstructions(GXMLTag* pTag, GCompiler* pCompiler)
{
	// Compile the bits
	Holder<EInstrArray*> hMB(Compile2(pCompiler));
	if(!hMB.Get())
	{
		pCompiler->CheckError();
		return false;
	}

	// Put the bits in the XML tags
	hMB.Get()->SetBinTags(pTag);

	return true;
}


EInstrArray* COMethod::Compile2(GCompiler* pCompiler)
{
	// Compile the method start
	if(!pCompiler->CompileMethodStart(this))
	{
		pCompiler->CheckError();
		return NULL;
	}

	// Put Parameters on the variable stack
	int n;
	int nParamCount = GetParameterCount();
	COVariable* pVariable;
	for(n = 0; n < nParamCount; n++)
	{
		pVariable = GetParameter(n);
		if(!pCompiler->CompileMethodParameter(pVariable))
		{
			pCompiler->CheckError();
			return NULL;
		}
	}
	if(!pCompiler->CompileMethodPostParameters())
	{
		pCompiler->CheckError();
		return NULL;
	}

	// Compile each instruction
	COInstruction* pInstruction;
	COInstrArray* pInstructions = GetInstructions();
	int nInstructionCount = pInstructions->GetInstrCount();
	for(n = 0; n < nInstructionCount; n++)
	{
		pInstruction = pInstructions->GetInstr(n);
		if(!pInstruction->Compile(pCompiler, this, pInstruction))
		{
			pCompiler->CheckError();
			return NULL;
		}
	}

	// Finish up with the method
	EInstrArray* pMB = pCompiler->CompileMethodFinish(this);
	if(!pMB)
	{
		pCompiler->CheckError();
		return NULL;
	}
	return pMB;
}

EMethod* COMethod::GetEMethod()
{
	Library* pLibrary = GetLibrary();
	if(!pLibrary)
	{
		GAssert(false, "not a library method");
		return NULL;
	}
	struct MethodRef mr;
	if(!pLibrary->FindMethod(&mr, this))
	{
		GAssert(false, "method not found");
		return NULL;
	}
	if(mr.bVirtual)
	{
		GAssert(false, "method is abstract");
		return NULL;
	}
	return pLibrary->GetEMethod(mr.nIndex);
}
