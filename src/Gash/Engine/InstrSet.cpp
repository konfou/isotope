/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "InstrSet.h"
#include "../Include/GashEngine.h"
#include "../../GClasses/GMacros.h"
#include "EInterface.h"
#include "EClass.h"
#include "../BuiltIns/GashFloat.h"


void Instr_NotImplYet() {} // todo: remove this function

void NoOpCallBack(CallBackData* pData)
{
}









//------------------------------------------------------------------
// AddInteger
//		Param1: dest variable
//		Param2: source variable
//------------------------------------------------------------------
void Instr_AddInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pDest->pIntObject->m_value += pSource->pIntObject->m_value;
}

//------------------------------------------------------------------
// AndInteger
//		Param1: dest variable
//		Param2: source variable
//------------------------------------------------------------------
void Instr_AndInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pDest->pIntObject->m_value &= pSource->pIntObject->m_value;
}

void Instr_BranchIfFalse(GVM* pEngine)
{
	EVar* pBool = pEngine->GetVarParam();
	int nOffset = pEngine->GetIntParam();
	if(!pBool->pIntObject->m_value)
		pEngine->m_pInstructionPointer += nOffset;
}

void Instr_BreakPoint(GVM* pEngine)
{
	pEngine->m_pInstructionPointer--;
	if(!pEngine->m_pBreakPointHandler)
		pEngine->ThrowEngineError(L"Hit a breakpoint but there is no handler");
	pEngine->m_pBreakPointHandler(pEngine->m_pBreakPointThis);
}






void CallMachine0(WrapperObject* pThis, GVM* pVM, EMethodPointerHolder* pMeth)
{
	(pThis->*pMeth->m_m0)(pVM);
}

void CallMachine1(WrapperObject* pThis, GVM* pVM, EMethodPointerHolder* pMeth)
{
	GVMStack* pStack = pVM->m_pStack;
	(pThis->*pMeth->m_m1)(pVM, pStack->GetVariableFromTop(-1));
}

void CallMachine2(WrapperObject* pThis, GVM* pVM, EMethodPointerHolder* pMeth)
{
	GVMStack* pStack = pVM->m_pStack;
	(pThis->*pMeth->m_m2)(pVM, pStack->GetVariableFromTop(-2),
						pStack->GetVariableFromTop(-1));
}

void CallMachine3(WrapperObject* pThis, GVM* pVM, EMethodPointerHolder* pMeth)
{
	GVMStack* pStack = pVM->m_pStack;
	(pThis->*pMeth->m_m3)(pVM, pStack->GetVariableFromTop(-3),
						pStack->GetVariableFromTop(-2),
						pStack->GetVariableFromTop(-1));
}

void CallMachine4(WrapperObject* pThis, GVM* pVM, EMethodPointerHolder* pMeth)
{
	GVMStack* pStack = pVM->m_pStack;
	(pThis->*pMeth->m_m4)(pVM, pStack->GetVariableFromTop(-4),
						pStack->GetVariableFromTop(-3),
						pStack->GetVariableFromTop(-2),
						pStack->GetVariableFromTop(-1));
}

void CallMachine5(WrapperObject* pThis, GVM* pVM, EMethodPointerHolder* pMeth)
{
	GVMStack* pStack = pVM->m_pStack;
	(pThis->*pMeth->m_m5)(pVM, pStack->GetVariableFromTop(-5),
						pStack->GetVariableFromTop(-4),
						pStack->GetVariableFromTop(-3),
						pStack->GetVariableFromTop(-2),
						pStack->GetVariableFromTop(-1));
}

void CallMachine6(WrapperObject* pThis, GVM* pVM, EMethodPointerHolder* pMeth)
{
	GVMStack* pStack = pVM->m_pStack;
	(pThis->*pMeth->m_m6)(pVM, pStack->GetVariableFromTop(-6),
						pStack->GetVariableFromTop(-5),
						pStack->GetVariableFromTop(-4),
						pStack->GetVariableFromTop(-3),
						pStack->GetVariableFromTop(-2),
						pStack->GetVariableFromTop(-1));
}

