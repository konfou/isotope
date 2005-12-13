/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#ifdef WIN32
#include <Memory.h>
#endif // WIN32
#include "GVMStack.h"
#include "TagNames.h"
#include "EInstrArray.h"
#include "InstrTable.h"
#include "InstrSet.h"
#include "EMethod.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GXML.h"
#include "../Include/GaspEngine.h"
#include "../CodeObjects/Method.h"
#include "EClass.h"
#include "Error.h"
#include <wchar.h>

//#define LOG_SPEW

#ifdef _DEBUG
#define ValidateRefCount(pOb, pnException) _ValidateRefCount(pOb, pnException)
#else
#define ValidateRefCount(pOb, pnException)
#endif // _DEBUG

// ************************************************************************
// GVM Constructor / Destructor / Methods
// ************************************************************************

GVM::GVM(Library* pLibrary, CallBackGetter* pCallBackGetter, ErrorHandler* pErrorHandler, const char* szChrootJail)
 : Engine(pLibrary, pCallBackGetter, szChrootJail)
{
    // Make a stack
	m_pStack = new GVMStack();

	m_pInstructionTable = (GVMInstrPointer*)new char[256 * sizeof(GVMInstrPointer)];
	GVMInstrPointer tmp = GetInstructionStruct(0)->pMethod;
	int n;
	for(n = 0; n < 256; n++)
		m_pInstructionTable[n] = tmp;
	int nInstrCount = GetInstrCount();
	for(n = 0; n < nInstrCount; n++)
		m_pInstructionTable[n] = GetInstructionStruct(n)->pMethod;
	m_cCallLinkedInstr = GetFuncEnum(Instr_CallLinked);
	m_cTryCallLinkedInstr = GetFuncEnum(Instr_TryCallLinked);
	m_cMCallLinkedInstr = GetFuncEnum(Instr_MCallLinked);
	
	m_pErrorHandler = pErrorHandler;
	m_nNestedCalls = 0;

	// Debugger Stuff
	m_pBreakPointThis = NULL;
	m_pBreakPointHandler = NULL;
	m_pGUIHandler = NULL;
	m_pHandlerParam = NULL;
}

GVM::~GVM()
{
	while(m_pStack->GetStackSize() > 0)
		PopAndDestruct();
	CollectTheGarbage();
	delete((char*)m_pInstructionTable);
	delete(m_pStack);
}

void GVM::Run()
{
	GVMInstrPointer pInstr;
	do
	{
		try
		{
			do
			{
				// To see current offset, watch "m_pInstructionPointer - m_pCurrentMethod"
				pInstr = m_pInstructionTable[*m_pInstructionPointer];
#ifdef LOG_SPEW
				fprintf(stderr, "%s ------------- Offset=%d\n", GetInstructionStruct(*m_pInstructionPointer)->szName, m_pInstructionPointer - m_pCurrentMethod);
#endif // LOG_SPEW
				m_pInstructionPointer++;
				pInstr(this);
#ifdef LOG_SPEW
				DumpStackToStdErr();
#endif // LOG_SPEW
				//CheckRefCountsForEveryObjectOnTheHeap();
			}
			while(m_bKeepGoing);
		}
		catch(VarHolder* pException)
		{
			pException; // so the compiler doesn't whine about pException being unreferenced
			GAssert(pException == m_pException, "unexpected thrower");
			UnwindStackForException();
		}
		catch(const char* szException)
		{
			GString s;
			s.Copy(szException);
			const wchar_t* wszException = s.GetString();
			GObject* pException = MakeException("NullReferenceException", wszException);
			m_pException->SetGObject(pException);
			UnwindStackForException();
		}
		catch(const wchar_t* wszException)
		{
			GObject* pException = MakeException("NullReferenceException", wszException);
			m_pException->SetGObject(pException);
			UnwindStackForException();
		}
		catch(...)
		{
			GAssert(false, "C++ exception");
			GObject* pException = MakeException("NullReferenceException", L"A C++ exception was thrown.  Check your parameters and make sure your not trying to do something invalid.");
			m_pException->SetGObject(pException);
			UnwindStackForException();
		}
	}
	while(m_bKeepGoing); // m_bKeepGoing will be true here if an exception was handled
}

void GVM::Step()
{
	bool bOldKeepGoing = m_bKeepGoing;
	m_bKeepGoing = false;
	Run();
	m_bKeepGoing = bOldKeepGoing;
}

