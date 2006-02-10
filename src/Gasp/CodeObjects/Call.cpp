/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Call.h"
#include "Method.h"
#include "Project.h"
#include "Instruction.h"
#include "Class.h"
#include "Expression.h"
#include "Interface.h"
#include "InstrArray.h"
#include "Variable.h"
#include "../Engine/GCompiler.h"
#include "../Engine/EInstrArray.h"
#include "../Engine/InstrSet.h"
#include "../Engine/EvalExprResult.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GMacros.h"
#include <wchar.h>


COCall::COCall(int nLine, int nCol, int nWid, COInstrArray* pParent, COExpressionArray* pParameters)
: COInstruction(nLine, nCol, nWid, pParent)
{
	m_pCatcher = NULL;
	m_pParameters = NULL;
	SetParameters(pParameters);
}

COCall::~COCall()
{
	delete(m_pCatcher);
	SetParameters(NULL);
}

void COCall::SetCatcher(COExpression* pCatcher)
{
	delete(m_pCatcher);
	m_pCatcher = pCatcher;
}

void COCall::SetParameters(COExpressionArray* pParameters)
{
	if(m_pParameters == pParameters)
		return;
	delete(m_pParameters);
	m_pParameters = pParameters;
}

COExpressionArray* COCall::DropParameters()
{
	COExpressionArray* pParams = m_pParameters;
	m_pParameters = NULL;
	return pParams;
}

int COCall::GetParamCount()
{
	if(m_pParameters)
		return m_pParameters->GetSize();
	else
		return 0;
}

COExpression* COCall::GetParam(int n)
{
	return m_pParameters->GetExpression(n);
}

