/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "Instruction.h"

class COInstrArray;

// A Block is a section of code with a comment that says what it does
class COBlock : public COInstruction
{
protected:
	char* m_szComment;
	COInstrArray* m_pInstrArray;

public:
	COBlock(int nLine, int nCol, int nWid, COInstrArray* pParent, const char* szComment);
	virtual ~COBlock();

	virtual COInstrArray* GetChildInstructions() { return m_pInstrArray; }
	virtual InstructionType GetInstructionType() { return IT_BLOCK; }
	void SetComment(const char* szComment);
	const char* GetComment() { return m_szComment; }
	static COBlock* FromXML(GXMLTag* pTag, COInstrArray* pParent, COProject* pCOProject, bool bPartial);
	virtual GXMLTag* SaveToXML(COInstrArray* pParent);
	void SaveToClassicSyntax(GQueue* pQ, int nTabs);
	static void FromClassicSyntax(ClassicSyntax* pParser, GXMLTag* pParentTag);
	virtual COVariable* FindVariable(const char* pName, int nLength);
	void LoadChildInstructions(GXMLTag* pTag, COProject* pCOProject, bool bPartial, int* pnInstructionIndex);
	virtual bool Compile(GCompiler* pCompiler, COMethod* pMethod, COInstruction* pSymbolInstr);
	virtual COInstruction* FindInstruction(int nIndex);
};

#endif // __BLOCK_H__