void CallMachine7(WrapperObject* pThis, GVM* pVM, EMethodPointerHolder* pMeth)
{
	GVMStack* pStack = pVM->m_pStack;
	(pThis->*pMeth->m_m7)(pVM, pStack->GetVariableFromTop(-7),
						pStack->GetVariableFromTop(-6),
						pStack->GetVariableFromTop(-5),
						pStack->GetVariableFromTop(-4),
						pStack->GetVariableFromTop(-3),
						pStack->GetVariableFromTop(-2),
						pStack->GetVariableFromTop(-1));
}

void CallMachine8(WrapperObject* pThis, GVM* pVM, EMethodPointerHolder* pMeth)
{
	GVMStack* pStack = pVM->m_pStack;
	(pThis->*pMeth->m_m8)(pVM, pStack->GetVariableFromTop(-8),
						pStack->GetVariableFromTop(-7),
						pStack->GetVariableFromTop(-6),
						pStack->GetVariableFromTop(-5),
						pStack->GetVariableFromTop(-4),
						pStack->GetVariableFromTop(-3),
						pStack->GetVariableFromTop(-2),
						pStack->GetVariableFromTop(-1));
}

typedef void (*CallMachineWrapper)(WrapperObject*, GVM*, EMethodPointerHolder*);
#define MAX_MACHINE_PARAMS 8
const CallMachineWrapper MachineWrapperTable[MAX_MACHINE_PARAMS + 1] =
{
	CallMachine0,
	CallMachine1,
	CallMachine2,
	CallMachine3,
	CallMachine4,
	CallMachine5,
	CallMachine6,
	CallMachine7,
	CallMachine8,
};

//------------------------------------------------------------------
// MCallLinked
//		Param1: 'this' WrapperObject
//		Param2: pointer to the callback function (casted to a uint)
//		Param3: number of parameters
//------------------------------------------------------------------
void Instr_MCallLinked(GVM* pEngine)
{
	// Get Params
	EVar* pVarThis = pEngine->GetVarParam();
	unsigned int nParam2 = pEngine->GetUIntParam();
	unsigned int nParamCount = pEngine->GetUIntParam();
	EMethodPointerHolder* pMachineMethod = (EMethodPointerHolder*)nParam2;

	// Check params
	GAssert(pMachineMethod, "Machine method is NULL");
	GAssert(nParamCount <= MAX_MACHINE_PARAMS, "Machine calls with that many parameters is not supported");

	// Make the call
	CallMachineWrapper pMachineWrapper = MachineWrapperTable[nParamCount];
	pEngine->m_pMachineThis = pVarThis;
	pMachineWrapper(pVarThis->pWrapperObject, pEngine, pMachineMethod);
}

void Instr_Cast(GVM* pEngine)
{
	int nDestTypeID = pEngine->GetUIntParam();
	EVar* pSource = pEngine->GetVarParam();
	if(!pSource->pOb)
		return;
	EType* pDestType = pEngine->m_pLibrary->GetEType(nDestTypeID);
	EType* pSourceType = pSource->pOb->GetType();
	if(!pSourceType->CanCastTo(pDestType, pEngine->m_pLibrary))
		pEngine->ThrowCastError(pSourceType, pDestType);
}

