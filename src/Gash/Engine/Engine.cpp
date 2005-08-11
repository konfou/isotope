/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "TagNames.h"
#include "../../GClasses/GPointerQueue.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GHeap.h"
#include "../../GClasses/GFile.h"
#include "../../GClasses/GSpinLock.h"
#include "../../GClasses/GHashTable.h"
#include "../Include/GashEngine.h"
#include "../BuiltIns/GashStream.h"
#include "../BuiltIns/GashString.h"
#include "../CodeObjects/Method.h"
#include "EClass.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

#ifdef WIN32
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif // !WIN32

#define STACK_LAYER_OBJECT_MEMBER_COUNT 1

#define TOUCHED_FLAG (1 << (8 * sizeof(unsigned int) - 1))

inline void TouchObject(GObject* pObject)
{
	pObject->nPinnedRefs |= TOUCHED_FLAG;
}

inline void UntouchObject(GObject* pObject)
{
	pObject->nPinnedRefs &= ~TOUCHED_FLAG;
}

inline bool IsObjectTouched(GObject* pObject)
{
	return((pObject->nPinnedRefs & TOUCHED_FLAG) == TOUCHED_FLAG);
}


inline bool IsObjectClassType(GObject* pObject, Library* pLibrary)
{
	EType* pEType = pObject->GetType();
	if(!pEType->GetTypeType() == EType::TT_CLASS)
		return false;
	if(((EClass*)pEType)->IsIntegerType())
		return false;
	return true;
}

Engine::Engine(Library* pLibrary, CallBackGetter* pCallBackGetter, const char* szChrootJail)
{
	// Initialize Garbage Collection Stuff
	memset(&m_objectList, '\0', sizeof(GObject));
	m_nObjectCount = 0;
	m_pTodoQueue = new GPointerQueue();
#ifdef _DEBUG
	m_nObjectID = 0;
	m_pVarHolders = NULL;
#endif // _DEBUG

	m_pHeap = new GHeap();
	m_pLibrary = pLibrary;
	m_pMachineThis = NULL;

	char szChrootJailBuf[512];
	strcpy(szChrootJailBuf, szChrootJail);
	GetAbsoluteDirectory(szChrootJailBuf, 512, true);
	m_szChrootJail = new char[strlen(szChrootJailBuf) + 1];
	strcpy(m_szChrootJail, szChrootJailBuf);

	// Other
	GAssert(pCallBackGetter, "pCallBackGetter can't be NULL");
	m_pCallBackGetter = pCallBackGetter;
	m_pException = new VarHolder(this);
	m_pGlobalObject = new VarHolder(this);
}

Engine::~Engine()
{
	m_pGlobalObject->SetGObject(NULL);
	delete(m_pGlobalObject);
	GAssert(!m_pException->GetGObject(), "There's still an object being thrown!");
	delete(m_pException);
	GAssert(!m_pVarHolders, "There are still VarHolders associated with this Engine");
	GAssert(m_nObjectCount == 0 && m_objectList.pNext == NULL, "There are still objects in the heap.  Make sure all pinned objects were released.  Also, destructors of classes that inherrit from Engine must empty the stack and call CollectTheGarbage in their destructor");
	GAssert(m_pTodoQueue->GetSize() == 0, "There is still unfinished business in the TodoQueue");
	delete(m_pTodoQueue);
	delete(m_pHeap);
	delete(m_szChrootJail);
}

GObject* Engine::Allocate(EClass* pClass)
{
	// Allocate the memory
	GAssert(((EType*)pClass)->GetTypeType() == EType::TT_CLASS, "Only class objects can be allocated here.  Machine objects should be allocated with 'new'");
	unsigned int nObjectSize;
	if(pClass->IsIntegerType())
		nObjectSize = sizeof(IntObject);
	else
		nObjectSize = sizeof(GObject) +	pClass->GetTotalMemberCount() * sizeof(GObject*);
	GObject* pObject = (GObject*)m_pHeap->Allocate((nObjectSize + sizeof(unsigned int) - 1) / sizeof(unsigned int));
	memset(pObject, '\0', nObjectSize);
	pObject->m_pType = pClass;

#ifdef _DEBUG
	// Set Alloc ID
	pObject->nAllocID = m_nObjectID++;
#endif // _DEBUG

	// Track this object for garbage collection
	LinkObject(pObject, &m_objectList);
	m_nObjectCount++;

	return pObject;
}

