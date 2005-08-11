/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "EInstrArray.h"
#include "EMethod.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GPointerQueue.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GLList.h"
#include <string.h>
#include "TagNames.h"
#include "../Include/GashEngine.h"
#include "InstrTable.h"
#include "InstrSet.h"
#include "GCompiler.h"
#include "../CodeObjects/Instruction.h"
#include "../CodeObjects/Method.h"
#include "EType.h"

#define DATA_BYTES_PER_XML_LINE 1024

unsigned int InstrBin::GetParam(int n)
{
#ifdef DARWIN
	return ReverseEndian(pParams[n]);
#else // DARWIN
	return pParams[n];
#endif // !DARWIN
}

void InstrBin::SetParam(int nIndex, unsigned int nValue)
{
#ifdef DARWIN
	pParams[nIndex] = ReverseEndian(nValue);
#else // DARWIN
	pParams[nIndex] = nValue;
#endif // !DARWIN
}

InstructionStruct* InstrBin::GetInstrStruct()
{
	return GetInstructionStruct(cInstr);
}

// --------------------------------------------------------------------

EInstrArray::EInstrArray(unsigned char* pData, int nSize)
{
	m_pData = pData;
	m_nSize = nSize;
	m_bNeedToDeleteData = false;
	m_instrIndexes = NULL;
	m_nInstrCount = 0;
	m_pSymbols = NULL;
	m_stackInstructions = NULL;
	m_nDisassemblyStartLine = -1;
}

EInstrArray::EInstrArray(GXMLTag* pMethodTag, COMethod* pMethod)
{
	m_pSymbols = NULL;
	m_instrIndexes = NULL;
	m_nInstrCount = 0;
	m_stackInstructions = NULL;

	// Count the space needed for the data
	m_nSize = 0;
	GXMLTag* pBin;
	bool bGotSymbols = false;
	for(pBin = pMethodTag->GetFirstChildTag(); pBin; pBin = pMethodTag->GetNextChildTag(pBin))
	{
		if(stricmp(pBin->GetName(), TAG_NAME_BIN) != 0)
		{
			if(pMethod && !bGotSymbols && stricmp(pBin->GetName(), TAG_NAME_SYM) == 0)
				bGotSymbols = true;
			continue; // skip non "Bin" tags
		}
		GXMLAttribute* pData = pBin->GetAttribute(ATTR_DATA);
		if(!pData)
		{
			GAssert(false, "No Data attribute");
			continue;
		}
		int nLen = strlen(pData->GetValue());
		if((nLen < 1) || (nLen & 1))
		{
			GAssert(false, "Invalid string length");
			continue;
		}
		m_nSize += (nLen >> 1);
	}

	// Convert to binary data
	m_pData = new unsigned char[m_nSize];
	m_bNeedToDeleteData = true;
	int nPos = 0;
	int nParamBytes = 0;
	for(pBin = pMethodTag->GetFirstChildTag(); pBin; pBin = pMethodTag->GetNextChildTag(pBin))
	{
		if(stricmp(pBin->GetName(), TAG_NAME_BIN) != 0)
			continue; // skip non "Bin" tags
		GXMLAttribute* pData = pBin->GetAttribute(ATTR_DATA);
		if(!pData)
		{
			GAssert(false, "No Data attribute");
			continue;
		}
		const char* szData = pData->GetValue();
		char h1, h2;
		while(true)
		{
			h1 = *szData;
			if(!h1)
				break;
			szData++;
			h2 = *szData;
			if(!h2)
			{
				GAssert(false, "uneven number of hex digits");
				break;
			}
			szData++;
			GAssert(nPos < m_nSize, "buffer overrun");
			unsigned char byte = HexToByte(h1, h2);
			m_pData[nPos] = byte;
			nPos++;
		}
	}

	// Load the symbols
	if(bGotSymbols && pMethod)
	{
		int nInstrCount = GetInstrCount();
		m_pSymbols = new COInstruction*[nInstrCount];
		int nInstr = 0;
		GXMLTag* pSymTag;
		for(pSymTag = pMethodTag->GetFirstChildTag(); pSymTag; pSymTag = pMethodTag->GetNextChildTag(pSymTag))
		{
			if(stricmp(pSymTag->GetName(), TAG_NAME_SYM) != 0)
				continue; // skip non "Sym" tags
			GXMLAttribute* pData = pSymTag->GetAttribute(ATTR_DATA);
			if(!pData)
			{
				GAssert(false, "No Data attribute");
				continue;
			}
			const char* szData = pData->GetValue();
			GAssert(sizeof(int) == 4, "unexpected int size");
			while(nInstr < nInstrCount &&
					szData[0] != '\0' &&
					szData[1] != '\0' &&
					szData[2] != '\0' &&
					szData[3] != '\0' &&
					szData[4] != '\0' &&
					szData[5] != '\0' &&
					szData[6] != '\0' &&
					szData[7] != '\0')
			{
				int nIndex;
				HexToBuffer(szData, 8, (unsigned char*)&nIndex);
				m_pSymbols[nInstr++] = pMethod->FindInstruction(nIndex);
				szData += 8;
			}
		}
		if(nInstr < nInstrCount)
		{
			GAssert(false, "Incomplete symbols");
			while(nInstr < nInstrCount)
				m_pSymbols[nInstr++] = NULL;
		}
	}
}