//------------------------------------------------------------------
// MCall
//		Param1: 'this' WrapperObject
//		Param2: type id
//		Param3: MethodDecl index
//------------------------------------------------------------------
void Instr_MCall(GVM* pEngine)
{
	// Get the parameters
	unsigned int nOb = pEngine->GetUIntParam();
	int nTypeID = pEngine->GetIntParam();
	int nMethodDeclIndex = pEngine->GetIntParam();

	// Find the machine procedure to call
	EMachineClass* pMachineClass = (EMachineClass*)pEngine->m_pLibrary->GetEType(nTypeID);
	if(!pMachineClass)
		pEngine->ThrowEngineError(L"Invalid Type ID");
	if(((EType*)pMachineClass)->GetTypeType() != EType::TT_MACHINE)
		pEngine->ThrowEngineError(L"Expected a machine type");
	EMethodPointerHolder* pMachineMethod = pMachineClass->GetMachineProc(nMethodDeclIndex, pEngine);
	if(!pMachineMethod)
		pEngine->ThrowEngineError(L"Machine method not found");
	int nParamCount = pMachineClass->GetParamCount(nMethodDeclIndex);

	// Back up the instruction pointer
	pEngine->m_pInstructionPointer -= (3 * sizeof(unsigned int));
	GAssert(pEngine->m_pInstructionTable[*(pEngine->m_pInstructionPointer - 1)] == Instr_MCall, "Failed to find this instruction");

	// Backpatch the new values
	*(pEngine->m_pInstructionPointer - 1) = pEngine->m_cMCallLinkedInstr;
	unsigned int* pParams = (unsigned int*)pEngine->m_pInstructionPointer;
#ifdef DARWIN
	pParams[1] = ReverseEndian((unsigned int)pMachineMethod);
	pParams[2] = ReverseEndian(nParamCount);
#else // DARWIN
	pParams[1] = (unsigned int)pMachineMethod;
	pParams[2] = nParamCount;
#endif // !DARWIN

	// Now repeat this instruction as a linked machine call
	Instr_MCallLinked(pEngine);
}

//------------------------------------------------------------------
// CallLinked
//		Param1: method pointer
//------------------------------------------------------------------
void Instr_CallLinked(GVM* pEngine)
{
	unsigned int nTarget = pEngine->GetUIntParam();
	pEngine->BeginNewStackLayer();
	pEngine->m_pInstructionPointer = (unsigned char*)nTarget;
#ifdef _DEBUG
	pEngine->m_pCurrentMethod = pEngine->m_pInstructionPointer;
#endif // _DEBUG
}

//------------------------------------------------------------------
// Call
//		Param1: method id
//------------------------------------------------------------------
void Instr_Call(GVM* pEngine)
{
	unsigned int nParam1 = pEngine->GetUIntParam();
	int nSize;
	unsigned char* pMethod = pEngine->GetCompiledMethod(nParam1, &nSize);
	if(nSize <= 0)
		pEngine->ThrowEngineError(L"Attempted to call an abstract method");
	pEngine->m_pInstructionPointer -= sizeof(unsigned int);
	GAssert(pEngine->m_pInstructionTable[*(pEngine->m_pInstructionPointer - 1)] == Instr_Call, "Failed to find this instruction");
	*(pEngine->m_pInstructionPointer - 1) = pEngine->m_cCallLinkedInstr;
#ifdef DARWIN
	*(unsigned int*)pEngine->m_pInstructionPointer = ReverseEndian((unsigned int)pMethod);
#else // DARWIN
	*(unsigned int*)pEngine->m_pInstructionPointer = (unsigned int)pMethod;
#endif // !DARWIN
	Instr_CallLinked(pEngine);
}

//------------------------------------------------------------------
// VCall
//		Param1: virtual table index
//------------------------------------------------------------------
void Instr_VCall(GVM* pEngine)
{
	EVar* pObj = pEngine->GetVarParam();
	unsigned int nIndex = pEngine->GetUIntParam();
	EClass* pEClass = (EClass*)pObj->pOb->GetType(); // todo: check if it's really a class first
	int* pVirtualTable = pEClass->GetVirtualTable();
	int nSize;
	unsigned char* pMethod = pEngine->GetCompiledMethod(pVirtualTable[nIndex], &nSize);
	if(nSize <= 0)
		pEngine->ThrowEngineError(L"Attempted to call an abstract method");
	pEngine->BeginNewStackLayer();
	pEngine->m_pInstructionPointer = pMethod;
}

//------------------------------------------------------------------
// CopyInt
//		Param1: dest variable
//		Param2: source variable
//------------------------------------------------------------------
void Instr_CopyInt(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pDest->pIntObject->m_value = pSource->pIntObject->m_value;
}

//------------------------------------------------------------------
// CopyVar
//		Param1: source variable
//		Param2: dest variable
//------------------------------------------------------------------
void Instr_CopyVar(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pEngine->SetVar(pDest, pSource->pOb);
}