void GVM::Terminate()
{
	m_bKeepGoing = false;
}

void GVM::DumpStackToStdErr()
{
	GString s;
	m_pStack->DumpStack(&s, this, 5, true);
	fwprintf(stderr, s.GetString());
}

/*virtual*/ void GVM::DumpStack(GString* pString)
{
	m_pStack->DumpStack(pString, this, 4, false);
}

// Whenever an object is placed on the stack, an unsigned char
// that indicates the object-type is placed on top of it.  This
// method pops off the pair and calls any destructors necessary.
void GVM::PopAndDestruct()
{
	// Pop the object-type byte
	EVar* pVar = m_pStack->Pop();

	// Pop and destruct the object
	if(pVar->eObType == VT_OB_REF)
		Release(pVar->pOb);
}

void GVM::FlushStack()
{
	while(m_pStack->GetStackSize() > 0)
		PopAndDestruct();
	GAssert(m_pStack->GetSizeAboveBase() == 0, "stack Err");
}

// The purpose of this class is to make sure that
// the variable that holds the thrown object is cleared
// after the exception is handled by the error handler.
// (Since it's likely that an error handler may throw,
// we use a destructor to make sure that this will
// happen when the stack is unwound.)
class CallExceptionHandlerHolder
{
protected:
	ExceptionError m_eh;
	VarHolder* m_pException;

public:
	CallExceptionHandlerHolder(VarHolder* pException, bool bExpected)
	{
		m_pException = pException;
		m_eh.SetError(pException, bExpected);
	}

	~CallExceptionHandlerHolder()
	{
		m_pException->SetGObject(NULL);
	}

	void HandleError(ErrorHandler* pHandler)
	{
		pHandler->OnError(&m_eh);
	}
};

bool GVM::Call(struct MethodRef* pMethod, VarHolder** pParams, int nParams)
{
	GAssert(!m_pException->GetGObject(), "There is already a throwing object!");
	int nMethodID;
	if(pMethod->bVirtual)
	{
		// The target method is abstract, so find the overloading virtual method
		if(nParams < 1)
		{
			GAssert(false, "virtual call requires at least one parameter");
			return false;
		}
		EType* pType = pParams[0]->GetGObject()->GetType();
		if(pType->GetTypeType() != EType::TT_CLASS)
		{
			GAssert(false, "virtual call requires the first parameter to be an object");
			return false;
		}
		int* pVirtualTable = ((EClass*)pType)->GetVirtualTable();
		nMethodID = pVirtualTable[pMethod->nIndex];
		if(nMethodID < 0)
		{
			GAssert(false, "attempted to call an abstract method, or perhaps the method wasn't imported.");
			return false;
		}
	}
	else
		nMethodID = pMethod->nIndex;
#ifdef LOG_SPEW
	fprintf(stderr, "################## calling %s.%s\n", m_pLibrary->GetEMethod(nMethodID)->GetClass()->GetName(), m_pLibrary->GetEMethod(nMethodID)->GetName());
#endif // LOG_SPEW
	int nSize;
	unsigned char* pMethodEntryPoint = GetCompiledMethod(nMethodID, &nSize);
	if(nSize <= 0)
	{
		GAssert(false, "attempted to call an abstract method");
		return false;
	}
	GAssert(pMethodEntryPoint, "raw data invalid");
	GAssert(nParams == m_pLibrary->GetEMethod(nMethodID)->CountParams(), "Wrong number of params"); // todo: throw an exception or return false.  Also, make this check faster somehow
	int nStackSize = m_pStack->GetStackSize();

	// Push the parameters
	EVar* pVar;
	int n;
	for(n = 0; n < nParams; n++)
	{
		pVar = pParams[n]->GetVariable();
		AddRef(pVar->pOb);
		m_pStack->Push(pVar);
	}

	// Push Base Pointer and Instruction Pointer
	m_pInstructionPointer = NULL;
	BeginNewStackLayer();
	m_pInstructionPointer = pMethodEntryPoint;
#ifdef _DEBUG
	m_pCurrentMethod = m_pInstructionPointer;
#endif // _DEBUG
#ifdef LOG_SPEW
	m_pCurrentMethod = m_pInstructionPointer;
#endif // LOG_SPEW

	// Go!
	m_nNestedCalls++;
	m_bKeepGoing = true;
	Run();
	m_nNestedCalls--;

	// Clean up the stack
	EVar* pvv;
	int nCurrentStackSize = m_pStack->GetStackSize();
	while(nCurrentStackSize > nStackSize + nParams)
	{
		pvv = m_pStack->Pop();
		if(pvv->eObType == VT_OB_REF)
			Release(pvv->pOb);
		nCurrentStackSize = m_pStack->GetStackSize();
	}

	// Pop the parameters
	n = nParams - 1;
	while(nCurrentStackSize > nStackSize)
	{
		pVar = m_pStack->Pop();
		GAssert(pVar->eObType == VT_OB_REF, "Expected a pointer var");
		pParams[n]->SetGObject(pVar->pOb);
		Release(pVar->pOb);
		nCurrentStackSize = m_pStack->GetStackSize();
		n--;
	}

	// If there was an unhandled exception
	if(m_pException->GetGObject())
	{
		if(m_nNestedCalls > 0)
			throw m_pException; // rethrow the exception in the C++ realm
		else
		{
			CallExceptionHandlerHolder cehh(m_pException, m_bExpectAnException);
			cehh.HandleError(m_pErrorHandler);
			return false;
		}
	}

	return true;
}