EInstrArray::~EInstrArray()
{
	delete(m_stackInstructions);
	if(m_bNeedToDeleteData)
		delete(m_pData);
	delete(m_instrIndexes);
	delete(m_pSymbols);
}

void EInstrArray::IndexInstructions()
{
	int nPos;
	int nCount;
	int nPass;
	int nParamBytes = 0;

	// Pass 1 -- count instructions
	// Pass 2 -- build the array that indexes to the code
	for(nPass = 0; nPass < 2; nPass++)
	{
		if(nPass == 1)
		{
			m_nInstrCount = nCount;
			delete(m_instrIndexes);
			m_instrIndexes = new int[m_nInstrCount];
		}
		nCount = 0;
		for(nPos = 0; nPos < m_nSize; nPos++)
		{
			unsigned char byte = m_pData[nPos];
			if(nParamBytes == 0)
			{
				if(nPass == 1)
					m_instrIndexes[nCount] = nPos;
				nCount++;
				nParamBytes = GetInstructionStruct(byte)->nParamCount * 4;
			}
			else
				nParamBytes--;
		}
	}
}

int EInstrArray::GetInstrCount()
{
	if(!m_instrIndexes)
		IndexInstructions();
	return m_nInstrCount;
}

int EInstrArray::GetOffset(int nInstr)
{
	if(!m_instrIndexes)
		IndexInstructions();
	GAssert(nInstr >=0 && nInstr < m_nInstrCount, "Out of range");
	GAssert(m_instrIndexes[nInstr] >= 0 && m_instrIndexes[nInstr] < m_nSize, "indexing problem");
	return m_instrIndexes[nInstr];
}

struct InstrBin* EInstrArray::GetInstr(int nInstr)
{
	if(!m_instrIndexes)
		IndexInstructions();
	GAssert(nInstr >= 0 && nInstr < m_nInstrCount, "Out of range");
	GAssert(m_instrIndexes[nInstr] >= 0 && m_instrIndexes[nInstr] < m_nSize, "indexing problem");
	return (struct InstrBin*)&m_pData[m_instrIndexes[nInstr]];
}