/*static*/ COCall* COCall::FromXML(GXMLTag* pTag, COInstrArray* pParent, COProject* pCOProject, bool bPartial, int* pnInstructionIndex)
{
	int nLine = pTag->GetLineNumber();
	int nCol, nWid;
	pTag->GetOffsetAndWidth(&nCol, &nWid);

	// Get the name of the method we're calling
	GXMLAttribute* pNameAttr = pTag->GetAttribute(ATTR_NAME);
	const char* szName = "";
	if(pNameAttr)
		szName = pNameAttr->GetValue();

	// Figure out the class or interface of the method we're calling
	Holder<COExpressionArray*> hParamList(NULL);
	COMethod* pCallingMethod = pParent->GetMethod();
	COType* pType;
	bool bStatic = true;
	GXMLAttribute* pThisAttr = pTag->GetAttribute(ATTR_THIS);
	if(pThisAttr)
	{
		// Check for class-specified procedure call
		pType = pCOProject->FindClass(pThisAttr->GetValue());

		// Check for method or interface call
		if(!pType)
		{
			bStatic = false;
			COExpression* pParam = COExpression::LoadFromExpression(pThisAttr->GetValue(), pParent, pCOProject, pTag, bPartial);
			hParamList.Set(new COExpressionArray());
			hParamList.Get()->AddExpression(pParam);
			pType = pParam->GetType(pCOProject);
		}
	}
	else
		pType = pCallingMethod->GetClass();

	// Parse the catcher
	COExpression* pCatcher = NULL;
	GXMLAttribute* pCatcherAttr = pTag->GetAttribute(ATTR_CATCH);
	if(pCatcherAttr)
		pCatcher = COExpression::LoadFromExpression(pCatcherAttr->GetValue(), pParent, pCOProject, pTag, bPartial);

	// Determine if this call can have children
	bool bCanHaveChildren = false;
	if(stricmp(szName, "if") == 0 || stricmp(szName, "while") == 0) // todo: unmagic
		bCanHaveChildren = true;

	// Load parameters (and find child instructions tag if there is one)
	GXMLTag* pInstructionsTag = NULL;
	LoadParameters(pTag, &hParamList, bCanHaveChildren, pParent, &pInstructionsTag, pCOProject, bPartial);

	// See if it's an interface call
	Holder<COCall*> hNewCall(NULL);
	ErrorStruct* pErrorMessage = &Error::METHOD_NOT_FOUND;
	int nErrorParam;
	COType* pTypeOrig = pType;
	if(pType->GetTypeType() != COType::TT_CLASS)
	{
		COInterface* pInterface = (COInterface*)pType;
		int nIndex;
		COMethodDecl* pMethodDecl = pInterface->FindMethodDecl(&nIndex, szName, hParamList.Get(), &pErrorMessage, &nErrorParam, pCOProject);
		if(pMethodDecl)
			hNewCall.Set(new COInterfaceCall(nLine, nCol, nWid, pInterface, nIndex, pParent, hParamList.Drop()));
		else
			pType = pCOProject->m_pObject; // all interfaces and machine objects inherrit directly from Object, so the only place a match could be found now is in Object
	}

	// See if it's a normal method call
	if(!hNewCall.Get())
	{
		// Look for a method that we can call statically
		COMethod* pMethod = ((COClass*)pType)->FindMethod(szName, hParamList.Get(), &pErrorMessage, &nErrorParam, pCOProject);
		if(!pMethod && bPartial)
			pMethod = pCOProject->GetNoop(); // a bogus method
		if(pMethod)
		{
			GAssert(!bCanHaveChildren || pMethod == pCOProject->m_pObject_if || pMethod == pCOProject->m_pObject_while, "Problem determining if it can have children");
			if(bCanHaveChildren)
				hNewCall.Set(new COMethodCallThatCanHaveChildren(nLine, nCol, nWid, pMethod, pParent, hParamList.Drop()));
			else
				hNewCall.Set(new COMethodCall(nLine, nCol, nWid, pMethod, pParent, hParamList.Drop()));
		}
	}

	// See if it's a virtual call
	if(!hNewCall.Get())
	{
		if(pType->GetTypeType() == COType::TT_CLASS)
		{
			int nIndex;
			COClass* pClass = (COClass*)pType;
			ErrorStruct* pOldMessage = pErrorMessage;
			COMethodDecl* pVirtualMethodDecl = pClass->FindVirtualTableIndex(&nIndex, szName, hParamList.Get(), &pErrorMessage, &nErrorParam, pCOProject);
			if(pVirtualMethodDecl)
				hNewCall.Set(new COVirtualCall(nLine, nCol, nWid, pClass, nIndex, pVirtualMethodDecl, pParent, hParamList.Drop()));
			else
			{
				if(pErrorMessage == &Error::METHOD_NOT_FOUND) // the old message was probably more useful
					pErrorMessage = pOldMessage;
			}
		}
	}

	// If we haven't found anything to call yet, something's wrong
	if(!hNewCall.Get())
	{
		pType = pTypeOrig;
		if(pErrorMessage == &Error::METHOD_NOT_FOUND)
		{
			char* szError = (char*)alloca(strlen(pType->GetName()) + strlen(szName) + 10);
			strcpy(szError, pType->GetName());
			strcat(szError, ".");
			strcat(szError, szName);
			pCOProject->ThrowError(pErrorMessage, pTag, szError);
		}
		else
			pCOProject->ThrowError(pErrorMessage, pTag); // todo: indicate which param had problem
	}

	// Set the catcher (if there is one)
	if(pCatcher)
		hNewCall.Get()->SetCatcher(pCatcher);

	// Load child instructions
	if(pInstructionsTag)
	{
		if(!bCanHaveChildren)
			pCOProject->ThrowError(&Error::INSTRUCTION_CANT_HAVE_CHILDREN, pTag);
		COMethodCallThatCanHaveChildren* pThisCall = (COMethodCallThatCanHaveChildren*)hNewCall.Get();
		pThisCall->m_pInstrArray->LoadFromXML(pInstructionsTag, pCOProject, bPartial, pnInstructionIndex);
	}

	return hNewCall.Drop();
}

/*static*/ bool COCall::LoadParameters(GXMLTag* pTag, Holder<COExpressionArray*>* pParamListHolder, bool bCanHaveChildren, COInstrArray* pParent, GXMLTag** ppInstructionsTag, COProject* pCOProject, bool bPartial)
{
	*ppInstructionsTag = NULL;
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		if(stricmp(pChild->GetName(), TAG_NAME_PARAM) == 0)
		{
			Holder<COExpression*> hParam(COExpression::LoadFromXML(pChild, pParent, pCOProject, bPartial));
			if(!pParamListHolder->Get())
				pParamListHolder->Set(new COExpressionArray());
			pParamListHolder->Get()->AddExpression(hParam.Drop());
		}
		else if(stricmp(pChild->GetName(), TAG_NAME_INSTRUCTIONS) == 0)
			*ppInstructionsTag = pChild;
		else
			pCOProject->ThrowError(&Error::EXPECTED_PARAM_TAG, pChild);
	}
	return true;
}

