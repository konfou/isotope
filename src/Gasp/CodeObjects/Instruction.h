/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COINSTRUCTION_H__
#define __COINSTRUCTION_H__

#include "CodeObject.h"
#include "../Engine/ClassicSyntax.h"
#include "../../GClasses/GMacros.h"
#include "Scope.h"

class COVariable;
class COCall;
class COMethod;
class COProject;
class GQueue;
class ClassicSyntax;
class COInstrArray;
class COScope;
class GCompilerBase;

class COInstruction : public COScope
{
protected:
	int m_nIndex;

public:
	enum InstructionType
	{
		IT_CALL,
		IT_BLOCK,
	};

public:
	COInstruction(int nLine, int nCol, int nWid, COInstrArray* pParent);
	virtual ~COInstruction();

	virtual InstructionType GetInstructionType() = 0;

	// Methods that deal with child instructions
	virtual COInstrArray* GetChildInstructions() { return NULL; }

	virtual COInstruction* FindInstruction(int nIndex) { return NULL; }

	// Methods for loading and saving
	static COInstruction* FromXML(GXMLTag* pTag, COInstrArray* pParent, COProject* pCOProject, bool bPartial, int* pnInstructionIndex);
	virtual GXMLTag* SaveToXML(COInstrArray* pParent) = 0;
	void SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay = false);
	static void FromClassicSyntax(ClassicSyntax* pParser, GXMLTag* pParentTag, ClassicSyntax::InstrType eType);

	// Misc
	int GetIndex() { return m_nIndex; }

	virtual COVariable* FindVariable(const char* pName, int nLength) = 0;
	virtual ScopeType GetScopeType() { return ST_INSTRUCTION; }
	virtual bool Compile(GCompiler* pCompiler, COMethod* pMethod, COInstruction* pSymbolInstr) = 0;
	void GetStepOverInstructions(GPointerQueue* pInstrQueue);
};

#endif // __COINSTRUCTION_H__
