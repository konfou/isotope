/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GASHENGINE_H__
#define __GASHENGINE_H__

#include <stdio.h>
#include "../../GClasses/GString.h"
#ifndef WIN32
#include <cassert>
#endif // WIN32

class CallBackData;
class CallBackGetter;
class CBData;
class COMethod;
class CompileError;
class COProject;
class COType;
class EClass;
class EType;
class EInterface;
class EMethod;
class EMethodSignature;
class Engine;
class ErrorHandler;
class ErrorHolder;
class EType;
class FileHolder;
class GashFloat;
class GashStream;
class GashString;
class GCallStackLayerArray;
class GCompiler;
class GConstStringHashTable;
class GHeap;
class GIntArray;
class GObject;
class GPointerArray;
class GHashTable;
class GPointerQueue;
class GQueue;
class GString;
class GVM;
class GVMStack;
class GXMLAttribute;
class GXMLTag;
class IntObject;
class Library;
class MBigInt;
class ObjectObject;
class VMCallBackData;
class WrapperObject;

struct EImplementation;
struct MethodRef;

#ifndef GAssert
#ifdef _DEBUG
#ifdef WIN32
#define GAssert(x,y)				\
				{					\
					if(!(x))		\
					{				\
						__asm int 3	\
					}				\
				}
#else // WIN32
#define GAssert(x,y)  assert(x)
#endif // !WIN32
#else // _DEBUG
#define GAssert(x,y)	((void)0)
#endif // else _DEBUG
#endif // GAssert





class Primitive
{
public:
	enum PrimitiveType
	{
		PT_INTEGER,
		PT_BOOL,
		PT_STRING,
		PT_FLOAT,
	};

	virtual PrimitiveType GetType() = 0;
};

class PrimitiveInteger : public Primitive
{
public:
	int m_nValue;

	virtual PrimitiveType GetType() { return PT_INTEGER; }
};

class PrimitiveBool : public Primitive
{
public:
	bool m_bValue;

	virtual PrimitiveType GetType() { return PT_BOOL; }
};

class PrimitiveFloat : public Primitive
{
public:
	double m_dValue;

	virtual PrimitiveType GetType() { return PT_FLOAT; }
};

class PrimitiveString : public Primitive
{
protected:
	wchar_t* m_sValue;

public:
	PrimitiveString() { m_sValue = NULL; }
	~PrimitiveString() { delete(m_sValue); }
	virtual PrimitiveType GetType() { return PT_STRING; }
	wchar_t* GetValue() { return m_sValue; }
	void SetValue(const wchar_t* sValue);
};







enum VAR_TYPE
{
	VT_OB_REF = 0,
	VT_RETURN_POSITION = 1,
	VT_BASE_POINTER = 2,
	VT_LOCAL_POINTER = 3,
};


typedef struct
{
	VAR_TYPE eObType;
	union
	{
		GObject* pOb;
		IntObject* pIntObject;
		ObjectObject* pObjectObject;
		WrapperObject* pWrapperObject;
		GashString* pStringObject;
		GashFloat* pFloatObject;
		GashStream* pStreamObject;
		MBigInt* pBigInt;
	};
} EVar;








typedef void (WrapperObject::*MachineMethod0)(Engine*);
typedef void (WrapperObject::*MachineMethod1)(Engine*, EVar*);
typedef void (WrapperObject::*MachineMethod2)(Engine*, EVar*, EVar*);
typedef void (WrapperObject::*MachineMethod3)(Engine*, EVar*, EVar*, EVar*);
typedef void (WrapperObject::*MachineMethod4)(Engine*, EVar*, EVar*, EVar*, EVar*);
typedef void (WrapperObject::*MachineMethod5)(Engine*, EVar*, EVar*, EVar*, EVar*, EVar*);
typedef void (WrapperObject::*MachineMethod6)(Engine*, EVar*, EVar*, EVar*, EVar*, EVar*, EVar*);
typedef void (WrapperObject::*MachineMethod7)(Engine*, EVar*, EVar*, EVar*, EVar*, EVar*, EVar*, EVar*);
typedef void (WrapperObject::*MachineMethod8)(Engine*, EVar*, EVar*, EVar*, EVar*, EVar*, EVar*, EVar*, EVar*);