//------------------------------------------------------------------
// DecrementInteger
//		Param1: dest variable
//------------------------------------------------------------------
void Instr_DecrementInteger(GVM* pEngine)
{
	EVar* pVar = pEngine->GetVarParam();
	pVar->pIntObject->m_value--;
}

void Instr_DivideInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pDest->pIntObject->m_value /= pSource->pIntObject->m_value;
}

//------------------------------------------------------------------
// EndScope
//------------------------------------------------------------------
void Instr_EndScope(GVM* pEngine)
{
	EVar* pVar;
	while(true)
	{
		pVar = pEngine->m_pStack->Pop();
		if(pVar->eObType == VT_OB_REF)
			pEngine->Release(pVar->pOb);
		else
		{
			GAssert(pVar->eObType == VT_LOCAL_POINTER, "Expected a OT_LOCAL_POINTER");
			pEngine->m_pStack->SetLocalPointer((int)pVar->pOb);
			return;
		}
	}
}

void Instr_GetMember(GVM* pEngine)
{
	// todo: check the range and make sure it's really a class object
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	unsigned int nMember = pEngine->GetUIntParam();
	if(!pSource->pOb)
		pEngine->ThrowNullReferenceGetError();
	GObject* pTmp = pDest->pOb;
	pEngine->SetVar(pDest, pSource->pObjectObject->arrFields[nMember]);
}

void Instr_ICall(GVM* pEngine)
{
	EVar* pObj = pEngine->GetVarParam();
	int nInterfaceID = pEngine->GetUIntParam();
	unsigned int nMethodDeclID = pEngine->GetUIntParam();
	EClass* pEClass = (EClass*)pObj->pOb->GetType(); // todo: check if it's really a class first
	int* pVirtualTable = pEClass->GetVirtualTable();
	int nPos = 0;
	while(nPos >= 0)
	{
		if(pVirtualTable[nPos] == nInterfaceID)
		{
			int nSize;
			unsigned char* pMethod = pEngine->GetCompiledMethod(pVirtualTable[nPos + 2 + nMethodDeclID], &nSize);
			if(nSize <= 0)
				pEngine->ThrowEngineError(L"Attempted to call an abstract method");
			pEngine->BeginNewStackLayer();
			pEngine->m_pInstructionPointer = pMethod;
			return;
		}
		nPos = pVirtualTable[nPos + 1];
	}
	pEngine->ThrowEngineError(L"Attempted to call an interface that isn't implemented by this object");
}


//------------------------------------------------------------------
// IncrementInteger
//		Param1: dest variable
//------------------------------------------------------------------
void Instr_IncrementInteger(GVM* pEngine)
{
	EVar* pVar = pEngine->GetVarParam();
	pVar->pIntObject->m_value++;
}


void Instr_InvertInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	pDest->pIntObject->m_value = ~pDest->pIntObject->m_value;
}




void Instr_IsVarEqual(GVM* pEngine)
{
	EVar* pBool = pEngine->GetVarParam();
	EVar* pA = pEngine->GetVarParam();
	EVar* pB = pEngine->GetVarParam();
	pBool->pIntObject->m_value = (pA->pOb == pB->pOb ? 1 : 0);
}

void Instr_IsVarNotEqual(GVM* pEngine)
{
	EVar* pBool = pEngine->GetVarParam();
	EVar* pA = pEngine->GetVarParam();
	EVar* pB = pEngine->GetVarParam();
	pBool->pIntObject->m_value = (pA->pOb != pB->pOb ? 1 : 0);
}

void Instr_IsIntEqual(GVM* pEngine)
{
	EVar* pBool = pEngine->GetVarParam();
	EVar* pA = pEngine->GetVarParam();
	EVar* pB = pEngine->GetVarParam();
	pBool->pIntObject->m_value = (pA->pIntObject->m_value == pB->pIntObject->m_value ? 1 : 0);
}

void Instr_IsIntNotEqual(GVM* pEngine)
{
	EVar* pBool = pEngine->GetVarParam();
	EVar* pA = pEngine->GetVarParam();
	EVar* pB = pEngine->GetVarParam();
	pBool->pIntObject->m_value = (pA->pIntObject->m_value != pB->pIntObject->m_value ? 1 : 0);
}