// This is the *ONLY* place where a GObject is *EVER* deleted
void Engine::DestructObject(GObject* pObject)
{
	GAssert(pObject->nRefCount == 0, "Object not ready for destruction");
	pObject->nRefCount++;
	GAssert(m_pTodoQueue->GetSize() == 0, "There is still unfinished business in the queue");
	m_pTodoQueue->Push(pObject);
	while(m_pTodoQueue->GetSize() > 0)
	{
		// Decrement the owner count
		pObject = (GObject*)m_pTodoQueue->Pop();
		if(--pObject->nRefCount != 0 || pObject->nPinnedRefs != 0)
			continue;

		// Remove from list
		UnlinkObject(pObject);

		// Do special destructor operations
		EType* pEType = pObject->GetType();
		if(pEType->GetTypeType() == EType::TT_CLASS)
		{
			// Object--release the members
			EClass* pEClass = (EClass*)pEType;
			if(!pEClass->IsIntegerType())
			{
				int nMemberCount = pEClass->GetTotalMemberCount();
				int n;
				ObjectObject* pOb = (ObjectObject*)pObject;
				for(n = 0; n < nMemberCount; n++)
				{
					if(pOb->arrFields[n])
						m_pTodoQueue->Push(pOb->arrFields[n]);
				}
			}
			m_pHeap->Deallocate((unsigned int*)pObject);
		}
		else if(pEType->GetTypeType() == EType::TT_MACHINE)
			delete((WrapperObject*)pObject);
		else
		{
			GAssert(pEType->GetTypeType() == EType::TT_INTERFACE, "unrecognized type");
			m_pHeap->Deallocate((unsigned int*)pObject);
		}

		m_nObjectCount--;
	}
}

void Engine::SetThis(WrapperObject* pNewThis)
{
	SetVar(m_pMachineThis, pNewThis);
}


inline void LinkOb(GObject** ppListHead, GObject* pOb)
{
	pOb->pNext = *ppListHead;
	*ppListHead = pOb;
}

void Engine::ClassToStream(GObject* pOb, EVar* pStream, EVar* pOutRefs)
{
	EClass* pEClass = (EClass*)pOb->GetType();
	if(pEClass->IsIntegerType())
	{
		GQueue* pQ = &pStream->pStreamObject->m_value;
		IntObject* pObject = (IntObject*)pOb;
		pQ->Push(pObject->m_value);
	}
	else
	{
		GQueue* pQ = &pOutRefs->pStreamObject->m_value;
		ObjectObject* pObject = (ObjectObject*)pOb;
		int n;
		for(n = 0; n < pEClass->GetTotalMemberCount(); n++)
			pQ->Push(pObject->arrFields[n]);
	}
}

GObject* Engine::ClassFromStream(EClass* pClass, EVar* pStream)
{
	GObject* pOb = Allocate(pClass);
	if(pClass->IsIntegerType())
	{
		IntObject* pObject = (IntObject*)pOb;
		pStream->pStreamObject->m_value.Pop(&pObject->m_value);
	}
	else
	{
		// classes have no non-ref data
	}
	return pOb;
}

void Engine::ClassSetRefs(GObject* pOb, EVar* pRefs)
{
	// todo: throw on error conditions
	EClass* pClass = (EClass*)pOb->GetType();
	if(pClass->IsIntegerType())
		return;
	GQueue* pQ = &pRefs->pStreamObject->m_value;
	ObjectObject* pObject = (ObjectObject*)pOb;
	GObject* pTmp;
	int n;
	for(n = 0; n < pClass->GetTotalMemberCount(); n++)
	{
		pQ->Pop((void**)&pTmp);
		SetField(pObject, n, pTmp);
	}
}

