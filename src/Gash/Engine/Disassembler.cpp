/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <string.h>
#include "Disassembler.h"
#include "TagNames.h"
#include "InstrTable.h"
#include "EInstrArray.h"
#include "InstrSet.h"
#include "EMethod.h"
#include "EClass.h"
#include "EInterface.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GXML.h"
#include "../Include/GashEngine.h"
#include <stdlib.h>

Disassembler::Disassembler(Library* pLibrary)
{
	m_pLibrary = pLibrary;
	m_nCurrentLine = 1;
}

Disassembler::~Disassembler()
{

}

char* Disassembler::DisassembleLibraryToText(Library* pLibrary, int* pnOutLength)
{
	Disassembler tmp(pLibrary);
	return tmp.LibraryToText(pnOutLength);
}

char* Disassembler::DisassembleEInstrArrayToText(EInstrArray* pEInstrArray, int* pnOutLength)
{
	Disassembler tmp(NULL);
	int nCount = pEInstrArray->GetInstrCount();
	int n;
	for(n = 0; n < nCount; n++)
		tmp.InstructionToText(pEInstrArray, n);
	return tmp.QueueToBuffer(pnOutLength);
}

void Disassembler::Add(const char* szText, int* pnPos)
{
	int n;
	for(n = 0; szText[n] != '\0'; n++)
	{
		if(szText[n] == '\n')
		{
			m_nCurrentLine++;
			if(pnPos)
				*pnPos = 0;
		}
	}
	m_q.Push(szText);
	if(pnPos)
		(*pnPos) += strlen(szText);
}

char* Disassembler::QueueToBuffer(int* pnOutLength)
{
	int nSize = m_q.GetSize();
	char* szDisassembly = new char[nSize + 1];
	int n;
	bool bRet;
	char c;
	for(n = 0; n < nSize; n++)
	{
		bRet = m_q.Pop(&c);
		GAssert(bRet, "Error with queue");
		szDisassembly[n] = c;
	}
	szDisassembly[n] = '\0';
	*pnOutLength = nSize;
	return szDisassembly;
}

char* Disassembler::LibraryToText(int* pnOutLength)
{
	int nTypeCount = m_pLibrary->GetTypeCount();
	int n;
	for(n = 1; n < nTypeCount; n++) // start at 1 because 0 is the "Object" class
	{
		EType* pEType = m_pLibrary->GetEType(n);
		if(pEType->GetTypeType() == EType::TT_CLASS)
		{
			EClass* pEClass = (EClass*)pEType;
			GXMLTag* pClassTag = pEType->GetTag();
			GAssert(stricmp(pClassTag->GetName(), TAG_NAME_CLASS) == 0, "expected a class tag");

			// Show the class name
			Add("class ");
			Add(pEClass->GetName());
		
			// Show the class ID
			char szTmp[32];
			itoa(n, szTmp, 10);
			Add(" // [ID: ");
			Add(szTmp);
			Add("]");

			// Show the class contents
			Add("\n{\n");
			//todo: show the members
			int i;
			for(i = 0; i < pEClass->GetMethodCount(); i++)
				MethodToText(pEClass->GetFirstMethodID() + i);
			Add("}\n\n");
		}
		else
		{
			EInterface* pInterface = (EInterface*)pEType;
			Add("interface ");
			Add(pInterface->GetName());
			Add("\n{\n");
			Add("\ttodo: show method decls here\n");
			Add("}\n\n");
		}
	}

	// Dump the queue to a text buffer
	return QueueToBuffer(pnOutLength);
}

void Disassembler::MethodToText(int nMethodID)
{
	EMethod* pEMethod = m_pLibrary->GetEMethod(nMethodID);
	GAssert(stricmp(pEMethod->GetTag()->GetName(), TAG_NAME_METHOD) == 0 || stricmp(pEMethod->GetTag()->GetName(), TAG_NAME_PROCEDURE) == 0, "expected a method or procedure tag");

	// Show the method name
	Add(" method ");
	Add(pEMethod->GetName());

	// Show the method ID
	char szTmp[32];
	itoa(nMethodID, szTmp, 10);
	Add(" // [ID: ");
	Add(szTmp);
	Add("]");
	Add("\n {\n");

	// Show the instructions in the method
	int nInstr;
	EInstrArray* pMB = pEMethod->GetEInstrArray();
	pMB->SetDisassemblyStartLine(m_nCurrentLine);
	int nSize = pMB->GetInstrCount();
	for(nInstr = 0; nInstr < nSize; nInstr++)
		InstructionToText(pMB, nInstr);
	Add(" }\n\n");
}