/*virtual*/ COVariable* COCall::FindVariable(const char* pName, int nLength)
{
	if(!m_pParameters)
		return NULL;
	COVariable* pVar;
	COExpression* pExpression;
	int nCount = GetParamCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pExpression = GetParam(n);
		pVar = pExpression->FindVariable(pName, nLength);
		if(pVar)
			return pVar;
	}
	return NULL;
}

bool COCall::SymbolModeCompile(GCompiler* pCompiler, COMethod* pMethod, COInstruction* pSymbolInstr)
{
	if(GetCallType() == CT_METHOD && 
		((COMethodCall*)this)->GetMethod() != pCompiler->m_pCOProject->GetNoop() &&
		((COMethodCall*)this)->GetMethod()->GetClass()->GetFile() == pCompiler->m_pCOProject->m_pBuiltIn)
		return pCompiler->CompileCallToBuiltInMethod((COMethodCall*)this, pMethod, pSymbolInstr);
	if(!pCompiler->CompileCallStart(this, pSymbolInstr))
	{
		pCompiler->CheckError();
		return false;
	}
	int nCount = GetParamCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pCompiler->m_pEInstrArrayBuilder->AddInstr(Instr_PushParameter, pSymbolInstr);
		pCompiler->m_pEInstrArrayBuilder->AddParam(0);
	}
	if(!pCompiler->CompileMakeTheCall(this, NULL, pSymbolInstr, 0))
	{
		pCompiler->CheckError();
		return false;
	}
	if(!pCompiler->CompileCallFinish(this, pSymbolInstr))
	{
		pCompiler->CheckError();
		return false;
	}
	return true;
}

/*virtual*/ bool COCall::Compile(GCompiler* pCompiler, COMethod* pSourceMethod, COInstruction* pSymbolInstr)
{
	if(pCompiler->m_bSymbolMode)
		return SymbolModeCompile(pCompiler, pSourceMethod, pSymbolInstr);

	if(!pCompiler->CheckParameters(this))
	{
		pCompiler->CheckError();
		return false;
	}

	// See if it's really a built-in asm Instruction
	if(GetCallType() == CT_METHOD && ((COMethodCall*)this)->GetMethod()->GetClass()->GetFile() == pCompiler->m_pCOProject->m_pBuiltIn)
		return pCompiler->CompileCallToBuiltInMethod((COMethodCall*)this, pSourceMethod, pSymbolInstr);

	// Compile the procedure call start
	if(!pCompiler->CompileCallStart(this, pSymbolInstr))
	{
		pCompiler->CheckError();
		return false;
	}

	bool bRet = true;
	{
		// Evaluate the catcher (if there is one)
		Holder<EvalExprResult*> hCatcherVar(NULL);
		if(GetCatcher())
		{
			if(!GetCatcher()->Compile(true, true, pCompiler, &hCatcherVar, pSymbolInstr))
			{
				pCompiler->CheckError();
				return false;
			}
		}

		// Compile the parameters
		ParamVarArrayHolder pvah(GetParamCount());
		if(!pCompiler->CompileParams(this, &pvah))
		{
			pCompiler->CheckError();
			return false;
		}

		// Push parameters on stack
		int nThisOffset;
		int nTempOffset;
		if(bRet)
		{
			int n;
			int nCount = GetParamCount();
			for(n = 0; n < nCount; n++)
			{
				COVariable* pVar = pvah.GetParamVar(n);
				if(!pCompiler->CompileCallParameter(pVar, n, pSymbolInstr, (n == 0 ? &nThisOffset : &nTempOffset)))
				{
					pCompiler->CheckError();
					bRet = false;
					break;
				}
			}
		}

		if(bRet)
		{
			// Make the call
			if(!pCompiler->CompileMakeTheCall(this, hCatcherVar.Get() ? hCatcherVar.Get()->GetResult() : NULL, pSymbolInstr, nThisOffset))
			{
				pCompiler->CheckError();
				bRet = false;
			}
		}

		// Copy modifiable variables back to originals
		if(bRet)
		{
			int n;
			for(n = GetParamCount() - 1; n >= 0; n--)
			{
				COVariable* pTargetVar = GetTargetParam(n);
				if(!pTargetVar->IsVarReadOnly())
				{
					COType* pParamType = GetParam(n)->GetType(pCompiler->m_pCOProject);
					bool bNeedCast;
					bool bOK = pTargetVar->GetType()->CanCastTo(pParamType, &bNeedCast);
					GAssert(bOK, "If the target var can't be casted to the param, the call should have never compiled");
					COVariable* pVar = pvah.GetParamVar(n);
					if(!pCompiler->CompileSetDeclFromParam(pVar, n, pSymbolInstr, bNeedCast ? pParamType : NULL))
					{
						pCompiler->CheckError();
						bRet = false;
						break;
					}
				}
			}
		}
	}

	if(bRet)
	{
		// Finish up with the call
		if(!pCompiler->CompileCallFinish(this, pSymbolInstr))
		{
			pCompiler->CheckError();
			bRet = false;
		}
	}

	return bRet;
}