void Engine::SerializeObject(GObject* pObject, EVar* pStream)
{
	// First Pass: Produce a queue of all transitively referenced objects.
	//             Store the object index in the ref count field.  Store the
	//             ref count value in a paralell queue so we can set it back
	//             when we're done.
	GQueue* pQ = &pStream->pStreamObject->m_value;
	GAssert(m_pTodoQueue->GetSize() == 0, "queue already has stuff in it");
	pQ->Push((int)0); // place holder for the object count
	m_pTodoQueue->Push(pObject);
	GPointerQueue qObs;
	GPointerQueue qRefCounts;
	VarHolder vRefs(this);
	GashStream* pRefs = new GashStream(this);
	vRefs.SetGObject(pRefs);
	GPointerQueue qRefsOut;
	GHashTable ht(31);
	int nIndex = 0;
	int nTypeIndex;
	while(m_pTodoQueue->GetSize() > 0)
	{
		GObject* pOb = (GObject*)m_pTodoQueue->Pop();
		if(!IsObjectTouched(pOb))
		{
			// Serialize the object
			EType* pType = pOb->GetType();
			if(ht.Get(pType, (void**)&nTypeIndex))
				pQ->Push(nTypeIndex);
			else
			{
				const char* szTypeName = pType->GetName();
				pQ->Push(-(int)strlen(szTypeName));
				pQ->Push(szTypeName);
				ht.Add(pType, (void*)nIndex);
			}
			switch(pType->GetTypeType())
			{
				case EType::TT_CLASS:
					// class objects have a bogus VTable pointer, so we can't call ObjectObject::toStream
					ClassToStream(pOb, pStream, vRefs.GetVariable());
					break;
				case EType::TT_MACHINE:
					pOb->toStream(this, pStream, vRefs.GetVariable());
					break;
				case EType::TT_INTERFACE:
					GAssert(false, "how'd you get an instance of an interface?");
					break;
				default:
					GAssert(false, "unexpected enumeration");
			}

			// Add all the referenced objects to the todo queue
			qRefsOut.Push((void*)(pRefs->m_value.GetSize() / sizeof(int)));
			while(pRefs->m_value.GetSize() > 0)
			{
				GObject* pRefOb;
				pRefs->m_value.Pop((void**)&pRefOb);
				if(pRefOb)
					m_pTodoQueue->Push(pRefOb);
				qRefsOut.Push(pRefOb);
			}
			TouchObject(pOb);
			qRefCounts.Push((void*)pOb->nRefCount);
			pOb->nRefCount = nIndex++;
			qObs.Push(pOb);
		}
	}
	pQ->SetFirstInt(nIndex);

	// Second Pass: Encode all the references
	while(qRefsOut.GetSize() > 0)
	{
		int nCount = (int)qRefsOut.Pop();
		pQ->Push(nCount);
		for( ; nCount > 0; nCount--)
		{
			GObject* pOb = (GObject*)qRefsOut.Pop();
			if(pOb)
			{
				GAssert(IsObjectTouched(pOb), "object not touched");
				pQ->Push((int)pOb->nRefCount);
			}
			else
				pQ->Push((int)-1);
		}
	}

	// Third Pass: Restore the ref counts and untouch the objects
	while(qObs.GetSize() > 0)
	{
		GObject* pOb = (GObject*)qObs.Pop();
		pOb->nRefCount = (unsigned int)qRefCounts.Pop();
		UntouchObject(pOb);
	}
}

EType* Engine::FindTypeFromStreamedName(int len, GQueue* pQ)
{
	if(pQ->GetSize() < len)
		ThrowError("DeserializationException", L"Invalid type name length");
	char* szTypeName = (char*)alloca(len + 1);
	char* pC = szTypeName;
	while(len > 0)
	{
		pQ->Pop(pC++);
		len--;
	}
	*pC = '\0';
	EType* pType = m_pLibrary->FindType(szTypeName);
	if(!pType)
	{
		ConvertAnsiToUnicode(szTypeName, wszTypeName);
		ThrowError("DeserializationException", L"Couldn't find type: %s", wszTypeName);
	}
	return pType;
}