void EInstrArray::SetBinTags(GXMLTag* pMethodTag)
{
	GAssert(stricmp(pMethodTag->GetName(), TAG_NAME_METHOD) == 0 || stricmp(pMethodTag->GetName(), TAG_NAME_PROCEDURE) == 0, "Not a method tag");

	// Delete all Bin tags
	GXMLTag* pPrev = NULL;
	GXMLTag* pCurrent;
	for(pCurrent = pMethodTag->GetFirstChildTag(); pCurrent; pCurrent = pMethodTag->GetNextChildTag(pCurrent))
	{
		if(stricmp(pCurrent->GetName(), TAG_NAME_BIN) == 0)
		{
			pMethodTag->DeleteChildTag(pPrev);
			pCurrent = pPrev;
		}
		pPrev = pCurrent;
	}

	// Make new Bin tags
	char szHex[2 * DATA_BYTES_PER_XML_LINE + 1];
	int n;
	int nHexPos = 0;
	for(n = 0; n < m_nSize; n++)
	{
		ByteToHex(m_pData[n], szHex + nHexPos);
		nHexPos += 2;
		if(nHexPos >= DATA_BYTES_PER_XML_LINE)
		{
			szHex[nHexPos] = '\0';
			GXMLTag* pBinTag = new GXMLTag(TAG_NAME_BIN);
			pBinTag->AddAttribute(new GXMLAttribute(ATTR_DATA, szHex));
			pMethodTag->AddChildTag(pBinTag);
			nHexPos = 0;
		}
	}
	if(nHexPos > 0)
	{
		szHex[nHexPos] = '\0';
		GXMLTag* pBinTag = new GXMLTag(TAG_NAME_BIN);
		pBinTag->AddAttribute(new GXMLAttribute(ATTR_DATA, szHex));
		pMethodTag->AddChildTag(pBinTag);
	}

	// Add Sym tags
	if(m_pSymbols)
	{
		nHexPos = 0;
		GAssert(DATA_BYTES_PER_XML_LINE % sizeof(int) == 0, "expected this value to be integer aligned");
		for(n = 0; n < m_nInstrCount; n++)
		{
			COInstruction* pInstr = m_pSymbols[n];
			int nIndex = pInstr ? pInstr->GetIndex() : -1;
			unsigned char* pBytes = (unsigned char*)&nIndex;
			BufferToHex(pBytes, sizeof(int), szHex + nHexPos);
			nHexPos += (2 * sizeof(int));
			if(nHexPos >= DATA_BYTES_PER_XML_LINE)
			{
				GAssert(nHexPos == DATA_BYTES_PER_XML_LINE, "bad alignment");
				szHex[nHexPos] = '\0';
				GXMLTag* pSymTag = new GXMLTag(TAG_NAME_SYM);
				pSymTag->AddAttribute(new GXMLAttribute(ATTR_DATA, szHex));
				pMethodTag->AddChildTag(pSymTag);
				nHexPos = 0;
			}			
		}
		if(nHexPos > 0)
		{
			szHex[nHexPos] = '\0';
			GXMLTag* pSymTag = new GXMLTag(TAG_NAME_SYM);
			pSymTag->AddAttribute(new GXMLAttribute(ATTR_DATA, szHex));
			pMethodTag->AddChildTag(pSymTag);
		}
	}
}

unsigned char* EInstrArray::GetData()
{
	return m_pData;
}

int EInstrArray::FindInstrByOffset(int nOffset)
{
	int nMin = 0;
	int nMax = GetInstrCount();
	int nMid;
	int nTmp;
	while(true)
	{
		nMid = (nMin + nMax) / 2;
		nTmp = GetOffset(nMid);
		if(nTmp < nOffset)
		{
            if(nMid == nMin)
				nMin++;
            else
			    nMin = nMid;
		}
		else if(nTmp > nOffset)
		{
            if(nMid + 1 == nMax)
                nMax--;
            else
    			nMax = nMid + 1;
		}
		else
			return nMid;
        if(nMax <= nMin)
            break;
	}
	return -1;
}

bool EInstrArray::DoesInstrPush(int nInstr)
{
	struct InstrBin* pInstr = GetInstr(nInstr);
	struct InstructionStruct* pInsStr = pInstr->GetInstrStruct();
	if(pInsStr->pMethod == Instr_StartScope ||
		pInsStr->pMethod == Instr_NewVariable ||
		pInsStr->pMethod == Instr_PushParameter)
		return true;
	else
		return false;
}

bool EInstrArray::IsStartScope(int nInstr)
{
	struct InstrBin* pInstr = GetInstr(nInstr);
	struct InstructionStruct* pInsStr = pInstr->GetInstrStruct();
	if(pInsStr->pMethod == Instr_StartScope)
		return true;
	else
		return false;
}

bool EInstrArray::IsEndScope(int nInstr)
{
	struct InstrBin* pInstr = GetInstr(nInstr);
	struct InstructionStruct* pInsStr = pInstr->GetInstrStruct();
	if(pInsStr->pMethod == Instr_EndScope)
		return true;
	else
		return false;
}