/*static*/ bool COCall::CompileImplicitCall(GCompiler* pCompiler, COType* pType, EMethodSignature* pSig, COExpressionArray* pParamList, COInstruction* pSymbolInstr, COInstrArray* pParent)
{
	int nLine = pSymbolInstr->GetLineNumber();
	int nCol, nWid;
	pSymbolInstr->GetColumnAndWidth(&nCol, &nWid);
	switch(pType->GetTypeType())
	{
		case COType::TT_CLASS:
			{
				// Find the method
				COClass* pClass = (COClass*)pType;
				COMethod* pMethod = pClass->FindMethod(pSig);
				//if(!pMethod && pClass->DoesInherritFrom(pCompiler->m_pCOProject->m_pInteger))
				//	pMethod = 
				if(!pMethod)
				{
					const wchar_t* wszSig = pSig->GetString();
					ConvertUnicodeToAnsi(wszSig, szSig);
					pCompiler->SetError(&Error::COULDNT_FIND_IMPLICIT_CALL_TARGET, pSymbolInstr, szSig);
					return false;
				}

				// Call the method
				COMethodCall call(nLine, nCol, nWid, pMethod, pParent, pParamList);
				if(!call.Compile(pCompiler, NULL, pSymbolInstr))
				{
					call.DropParameters();
					pCompiler->CheckError();
					return false;
				}
				call.DropParameters();
			}
			break;

		case COType::TT_MACHINE:
			{
				// Find the method decl
				COMachineClass* pMachineClass = (COMachineClass*)pType;
				int nMethodDeclIndex = pMachineClass->FindMethodDecl(pSig);
				if(nMethodDeclIndex < 0)
				{
					const wchar_t* wszSig = pSig->GetString();
					ConvertUnicodeToAnsi(wszSig, szSig);
					pCompiler->SetError(&Error::COULDNT_FIND_IMPLICIT_CALL_TARGET, pSymbolInstr, szSig);
					return false;
				}

				// Call the machine method
				COInterfaceCall call(nLine, nCol, nWid, pMachineClass, nMethodDeclIndex, pParent, pParamList);
				if(!call.Compile(pCompiler, NULL, pSymbolInstr))
				{
					call.DropParameters();
					pCompiler->CheckError();
					return false;
				}
				call.DropParameters();
			}
			break;

		default:
			{
				const wchar_t* wszSig = pSig->GetString();
				ConvertUnicodeToAnsi(wszSig, szSig);
				pCompiler->SetError(&Error::COULDNT_FIND_IMPLICIT_CALL_TARGET, pSymbolInstr, szSig);
				return false;
			}
	}
	return true;
}

// --------------------------------------------------------------------------

COMethodCall::COMethodCall(int nLine, int nCol, int nWid, COMethod* pTarget, COInstrArray* pParent, COExpressionArray* pParameters)
 : COCall(nLine, nCol, nWid, pParent, pParameters)
{
	m_pMethod = pTarget;
}

COMethodCall::~COMethodCall()
{

}

bool COMethodCall::SetMethod(COMethod* pMethod, COExpressionArray* pParameters)
{
	m_pMethod = pMethod;
	SetParameters(pParameters);
	return true;
}

/*virtual*/ int COMethodCall::GetTargetParamCount()
{
	return m_pMethod->GetParameterCount();
}