unsigned char* GVM::GetCompiledMethod(int nMethodID, int* pnSize)
{
	if(nMethodID < 0 || nMethodID >= m_pLibrary->GetMethodCount())
	{
		GAssert(false, "Out of range");
		return NULL;
	}
	EMethod* pMethod = m_pLibrary->GetEMethod(nMethodID);
	COProject* pProject = m_pLibrary->GetProject();
	EInstrArray* pInstructions = pMethod->GetEInstrArray(pProject ? pMethod->GetCOMethod(pProject) : NULL);
	*pnSize = pInstructions->GetSize();
	return pInstructions->GetData();
}

int GVM::FindWhichMethodThisIsIn(unsigned char* pLocation)
{
    int nCount = m_pLibrary->GetMethodCount();
	unsigned char* pRawData;
	int nRawDataSize;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pRawData = GetCompiledMethod(n, &nRawDataSize);
		if(pLocation >= pRawData && pLocation < pRawData + nRawDataSize)
			return n;
	}
	return -1;
}

void GVM::UnwindStackForException()
{
	GAssert(m_pException->GetGObject(), "No object to throw");
	bool bFoundCatcher = false;
	int nCatcherStackPos = 0;
	EVar* pStackLayerObject;

	// Unwind the stack until we find a catcher
	while(true)
	{
		// Unwind the current stack layer local variables
		while(m_pStack->GetSizeAboveBase())
			PopAndDestruct();

		// Look for a catcher
		pStackLayerObject = m_pStack->GetVariable(-1); // The stack layer object is always at -1 (relative to base pointer)
		if(pStackLayerObject->pOb)
		{
			ObjectObject* pObjectObject = (ObjectObject*)pStackLayerObject->pOb;
			if(pObjectObject->arrFields[0]) // todo: unmagic the field number
			{
				bFoundCatcher = true;
				nCatcherStackPos = ((IntObject*)pObjectObject->arrFields[0])->m_value;
			}
		}

		// Unwind the current stack head
		EndStackLayer();
		if(!m_pInstructionPointer)
		{
			GAssert(!bFoundCatcher, "a catcher is for the stack-layer below this one.  But there is no layer below this one.");
			break;
		}

		// Get catched (no it's not "caught", the proper term here is "catched")
		if(bFoundCatcher)
		{
			EVar* pCatcherVar = m_pStack->GetVariable(nCatcherStackPos);
			GObject* pExceptionObject = m_pException->GetGObject();
			SetVar(pCatcherVar, pExceptionObject);
			m_pException->SetGObject(NULL);
			return;
		}
	}

	// No catcher.  The VM will throw a C++ exception after it returns
	m_bKeepGoing = false;
}

// This lazily makes sure there is a stack layer object on
// the current stack layer and returns a pointer to it.
ObjectObject* GVM::GetCurrentStackLayerObject()
{
	EVar* pVar = m_pStack->GetVariable(-1); // The stack layer object is always at -1 (relative to base pointer)
	GAssert(pVar->eObType == VT_OB_REF, "expected a pointer type");
	if(!pVar->pOb)
		SetVar(pVar, Allocate(m_pLibrary->GetStackLayer()));
	return pVar->pObjectObject;
}