GObject* Engine::DeserializeObject(EVar* pStream)
{
	// First Pass: Deserialize all the objects
	int nObjectCount;
	GQueue* pQ = &pStream->pStreamObject->m_value;
	if(!pQ->Pop(&nObjectCount))
		ThrowError("DeserializationException", L"Expected an object count");
	if(nObjectCount <= 0)
		ThrowError("DeserializationException", L"Expected the object count to be positive");
	GTEMPBUF(pTempBuf, nObjectCount * sizeof(GObject*));
	GObject** pObs = (GObject**)pTempBuf;
	int nID;
	GObject* pOb;
	EType* pType;
	int n;
	for(n = 0; n < nObjectCount; n++)
	{
		if(!pQ->Pop(&nID))
			ThrowError("DeserializationException", L"Expected an object index or string length");
		if(nID >= 0)
		{
			if(nID >= n)
				ThrowError("DeserializationException", L"Object index out of range");
			pType = pObs[nID]->GetType();
		}
		else
			pType = FindTypeFromStreamedName(-nID, pQ);
		pOb = NULL;
		if(pType->GetTypeType() == EType::TT_MACHINE)
		{
			// Machine object
			EMethodSignature sig(SIG_FROM_STREAM);
			EMethodPointerHolder* pFromStream = m_pCallBackGetter->GetCallBack(pType->GetName(), &sig);
			if(!pFromStream)
				ThrowError("DeserializationException", L"Missing a method with the signature \"method !fromStream(&Stream)\"");
			WrapperObject* pObWrapper = NULL;
			EVar ev;
			memset(&ev, '\0', sizeof(EVar));
			m_pMachineThis = &ev;
			(pObWrapper->*pFromStream->m_m1)(this, pStream);
			pOb = m_pMachineThis->pWrapperObject;
			if(!pOb)
				ThrowError("DeserializationException", L"The fromStream method didn't call Engine::SetThis with a non-null value");
			pOb->nRefCount--; // Clear the one ref that was added for "ev" which will now go out of scope, but we don't want to destroy the object obviously
		}
		else
		{
			if(pType->GetTypeType() != EType::TT_CLASS)
				ThrowError("DeserializationException", L"Expected a class type");
			pOb = ClassFromStream((EClass*)pType, pStream);
		}
		pObs[n] = pOb;
	}

	// Pass 2: Set the refs
	VarHolder vRefs(this);
	GashStream* pRefs = new GashStream(this);
	vRefs.SetGObject(pRefs);
	for(n = 0; n < nObjectCount; n++)
	{
		// Get the number of references
		int nReferences;
		if(!pQ->Pop(&nReferences))
			return NULL;
		if(nReferences == 0)
			continue;

		// Load a queue with the references
		int i;
		int nIndex;
		for(i = 0; i < nReferences; i++)
		{
			if(!pQ->Pop(&nIndex))
				ThrowError("DeserializationException", L"Expected more references");
			if(nIndex == -1) // -1 represents NULL
				pRefs->m_value.Push((void*)NULL);
			else
			{
				if(nIndex < 0 || nIndex >= nObjectCount)
					ThrowError("DeserializationException", L"Referenced an out of range object");
				pRefs->m_value.Push((void*)pObs[nIndex]);
			}
		}

		// Call the routine
		EType* pType = pObs[n]->GetType();
		if(pType->GetTypeType() == EType::TT_MACHINE)
			pObs[n]->setRefs(this, vRefs.GetVariable());
		else
			ClassSetRefs(pObs[n], vRefs.GetVariable());
	}

	// Return the root object
	if(pQ->GetSize() > 0)
		ThrowError("DeserializationException", L"Extra data in blob");
	return pObs[0];
}

GObject* Engine::MakeException2(const char* szTypeName, const wchar_t* wszMessage)
{
	// Find the type
	EType* pType = m_pLibrary->FindTypeConst(szTypeName);
	if(!pType || pType->GetTypeType() != EType::TT_CLASS)
		return NULL;
	EClass* pExceptionClass = (EClass*)pType;

	// Allocate the message string
	VarHolder vMessage(this);
	GashString* pMessage = new GashString(this);
	if(!pMessage)
		return NULL;
	GString* pString = &pMessage->m_value;
	pString->Copy(wszMessage);
	pString->Add(L"\n\n");
	DumpStack(pString);
	vMessage.SetGObject(pMessage);

	// Find the method with the signature "!new(String)"
	EMethodSignature sig(SIG_EXCEPTION_NEW);
	struct MethodRef mr;
	if(!m_pLibrary->FindMethod(&mr, pExceptionClass, &sig))
		return NULL;

	// Call the method
	VarHolder vException(this);
	VarHolder* params[2];
	params[0] = &vException;
	params[1] = &vMessage;
	bool bOK = Call(&mr, params, 2);
	GAssert(bOK, "error making exception object");

	// AddRef the exception object so it won't be deleted when we return
	GObject* pException = vException.GetGObject();
	AddRef(pException);
	return pException;
}

GObject* Engine::MakeException(const char* szTypeName, const wchar_t* wszMessage)
{
	GObject* pException = MakeException2(szTypeName, wszMessage);
	if(pException)
	{
		pException->nRefCount--; // turn it into a zombie object
		GAssert(pException->nRefCount == 0, "who is referencing this object?");
	}
	else
	{
		GAssert(false, "Failed to make an exception object!  Defaulting to an Integer object.");
		pException = Allocate(m_pLibrary->GetInteger());
	}
	return pException;
}

