/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __EInstrArray_H__
#define __EInstrArray_H__

#include "InstrTable.h"

class GXMLTag;
class COInstruction;
class GCompiler;
class GQueue;
class GPointerQueue;
class GLList;
class Disassembly;
struct InstructionStruct;

// todo: get rid of the #pragma
#pragma pack(1)

struct InstrBin // todo: rename to EInstruction
{
	unsigned char cInstr;
protected:
	unsigned int pParams[3];

public:
	InstructionStruct* GetInstrStruct();
	unsigned int GetParam(int n);
	void SetParam(int nIndex, unsigned int nValue);
};

#pragma pack()



class EInstrArray // todo: rename to EInstrArray
{
protected:
	bool m_bNeedToDeleteData;
	unsigned char* m_pData;
	int m_nSize;
	int* m_instrIndexes;
	int m_nInstrCount;
	COInstruction** m_pSymbols;
	int* m_stackInstructions;
	int m_nDisassemblyStartLine;

public:
	// Construct a EInstrArray from the XML method tag in an xlib file
	EInstrArray(GXMLTag* pMethod);

	// Construct a EInstrArray from a compiled method in memory
	// Note: this takes ownership of pData
	EInstrArray(unsigned char* pData, int nSize);

	// Destructor
	virtual ~EInstrArray();

	// Returns the number of instructions
	int GetInstrCount();

	// returns a pointer to the instruction
	struct InstrBin* GetInstr(int nInstr);

	// Returns the offset of the specified instruction
	int GetOffset(int nInstr);

	// Saves the bits to XML
	void SetBinTags(GXMLTag* pMethodTag);

	// Returns the total size of the method
	int GetSize() { return m_nSize; }

	// returns a pointer to the method data.
	unsigned char* GetData();

	// This returns the zero-based instruction number of the
	// instruction with the specified offset, or -1 if there is no
	// instruction with that exact offset.
	int FindInstrByOffset(int nOffset);

	// Returns true if this EInstrArray has any symbolic info.
	bool HaveSymbolicInfo() { return m_pSymbols ? true : false; }

	// If there is symbolic info, returns the COInstruction that
	// corresponds to instruction nInstr.  If there's not, returns NULL
	COInstruction* GetCOInstruction(int nInstr);

	// Set symbolic info for instruction nInstr
	void SetCOInstruction(int nInstr, COInstruction* pInstr);

	// This copies the symbols from pTarget to this.  It uses a
	// "diff"-like algorithm and stretches unmatching areas to fit.
	void AcquireSymbols(EInstrArray* pPartialBits);

	// returns true if instruction nInstr pushes a variable on the stack
	bool DoesInstrPush(int nInstr);

	// returns true if instruction nInstr is an start/end scope
	bool IsEndScope(int nInstr);
	bool IsStartScope(int nInstr);

	// This returns the instructions that can follow nInstr.  For
	// non-branching instructions, pnNext1 will hold the next instruction
	// and pnNext2 will hold -1.  For branching instructions, both will
	// be valid.  For "return", both values will be -1.
	void GetNextInstructions(int nInstr, int* pnNext1, int* pnNext2);

	// This returns the next instruction on the stack that pushed a variable
	int GetStackInstr(int nInstr);

	// Returns the number of variables on the stack at nInstr
	int CountStackDepth(int nInstr);

	// Get/Set the line number where this method starts in the disassembly
	int GetDisassemblyStartLine() { return m_nDisassemblyStartLine; }
	void SetDisassemblyStartLine(int nLine) { m_nDisassemblyStartLine = nLine; }

	// Adds all method and type dependencies (deep) to the hash tables
	void GetDependentMethods(GHashTable* pMethodTable, GHashTable* pTypeTable, Library* pLibrary, COProject* pProject);

protected:
	void IndexInstructions();
	int FindJumpTargetInstr(int nInstr, struct InstructionStruct* pInstr, int nJumpOffset);
	int* GetStackInstructions();
};



class EInstrArrayBuilder
{
protected:
	GQueue* m_pQueue;
	GLList* m_pFixUpSpots;
	GPointerQueue* m_pSymbols;
	int m_nInstrCount;

#ifdef _DEBUG
	int m_nExpectedParams;
#endif // _DEBUG

public:
	EInstrArrayBuilder();
	virtual ~EInstrArrayBuilder();

	void AddInstr(GVMInstrPointer pMeth, COInstruction* pInstruction);
	void AddParam(int nParam);
	EInstrArray* MakeEInstrArray();
	void Flush();
	void AddFixUpSpot(int nPos, int nVal);
	int GetSize();

protected:
	bool ReadInstr(char* pInstr);
	bool ReadParam(int* pParam);

};


#endif // __EInstrArray_H__