void Disassembler::InstructionToText(EInstrArray* pEInstrArray, int nInstr)
{
	// Tabulate scope
	int nPos = 0;
	Add("  ", &nPos);
	int nTmp = pEInstrArray->GetStackInstr(nInstr);
	while(nTmp >= 0)
	{
		Add(" ", &nPos);
		nTmp = pEInstrArray->GetStackInstr(nTmp);
	}

	// Show the Instruction
	InstrBin* pInstruction = pEInstrArray->GetInstr(nInstr);
	struct InstructionStruct* pInstr = pInstruction->GetInstrStruct();
	Add(pInstr->szName, &nPos);

	// Show Parameters
	char szTmp[64];
	int nParamCount = pInstr->nParamCount;
	if(pInstr->pMethod != Instr_StartScope && pInstr->pMethod != Instr_EndScope)
	{
		Add("(", &nPos);
		int n;
		for(n = 0; n < nParamCount; n++)
		{
			itoa(pInstruction->GetParam(n), szTmp, 10);
			Add(szTmp, &nPos);
			if(n < nParamCount - 1)
				Add(", ", &nPos);
		}
		Add(")", &nPos);
	}

	// Add comment marks
	while(nPos < 47)
		Add(" ", &nPos);
	Add("//", &nPos);

	// Show the instruction number
	while(nPos < 50)
		Add(" ", &nPos);
	itoa(nInstr, szTmp, 10);
	Add(szTmp, &nPos);

	// Show the code position
	while(nPos < 55)
		Add(" ", &nPos);
	itoa(pEInstrArray->GetOffset(nInstr), szTmp, 10);
	Add(szTmp, &nPos);

	// Show the stack Instr
	while(nPos < 60)
		Add(" ", &nPos);
	itoa(pEInstrArray->GetStackInstr(nInstr), szTmp, 10);
	Add(szTmp, &nPos);

	// Show extra info
	if(pInstr->pMethod == Instr_Call)
	{
		if(m_pLibrary)
		{
			// Show the name of the called method
			EMethod* pEMethod = m_pLibrary->GetEMethod(pInstruction->GetParam(nParamCount - 1));
			const char* szName = pEMethod->GetName();
			GXMLTag* pClassTag = pEMethod->GetTag();
			GXMLAttribute* pClassName = pClassTag->GetAttribute(ATTR_NAME);
			if(pClassName && szName)
			{
				Add(" [");
				Add(pClassName->GetValue());
				Add(".");
				Add(szName);
				Add("]");
			}
		}
	}
	else if(pInstr->pMethod == Instr_MCall)
	{
		// todo: rewrite this to utilize szParamTypes in InstrStruct
	}
	else if(pInstr->pMethod == Instr_NewVariable)
	{
		// Show the variable number
		nTmp = pEInstrArray->GetStackInstr(nInstr);
		int nCount = 0;
		while(nTmp >= 0)
		{
			nCount++;
			nTmp = pEInstrArray->GetStackInstr(nTmp);
		}
		Add(" [Pos: ");
		char szTmp[32];
		itoa(nCount, szTmp, 10);
		Add(szTmp);
		Add("]");
	}
	else if(pInstr->szParamTypes[pInstr->nParamCount - 1] == 'j')
	{
		// Show the target code position
		char szTmp[32];
		itoa(pEInstrArray->GetOffset(nInstr) + 4 * nParamCount + 1 + pInstruction->GetParam(nParamCount - 1), szTmp, 10);
		Add(" [Target: ");
		Add(szTmp);
		Add("]");
	}
	Add("\n");
}