COVariable* COMethodCall::GetTargetParam(int n)
{
	return m_pMethod->GetParameter(n);
}

/*virtual*/ COType* COMethodCall::GetTargetType()
{
	return m_pMethod->GetClass();
}

GXMLTag* COMethodCall::SaveToXML(COInstrArray* pParent)
{
	// Make the call tag
	GXMLTag* pCallTag = new GXMLTag(TAG_NAME_CALL);
	int n = 0;
	if(m_pMethod->IsStatic())
	{
		COMethod* pCallingMethod = pParent->GetMethod();
		if(m_pMethod->GetClass() != pCallingMethod->GetClass())
			pCallTag->AddAttribute(new GXMLAttribute(ATTR_THIS, m_pMethod->GetClass()->GetName()));
	}
	else
	{
		GAssert(GetParamCount() > 0, "No 'This' param");
		COExpression* pParam = GetParam(0);
		int nSize = pParam->ToString(NULL);
		GTEMPBUF(pBuff, nSize);
		pParam->ToString(pBuff);
		pCallTag->AddAttribute(new GXMLAttribute(ATTR_THIS, pBuff));
		n++;
	}
	pCallTag->AddAttribute(new GXMLAttribute(ATTR_NAME, m_pMethod->GetName()));

	// Add Parameters
	int nCount = GetParamCount();
	for( ; n < nCount; n++)
	{
		COExpression* pParam = GetParam(n);
		pCallTag->AddChildTag(pParam->SaveToXML());
	}

	return pCallTag;
}

COMethodCallThatCanHaveChildren::COMethodCallThatCanHaveChildren(int nLine, int nCol, int nWid, COMethod* pTarget, COInstrArray* pParent, COExpressionArray* pParameters)
	: COMethodCall(nLine, nCol, nWid, pTarget, pParent, pParameters) // takes ownership of pParameters
{
	m_pInstrArray = new COInstrArray(nLine, nCol, nWid, pParent);
}

COMethodCallThatCanHaveChildren::~COMethodCallThatCanHaveChildren()
{
	delete(m_pInstrArray);
}


GXMLTag* COMethodCallThatCanHaveChildren::SaveToXML(COInstrArray* pParent)
{
	GXMLTag* pCallTag = COMethodCall::SaveToXML(pParent);

	// Add child instructions
	m_pInstrArray->SaveToXML(pCallTag);
	
	return pCallTag;
}

void COMethodCallThatCanHaveChildren::SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay)
{
	// Method name
	pQ->Push(GetMethod()->GetName());
	pQ->Push("(");

	// Parameters
	int nCount = GetParamCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COExpression* pParam = GetParam(n);
		if(n > 0)
			pQ->Push(" ");
		pParam->SaveToClassicSyntax(pQ);
	}
	pQ->Push(")");
	if(bDisplay)
		return;
	pQ->Push("\n");

	// Add child instructions
	m_pInstrArray->SaveToClassicSyntax(pQ, nTabs);
}

/*virtual*/ COInstruction* COMethodCallThatCanHaveChildren::FindInstruction(int nIndex)
{
	if(m_pInstrArray)
		return m_pInstrArray->FindInstruction(nIndex);
	else
		return NULL;
}

void COMethodCall::SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay)
{
	// Tabs
	int n;
	for(n = 0; n < nTabs; n++)
		pQ->Push("\t");

	// Special case for "else"
	if(stricmp(GetMethod()->GetClass()->GetName(), CLASS_NAME_ASM) == 0 &&
		stricmp(GetMethod()->GetName(), METHOD_NAME_ELSE) == 0)
	{
		pQ->Push("}");
		for(n = 0; n < nTabs; n++)
			pQ->Push("\t");
		pQ->Push(GetMethod()->GetName());
		pQ->Push("\n");
		for(n = 0; n < nTabs; n++)
			pQ->Push("\t");
		pQ->Push("{");
		return;
	}

	// 'This' variable
	COExpression* pParam = NULL;
	n = 0;
	if(GetMethod()->IsStatic())
		pQ->Push(GetMethod()->GetClass()->GetName());
	else
	{
		pParam = GetParam(0);
		pParam->SaveToClassicSyntax(pQ);
		n++;
	}

	// Method name
	pQ->Push(".");
	pQ->Push(GetMethod()->GetName());
	pQ->Push("(");

	// Parameters
	bool bFirst = true;
	int nCount = GetParamCount();
	for( ; n < nCount; n++)
	{
		pParam = GetParam(n);
		if(bFirst)
			bFirst = false;
		else
			pQ->Push(", ");
		pParam->SaveToClassicSyntax(pQ);
	}
	pQ->Push(")");
	if(!bDisplay)
		pQ->Push("\n");
}