COInstruction* EInstrArray::GetCOInstruction(int nInstr)
{
	if(!m_pSymbols)
		return NULL;
	GAssert(nInstr >= 0 && nInstr < GetInstrCount(), "out of range");
	return m_pSymbols[nInstr];
}

void EInstrArray::SetCOInstruction(int nInstr, COInstruction* pInstr)
{
	if(!m_pSymbols)
	{
		int nInstrCount = GetInstrCount();
		m_pSymbols = new COInstruction*[nInstrCount];
		memset(m_pSymbols, '\0', nInstrCount * sizeof(COInstruction*));
	}
	GAssert(nInstr >= 0 && nInstr < m_nInstrCount, "out of range");
	m_pSymbols[nInstr] = pInstr;
}

void EInstrArray::AcquireSymbols(EInstrArray* pOther)
{

	// *** Debugging Code
/*
	int nLen;
	char* szDisassembly = Disassembler::DisassembleEInstrArrayToText(this, &nLen);
	FILE* pFile = fopen("full.txt", "w");
	fwrite(szDisassembly, nLen, 1, pFile);
	fclose(pFile);
	delete(szDisassembly);
	ShellExecute(NULL, NULL, "full.txt", NULL, NULL, SW_SHOW);

	szDisassembly = Disassembler::DisassembleEInstrArrayToText(pOther, &nLen);
	pFile = fopen("partial.txt", "w");
	fwrite(szDisassembly, nLen, 1, pFile);
	fclose(pFile);
	delete(szDisassembly);
	ShellExecute(NULL, NULL, "partial.txt", NULL, NULL, SW_SHOW);
*/

	int nThisPos = 0;
	int nOtherPos = 0;
	int nThisCount = GetInstrCount();
	int nOtherCount = pOther->GetInstrCount();
	delete(m_pSymbols);
	m_pSymbols = new COInstruction*[nThisCount];
	memset(m_pSymbols, '\0', nThisCount * sizeof(COInstruction*));
	while(nThisPos < nThisCount)
	{
		// Consume while instructions match
		while(true)
		{
			if(nThisPos >= nThisCount)
				break;
			if(nOtherPos >= nOtherCount)
				break;
			InstrBin* pThisInstr = GetInstr(nThisPos);
			InstrBin* pOtherInstr = pOther->GetInstr(nOtherPos);
			if(pThisInstr->cInstr != pOtherInstr->cInstr)
				break;
			SetCOInstruction(nThisPos, pOther->GetCOInstruction(nOtherPos));
			nThisPos++;
			nOtherPos++;
		}

		// Find next matching instructions
		int nDepth;
		int nThisInstrs[INSTRUCTION_COUNT];
		int nOtherInstrs[INSTRUCTION_COUNT];
		int n;
		for(n = 0; n < INSTRUCTION_COUNT; n++)
		{
			nThisInstrs[n] = -1;
			nOtherInstrs[n] = -1;
		}
		int nMatchThis = -1;
		int nMatchOther = -1;
		for(nDepth = 0; nThisPos + nDepth < nThisCount || nOtherPos + nDepth < nOtherCount; nDepth++)
		{
			// Remember the instructions we've seen so far on both sides
			unsigned char cThisInstr;
			unsigned char cOtherInstr;
			if(nThisPos + nDepth < nThisCount)
			{
				cThisInstr = GetInstr(nThisPos + nDepth)->cInstr;
				GAssert(cThisInstr < INSTRUCTION_COUNT, "bad instruction");
				if(nThisInstrs[cThisInstr] == -1)
					nThisInstrs[cThisInstr] = nThisPos + nDepth;
			}
			if(nOtherPos + nDepth < nOtherCount)
			{
				cOtherInstr = pOther->GetInstr(nOtherPos + nDepth)->cInstr;
				GAssert(cOtherInstr < INSTRUCTION_COUNT, "bad instruction");
				if(nOtherInstrs[cOtherInstr] == -1)
					nOtherInstrs[cOtherInstr] = nOtherPos + nDepth;
			}

			// Check for a match
			if(nThisPos + nDepth < nThisCount && nOtherInstrs[cThisInstr] != -1)
			{
				nMatchThis = nThisPos + nDepth;
				nMatchOther = nOtherInstrs[cThisInstr];
				break;
			}
			if(nOtherPos + nDepth < nOtherCount && nThisInstrs[cOtherInstr] != -1)
			{
				nMatchOther = nOtherPos + nDepth;
				nMatchThis = nThisInstrs[cOtherInstr];
				break;
			}
		}

		// Stretch unmatching areas to fit together
		nMatchThis = (nMatchThis == -1 ? nThisCount : nMatchThis);
		nMatchOther = (nMatchOther == -1 ? nOtherCount : nMatchOther);
		int nMismatchThisStart = nThisPos;
		int nMismatchOthertStart = nOtherPos;
		while(nThisPos < nMatchThis)
		{
			int nInterpolatePos = nMismatchOthertStart + (nThisPos - nMismatchThisStart) * (nMatchOther - nMismatchOthertStart) / (nMatchThis - nMismatchThisStart);
			if(nInterpolatePos >= nOtherCount)
				nInterpolatePos = nOtherCount - 1;
			COInstruction* pInstr = pOther->GetCOInstruction(nInterpolatePos);
			SetCOInstruction(nThisPos, pInstr);
			nThisPos++;
		}
		nOtherPos = nMatchOther;
	}
}