void Engine::ThrowError(const char* szTypeName, const wchar_t* wszMessage)
{
	GObject* pException = MakeException(szTypeName, wszMessage);
	GAssert(pException, "Failed to create the exception object");
	GAssert(!m_pException->GetGObject(), "There's already an exception being thrown!");
	m_pException->SetGObject(pException);
	fflush(stdout); // make sure any logging has been flushed
	if(!m_bExpectAnException)
		GotAnError();
	throw m_pException;
}

void Engine::ThrowError(const char* szTypeName, const wchar_t* wszMessage, const wchar_t* wszVar1)
{
	GTEMPBUF(pBuf, sizeof(wchar_t) * (wcslen(wszMessage) + wcslen(wszVar1) + 10));
	wchar_t* wszBuf = (wchar_t*)pBuf;
	swprintf(wszBuf, 1024, wszMessage, wszVar1);
	ThrowError(szTypeName, wszBuf);
}

void Engine::ThrowError(const char* szTypeName, const wchar_t* wszMessage, const wchar_t* wszVar1, const wchar_t* wszVar2)
{
	GTEMPBUF(pBuf, sizeof(wchar_t) * (wcslen(wszMessage) + wcslen(wszVar1) + wcslen(wszVar2) + 20));
	wchar_t* wszBuf = (wchar_t*)pBuf;
#ifdef WIN32	
	swprintf(wszBuf, wszMessage, wszVar1, wszVar2);
#else
	swprintf(wszBuf, 1024, wszMessage, wszVar1, wszVar2);
#endif // WIN32
	ThrowError(szTypeName, wszBuf);
}

void Engine::ThrowEngineError(const wchar_t* wszMessage)
{
	ThrowError("EngineException", wszMessage);
}

void Engine::ThrowCastError(EType* pSourceType, EType* pDestType)
{
	ConvertAnsiToUnicode(pSourceType->GetName(), wszSourceType);
	ConvertAnsiToUnicode(pDestType->GetName(), wszDestType);
	ThrowError("CastException", L"Can't cast a \"%s\" to a \"%s\".", wszSourceType, wszDestType);
}

void Engine::ThrowNullReferenceGetError()
{
	ThrowError("NullReferenceException", L"Attempted to get a member from a null variable");
}

void Engine::ThrowNullReferenceSetError()
{
	ThrowError("NullReferenceException", L"Attempted to set a member of a null variable");
}

void Engine::ThrowFileNotFoundError(const wchar_t* wszFilename)
{
	char szCurDir[512];
	getcwd(szCurDir, 512);
	ConvertAnsiToUnicode(szCurDir, wszCurDir);
	ThrowError("IOException", L"The file \"%s\" was not found.  (The current directory is \"%s\".)", wszFilename, wszCurDir);
}

void Engine::ThrowIOError(const wchar_t* wszMessage, const wchar_t* wszVar1)
{
	ThrowError("IOException", wszMessage, wszVar1);
}

void Engine::ThrowIOError(const wchar_t* wszMessage, const char* szVar1, const char* szVar2)
{
	ConvertAnsiToUnicode(szVar1, wszVar1);
	ConvertAnsiToUnicode(szVar2, wszVar2);
	ThrowError("IOException", wszMessage, wszVar1, wszVar2);
}

void Engine::ThrowXmlError(const char* szMessage, int nLine, int nCol)
{
	GString s;
	s.Add(L"XML error at line ");
	s.Add(nLine);
	s.Add(L", column ");
	s.Add(nCol);
	s.Add(L": ");
	s.Add(szMessage);
	ThrowError("XmlException", s.GetString());
}

void Engine::ThrowXmlError(GXMLTag* pTag, const wchar_t* wszMessage)
{
	GString s;
	s.Add(L"XML error at line ");
	s.Add(pTag->GetLineNumber());
	s.Add(L", column ");
	int nCol, nWid;
	pTag->GetOffsetAndWidth(&nCol, &nWid);
	s.Add(nCol);
	s.Add(L": ");
	s.Add(wszMessage);
}

void Engine::ThrowSdlError(const wchar_t* wszMessage)
{
	ThrowError("SdlException", wszMessage);
}