class EMethodPointerHolder // todo: move to non-included header
{
public:
	union
	{
		MachineMethod0 m_m0;
		MachineMethod1 m_m1;
		MachineMethod2 m_m2;
		MachineMethod3 m_m3;
		MachineMethod4 m_m4;
		MachineMethod5 m_m5;
		MachineMethod6 m_m6;
		MachineMethod7 m_m7;
		MachineMethod8 m_m8;
	};

	EMethodPointerHolder(MachineMethod0 m) { m_m0 = m; }
	EMethodPointerHolder(MachineMethod1 m) { m_m1 = m; }
	EMethodPointerHolder(MachineMethod2 m) { m_m2 = m; }
	EMethodPointerHolder(MachineMethod3 m) { m_m3 = m; }
	EMethodPointerHolder(MachineMethod4 m) { m_m4 = m; }
	EMethodPointerHolder(MachineMethod5 m) { m_m5 = m; }
	EMethodPointerHolder(MachineMethod6 m) { m_m6 = m; }
	EMethodPointerHolder(MachineMethod7 m) { m_m7 = m; }
	EMethodPointerHolder(MachineMethod8 m) { m_m8 = m; }
	virtual ~EMethodPointerHolder() { }
};











class GObject
{
friend class Engine;
public:
	virtual ~GObject() {}

	unsigned int nRefCount; // references by GObjects
	unsigned int nPinnedRefs; // references by C++ objects
	GObject* pNext;
	GObject* pPrev;

protected:
	EType* m_pType;

public:
#ifdef _DEBUG
	int nAllocID; // The number of allocations before this object was allocated
#endif // _DEBUG

	EType* GetType() { return m_pType; }
	virtual void toStream(Engine* pEngine, EVar* pStream, EVar* pRefs) = 0;
	virtual void setRefs(Engine* pEngine, EVar* pRefs) = 0;
};



class ObjectObject : public GObject // todo: rename to "ClassObject"
{
public:
	GObject* arrFields[8]; // actually variable-sized array

	virtual void toStream(Engine* pEngine, EVar* pStream, EVar* pRefs);
	void fromStream(Engine* pEngine, EVar* pStream);
	virtual void setRefs(Engine* pEngine, EVar* pRefs);
};


class IntObject : public GObject
{
public:
	int m_value;

	virtual void toStream(Engine* pEngine, EVar* pStream, EVar* pRefs);
	void fromStream(Engine* pEngine, EVar* pStream);
	virtual void setRefs(Engine* pEngine, EVar* pRefs);
};



typedef void (*RegisterMethods)(GConstStringHashTable* pTable);


class WrapperObject : public GObject // todo: rename to "MachineObject"
{
friend class Engine;
public:
	WrapperObject(Engine* pEngine, const char* szClassName);
	virtual ~WrapperObject();

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize) = 0;
	static EMethodPointerHolder* FindMachineMethod(GConstStringHashTable* pClassesTable, const char* szClassName, EMethodSignature* pMethodSignature);
	static void RegisterMachineClass(GConstStringHashTable** ppClassesTable, const char* szClassName, RegisterMethods pRegisterFunc);
};










inline void UnlinkObject(GObject* pObject)
{
	if(pObject->pPrev)
		pObject->pPrev->pNext = pObject->pNext;
	if(pObject->pNext)
		pObject->pNext->pPrev = pObject->pPrev;
}

inline void LinkObject(GObject* pObject, GObject* pPrev)
{
	pObject->pPrev = pPrev;
	pObject->pNext = pPrev->pNext;
	if(pPrev->pNext)
		pPrev->pNext->pPrev = pObject;
	pPrev->pNext = pObject;
}








class VarHolder
{
friend class Engine; // to access protected constructor
friend class GVM; // to access m_pNext
protected:
	EVar m_var;
	Engine* m_pEngine;
#ifdef _DEBUG
	VarHolder* m_pPrev;
	VarHolder* m_pNext;
#endif // _DEBUG

public:
#ifdef _DEBUG
	const char* m_szID;
#endif // _DEBUG

	VarHolder(Engine* pEngine); // Makes a new variable initialized to Null
	VarHolder(Engine* pEngine, Primitive* pPrimitive); // Allocates a GObject and copies the primitive value to it
	virtual ~VarHolder();