int EInstrArray::FindJumpTargetInstr(int nInstr, struct InstructionStruct* pInstr, int nJumpOffset)
{
	int nOffset = GetOffset(nInstr);
	nOffset += sizeof(unsigned char); // for the instruction
	nOffset += pInstr->nParamCount * sizeof(int); // parameters
	nOffset += nJumpOffset;
	int nNextInstr = FindInstrByOffset(nOffset);
	GAssert(nNextInstr >= 0, "Bad jump target");
	return nNextInstr;
}

void EInstrArray::GetNextInstructions(int nInstr, int* pnNext1, int* pnNext2)
{
	struct InstrBin* pInstrBin = GetInstr(nInstr);
	struct InstructionStruct* pInstr = pInstrBin->GetInstrStruct();
	if(pInstr->pMethod == Instr_Return)
	{
		*pnNext1 = -1;
		*pnNext2 = -1;
		return;
	}
	if(pInstr->pMethod == Instr_Jump)
	{
		GAssert(pInstr->nParamCount == 1, "unexpected number of params");
		*pnNext1 = FindJumpTargetInstr(nInstr, pInstr, pInstrBin->GetParam(0));
		*pnNext2 = -1;
		return;
	}

	*pnNext1 = nInstr + 1;
	*pnNext2 = -1;

	if(pInstr->szParamTypes[pInstr->nParamCount - 1] == 'j')
	{
		GAssert(pInstr->nParamCount == 2, "unexpected number of params");
		*pnNext2 = FindJumpTargetInstr(nInstr, pInstr, pInstrBin->GetParam(pInstr->nParamCount - 1));
	}
}

int* EInstrArray::GetStackInstructions()
{
	if(m_stackInstructions)
		return m_stackInstructions;
	int nCount = GetInstrCount();
	if(nCount < 1)
		return NULL;
	m_stackInstructions = new int[nCount];
	int n;
	for(n = 0; n < nCount; n++)
		m_stackInstructions[n] = -1;
	GPointerQueue todoQueue;
	todoQueue.Push((void*)-1);
	todoQueue.Push((void*)0);

	// Run tree and check stack at each point
	bool bOK = true;
	while(bOK && todoQueue.GetSize() > 0)
	{
		int nPrev = (int)todoQueue.Pop();
		int nCurr = (int)todoQueue.Pop();
		if(m_stackInstructions[nCurr] != -1) // If this node has been visited before
		{
			// Make sure they come from the same stack
			int nStack = DoesInstrPush(nPrev) ? nPrev : m_stackInstructions[nPrev];
			if(m_stackInstructions[nCurr] != nStack)
			{
				bOK = false;
				GAssert(false, "merging code paths have different local variable stacks.");
				break;
			}
		}
		else
		{
			// Determine the stack instruction
			int nStackBelow;
			if(IsEndScope(nCurr))
			{
				nStackBelow = nPrev;
				while(nStackBelow != -1 && !IsStartScope(nStackBelow))
					nStackBelow = m_stackInstructions[nStackBelow];
				if(nStackBelow == -1)
				{
					bOK = false;
					GAssert(false, "end scope with no start scope");
					break;
				}
				nStackBelow = m_stackInstructions[nStackBelow];
			}
			else
			{
				if(nPrev == -1 || DoesInstrPush(nPrev))
					nStackBelow = nPrev;
				else
					nStackBelow = m_stackInstructions[nPrev];
			}
			m_stackInstructions[nCurr] = nStackBelow;

			// Add the next instruction(s) to the queue
			int nNext1, nNext2;
			GetNextInstructions(nCurr, &nNext1, &nNext2);
			if(nNext1 >= 0)
			{
				todoQueue.Push((void*)nCurr);
				todoQueue.Push((void*)nNext1);
			}
			if(nNext2 >= 0)
			{
				todoQueue.Push((void*)nCurr);
				todoQueue.Push((void*)nNext2);
			}
		}
	}
	return m_stackInstructions;
}