void Instr_IsIntLessThan(GVM* pEngine)
{
	EVar* pBool = pEngine->GetVarParam();
	EVar* pA = pEngine->GetVarParam();
	EVar* pB = pEngine->GetVarParam();
	pBool->pIntObject->m_value = (pA->pIntObject->m_value < pB->pIntObject->m_value ? 1 : 0);
}

void Instr_IsIntGreaterThan(GVM* pEngine)
{
	EVar* pBool = pEngine->GetVarParam();
	EVar* pA = pEngine->GetVarParam();
	EVar* pB = pEngine->GetVarParam();
	pBool->pIntObject->m_value = (pA->pIntObject->m_value > pB->pIntObject->m_value ? 1 : 0);
}


//------------------------------------------------------------------
// Jump
//		Param1: offset to jump
//------------------------------------------------------------------
void Instr_Jump(GVM* pEngine)
{
	pEngine->m_pInstructionPointer += pEngine->GetIntParam();
}

void Instr_MaxInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	if(pSource->pIntObject->m_value > pDest->pIntObject->m_value)
		pDest->pIntObject->m_value = pSource->pIntObject->m_value;
}

void Instr_MinInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	if(pSource->pIntObject->m_value < pDest->pIntObject->m_value)
		pDest->pIntObject->m_value = pSource->pIntObject->m_value;
}

void Instr_ModulusInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pDest->pIntObject->m_value = ABS(pDest->pIntObject->m_value % pSource->pIntObject->m_value);
}

void Instr_MultiplyInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pDest->pIntObject->m_value *= pSource->pIntObject->m_value;
}

//------------------------------------------------------------------
// NewInteger
//		Param1: destination variable
//------------------------------------------------------------------
void Instr_NewInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	pEngine->SetVar(pDest, pEngine->Allocate(pEngine->m_pLibrary->GetInteger()));
}

//------------------------------------------------------------------
// NewObject
//		Param1: destination variable
//		Param2: class id
//------------------------------------------------------------------
void Instr_NewObject(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	int nClassID = pEngine->GetUIntParam();
	pEngine->SetVar(pDest, pEngine->Allocate((EClass*)pEngine->m_pLibrary->GetEType(nClassID))); // todo: make sure it's really a class
}

//------------------------------------------------------------------
// NewVariable
//------------------------------------------------------------------
void Instr_NewVariable(GVM* pEngine)
{
	EVar vv;
	vv.eObType = VT_OB_REF;
	vv.pOb = NULL;
	pEngine->m_pStack->Push(&vv);
}

//------------------------------------------------------------------
// OrInteger
//		Param1: dest variable
//		Param2: source variable
//------------------------------------------------------------------
void Instr_OrInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pDest->pIntObject->m_value |= pSource->pIntObject->m_value;
}

//------------------------------------------------------------------
// PushParameter
//		Param1: variable to push
//------------------------------------------------------------------
void Instr_PushParameter(GVM* pEngine)
{
	EVar* pVar = pEngine->GetVarParam();
	pEngine->AddRef(pVar->pOb);
	pEngine->m_pStack->Push(pVar);
}

//------------------------------------------------------------------
// Return
//------------------------------------------------------------------
void Instr_Return(GVM* pEngine)
{
	while(pEngine->m_pStack->GetSizeAboveBase())
		pEngine->PopAndDestruct();
	pEngine->EndStackLayer();
	if(!pEngine->m_pInstructionPointer)
		pEngine->m_bKeepGoing = false;
}

void Instr_SetFloatWithConst(GVM* pEngine)
{
	EVar* pVar = pEngine->GetVarParam();
	unsigned int* pUInts = (unsigned int*)&pVar->pFloatObject->m_value;
	pUInts[0] = pEngine->GetUIntParam();
	pUInts[1] = pEngine->GetUIntParam();
}