	inline GObject* GetGObject() { return m_var.pOb; }
	void SetGObject(GObject* pObject);
	EVar* GetVariable() { return &m_var; }
	Engine* GetEngine() { return m_pEngine; }

protected:
	void Init(Engine* pEngine);
};








extern GConstStringHashTable* g_pBuiltInMachineObjects;

void RegisterBuiltInClasses();

class CallBackGetter
{
public:
	CallBackGetter() {}
	virtual ~CallBackGetter() {}

	virtual EMethodPointerHolder* GetCallBack(const char* szClassName, EMethodSignature* pMethodSignature)
	{
		if(!g_pBuiltInMachineObjects)
			RegisterBuiltInClasses();
		EMethodPointerHolder* pMeth = WrapperObject::FindMachineMethod(g_pBuiltInMachineObjects, szClassName, pMethodSignature);
		if(pMeth)
			return pMeth;
		return NULL;
	}
};






typedef void (*GuiFunc)(void*);

class Engine
{
friend class WrapperObject;
friend class VarHolder;
public:
	Library* m_pLibrary;
	EVar* m_pMachineThis; // Before calling a machine method, you should set m_pMachineThis to point to the variable that will accept changes of the machine method's "this" object

protected:
	// GHeap is faster than the C++ heap because it doesn't use syscalls and it doesn't waste time being thread-safe
	// Warning: that means you'd better not use an Engine object asynchronously!
	GHeap* m_pHeap; 

	// Members for Garbage Collection
	GPointerQueue* m_pTodoQueue; // used to avoid recursion when Releasing objects
	IntObject m_objectList;
	int m_nObjectCount;

	// Other
	CallBackGetter* m_pCallBackGetter;
	char* m_szChrootJail;
	bool m_bExpectAnException;
	VarHolder* m_pGlobalObject;
#ifdef _DEBUG
	int m_nObjectID;
	VarHolder* m_pVarHolders;
#endif // _DEBUG

public:
	VarHolder* m_pException; // holds the exception while it's being thrown

	Engine(Library* pLibrary, CallBackGetter* pCallBackGetter, const char* szChrootJail);
	virtual ~Engine();

	// Abstracts
	virtual bool Call(struct MethodRef* pMethod, VarHolder** pParams, int nParams) = 0; // todo: make it take a EVar array as parameters instead of a VarHolder array
	virtual void CallInGUIThread(GuiFunc pGuiFunc, void* pParam) = 0;
	virtual void DumpStack(GString* pString) = 0;

	// Getters
	Library* GetLibrary() { return m_pLibrary; }

	// This returns the number of currently allocated objects
	int GetObjectCount() { return m_nObjectCount; }

	// This does a full garbage collection
	void CollectTheGarbage();

	GObject* Allocate(EClass* pClass);
	void DestructObject(GObject* pObject);
	inline void AddRef(GObject* pObject); // for references by a GObject
	inline void Release(GObject* pObject); // for references by a GObject
	inline void PinObject(GObject* pObject); // for references by a C++ object
	inline void UnpinObject(GObject* pObject); // for references by a C++ object
	CallBackGetter* GetCallBackGetter() { return m_pCallBackGetter; }

	// Machine methods can call this method to change the value of "this"
	void SetThis(WrapperObject* pThis);

	inline void SetVar(EVar* pVar, GObject* pOb)
	{
		AddRef(pOb);
		Release(pVar->pOb);
		pVar->pOb = pOb;
	}

	inline void SetField(ObjectObject* pOb, int nField, GObject* pValue)
	{
		AddRef(pValue);
		Release(pOb->arrFields[nField]);
		pOb->arrFields[nField] = pValue;
	}

	void SerializeObject(GObject* pObject, EVar* pStream);
	GObject* DeserializeObject(EVar* pStream);

	void OpenFile(FileHolder* pHolder, const char* szFilename, const char* szMode);
	void OpenFile(FileHolder* pHolder, GString* pFilename, const char* szMode);
	char* LoadFile(const char* szFilename, int* pnFileSize);
	char* LoadFile(GString* pFilename, int* pnFileSize);

	void SetExpectAnException(bool b) { m_bExpectAnException = b; }

	GObject* GetGlobalObject() { return m_pGlobalObject->GetGObject(); }
	void SetGlobalObject(GObject* pOb) { m_pGlobalObject->SetGObject(pOb); }

