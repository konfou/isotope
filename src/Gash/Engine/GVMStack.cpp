/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "../Include/GashEngine.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#include "GVMStack.h"
#include <string.h>
#include "EClass.h"
#include "EMethod.h"
#include "TagNames.h"
#include "EInstrArray.h"
#include "../CodeObjects/Instruction.h"
#include "../CodeObjects/Class.h"
#include "../CodeObjects/Method.h"
#include "../CodeObjects/File.h"

// if you change these values, it will break push and pop
#define CHUNKS_PER_TABLE 2048
#define OBS_PER_CHUNK 1024

GVMStack::GVMStack()
{
#ifdef _DEBUG
	m_pDEBUGStack = NULL;
#endif // _DEBUG
	//GAssert(sizeof(struct StackObject) == 5, "Stack Object is unexpected size.  You need to #pragma Pack(1)");
	GAssert(sizeof(struct StackObject*) == 4, "Stack Object pointer is unexpected size");
	m_pChunkTable = new EVar*[CHUNKS_PER_TABLE];
	memset(m_pChunkTable, 0, CHUNKS_PER_TABLE * sizeof(EVar*));
	m_nBasePointer = 0;
	m_nLocalPointer = 0;
	m_nStackPointer = 0;
}

GVMStack::~GVMStack()
{
	GAssert(m_nStackPointer == 0, "There's still stuff on the stack");
	int n;
	for(n = 0; m_pChunkTable[n] != NULL; n++)
		delete m_pChunkTable[n];
	delete(m_pChunkTable);
}

void GVMStack::Push(EVar* pCopyMeOntoStack)
{
	EVar* pChunk = m_pChunkTable[m_nStackPointer >> 10];
	if(!pChunk)
	{
		if((m_nStackPointer >> 10) >= CHUNKS_PER_TABLE - 1)
		{
			// Stack overflow--we're hosed!
			GAssert(false, "Stack Overflow");
			throw "stack overflow";
		}
		m_pChunkTable[m_nStackPointer >> 10] = new EVar[OBS_PER_CHUNK];
		pChunk = m_pChunkTable[m_nStackPointer >> 10];
		GAssert(pChunk, "Memory allocation error");
#ifdef _DEBUG
		if((m_nStackPointer >> 10) == 0)
			m_pDEBUGStack = (struct DEBUG_Stack*)pChunk;
#endif // _DEBUG
	}
	memcpy(&pChunk[m_nStackPointer & 1023], pCopyMeOntoStack, sizeof(EVar));
	m_nStackPointer++;
}

EVar* GVMStack::Pop()
{
	GAssert(m_nStackPointer > 0, "Underflow--nothing on the stack to pop");
	m_nStackPointer--;
	return(&(m_pChunkTable[m_nStackPointer >> 10])[m_nStackPointer & 1023]);
}

void GVMStack::StartScope()
{
	EVar vv;
	vv.eObType = VT_LOCAL_POINTER;
	vv.pOb = (GObject*)m_nLocalPointer;
	Push(&vv);
	m_nLocalPointer = m_nStackPointer;
}

// This will return an array of "struct CallStackLayer"s, but only the
// pCodePos, and nStackTop fields will be initialized
void GVMStack::GetCallStack(GCallStackLayerArray* pOutCallStack, unsigned char* pInstructionPointer)
{
	pOutCallStack->Clear();
	struct CallStackLayer csl;
	memset(&csl, '\0', sizeof(struct CallStackLayer));

	// Add top layer
	csl.pCodePos = pInstructionPointer;
	csl.nStackTop = m_nStackPointer;
	pOutCallStack->AddLayer(&csl);

	// Find and add all the other layers
	int nStackPos = m_nBasePointer;
	EVar* pvv;
	while(true)
	{
		nStackPos--;
		pvv = GetVariableNotRelative(nStackPos);
		GAssert(pvv->eObType == VT_OB_REF, "stack problem");

		nStackPos--;
		pvv = GetVariableNotRelative(nStackPos);
		GAssert(pvv->eObType == VT_RETURN_POSITION, "stack problem");
		if(!pvv->pOb)
			break;
		csl.pCodePos = (unsigned char*)pvv->pOb;

		nStackPos--;
		pvv = GetVariableNotRelative(nStackPos);
		GAssert(pvv->eObType == VT_BASE_POINTER, "stack problem");
		GAssert((int)pvv->pOb < nStackPos && (int)pvv->pOb > 0, "stack problem");
		csl.nStackTop = nStackPos;
		nStackPos = (int)pvv->pOb;
		
		pOutCallStack->AddLayer(&csl);
	}
}