void Engine::ThrowCompileError(CompileError* pError)
{
	GString s;
	pError->ToString(&s);
	ThrowError("CompileException", s.GetString());
}

inline void CollectGarbageHelper(GObject* pObject, GObject* pNewList, Library* pLibrary, GPointerQueue* pQueue)
{
	TouchObject(pObject);
	UnlinkObject(pObject);
	LinkObject(pObject, pNewList);
	if(IsObjectClassType(pObject, pLibrary))
		pQueue->Push(pObject);
}

void Engine::CollectTheGarbage()
{
	IntObject tmpList;
	memset(&tmpList, '\0', sizeof(GObject));

	// Move all the objects owned by stack variables to the temp list
	EVar* pVar;
	int n;
	for(n = GetStackSize() - 1; n >= 0; n--)
	{
		pVar = GetStackVariable(n);
		if(pVar->eObType == VT_OB_REF && pVar->pOb)
			CollectGarbageHelper(pVar->pOb, &tmpList, m_pLibrary, m_pTodoQueue);
	}

	// Move all the pinned objects to the temp list
	GObject* pTempNext = NULL;
	GObject* pObject;
	for(pObject = m_objectList.pNext; pObject; pObject = pTempNext)
	{
		pTempNext = pObject->pNext;
		if(pObject->nPinnedRefs != 0)
		{
			GAssert(pObject->nPinnedRefs < 0x7fffffff, "looks like the number of pins is invalid");
			CollectGarbageHelper(pObject, &tmpList, m_pLibrary, m_pTodoQueue);
		}
	}

	// Move all the objects owned by owned-objects to the temp list
	ObjectObject* pObjectObject;
	while(m_pTodoQueue->GetSize() > 0)
	{
		pObjectObject = (ObjectObject*)m_pTodoQueue->Pop();
		GObject* pTmp;
		EType* pType = pObjectObject->GetType();
		GAssert(pType->GetTypeType() == EType::TT_CLASS, "all objects should be class-types");
		int nMemberCount = ((EClass*)pType)->GetTotalMemberCount();
		for(n = 0; n < nMemberCount; n++)
		{
			pTmp = pObjectObject->arrFields[n];
			if(pTmp && !IsObjectTouched(pTmp))
				CollectGarbageHelper(pTmp, &tmpList, m_pLibrary, m_pTodoQueue);
		}
	}

	// Unmark all the objects in the temp list
	for(pObject = tmpList.pNext; pObject; pObject = pObject->pNext)
	{
		GAssert(IsObjectTouched(pObject), "This object isn't marked");
		UntouchObject(pObject);
	}

	// Swap the lists (so m_objectList will only contain referenced objects)
	pObject = tmpList.pNext;
	tmpList.pNext = m_objectList.pNext;
	m_objectList.pNext = pObject;
	if(pObject)
		pObject->pPrev = &m_objectList;
	if(tmpList.pNext)
		tmpList.pNext->pPrev = &tmpList;

	// Unlink everything in the temp list (the objects we want to collect)
	IntObject deathRow;
	memset(&deathRow, '\0', sizeof(GObject));
	pObject = tmpList.pNext;
	while(pObject)
	{
		GAssert(pObject->nPinnedRefs == 0, "pinned objects should have been moved to the other list");
		UnlinkObject(pObject);
		LinkObject(pObject, &deathRow);
		if(IsObjectClassType(pObject, m_pLibrary))
		{
			AddRef(pObject); // so it won't get deleted while we're working with it

			// Set all the members to Null
			int n;
			EType* pType = pObject->GetType();
			GAssert(pType->GetTypeType() == EType::TT_CLASS, "all objects should be class-types");
			int nMemberCount = ((EClass*)pType)->GetTotalMemberCount();
			pObjectObject = (ObjectObject*)pObject;
			for(n = 0; n < nMemberCount; n++)
			{
				Release(pObjectObject->arrFields[n]);
				pObjectObject->arrFields[n] = NULL;
			}

			Release(pObject); // if this doesn't kill it, then another object in the list still references it, but it will die before we get out of this loop
		}
		pObject = tmpList.pNext;
	}

	GAssert(!deathRow.pNext, "Some objects have ref-counting problems!");

	// Wipe out the objects with ref-counting problems.  It is now safe to do
	// this because we already know they don't reference anything (since we
	// set all the members to null) and nothing references them (or they wouldn't
	// be in this list), so it won't mess up any ref-counting to forcibly delete them.
	while(deathRow.pNext)
	{
		deathRow.pNext->nRefCount = 1; // force its ref-count to 1
		Release(deathRow.pNext); // b-bye, see ya!
	}
}