int EInstrArray::GetStackInstr(int nInstr)
{
	GAssert(nInstr >= 0 && nInstr < GetInstrCount(), "out of range");
	int* pNextInstrs = GetStackInstructions();
	return pNextInstrs[nInstr];
}

int EInstrArray::CountStackDepth(int nInstr)
{
	int n = 0;
	int nPos = GetStackInstr(nInstr);
	while(nPos >= 0)
	{
		n++;
		nPos = GetStackInstr(nPos);
	}
	return n;
}

void EInstrArray::GetDependentMethods(GHashTable* pMethodTable, GHashTable* pTypeTable, Library* pLibrary, COProject* pProject)
{
	int nCount = GetInstrCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		InstrBin* pInstr = GetInstr(n);
		InstructionStruct* pInstrStruct = pInstr->GetInstrStruct();
		int i;
		for(i = 0; pInstrStruct->szParamTypes[i] != '\0'; i++)
		{
			if(pInstrStruct->szParamTypes[i] == 'm')
			{
				int nMethodID = pInstr->GetParam(i);
				EMethod* pMethod = pLibrary->GetEMethod(nMethodID);
				pMethod->GetDependentMethods(pMethodTable, pTypeTable, pLibrary, pProject); // put the Method ID in the method table
			}
			else if(pInstrStruct->szParamTypes[i] == 't')
			{
				int nTypeID = pInstr->GetParam(i);
				EType* pEType = pLibrary->GetEType(nTypeID);
				COType* pCOType = pEType->GetCOType(pProject);
				void* pTmp;
				if(!pTypeTable->Get(pCOType, &pTmp))
					pTypeTable->Add(pCOType, NULL);	// put the COType in the type table
			}
		}
	}
}




// ----------------------------------------------------------------------------------


// internal class used by EInstrArrayBuilder
class FixUpSpot : public GBucket
{
public:
	int m_nPos;
	int m_nVal;

public:
	FixUpSpot(int nPos, int nVal) : GBucket() { m_nPos = nPos; m_nVal = nVal; }
	virtual ~FixUpSpot() {}

	virtual int Compare(GBucket* pBucket)
	{
		int nPos = ((FixUpSpot*)pBucket)->m_nPos;
		if(m_nPos < nPos)
			return -1;
		if(m_nPos > nPos)
			return 1;
		return 0;
	}
};

// ----------------------------------------------------------------------------------


EInstrArrayBuilder::EInstrArrayBuilder()
{
#ifdef _DEBUG
	m_nExpectedParams = 0;
#endif // _DEBUG
	m_pQueue = new GQueue();
	m_pFixUpSpots = new GLList();
	m_pSymbols = new GPointerQueue();
	m_nInstrCount = 0;
}

EInstrArrayBuilder::~EInstrArrayBuilder()
{
	delete(m_pSymbols);
	delete(m_pFixUpSpots);
	delete(m_pQueue);
}

void EInstrArrayBuilder::AddInstr(GVMInstrPointer pMeth, COInstruction* pInstruction)
{
//GAssert(m_nInstrCount != 1, "Uncomment this to debug when the nth instruction is added");
	char cInstr = GetFuncEnum(pMeth);
#ifdef _DEBUG
	GAssert(m_nExpectedParams == 0, "Expected more params");
	m_nExpectedParams = GetInstructionStruct(cInstr)->nParamCount;
#endif // _DEBUG
	m_pQueue->Push(cInstr);
	m_pSymbols->Push(pInstruction);
	m_nInstrCount++;
}