	// Throw Error methods
	void ThrowEngineError(const wchar_t* wszMessage);
	void ThrowCastError(EType* pSourceType, EType* pDestType);
	void ThrowNullReferenceGetError();
	void ThrowNullReferenceSetError();
	void ThrowFileNotFoundError(const wchar_t* wszFilename);
	void ThrowIOError(const wchar_t* wszMessage, const wchar_t* wszVar1);
	void ThrowIOError(const wchar_t* wszMessage, const char* szVar1, const char* szVar2);
	void ThrowXmlError(const char* szMessage, int nLine, int nCol);
	void ThrowXmlError(GXMLTag* pTag, const wchar_t* wszMessage);
	void ThrowSdlError(const wchar_t* wszMessage);
	void ThrowCompileError(CompileError* pError);

protected:
	// Methods for doing Heap management / Garbage Collection
	virtual int GetStackSize() = 0;
	virtual EVar* GetStackVariable(int nStackPos) = 0;
	void ClassToStream(GObject* pOb, EVar* pStream, EVar* pOutRefs);
	GObject* ClassFromStream(EClass* pClass, EVar* pStream);
	void ClassSetRefs(GObject* pOb, EVar* pRefs);
	EType* FindTypeFromStreamedName(int len, GQueue* pQ);
	GObject* MakeException(const char* szTypeName, const wchar_t* wszMessage);
	GObject* MakeException2(const char* szTypeName, const wchar_t* wszMessage);
	void GetAbsoluteDirectory(char* pBuf, int nBufSize, bool bAvoidVirtualCalls);
	void ThrowError(const char* szTypeName, const wchar_t* wszMessage);
	void ThrowError(const char* szTypeName, const wchar_t* wszMessage, const wchar_t* wszVar1);
	void ThrowError(const char* szTypeName, const wchar_t* wszMessage, const wchar_t* wszVar1, const wchar_t* wszVar2);
};





struct MethodRef
{
	int nIndex;
	bool bVirtual;
};





class Library
{
protected:
	GXMLTag* m_pLibraryTag;

	int m_nMethodCount;
	EMethod* m_pMethods;

	int m_nTypeCount;
	EType** m_pTypes;

	// Quick indexes to strings
	int m_nStringCount;
	const char** m_pStrings;

	// Tables for looking up types by name
	GConstStringHashTable* m_pTypeTableByName;
	GHashTable* m_pTypeTableByPointer;

	// Built in class IDs
	EClass* m_pObject;
	EClass* m_pInteger;
	EClass* m_pStackLayer;

#ifdef _DEBUG
	int m_nLoadPhase;
#endif // _DEBUG

	// Constructors are intentionally protected--use a public static method instead
	Library(GXMLTag* pLibraryTag); // takes ownership of pLibraryTag
	Library(const char* szFilename);

public:
	virtual ~Library();

	// This loads a library from a .xlib file.  You are responsible to
	// delete the library it returns when you are done with it.
	static Library* LoadFromFile(const char* szFilename);

	// This loads a library from a .xlib file that has already been loaded
	// into a buffer.  You are responsible to
	// delete the library it returns when you are done with it.
	static Library* LoadFromBuffer(const char* pBuffer, int nBufferSize); // you must delete the library when you're done with it

	// This creates a library from a .xlib file that you have already
	// parsed into a GXMLTag tree.  This takes ownership of the XML tree, so
	// don't delete it!  You are responsible to delete the library it returns
	// when you are done with it (and it will delete the XML tree you gave it).
	static Library* CreateFromXML(GXMLTag* pLibraryTag);

	// Getters
	int GetMethodCount() { return m_nMethodCount; }
	int GetStringCount() { return m_nStringCount; }
	int GetTypeCount() { return m_nTypeCount; }
	EMethod* GetEMethod(int nID);
	const char* GetStringFromTable(int nID);
	EType* GetEType(int nID);
	GXMLTag* GetLibraryTag() { return m_pLibraryTag; }
	EType* FindType(const char* szTypeName);
	EType* FindTypeConst(const char* szTypeName);
	bool FindMethod(struct MethodRef* pOutMethodRef, EClass* pClass, EMethodSignature* pSignature); // todo: move into the EClass type
	bool FindMethod(struct MethodRef* pOutMethodRef, COMethod* pMethod);
	bool FindMainProc(struct MethodRef* pMethodRef);
	static bool RenumberIDs(GXMLTag* pLibraryTag, COProject* pCOProject);
	char* GetXmlAsCppString(); // You are responsible to "delete" the string it returns
	bool SaveAsCppString(const char* szFilename);