// --------------------------------------------------------------------------

COVirtualCall::COVirtualCall(int nLine, int nCol, int nWid, COClass* pClass, int nVirtualTableIndex, COMethodDecl* pMethodDecl, COInstrArray* pParent, COExpressionArray* pParameters)
: COCall(nLine, nCol, nWid, pParent, pParameters)
{
	m_pClass = pClass;
	m_nVirtualTableIndex = nVirtualTableIndex;
	m_pMethodDecl = pMethodDecl;
}

/*virtual*/ COVirtualCall::~COVirtualCall()
{
}

/*virtual*/ int COVirtualCall::GetTargetParamCount()
{
	return m_pMethodDecl->GetParameterCount();
}

/*virtual*/ COVariable* COVirtualCall::GetTargetParam(int n)
{
	return m_pMethodDecl->GetParameter(n);
}

/*virtual*/ GXMLTag* COVirtualCall::SaveToXML(COInstrArray* pParent)
{
	GAssert(false, "todo: write me");
	return NULL;
}

/*virtual*/ void COVirtualCall::SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay)
{
	GAssert(false, "todo: write me");
}

/*virtual*/ COType* COVirtualCall::GetTargetType()
{
	return m_pClass;
}

// --------------------------------------------------------------------------

COInterfaceCall::COInterfaceCall(int nLine, int nCol, int nWid, COInterface* pInterface, int nMethodDeclIndex, COInstrArray* pParent, COExpressionArray* pParameters)
: COCall(nLine, nCol, nWid, pParent, pParameters)
{
	m_pInterface = pInterface;
	m_nMethodDeclIndex = nMethodDeclIndex;
}

COInterfaceCall::~COInterfaceCall()
{
}

COMethodDecl* COInterfaceCall::GetMethodDecl()
{
	return m_pInterface->GetMethodDecl(m_nMethodDeclIndex);
}

int COInterfaceCall::GetTargetParamCount()
{
	return GetMethodDecl()->GetParameterCount();
}

COVariable* COInterfaceCall::GetTargetParam(int n)
{
	return GetMethodDecl()->GetParameter(n);
}

COType* COInterfaceCall::GetTargetType()
{
	return m_pInterface;
}

GXMLTag* COInterfaceCall::SaveToXML(COInstrArray* pParent)
{
	// Make the call tag
	GXMLTag* pCall = new GXMLTag(TAG_NAME_CALL);
	COExpression* pParam = GetParam(0);
	int nSize = pParam->ToString(NULL);
	GTEMPBUF(pBuff, nSize);
	pParam->ToString(pBuff);
	pCall->AddAttribute(new GXMLAttribute(ATTR_THIS, pBuff));
	pCall->AddAttribute(new GXMLAttribute(ATTR_NAME, GetMethodDecl()->GetName()));

	// Add Parameters
	int nCount = GetParamCount();
	int n;
	for(n = 1; n < nCount; n++)
	{
		pParam = GetParam(n);
		pCall->AddChildTag(pParam->SaveToXML());
	}

	return pCall;
}

void COInterfaceCall::SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay)
{
	// Tabs
	int n;
	for(n = 0; n < nTabs; n++)
		pQ->Push("\t");

	// 'This' variable
	COExpression* pParam = GetParam(0);
	pParam->SaveToClassicSyntax(pQ);

	// MethodDecl name
	pQ->Push(".");
	pQ->Push(GetMethodDecl()->GetName());
	pQ->Push("(");

	// Parameters
	int nCount = GetParamCount();
	for(n = 1; n < nCount; n++)
	{
		pParam = GetParam(n);
		if(n > 1)
			pQ->Push(", ");
		pParam->SaveToClassicSyntax(pQ);
	}

	pQ->Push(")");
	if(!bDisplay)
		pQ->Push("\n");
}