void GVMStack::DumpObject(GString* pString, const char* szPrefix, const char* szVarName, GObject* pOb, Library* pLibrary, int nCurrentDepth, int nMaxDepth, bool bIncludeRefCounts)
{
	if(nCurrentDepth >= nMaxDepth)
		return;
	int n;
	for(n = 0; n < nCurrentDepth; n++)
		pString->Add(L"\t");
	if(szPrefix)
		pString->Add(szPrefix);
	EType* pType = NULL;
	if(pOb)
	{
		pType = pOb->GetType();
		pString->Add(pType->GetName());
	}
	if(szVarName)
	{
		if(pType)
			pString->Add(L":");
		pString->Add(szVarName);
	}
	if(!pOb)
	{
		if(szVarName)
			pString->Add(L" = ");
		pString->Add(L"<null>\n");
		return;
	}
	if(bIncludeRefCounts)
	{
		pString->Add(L" (");
		pString->Add((int)pOb->nRefCount);
		pString->Add(L", ");
		pString->Add((int)pOb->nPinnedRefs);
		pString->Add(L")");
	}
	if(pType->GetTypeType() == EType::TT_CLASS)
	{
		EClass* pClass = (EClass*)pType;
		if(pClass->IsIntegerType())
		{
			pString->Add(L" = ");
			pString->Add(((IntObject*)pOb)->m_value);
			pString->Add(L"\n");
		}
		else
		{
			pString->Add(L"\n");
			int nExtendedMembers = pClass->GetExtendedMemberCount();
			int nTotalMembers = pClass->GetTotalMemberCount();
			for(n = 0; n < nTotalMembers; n++)
			{
				GXMLTag* pTag = pClass->GetMemberTag(n, pLibrary);
				const char* szVarName = NULL;
				if(pTag)
				{
					GXMLAttribute* pExpAttr = pTag->GetAttribute(ATTR_EXP);
					if(pExpAttr)
					{
						szVarName = pExpAttr->GetValue();
						while(*szVarName != ':' && *szVarName != '\0')
							szVarName++;
						if(*szVarName == ':')
							szVarName++;
						if(*szVarName == '\0')
							szVarName = NULL;
					}
				}
				DumpObject(pString, NULL, szVarName, ((ObjectObject*)pOb)->arrFields[n], pLibrary, nCurrentDepth + 1, nMaxDepth, bIncludeRefCounts);
			}
		}
	}
	else
	{
		if(pType->GetTypeType() == EType::TT_MACHINE)
		{
			wchar_t pBuf[64];
			((WrapperObject*)pOb)->GetDisplayValue(pBuf, 64);
			pString->Add(L" = ");
			pString->Add(pBuf);
		}
		pString->Add(L"\n");
	}
}

void GVMStack::DumpStack(GString* pString, GVM* pVM, int nMaxObjectDepth, bool bIncludeRefCounts)
{
	// Print the location
	unsigned char* pInstrPointer = pVM->m_pInstructionPointer;
	Library* pLibrary = pVM->GetLibrary();
	EMethod* pMethod = NULL;
	int nMethod = -1;
	if(pInstrPointer)
	{
		nMethod = pVM->FindWhichMethodThisIsIn(pInstrPointer);
		if(nMethod >= 0)
		{
			int nSize;
			pMethod = pLibrary->GetEMethod(nMethod);
			unsigned char* pMethodStart = pVM->GetCompiledMethod(nMethod, &nSize);
			COProject* pProject = pLibrary->GetProject();
			EInstrArray* pInstrArray = pMethod->GetEInstrArray(pProject ? pMethod->GetCOMethod(pProject) : NULL);
			int nInstr = pInstrArray->FindInstrByOffset(pInstrPointer - pMethodStart);
			if(nInstr >= 0)
			{
				if(nInstr > 0)
					nInstr--; // Decrement because the instruction pointer should already be pointing to the next instruction
				COInstruction* pInstr = pInstrArray->GetCOInstruction(nInstr);
				if(pInstr)
				{
					COMethod* pMeth = pInstr->GetMethod();
					COClass* pClass = pMeth->GetClass();
					COFile* pFile = pClass->GetFile();
					pString->Add(L"File: ");
					pString->Add(pFile->GetFilename());
					pString->Add(L"\nLine ");
					pString->Add(pInstr->GetLineNumber());
					pString->Add(L": ");
					GQueue q;
					pInstr->SaveToClassicSyntax(&q, 0, true);
					Holder<char*> hInstr(q.DumpToString());
					pString->Add(hInstr.Get());
					pString->Add(L"\n\n");			
				}
			}
		}
	}

	// Print the stack trace
	int nParam = -1;
	const wchar_t* pSig;
	const char* szParamName;
	const char* szPrefix;
	int n;
	for(n = m_nStackPointer - 1; n >= 0; n--)
	{
		//pString->Add(n - m_nBasePointer);
		//pString->Add(L") ");
		EVar* pVar = GetVariableNotRelative(n);
		switch(pVar->eObType)
		{
			case VT_OB_REF:
				if(nParam >= 0)
				{
					szParamName = pMethod->FindParamName(nParam);
					szPrefix = "Parameter ";
				}
				else
				{
					szParamName = NULL;
					szPrefix = "Local ";
				}
				DumpObject(pString, szPrefix, szParamName, pVar->pOb, pLibrary, 1, nMaxObjectDepth, bIncludeRefCounts);
				nParam--;
				break;

			case VT_RETURN_POSITION:
				nMethod = pVM->FindWhichMethodThisIsIn(pInstrPointer);
				pMethod = pLibrary->GetEMethod(nMethod);
				pString->Add(pMethod->GetClass()->GetName());
				pString->Add(L".");
				pSig = pMethod->GetSignature()->GetString();
				pString->Add(pSig);
				pString->Add(L"\n");
				nParam = pMethod->CountParams() - 1;
				pInstrPointer = (unsigned char*)pVar->pOb;
				break;

			case VT_BASE_POINTER:
				//pString->Add(L"Base Pointer = ");
				//pString->Add((int)pVar->pOb - m_nBasePointer);
				//pString->Add(L"\n");
				break;

			case VT_LOCAL_POINTER:
				//pString->Add(L"\t{\n");
				break;

			default:
				GAssert(false, "Unexpected variable type on stack");
		}
	}
}

