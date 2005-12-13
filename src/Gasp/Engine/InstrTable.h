/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __INSTRTABLE_H__
#define __INSTRTABLE_H__

#include "../Include/GaspEngine.h"

struct InstructionStruct
{
	const char* szName;
	GVMInstrPointer pMethod;
	CompilerInstr pInstr;
	int nParamCount;
	const char* szParamTypes;
};

#define INSTRUCTION_COUNT 50

inline int GetInstrCount() { return INSTRUCTION_COUNT; }
struct InstructionStruct* GetInstructionStruct(unsigned char instruction);
char GetFuncEnum(GVMInstrPointer pMeth);

#endif // __INSTRTABLE_H__