void EInstrArrayBuilder::AddParam(int nParam)
{
#ifdef _DEBUG
	GAssert(m_nExpectedParams > 0, "No more params expected");
	m_nExpectedParams--;
#endif // _DEBUG
#ifdef DARWIN
	m_pQueue->Push(ReverseEndian(nParam));
#else // DARWIN
	m_pQueue->Push(nParam);
#endif // !DARWIN
}

bool EInstrArrayBuilder::ReadInstr(char* pInstr)
{
	return m_pQueue->Pop(pInstr);
}

bool EInstrArrayBuilder::ReadParam(int* pParam)
{
#ifdef DARWIN
	return ReverseEndian(m_pQueue->Pop(pParam));
#else // DARWIN
	return m_pQueue->Pop(pParam);
#endif // !DARWIN
}

EInstrArray* EInstrArrayBuilder::MakeEInstrArray()
{
	int nPos = 0;
	m_pFixUpSpots->Sort();
	Holder<FixUpSpot*> hSpot((FixUpSpot*)m_pFixUpSpots->Unlink(NULL));
	int nSize = GetSize();
	Holder<unsigned char*> hData(new unsigned char[nSize]);
	char cOverflow[3];
	int nOverflowPointer = 0;
	char c;
	int n;
	while(GetSize() > 0 || nOverflowPointer > 0)
	{
		if(nOverflowPointer > 0)
		{
			nOverflowPointer--;
			c = cOverflow[nOverflowPointer];
		}
		else
		{
			if(hSpot.Get() && nPos >= hSpot.Get()->m_nPos)
			{
				GAssert(nPos == hSpot.Get()->m_nPos, "FixUp spots weren't properly sorted!");
				int nValue = hSpot.Get()->m_nVal;
				ReadParam(&n);
				GAssert(n == 0, "Expected zero place holder");
#ifdef DARWIN
				c = ((char*)&nValue)[3];
				cOverflow[0] = ((char*)&nValue)[0];
				cOverflow[1] = ((char*)&nValue)[1];
				cOverflow[2] = ((char*)&nValue)[2];
#else // DARWIN
				c = ((char*)&nValue)[0];
				cOverflow[0] = ((char*)&nValue)[3];
				cOverflow[1] = ((char*)&nValue)[2];
				cOverflow[2] = ((char*)&nValue)[1];
#endif // !DARWIN
				nOverflowPointer = 3;
				hSpot.Set((FixUpSpot*)m_pFixUpSpots->Unlink(NULL));
			}
			else
			{
				if(!ReadInstr(&c))
					return NULL;
			}
		}
		hData.Get()[nPos] = c;
		nPos++;
	}
	if(hSpot.Get())
	{
		GAssert(false, "Failed to find at least one FixUp spot!--todo: set an error");
	}
	EInstrArray* pMB = new EInstrArray(hData.Drop(), nSize);
	n = 0;
	while(m_pSymbols->GetSize() > 0)
	{
		COInstruction* pInstr = (COInstruction*)m_pSymbols->Pop();
		GAssert(!pInstr || pInstr->GetInstructionType() == COInstruction::IT_CALL || pInstr->GetInstructionType() == COInstruction::IT_BLOCK,
			"If this throws or does something funky, it probably means some temp instruction was used as a symbol.  You can't do that.  To debug it, look at the value of 'n', restart, and break on the nth call to AddInstr");
		pMB->SetCOInstruction(n, pInstr);
		n++;
	}
	m_pFixUpSpots->Clear();
	return pMB;
}


void EInstrArrayBuilder::Flush()
{
	m_pQueue->Flush();
	m_pFixUpSpots->Clear();
}


void EInstrArrayBuilder::AddFixUpSpot(int nPos, int nVal)
{
	m_pFixUpSpots->Link(new FixUpSpot(nPos, nVal));
}


int EInstrArrayBuilder::GetSize()
{
	return m_pQueue->GetSize();
}