GSpinLock* g_pCurrentWorkingDirLock = new GSpinLock();

void Engine::GetAbsoluteDirectory(char* pBuf, int nBufSize, bool bAvoidVirtualCalls)
{
	char szOldCWD[512];
	getcwd(szOldCWD, 512);
	bool bOK = true;
	g_pCurrentWorkingDirLock->Lock("Engine::GetAbsoluteDirectory");
	if(pBuf[0] == '\0')
		strcpy(pBuf, ".");
	if(chdir(pBuf) != 0)
		bOK = false;
	else
		getcwd(pBuf, nBufSize);
	chdir(szOldCWD);
	g_pCurrentWorkingDirLock->Unlock();
	if(!bOK)
	{
		if(bAvoidVirtualCalls) // (ThrowIOError calls a virtual method, which will give unpredictable results if this object hasn't been fully constructed yet)
			throw "Invalid directory";
		else
			ThrowIOError(L"Unable to change to directory \"%s\" from directory \"%s\".", pBuf, szOldCWD);
	}
	int n;
	for(n = 0; pBuf[n] != '\0'; n++)
	{
#ifdef WIN32
		if(pBuf[n] == '\\')
			pBuf[n] = '/';
#endif // WIN32
	}
	if(n > 0 && pBuf[n - 1] != '/')
	{
		pBuf[n] = '/';
		pBuf[n + 1] = '\0';
	}
}

void Engine::OpenFile(FileHolder* pHolder, GString* pFilename, const char* szMode)
{
	char* szFilename = (char*)alloca(pFilename->GetLength() + 1);
	pFilename->GetAnsi(szFilename);
	OpenFile(pHolder, szFilename, szMode);
}

void Engine::OpenFile(FileHolder* pHolder, const char* szFilename, const char* szMode)
{
	char szFullPath[512];
	strcpy(szFullPath, m_szChrootJail);
	strcat(szFullPath, szFilename);
	char szTempDrive[10];
	char szTempDir[512];
	_splitpath(szFullPath, szTempDrive, szTempDir, NULL, NULL);
	char szNewDir[512];
	_makepath(szNewDir, szTempDrive, szTempDir, NULL, NULL);
	GetAbsoluteDirectory(szNewDir, 512, false);
	if(strnicmp(szNewDir, m_szChrootJail, strlen(m_szChrootJail)) != 0)
		ThrowIOError(L"The file \"%s\" is outside the allowed directory tree: \"%s\".", szFullPath, m_szChrootJail);
	FILE* pFile = fopen(szFullPath, szMode);
	if(!pFile)
	{
		ConvertAnsiToUnicode(szFilename, wszFilename);
		if(!GFile::DoesFileExist(szFilename))
			ThrowFileNotFoundError(wszFilename);
		ThrowIOError(L"Access is denied while trying to open the file \"%s\".", wszFilename);
	}
	pHolder->Set(pFile);
}

char* Engine::LoadFile(const char* szFilename, int* pnFileSize)
{
	FileHolder hFile(NULL);
	OpenFile(&hFile, szFilename, "rb");
	GAssert(hFile.Get(), "OpenFile should throw if it fails to open a file");
	int nFileSize = filelength(fileno(hFile.Get()));
	*pnFileSize = nFileSize;
	Holder<char*> hBuffer(new char[nFileSize + 1]);
	if(!hBuffer.Get())
		ThrowEngineError(L"Out of memory");
	int nBytesRead = fread(hBuffer.Get(), sizeof(char), nFileSize, hFile.Get());
	int err = ferror(hFile.Get());
	if(err != 0 || nBytesRead != nFileSize)
	{
		ConvertAnsiToUnicode(szFilename, wszFilename);
		ThrowIOError(L"There was an error reading from the file \"%s\".", wszFilename);
	}
	hBuffer.Get()[nFileSize] = '\0';
	return hBuffer.Drop();
}

char* Engine::LoadFile(GString* pFilename, int* pnFileSize)
{
	char* szBuf = (char*)alloca(pFilename->GetLength() + 1);
	pFilename->GetAnsi(szBuf);
	return LoadFile(szBuf, pnFileSize);
}