	inline EClass* GetObject() { return m_pObject; }
	inline EClass* GetInteger() { return m_pInteger; }
	inline EClass* GetStackLayer() { return m_pStackLayer; }

protected:
	bool Init();
	bool CountTypes(GXMLTag** ppStringTableTag);
	bool LoadTypesAndCountMethods();
	bool LoadMethods();
	bool PolishTypes();
	bool LoadStrings(GXMLTag* pStringTableTag);
	GXMLTag* FindMethodTag(GXMLTag* pClassTag, const char* szMethodName);
	int DumpXmlTagToCppString(GXMLTag* pTag, char* pBuffer, int nIndent);
	int DumpXmlAttrToCppString(GXMLAttribute* pAttr, char* pBuffer);
	static int DumpMethodParameters(GXMLTag* pMethodTag, char* pBuffer);
	static int DumpXmlToHFile(GXMLTag* pLibrary, char* pBuffer, const char* szTitle);
	static int DumpXmlToCppFile(GXMLTag* pLibrary, char* pBuffer, const char* szTitle);
	static void CheckForReferencesToUnimportedTypes(GXMLTag* pLibraryTag, COProject* pCOProject);
};






#ifdef _DEBUG
struct DEBUG_Stack
{
	EVar stack[200];
};
#endif // _DEBUG

class GVMStack
{
protected:
#ifdef _DEBUG
	struct DEBUG_Stack* m_pDEBUGStack; // a good variable to watch in the debugger
#endif // _DEBUG
	EVar** m_pChunkTable;
	int m_nBasePointer;
	int m_nLocalPointer;
	int m_nStackPointer;

public:
	GVMStack();
	virtual ~GVMStack();

	void Push(EVar* pCopyMeOntoStack);
	EVar* Pop();

	void StartScope();
	inline void SetLocalPointer(int n) { m_nLocalPointer = n; }
	int GetStackSize() { return m_nStackPointer; }
	int GetSizeAboveBase() { return m_nStackPointer - m_nBasePointer; }
	void GetCallStack(GCallStackLayerArray* pOutCallStack, unsigned char* pInstructionPointer);
	void DumpObject(GString* pString, const char* szVarName, GObject* pOb, Library* pLibrary, int nCurrentDepth, int nMaxDepth, bool bIncludeRefCounts);
	void DumpStack(GString* pString, GVM* pVM, int nMaxObjectDepth, bool bIncludeRefCounts);

	inline void PushBasePointerAndInstructionPointer(unsigned char* pInstructionPointer)
	{
		EVar vv;
		vv.eObType = VT_BASE_POINTER;
		vv.pOb = (GObject*)m_nBasePointer;
		Push(&vv);
		vv.eObType = VT_RETURN_POSITION;
		vv.pOb = (GObject*)pInstructionPointer;
		Push(&vv);
		m_nBasePointer = m_nStackPointer + 1; // the "+ 1" is to make room for the stack-layer-object
	}

	inline unsigned char* PopInstructionPointerAndBasePointer()
	{
		EVar* pvv = Pop();
		GAssert(pvv->eObType == VT_RETURN_POSITION, "expected return position");
		unsigned char* pInstructionPointer = (unsigned char*)pvv->pOb;
		pvv = Pop();
		GAssert(pvv->eObType == VT_BASE_POINTER, "expected base pointer");
		m_nBasePointer = (int)pvv->pOb;
		return pInstructionPointer;
	}

#define GetStackObject(n) (&(m_pChunkTable[(n) >> 10])[(n) & 1023])
	// Returns an object from the stack (offset relative to Base Pointer)
	inline EVar* GetVariable(int nOffset)
	{
		nOffset += m_nBasePointer;
		GAssert(nOffset >= 0 && nOffset < m_nStackPointer, "out of range");
		return(GetStackObject(nOffset));
	}

	// Returns an object from the stack (offset relative to bottom)
	inline EVar* GetVariableNotRelative(int nOffset)
	{
		GAssert(nOffset >= 0 && nOffset < m_nStackPointer, "out of range");
		return(GetStackObject(nOffset));
	}

