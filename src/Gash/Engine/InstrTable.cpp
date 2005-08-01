/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "InstrTable.h"
#include "InstrSet.h"

// Param Types  (Capital letter = may be modified by the instruction)
// -----------
// a = actual memory address
// f = member index
// I = dest integer
// i = source integer
// j = jump offset (has to be the last param due to logic in Disassembler & EInstrArray)
// m = method id
// M = method decl index
// O = dest object
// o = source object
// p = parameter count
// t = type id
// v = integer value
// V = virtual table index

static struct InstructionStruct TheMightyInstructionTable[INSTRUCTION_COUNT] = 
{
//	Display Name			VM Method						Compiler Instr				ParamCount		Params
//	------------			---------						--------------				----------		------
	{"{",					Instr_StartScope,				Instr_NotImplYet,			0,				""		},
	{"}",					Instr_EndScope,					Instr_NotImplYet,			0,				""		},
	{"Add",					Instr_AddInteger,				Instr_NotImplYet,			2,				"Ii"	},
	{"And",					Instr_AndInteger,				Instr_NotImplYet,			2,				"Ii"	},
	{"BranchIfFalse",		Instr_BranchIfFalse,			Instr_NotImplYet,			2,				"ij"	},
	{"BreakPoint",			Instr_BreakPoint,				Instr_NotImplYet,			0,				""		},
	{"Call",				Instr_Call,						Instr_NotImplYet,			1,				"m"		},
	{"CallLinked",			Instr_CallLinked,				Instr_NotImplYet,			1,				"a"		},
	{"Cast",				Instr_Cast,						Instr_NotImplYet,			2,				"to"	},
	{"CopyInt",				Instr_CopyInt,					Instr_NotImplYet,			2,				"Ii"	},
	{"CopyVar",				Instr_CopyVar,					Instr_NotImplYet,			2,				"Oo"	},
	{"Decrement",			Instr_DecrementInteger,			Instr_NotImplYet,			1,				"I"		},
	{"Divide",				Instr_DivideInteger,			Instr_NotImplYet,			2,				"Ii"	},
	{"GetMember",			Instr_GetMember,				Instr_NotImplYet,			3,				"Oof"	},
	{"ICall",               Instr_ICall,					Instr_NotImplYet,			3,				"otM"	},
	{"Increment",			Instr_IncrementInteger,			Instr_NotImplYet,			1,				"I"		},
	{"InvertInt",			Instr_InvertInteger,			Instr_NotImplYet,			1,				"I"		},
	{"IsVarEqual",			Instr_IsVarEqual,				Instr_NotImplYet,			3,				"Ioo"	},
	{"IsVarNotEqual",		Instr_IsVarNotEqual,			Instr_NotImplYet,			3,				"Ioo"	},
	{"IsIntEqual",			Instr_IsIntEqual,				Instr_NotImplYet,			3,				"Iii"	},
	{"IsIntNotEqual",		Instr_IsIntNotEqual,			Instr_NotImplYet,			3,				"Iii"	},
	{"IsIntLessThan",		Instr_IsIntLessThan,			Instr_NotImplYet,			3,				"Iii"	},
	{"IsIntGreaterThan",	Instr_IsIntGreaterThan,			Instr_NotImplYet,			3,				"Iii"	},
	{"Jump",				Instr_Jump,						Instr_NotImplYet,			1,				"j"		},
	{"Max",					Instr_MaxInteger,				Instr_NotImplYet,			2,				"Ii"	},
	{"MCall",				Instr_MCall,					Instr_NotImplYet,			3,				"otM"	},
	{"MCallLinked",			Instr_MCallLinked,				Instr_NotImplYet,			3,				"oap"	},
	{"Min",					Instr_MinInteger,				Instr_NotImplYet,			2,				"Ii"	},
	{"Modulus",				Instr_ModulusInteger,			Instr_NotImplYet,			2,				"Ii"	},
	{"Multiply",			Instr_MultiplyInteger,			Instr_NotImplYet,			2,				"Ii"	},
	{"NewIntObj",			Instr_NewInteger,				Instr_NotImplYet,			1,				"O"		},
	{"NewObject",			Instr_NewObject,				Instr_NotImplYet,			2,				"Ot"	},
	{"NewVariable",			Instr_NewVariable,				Instr_NotImplYet,			0,				""		},
	{"Or",					Instr_OrInteger,				Instr_NotImplYet,			2,				"Ii"	},
	{"PushParam",			Instr_PushParameter,			Instr_NotImplYet,			1,				"o"		},
	{"Return",				Instr_Return,					Instr_NotImplYet,			0,				""		},
	{"SetMember",			Instr_SetMember,				Instr_NotImplYet,			3,				"Ofo"	},
	{"SetIntWithConst",		Instr_SetIntWithConst,			Instr_NotImplYet,			2,				"Iv"	},
	{"Subtract",			Instr_SubtractInteger,			Instr_NotImplYet,			2,				"Ii"	},
	{"Throw",				Instr_Throw,					Instr_NotImplYet,			1,				"o"		},
	{"Try",					Instr_TryCall,					Instr_NotImplYet,			2,				"Om"	},
	{"TryLinked",			Instr_TryCallLinked,			Instr_NotImplYet,			2,				"Oa"	},
	{"VCall",				Instr_VCall,					Instr_NotImplYet,			2,				"oV"	},
	{"Xor",					Instr_XorInteger,				Instr_NotImplYet,			2,				"Ii"	},
	{"SetFloatWithConst",	Instr_SetFloatWithConst,		Instr_NotImplYet,			3,				"Ovv"	},
	{"ShiftLeft",			Instr_ShiftLeft,				Instr_NotImplYet,			2,				"Ii"	},
	{"ShiftRight",			Instr_ShiftRight,				Instr_NotImplYet,			2,				"Ii"	},
};

struct InstructionStruct* GetInstructionStruct(unsigned char instruction)
{
	GAssert(instruction < INSTRUCTION_COUNT, "Out of range");
	return &TheMightyInstructionTable[instruction];
}

// todo: this function makes the compiler slow.  Major compile-time perf bonus to make it faster.
char GetFuncEnum(GVMInstrPointer pMeth)
{
	unsigned char n;
	for(n = 0; n < INSTRUCTION_COUNT; n++)
	{
		if(pMeth == TheMightyInstructionTable[n].pMethod)
			return (char)n;
	}
	GAssert(false, "Error, that method is not in the table");
	return 0;
}