// This uses GVMStack::GetCallStack to get an array of "struct CallStackLayer"s,
// but only two fields in each object are initialized, so this method figures
// out and sets the rest of the fields
void GVM::GetCallStack(GCallStackLayerArray* pOutCallStack)
{
	m_pStack->GetCallStack(pOutCallStack, m_pInstructionPointer);
	int nCount = pOutCallStack->GetSize();
	int n;
	struct CallStackLayer* pLayer;
	unsigned char* pRawData;
	int nRawDataSize;
	int nID;
	for(n = 0; n < nCount; n++)
	{
		pLayer = (struct CallStackLayer*)pOutCallStack->GetLayer(n);
		nID = FindWhichMethodThisIsIn(pLayer->pCodePos);
		if(nID < 0)
		{
			pLayer->nMethodID = -1;
			pLayer->nOffset = -1;
		}
		else
		{
			pRawData = GetCompiledMethod(n, &nRawDataSize);
			pLayer->nMethodID = nID;
			pLayer->nOffset = pLayer->pCodePos - pRawData;
		}
	}
}

bool GVM::SwapByte(int nMethodID, int nOffset, unsigned char cInByte, unsigned char* pcOutByte)
{
	if(nMethodID < 0 || nMethodID >= m_pLibrary->GetMethodCount())
	{
		GAssert(false, "method id out of range");
		return false;
	}
	int nSize;
	unsigned char* pEMethod = GetCompiledMethod(nMethodID, &nSize);
	GAssert(nOffset >= 0 && nOffset < nSize, "out of range");
	*pcOutByte = pEMethod[nOffset];
	pEMethod[nOffset] = cInByte;
	return true;
}

// Because the UI for the debugger runs in a modal window
// which takes a lock on Qt UI control, it is necessary for
// functions that create a window to do it in the same
// thread as the debugger dialog or it will wait forever
// for the debugger dialog to release the lock.  But the
// VM is running in another thread, so when the VM does
// a callback to a function that wants to create a window,
// that window will call this function which will call
// the function it passes in with a guarantee that it will
// be called in the same thread as the dialog that currently
// holds the UI lock.
void GVM::CallInGUIThread(GuiFunc pGuiFunc, void* pParam)
{
	if(m_pGUIHandler == NULL)
	{
		// No GUI Handling function has been provided, so
		// we will assume that the VM is running in the
		// GUI thread.
		pGuiFunc(pParam);
		return;
	}
	else
	{
		// The debugger has provided a special handler that
		// we can use to call pGuiFunc in the GUI thread.
		m_pGUIHandler(m_pHandlerParam, pGuiFunc, pParam);
		return;
	}
}

#ifdef _DEBUG
inline bool IsObjectClassType(GObject* pObject, Library* pLibrary)
{
	EType* pEType = pObject->GetType();
	if(!pEType->GetTypeType() == EType::TT_CLASS)
		return false;
	if(((EClass*)pEType)->IsIntegerType())
		return false;
	return true;
}

// This runs through every object on the heap and verifies
// that the ref-count on every object is correct by finding
// all the objects that ref it.
void GVM::CheckRefCountsForEveryObjectOnTheHeap()
{
	GObject* pObject;
	for(pObject = m_objectList.pNext; pObject; pObject = pObject->pNext)
	{
		int nRefs = pObject->nRefCount;
		int nCount = 0;

		// Find stack variables that ref it
		int n;
		for(n = m_pStack->GetStackSize() - 1; n >= 0; n--)
		{
			EVar* pVar = m_pStack->GetVariableNotRelative(n);
			if(pVar->eObType == VT_OB_REF && pVar->pOb == pObject)
				nCount++;
		}

		// Find object-members that ref it
		GObject* pOb;
		for(pOb = m_objectList.pNext; pOb; pOb = pOb->pNext)
		{
			if(!IsObjectClassType(pOb, m_pLibrary))
				continue;
			EType* pType = pOb->GetType();
			EClass* pClass = (EClass*)pType;
			int nFieldCount = pClass->GetTotalMemberCount();
			for(n = 0; n < nFieldCount; n++)
			{
				if(((ObjectObject*)pOb)->arrFields[n] == pObject)
					nCount++;
			}
		}

		GAssert(nCount == nRefs, "Ref Count is wrong");
	}
}
#endif // _DEBUG