	// Returns an object from the stack (offset relative to Local Pointer)
	inline EVar* GetVariableFromTop(int nOffset)
	{
		nOffset += m_nStackPointer;
		GAssert(nOffset >= 0 && nOffset < m_nStackPointer, "out of range");
		return(GetStackObject(nOffset));
	}
#undef GetStackObject

};



inline unsigned int RevEndian(unsigned int in)
{
	unsigned int out;
	((unsigned char*)&out)[0] = ((unsigned char*)&in)[3];
	((unsigned char*)&out)[1] = ((unsigned char*)&in)[2];
	((unsigned char*)&out)[2] = ((unsigned char*)&in)[1];
	((unsigned char*)&out)[3] = ((unsigned char*)&in)[0];
	return out;
}





typedef void (*CompilerInstr)();
typedef void (*GVMInstrPointer)(GVM* pThis);
typedef void (*BreakPointHandler)(void*);
typedef void (*GuiHandler)(void*, GuiFunc, void*);

class GVM : public Engine
{
public: //protected: todo: fix this
	// VM Stuff
	bool m_bKeepGoing;
	unsigned char* m_pInstructionPointer;
	GVMStack* m_pStack;

	// Tables
	GVMInstrPointer* m_pInstructionTable;

	// Misc
	char m_cCallLinkedInstr;
	char m_cTryCallLinkedInstr;
	char m_cMCallLinkedInstr;
	ErrorHandler* m_pErrorHandler;
	int m_nNestedCalls;
//#ifdef _DEBUG // todo: uncomment this
	const unsigned char* m_pCurrentMethod;
//#endif // _DEBUG

	// Debugger Stuff
	void* m_pBreakPointThis;
	BreakPointHandler m_pBreakPointHandler;
	GuiHandler m_pGUIHandler;
	void* m_pHandlerParam;

public:
	GVM(Library* pLibrary, CallBackGetter* pCallBackGetter, ErrorHandler* pErrorHandler, const char* szChrootJail);
	virtual ~GVM();

	virtual bool Call(struct MethodRef* pMethod, VarHolder** pParams, int nParams);
	virtual void DumpStack(GString* pString);
	void Error(const char* szMessage);
	void PopAndDestruct();
	void FlushStack();

	// Used by the debugger
	void GetCallStack(GCallStackLayerArray* pOutCallStack);
	bool SwapByte(int nMethodID, int nOffset, unsigned char cInByte, unsigned char* pcOutByte);
	void Step();
	void Terminate();
	void ContinueInNewThread() { Run(); }
	void SetBreakPointHandler(void* pThis, BreakPointHandler pBreakPointHandler) { m_pBreakPointThis = pThis; m_pBreakPointHandler = pBreakPointHandler; }
	int FindWhichMethodThisIsIn(unsigned char* pLocation);
	virtual void CallInGUIThread(GuiFunc pGuiFunc, void* pParam);
	void SetGUIHandler(GuiHandler newHandler, void* pHandlerParam) { m_pGUIHandler = newHandler; m_pHandlerParam = pHandlerParam; }
	int GetStackSizeAboveBase() { return m_pStack->GetSizeAboveBase(); }
	EVar* GetVariableFromStackNotRelative(int nStackPos)	{ return m_pStack->GetVariableNotRelative(nStackPos); }

	// used internally by instructions
	inline EVar* GetVarParam()
	{
		int nOffset = *((int*)m_pInstructionPointer);
		m_pInstructionPointer += sizeof(int);
#ifdef DARWIN
		return m_pStack->GetVariable(RevEndian(nOffset));
#else // DARWIN
		return m_pStack->GetVariable(nOffset);
#endif // !DARWIN
	}

	inline int GetIntParam()
	{
		int nParam = *((int*)m_pInstructionPointer);
		m_pInstructionPointer += sizeof(int);
#ifdef DARWIN
		return RevEndian(nParam);
#else // DARWIN
		return nParam;
#endif // !DARWIN
	}

