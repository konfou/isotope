/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __INSTRSET_H__
#define __INSTRSET_H__

class GVM;

void Instr_NotImplYet();

void Instr_AddInteger(GVM* pEngine);
void Instr_AndInteger(GVM* pEngine);
void Instr_BranchIfFalse(GVM* pEngine);
void Instr_BreakPoint(GVM* pEngine);
void Instr_Call(GVM* pEngine);
void Instr_CallLinked(GVM* pEngine);
void Instr_CopyInt(GVM* pEngine);
void Instr_CopyVar(GVM* pEngine);
void Instr_DecrementInteger(GVM* pEngine);
void Instr_DivideInteger(GVM* pEngine);
void Instr_EndScope(GVM* pEngine);
void Instr_GetConstString(GVM* pEngine);
void Instr_GetMember(GVM* pEngine);
void Instr_ICall(GVM* pEngine);
void Instr_IncrementInteger(GVM* pEngine);
void Instr_InvertInteger(GVM* pEngine);
void Instr_IsVarEqual(GVM* pEngine);
void Instr_IsVarNotEqual(GVM* pEngine);
void Instr_IsIntEqual(GVM* pEngine);
void Instr_IsIntNotEqual(GVM* pEngine);
void Instr_IsIntLessThan(GVM* pEngine);
void Instr_IsIntGreaterThan(GVM* pEngine);
void Instr_Jump(GVM* pEngine);
void Instr_MaxInteger(GVM* pEngine);
void Instr_MCall(GVM* pEngine);
void Instr_MCallLinked(GVM* pEngine);
void Instr_Cast(GVM* pEngine);
void Instr_MinInteger(GVM* pEngine);
void Instr_ModulusInteger(GVM* pEngine);
void Instr_MultiplyInteger(GVM* pEngine);
void Instr_NewInteger(GVM* pEngine);
void Instr_NewObject(GVM* pEngine);
void Instr_NewVariable(GVM* pEngine);
void Instr_OrInteger(GVM* pEngine);
void Instr_PushParameter(GVM* pEngine);
void Instr_Return(GVM* pEngine);
void Instr_SetFloatWithConst(GVM* pEngine);
void Instr_SetIntWithConst(GVM* pEngine);
void Instr_SetMember(GVM* pEngine);
void Instr_ShiftLeft(GVM* pEngine);
void Instr_ShiftRight(GVM* pEngine);
void Instr_StartScope(GVM* pEngine);
void Instr_SubtractInteger(GVM* pEngine);
void Instr_Throw(GVM* pEngine);
void Instr_TryCall(GVM* pEngine);
void Instr_TryCallLinked(GVM* pEngine);
void Instr_VCall(GVM* pEngine);
void Instr_XorInteger(GVM* pEngine);


#endif // __INSTRSET_H__