//------------------------------------------------------------------
// SetIntWithConst
//		Param1: dest variable
//		Param2: value
//------------------------------------------------------------------
void Instr_SetIntWithConst(GVM* pEngine)
{
	EVar* pVar = pEngine->GetVarParam();
	pVar->pIntObject->m_value = pEngine->GetUIntParam();
}

void Instr_SetMember(GVM* pEngine)
{
	// todo: check that its not an integer type
	EVar* pDest = pEngine->GetVarParam();
	unsigned int nMember = pEngine->GetUIntParam();
	EVar* pSource = pEngine->GetVarParam();
	if(!pDest->pOb)
		pEngine->ThrowNullReferenceSetError();
	GObject* pTmp = pDest->pObjectObject->arrFields[nMember];
	pEngine->SetField(pDest->pObjectObject, nMember, pSource->pOb);
}

void Instr_ShiftLeft(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pBits = pEngine->GetVarParam();
	pDest->pIntObject->m_value <<= pBits->pIntObject->m_value;
}

void Instr_ShiftRight(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pBits = pEngine->GetVarParam();
	pDest->pIntObject->m_value >>= pBits->pIntObject->m_value;
}


//------------------------------------------------------------------
// StartScope
//------------------------------------------------------------------
void Instr_StartScope(GVM* pEngine)
{
	pEngine->m_pStack->StartScope();
}

//------------------------------------------------------------------
// SubtractInteger
//		Param1: dest variable
//		Param2: source variable
//------------------------------------------------------------------
void Instr_SubtractInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pDest->pIntObject->m_value -= pSource->pIntObject->m_value;
}

void Instr_Throw(GVM* pEngine)
{
	int nExceptionObject = pEngine->GetIntParam();
	GObject* pExceptionObject = pEngine->m_pStack->GetVariable(nExceptionObject)->pOb;
	GAssert(!pEngine->m_pException->GetGObject(), "There's already an exception being thrown!");
	pEngine->m_pException->SetGObject(pExceptionObject);
	pEngine->UnwindStackForException();
}

void Instr_TryCallLinked(GVM* pEngine)
{
	int nCatchObject = pEngine->GetIntParam();
	unsigned int nTarget = pEngine->GetUIntParam();
	pEngine->BeginNewStackLayer();
	ObjectObject* pStackLayerObject = pEngine->GetCurrentStackLayerObject();
	GAssert(pStackLayerObject->arrFields[0] == NULL, "already has a catcher set"); // todo: unmagic the field number
	pEngine->SetField(pStackLayerObject, 0, pEngine->Allocate(pEngine->m_pLibrary->GetInteger())); // todo: unmagic the field number
	((IntObject*)pStackLayerObject->arrFields[0])->m_value = nCatchObject; // todo: unmagic the field number
	pEngine->m_pInstructionPointer = (unsigned char*)nTarget;
}

void Instr_TryCall(GVM* pEngine)
{
	int nCatchObject = pEngine->GetIntParam();
	unsigned int nParam1 = pEngine->GetUIntParam();
	int nSize;
	unsigned char* pMethod = pEngine->GetCompiledMethod(nParam1, &nSize);
	if(nSize <= 0)
		pEngine->ThrowEngineError(L"Attempted to call an abstract method");
	pEngine->m_pInstructionPointer -= (sizeof(int) + sizeof(unsigned int));
	GAssert(pEngine->m_pInstructionTable[*(pEngine->m_pInstructionPointer - 1)] == Instr_TryCall, "Failed to find this instruction");
	*(pEngine->m_pInstructionPointer - 1) = pEngine->m_cTryCallLinkedInstr;
	*(unsigned int*)(pEngine->m_pInstructionPointer + sizeof(int)) = (unsigned int)pMethod;
	Instr_TryCallLinked(pEngine);
}

//------------------------------------------------------------------
// XorInteger
//		Param1: dest variable
//		Param2: source variable
//------------------------------------------------------------------
void Instr_XorInteger(GVM* pEngine)
{
	EVar* pDest = pEngine->GetVarParam();
	EVar* pSource = pEngine->GetVarParam();
	pDest->pIntObject->m_value ^= pSource->pIntObject->m_value;
}