	inline unsigned int GetUIntParam()
	{
		unsigned int nParam = *((int*)m_pInstructionPointer);
		m_pInstructionPointer += sizeof(int);
#ifdef DARWIN
		return RevEndian(nParam);
#else // DARWIN
		return nParam;
#endif // !DARWIN
	}

public: //protected: todo: fix this
	// Helper methods
	void Init(Library* pLibrary);
	unsigned char* GetCompiledMethod(int nMethodID, int* pnSize);
	void Run();
	inline bool CheckCast(int nSourceClassID, int nDestClassID);
	ObjectObject* GetCurrentStackLayerObject();
	void UnwindStackForException();
	void DumpStackToStdErr();

	inline void BeginNewStackLayer()
	{
		m_pStack->PushBasePointerAndInstructionPointer(m_pInstructionPointer);
		EVar vv;
		vv.eObType = VT_OB_REF;
		vv.pOb = NULL;
		m_pStack->Push(&vv);
	}

	inline void EndStackLayer()
	{
		EVar* pvv = m_pStack->Pop();
		GAssert(pvv->eObType == VT_OB_REF, "expected stack-layer var");
		Release(pvv->pOb);
		m_pInstructionPointer = m_pStack->PopInstructionPointerAndBasePointer();
	}

#ifdef _DEBUG
	void CheckRefCountsForEveryObjectOnTheHeap();
#endif // _DEBUG

public:
	// Methods used by the Garbage Collector
	virtual int GetStackSize() { return m_pStack->GetStackSize(); }
	virtual EVar* GetStackVariable(int nStackPos) { return m_pStack->GetVariable(nStackPos); }

};




inline void Engine::AddRef(GObject* pObject)
{
	if(!pObject)
		return;
	pObject->nRefCount++;
}

inline void Engine::Release(GObject* pObject)
{
	if(!pObject)
		return;
	if((--pObject->nRefCount) == 0 && pObject->nPinnedRefs == 0)
		DestructObject(pObject);
}

inline void Engine::PinObject(GObject* pObject)
{
	if(!pObject)
		return;
	pObject->nPinnedRefs++;
}

inline void Engine::UnpinObject(GObject* pObject)
{
	if(!pObject)
		return;
	if((--pObject->nPinnedRefs) == 0 && pObject->nRefCount == 0)
		DestructObject(pObject);
}


class ErrorHandler
{
public:
	virtual void OnError(ErrorHolder* pErrorHolder) = 0;
};



enum FILETYPES
{
	FT_UNRECOGNIZED,
	FT_CLASSIC,
	FT_XML,
	FT_XLIB,
	FT_SOURCE,
};


// An EMethodSignature will distinguish between methods in a class.  It does
// not contain type info, so if you want to globally identify a method, you
// need a Library, Type, and MethodSignature triple.
class EMethodSignature
{
friend class COMethodDecl;
protected:
	GString m_sSignature;

	EMethodSignature(COMethodDecl* pMethod);

public:
	EMethodSignature(GXMLTag* pMethodTag);
	EMethodSignature(const char* szSignature);

	int Compare(EMethodSignature* pThat);
	const wchar_t* GetString() { return m_sSignature.GetString(); }

protected:
	void AddParams(GXMLTag* pTag);

};




typedef struct _ErrorStruct
{
	int nParams;
	const wchar_t* message;
} ErrorStruct;



class ErrorHolder
{
public:
	enum ERROR_TYPE
	{
		ET_GLOBAL,
		ET_PARSE,
		ET_COMPILE,
		ET_CLASSICSYNTAX,
		ET_EXCEPTION,
	};

	ErrorStruct* m_pError;
	char* m_szParam1;

	ErrorHolder();
	~ErrorHolder();

	virtual ERROR_TYPE GetErrorType() = 0;

	void CheckError();
	bool HaveError();
	virtual void ToString(GString* pString) = 0;
	void SetParam1(const char* szParam);
	void AddMessage(GString* pString);
};




FILETYPES IdentifyFileType(const char* szFile, const char** ppStart, int* pnLine);
char* LoadFile(ErrorHandler* pErrorHandler, const char* szFilename, FILETYPES* peType, const char** ppStart, int* pnStartLine);
bool CommandCpp(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv);
bool CommandBuild(ErrorHandler* pErrorHandler, const char* szGashPath, int argc, char** argv);
void CommandRun(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv, CallBackGetter* pCBG, bool bExpectAnException = false);
bool CommandDisassemble(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv);

#endif // __GASHENGINE_H__
