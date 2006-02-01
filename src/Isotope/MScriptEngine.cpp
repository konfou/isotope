/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MScriptEngine.h"

#include "../GClasses/GString.h"
#include "../GClasses/GHashTable.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GFile.h"
#include "../Gasp/BuiltIns/GaspStream.h"
#include "../Gasp/BuiltIns/GaspFloat.h"
#include "../Gasp/MachineObjects/MArray.h"
#include "../Gasp/Engine/Error.h"
#include "../Gasp/Engine/EType.h"
#include "../Gasp/CodeObjects/Project.h"
#include "../Gasp/CodeObjects/Class.h"
#include "../Gasp/CodeObjects/Interface.h"
#include "../Gasp/Engine/GCompiler.h"
#include "../Gasp/Engine/Disassembler.h"
#include "Main.h"
#include "MObject.h"
#include "MAnimation.h"
#include <math.h>
#include "MGameClient.h"
#include "MStore.h"
#include "VWave.h"
#include "MRealm.h"
#include "MGameImage.h"
#include "Controller.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN
#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else // WIN32
#include <unistd.h>
#endif // !WIN32

void IsotopeErrorHandler::OnError(ErrorHolder* pErrorHolder)
{
	GString s;
	pErrorHolder->ToString(&s);
	GTEMPBUF(pBuf, s.GetLength() + 1);
	s.GetAnsi(pBuf);
	GameEngine::ThrowError(pBuf);
}

// -------------------------------------------------------------------------------

GConstStringHashTable* g_pIsotopeMachineObjects = NULL;


void RegisterMGameMachine(GConstStringHashTable* pTable)
{
	pTable->Add("method getAnimation(!Animation, String)", new EMethodPointerHolder((MachineMethod2)&MGameMachine::getAnimation));
	pTable->Add("method getAvatar(!Avatar)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::getAvatar));
	pTable->Add("method getObjectById(!RealmObject, Integer)", new EMethodPointerHolder((MachineMethod2)&MGameMachine::getObjectById));
	pTable->Add("method getClosestObject(!RealmObject, Float, Float)", new EMethodPointerHolder((MachineMethod3)&MGameMachine::getClosestObject));
	pTable->Add("method getImage(!GImage, String)", new EMethodPointerHolder((MachineMethod2)&MGameMachine::getImage));
	pTable->Add("method followLink(String)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::followLink));
	pTable->Add("method getUid(&Integer)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::getUid));
	pTable->Add("method getTime(&Float)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::getTime));
	pTable->Add("method addObject(RealmObject)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::addObject));
	pTable->Add("method removeObject(Integer)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::removeObject));
	pTable->Add("method playSound(String)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::playSound));
	pTable->Add("method notifyServerAboutObjectUpdate(RealmObject)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::notifyServerAboutObjectUpdate));
	pTable->Add("method sendToClient(Object, Integer)", new EMethodPointerHolder((MachineMethod2)&MGameMachine::sendToClient));
	pTable->Add("method sendToServer(Object)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::sendToServer));
	pTable->Add("method addInventoryItem(String)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::addInventoryItem));
	pTable->Add("method checkForSolidObject(&Bool, Float, Float)", new EMethodPointerHolder((MachineMethod3)&MGameMachine::checkForSolidObject));
	pTable->Add("method getAccountVar(&String, String)", new EMethodPointerHolder((MachineMethod2)&MGameMachine::getAccountVar));
	pTable->Add("method setAccountVar(String, String)", new EMethodPointerHolder((MachineMethod2)&MGameMachine::setAccountVar));
	pTable->Add("method reportStats(Array)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::reportStats));
	pTable->Add("method setSkyImage(String)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::setSkyImage));
	pTable->Add("method setGroundImage(String)", new EMethodPointerHolder((MachineMethod1)&MGameMachine::setGroundImage));
	pTable->Add("method setPivotHeight(Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&MGameMachine::setPivotHeight));
	pTable->Add("method addMerit(String, Bool, Float, Float)", new EMethodPointerHolder((MachineMethod4)&MGameMachine::addMerit));
}

// Declared in MGameImage.cpp
void RegisterMGameImage(GConstStringHashTable* pTable);

// Declared in MAnimation.cpp
void RegisterMAnimation(GConstStringHashTable* pTable);

void RegisterIsotopeMachineClasses()
{
	WrapperObject::RegisterMachineClass(&g_pIsotopeMachineObjects, "GameMachine", RegisterMGameMachine);
	WrapperObject::RegisterMachineClass(&g_pIsotopeMachineObjects, "Animation", RegisterMAnimation);
	WrapperObject::RegisterMachineClass(&g_pIsotopeMachineObjects, "GImage", RegisterMGameImage);
}

// -------------------------------------------------------------------------------


MScriptEngine::MScriptEngine(const char* szScriptUrl, const char* szScript, int nScriptSize, ErrorHandler* pErrorHandler, GXMLTag* pMapTag, MGameClient* pGameClient, Controller* pController, MRealm* pRealm)
{
	m_pRealm = pRealm;

	// Parse the script into a project
	Holder<COProject*> hProj(new COProject("*bogus*"));
	if(!hProj.Get()->LoadLibraries(GameEngine::GetAppPath(), pErrorHandler))
	{
		GAssert(false, "The error handler should have thrown");
		GameEngine::ThrowError("Failed to load Gasp libraries from: %s", GameEngine::GetAppPath());
	}
	if(!hProj.Get()->LoadSources(&szScript, &szScriptUrl, 1, pErrorHandler))
	{
		GAssert(false, "The error handler should have thrown");
		GameEngine::ThrowError("Failed to parse the script file");
	}

	// Build the project into a library
	CompileError errorHolder;
	{
		COProject* pProj = hProj.Drop();
		GCompiler comp(pProj, &errorHolder);
		ImportObjectDependencies(&comp, pProj, pMapTag);
        m_pLibrary = comp.Compile(true);
	}
	if(!m_pLibrary)
	{
		pErrorHandler->OnError(&errorHolder);
		GAssert(false, "The error handler should have thrown");
		GameEngine::ThrowError("Failed to compile the script");
	}

//m_pLibrary->GetLibraryTag()->ToFile("c:\\dbg.xlib"); // todo: remove this code:	
/*int nSize;
Holder<char*> hDisassembly(Disassembler::DisassembleLibraryToText(m_pLibrary, &nSize));
GFile::SaveBufferToFile((unsigned char*)hDisassembly.Get(), nSize, "c:\\tmp.txt");*/

	// Load the library into a virtual machine
	char szCurDir[512];
	getcwd(szCurDir, 512);
	m_pCBG = new IsotopeCallBackGetter();
	m_pErrorHandler = pErrorHandler;
	m_pVM = new MVM(m_pLibrary, m_pCBG, m_pErrorHandler, szCurDir, pGameClient, pController, this);

	// Prefind the methods we're going to need
	FindMethod(&m_mrUpdate, "RealmObject", "method &update(Float)");
	FindMethod(&m_mrVerify, "RealmObject", "method verify(&Bool, Integer, RealmObject)");
	FindMethod(&m_mrGetFrame, "RealmObject", "method getFrame(!GImage, &Rect, Float)");
	FindMethod(&m_mrDoAction, "RealmObject", "method &doAction(Float, Float)");
	FindMethod(&m_mrOnGetFocus, "RealmObject", "method &onGetFocus()");
	FindMethod(&m_mrOnLoseFocus, "RealmObject", "method &onLoseFocus()");
	FindMethod(&m_mrReceiveFromClient, "Remote", "method &receiveFromClient(Object, Integer)");
	FindMethod(&m_mrReceiveFromServer, "Remote", "method &receiveFromServer(Object)");

	// Make some variables
	m_pRectVar = new VarHolder(m_pVM);
	GAssert(m_pRectVar->m_szID = "Script Engine Rect Var", "");
	m_pImageVar = new VarHolder(m_pVM);
	GAssert(m_pImageVar->m_szID = "Script Engine Image Var", "");
	m_pStreamVar = new VarHolder(m_pVM);
	GAssert(m_pStreamVar->m_szID = "Script Engine Stream Var", "");
	int n;
	for(n = 0; n < MAX_PARAMS; n++)
	{
		m_pStringVars[n] = new VarHolder(m_pVM);
		GAssert(m_pStringVars[n]->m_szID = "Script Engine String Var", "");
	}
	m_pFloatVar = new VarHolder(m_pVM);
	GAssert(m_pFloatVar->m_szID = "Script Engine Float Var", "");
	m_pFloatVar2 = new VarHolder(m_pVM);
	GAssert(m_pFloatVar2->m_szID = "Script Engine Float Var", "");
	PrimitiveInteger pi;
	pi.m_nValue = 0;
	m_pIntVar = new VarHolder(m_pVM, &pi);
	GAssert(m_pIntVar->m_szID = "Script Engine Int Var", "");
	m_pIntVar2 = new VarHolder(m_pVM, &pi);
	GAssert(m_pIntVar2->m_szID = "Script Engine Int Var 2", "");
	m_pTempVar = new VarHolder(m_pVM);
	GAssert(m_pTempVar->m_szID = "Script Engine Temp Var", "");
	m_pTempVar2 = new VarHolder(m_pVM);
	GAssert(m_pTempVar2->m_szID = "Script Engine Temp Var 2", "");

	// Preallocate a Rect object
	struct MethodRef mr;
	FindMethod(&mr, "Rect", "method !new()");
	if(!m_pVM->Call(&mr, &m_pRectVar, 1))
		GameEngine::ThrowError("Failed to allocate a Gasp object");

	// Preallocate some machine objects
	m_pFloatVar->SetGObject(new GaspFloat(m_pVM));
	m_pFloatVar2->SetGObject(new GaspFloat(m_pVM));
	m_pStreamVar->SetGObject(new GaspStream(m_pVM));
	for(n = 0; n < MAX_PARAMS; n++)
		m_pStringVars[n]->SetGObject(new GaspString(m_pVM));
}

/*virtual*/ MScriptEngine::~MScriptEngine()
{
	delete(m_pRectVar);
	delete(m_pImageVar);
	delete(m_pIntVar);
	delete(m_pIntVar2);
	delete(m_pFloatVar);
	delete(m_pFloatVar2);
	delete(m_pStreamVar);
	int n;
	for(n = 0; n < MAX_PARAMS; n++)
		delete(m_pStringVars[n]);
	delete(m_pTempVar);
	delete(m_pTempVar2);

	delete(m_pVM);
	delete(m_pCBG);
	delete(m_pLibrary);
}

void MScriptEngine::ImportMethodDependency(GCompiler* pCompiler, COClass* pClass, const char* szSig)
{
	EMethodSignature sig(szSig);
	COMethod* pMethod = pClass->FindMethod(&sig);
	if(pMethod)
		pCompiler->AddImportMethod(pMethod);
	else
	{
		//GAssert(false, "method not found");
	}
}

void MScriptEngine::ImportRealmObject(const char* szTypeName, GCompiler* pCompiler, COProject* pProject, int nConstructorParams)
{
	// Import the type
	COType* pType = pProject->FindType(szTypeName);
	//GAssert(pType, "type not found");
	if(!pType)
		return;
	pCompiler->AddImportType(pType);
	if(pType->GetTypeType() != COType::TT_CLASS)
	{
		//GAssert(false, "expected a class");
		return;
	}
	COClass* pClass = (COClass*)pType;

	// Make the constructor signature
	GQueue q;
	q.Push("method !new(");
	int n;
	for(n = 0; n < nConstructorParams; n++)
	{
		if(n > 0)
			q.Push(", ");
		q.Push("String");
	}
	q.Push(")");
	Holder<char*> hSigNew(q.DumpToString());

	// Import all the methods
	ImportMethodDependency(pCompiler, pClass, hSigNew.Get());
	ImportMethodDependency(pCompiler, pClass, "method &update(Float)");
	ImportMethodDependency(pCompiler, pClass, "method getFrame(!GImage, &Rect, Float)");
	ImportMethodDependency(pCompiler, pClass, "method &doAction(Float, Float)");
	ImportMethodDependency(pCompiler, pClass, "method &onGetFocus()");
	ImportMethodDependency(pCompiler, pClass, "method &onLoseFocus()");
	ImportMethodDependency(pCompiler, pClass, "method verify(&Bool, Integer, RealmObject)");
}

// The Gasp compiler optimizes by not including any uncalled methods in the library it builds.  But since
// we're calling several methods from C++ code, we have to tell the compiler to explicitly import the methods
// that we're expecting.
void MScriptEngine::ImportObjectDependencies(GCompiler* pCompiler, COProject* pProject, GXMLTag* pMapTag)
{
	if(pMapTag)
	{
		GXMLTag* pObjectsTag = pMapTag->GetChildTag("Objects");
		char szTmp1[32];
		strcpy(szTmp1, "Param");
		char szTmp2[32];
		GXMLTag* pObjTag;
		for(pObjTag = pObjectsTag->GetFirstChildTag(); pObjTag; pObjTag = pObjectsTag->GetNextChildTag(pObjTag))
		{
			GXMLAttribute* pClassAttr = pObjTag->GetAttribute("Class");
			if(pClassAttr)
			{
				// Count the parameters to the new method
				int n;
				for(n = 1; n <= MAX_PARAMS; n++)
				{
					itoa(n, szTmp2, 10);
					strcpy(szTmp1 + 5, szTmp2);
					GXMLAttribute* pAttrParam = pObjTag->GetAttribute(szTmp1);
					if(!pAttrParam)
						break;
				}

				// Import it
				ImportRealmObject(pClassAttr->GetValue(), pCompiler, pProject, n - 1);
			}
		}
	}

	// Built in stuff
	pCompiler->AddImportType(pProject->FindMachineClass("GameMachine"));
	ImportRealmObject("Avatar", pCompiler, pProject, 2);
	ImportRealmObject("Scenery", pCompiler, pProject, 1);
	ImportRealmObject("ChatCloud", pCompiler, pProject, 1);
	COClass* pClass = pProject->FindClass("Remote");
	if(pClass)
	{
		pCompiler->AddImportType(pClass);
		if(pMapTag)
		{
			GXMLAttribute* pRemoteAttr = pMapTag->GetAttribute("Remote");
			if(pRemoteAttr)
			{
				pClass = pProject->FindClass(pRemoteAttr->GetValue());
				ImportMethodDependency(pCompiler, pClass, "method &receiveFromClient(Object, Integer)");
				ImportMethodDependency(pCompiler, pClass, "method &receiveFromServer(Object)");
			}
		}
	}
	pClass = pProject->FindClass("Rect");
	if(pClass)
	{
		pCompiler->AddImportType(pClass);
		ImportMethodDependency(pCompiler, pClass, "method !new()");
	}
	COType* pType = pProject->FindType("Stream");
	if(pType)
	{
		pCompiler->AddImportType(pType);
	}
}

void MScriptEngine::FindMethod(struct MethodRef* pMethodRef, const char* szType, const char* szSig)
{
	// Find the allocate method
	EClass* pType = (EClass*)m_pLibrary->FindType(szType);
	if(!pType)
	{
		GlobalError eh;
		eh.SetError(&Error::TYPE_NOT_FOUND);
		eh.SetParam1(szType);
		m_pErrorHandler->OnError(&eh);
		GAssert(false, "Unable to find method");
		return;
	}
	EMethodSignature sig(szSig);
	if(!m_pLibrary->FindMethod(pMethodRef, pType, &sig))
	{
		GlobalError eh;
		eh.SetError(&Error::METHOD_NOT_FOUND);
		char* szFullSig = (char*)alloca(strlen(szType) + strlen(szSig) + 3);
		strcpy(szFullSig, szType);
		strcat(szFullSig, ".");
		strcat(szFullSig, szSig);
		eh.SetParam1(szFullSig);
		m_pErrorHandler->OnError(&eh);
		GAssert(false, "Unable to find method");
		return;
	}
	return;
}

int MScriptEngine::SerializeObject(GObject* pObject, unsigned char* pBuf, int nBufSize)
{
	GaspStream* pStream = (GaspStream*)m_pStreamVar->GetGObject();
	GQueue* pQ = &pStream->m_value;
	pQ->Flush();
	m_pVM->SerializeObject(pObject, m_pStreamVar->GetVariable());
	int nSize = pQ->GetSize();
	if(nSize > nBufSize)
		GameEngine::ThrowError("Object serializes too big for buffer");
	pQ->DumpToExistingBuffer((char*)pBuf);
	return nSize;
}

GObject* MScriptEngine::DeserializeObject(const unsigned char* pBuf, int nBufSize)
{
	GaspStream* pStream = (GaspStream*)m_pStreamVar->GetGObject();
	GQueue* pQ = &pStream->m_value;
	pQ->Flush();
	pQ->Push(pBuf, nBufSize);
	GObject* pOb = m_pVM->DeserializeObject(m_pStreamVar->GetVariable());
	return pOb;
}

GImage* MScriptEngine::CallGetFrame(ObjectObject* pObject, GRect* pRect, float fCameraDirection)
{
	m_pTempVar->SetGObject(pObject);
	m_pParams[0] = m_pTempVar;
	m_pImageVar->SetGObject(NULL);
	m_pParams[1] = m_pImageVar;
	m_pParams[2] = m_pRectVar;
	m_pFloatVar->GetVariable()->pFloatObject->m_value = fCameraDirection;
	m_pParams[3] = m_pFloatVar;
	if(!m_pVM->Call(&m_mrGetFrame, m_pParams, 4))
		GameEngine::ThrowError("Failed to call a Gasp method");
	ObjectObject* pOb = (ObjectObject*)m_pRectVar->GetGObject();
	pRect->x = ((IntObject*)pOb->arrFields[0])->m_value;
	pRect->y = ((IntObject*)pOb->arrFields[1])->m_value;
	pRect->w = ((IntObject*)pOb->arrFields[2])->m_value;
	pRect->h = ((IntObject*)pOb->arrFields[3])->m_value;
	GAssert(pRect->x >= 0 && pRect->y >= 0 && pRect->w >= 0 && pRect->h >= 0, "looks like a bad range");
	MGameImage* pImageObject = (MGameImage*)m_pParams[1]->GetGObject();
	return pImageObject ? pImageObject->GetImage() : NULL;
}

void MScriptEngine::CallUpdate(ObjectObject* pObject, double time)
{
	m_pTempVar->SetGObject(pObject);
	((GaspFloat*)m_pFloatVar->GetGObject())->m_value = time;
	m_pParams[0] = m_pTempVar;
	m_pParams[1] = m_pFloatVar;
	if(!m_pVM->Call(&m_mrUpdate, m_pParams, 2))
		GameEngine::ThrowError("Failed to call a Gasp method");
}

bool MScriptEngine::CallVerify(int nConnection, MObject* pOldObj, MObject* pNewObj)
{
	m_pTempVar->SetGObject(pOldObj->GetGObject());
	m_pIntVar->GetVariable()->pIntObject->m_value = 1;
	m_pIntVar2->GetVariable()->pIntObject->m_value = nConnection;
	m_pTempVar2->SetGObject(pNewObj->GetGObject());
	m_pParams[0] = m_pTempVar;
	m_pParams[1] = m_pIntVar;
	m_pParams[2] = m_pIntVar2;
	m_pParams[3] = m_pTempVar2;
	if(!m_pVM->Call(&m_mrVerify, m_pParams, 4))
		GameEngine::ThrowError("Failed to call a Gasp method");
	return m_pIntVar->GetVariable()->pIntObject->m_value ? true : false;
}

void MScriptEngine::CallDoAction(MObject* pObject, float x, float y)
{
	m_pTempVar->SetGObject(pObject->GetGObject());
	m_pFloatVar->GetVariable()->pFloatObject->m_value = x;
	m_pFloatVar2->GetVariable()->pFloatObject->m_value = y;
	m_pParams[0] = m_pTempVar;
	m_pParams[1] = m_pFloatVar;
	m_pParams[2] = m_pFloatVar2;
	if(!m_pVM->Call(&m_mrDoAction, m_pParams, 3))
		GameEngine::ThrowError("Failed to call a Gasp method");
}

void MScriptEngine::CallOnGetFocus(MObject* pObject)
{
	m_pTempVar->SetGObject(pObject->GetGObject());
	m_pParams[0] = m_pTempVar;
	if(!m_pVM->Call(&m_mrOnGetFocus, m_pParams, 1))
		GameEngine::ThrowError("Failed to call a Gasp method");
}

void MScriptEngine::CallOnLoseFocus(MObject* pObject)
{
	m_pTempVar->SetGObject(pObject->GetGObject());
	m_pParams[0] = m_pTempVar;
	if(!m_pVM->Call(&m_mrOnLoseFocus, m_pParams, 1))
		GameEngine::ThrowError("Failed to call a Gasp method");
}

void MScriptEngine::CallReceiveFromServer(GObject* pRemoteObj, GObject* pObj)
{
	m_pTempVar->SetGObject(pRemoteObj);
	m_pTempVar2->SetGObject(pObj);
	m_pParams[0] = m_pTempVar;
	m_pParams[1] = m_pTempVar2;
	if(!m_pVM->Call(&m_mrReceiveFromServer, m_pParams, 2))
		GameEngine::ThrowError("Failed to call a Gasp method");
}

void MScriptEngine::CallReceiveFromClient(GObject* pRemoteObj, GObject* pObj, int nConnection)
{
	m_pTempVar->SetGObject(pRemoteObj);
	m_pTempVar2->SetGObject(pObj);
	m_pIntVar->GetVariable()->pIntObject->m_value = nConnection;
	m_pParams[0] = m_pTempVar;
	m_pParams[1] = m_pTempVar2;
	m_pParams[2] = m_pIntVar;
	if(!m_pVM->Call(&m_mrReceiveFromClient, m_pParams, 3))
		GameEngine::ThrowError("Failed to call a Gasp method");
}

MObject* MScriptEngine::NewObject(const char* szClass, float x, float y, float z, float sx, float sy, float sz, const char** szParams, int nParamCount)
{
	// Generate the proper method signature
	GString sSig(L"method !new(");
	int n;
	for(n = 0; n < nParamCount; n++)
	{
		if(n > 0)
			sSig.Add(L", ");
		sSig.Add(L"String");
	}
	sSig.Add(L")");
	char* szSig = (char*)alloca(sSig.GetLength() + 1);
	sSig.GetAnsi(szSig);

	// Find the method
	struct MethodRef mr;
	FindMethod(&mr, szClass, szSig);

	// Create the parameters
	m_pParams[0] = m_pTempVar;
	if(nParamCount > MAX_PARAMS)
	{
		GAssert(false, "too many params");
		nParamCount = 0;
	}
	for(n = 0; n < nParamCount; n++)
	{
		((GaspString*)m_pStringVars[n]->GetGObject())->m_value.Copy(szParams[n]);
		m_pParams[1 + n] = m_pStringVars[n];
	}

	// Call the method
	if(!m_pVM->Call(&mr, m_pParams, 1 + nParamCount))
		GameEngine::ThrowError("Failed to create new object");

	// Wrap the object with an MObject
	MObject* pNewObject = new MObject(this);
	GObject* pObj = m_pTempVar->GetGObject();
	if(!pObj)
		GameEngine::ThrowError("Failed to allocate new object");
	pNewObject->SetGObject(pObj);
	pNewObject->SetGhostPos(x, y);

	// Set the position and size
	pNewObject->SetPos(x, y, z);
	if(sx != 0 && sy != 0 && sz != 0)
		pNewObject->SetSize(sx, sy, sz);

	return pNewObject;
}

void MScriptEngine::MakeRemoteObject(VarHolder* pVH, const char* szClassName)
{
	// Find the method
	struct MethodRef mr;
	FindMethod(&mr, szClassName, "method !new()");
	m_pParams[0] = pVH;
	if(!m_pVM->Call(&mr, m_pParams, 1))
		GameEngine::ThrowError("Failed to create the remote object");
}

VarHolder* MScriptEngine::CopyGlobalImage(const char* szGlobalID)
{
	Holder<VarHolder*> hVHLocal(new VarHolder(m_pVM));
	VarHolder* pVHLocal = hVHLocal.Get();
	GAssert(pVHLocal->m_szID = "local copy of global image", "");
	MGameImage* pMImage = new MGameImage(m_pVM, MGameImage::GlobalId, szGlobalID);
	pVHLocal->SetGObject(pMImage);
	MImageStore* pGlobalImageStore = GameEngine::GetGlobalImageStore();
	int nImageIndex = pGlobalImageStore->GetIndex(szGlobalID);
	VarHolder* pVHGlobal = pGlobalImageStore->GetVarHolder(nImageIndex);
	if(!pVHGlobal)
		GameEngine::ThrowError("There is no global image with the ID: %s", szGlobalID);
	MGameImage* pImageGlobal = (MGameImage*)pVHGlobal->GetGObject();
	pMImage->m_value.CopyImage(&pImageGlobal->m_value);
	return hVHLocal.Drop();
}

VarHolder* MScriptEngine::CopyGlobalAnimation(MImageStore* pLocalImageStore, const char* szGlobalID)
{
	// Get the image from the global animation
	MAnimationStore* pGlobalAnimationStore = GameEngine::GetGlobalAnimationStore();
	int nIndexGlobal = pGlobalAnimationStore->GetIndex(szGlobalID);
	if(nIndexGlobal < 0)
		GameEngine::ThrowError("There is no global animation with the ID: %s", szGlobalID);
	VarHolder* pVHAnimGlobal = pGlobalAnimationStore->GetVarHolder(nIndexGlobal);
	MAnimation* pAnimGlobal = (MAnimation*)pVHAnimGlobal->GetGObject();
	VarHolder* pVHImageGlobal = pAnimGlobal->GetImage();
	MGameImage* pImageGlobal = (MGameImage*)pVHImageGlobal->GetGObject();

	// Copy the image into the local store
	const char* szImageGlobalID = pImageGlobal->GetID();
	pLocalImageStore->AddImage(this, szImageGlobalID);
	int nImageIndex = pLocalImageStore->GetIndex(szImageGlobalID);
	VarHolder* pVHImageLocal = pLocalImageStore->GetVarHolder(nImageIndex);
	GAssert(pVHImageLocal, "Failed to copy image to local store");

	// Copy the global animation
	Holder<VarHolder*> hVHLocal(new VarHolder(m_pVM));
	VarHolder* pVHLocal = hVHLocal.Get();
	GAssert(pVHLocal->m_szID = "local copy of global animation", "");
	MAnimation* pAnimation = new MAnimation(m_pVM);
	pVHLocal->SetGObject(pAnimation);
	pAnimation->CopyDataAcrossEngines(pAnimGlobal, pVHImageLocal, szImageGlobalID);
	return hVHLocal.Drop();
}

VarHolder* MScriptEngine::LoadPNGImage(const char* szRemotePath, const char* szUrl, const char* szID)
{
	Holder<VarHolder*> hVH(new VarHolder(m_pVM));
	VarHolder* pVH = hVH.Get();
	GAssert(pVH->m_szID = "PNG Image", "");
	MGameImage* pMImage = new MGameImage(m_pVM, MGameImage::Store, szID);
	pVH->SetGObject(pMImage);
	GImage* pImage = pMImage->GetImage();
	int nSize;
	char* pFile;
	if(szRemotePath)
		pFile = m_pVM->m_pController->LoadFileFromUrl(szRemotePath, szUrl, &nSize);
	else
	{
		const char* szAppPath = GameEngine::GetAppPath();
		char* szPath = (char*)alloca(strlen(szAppPath) + strlen(szUrl) + 20);
		strcpy(szPath, szAppPath);
		strcat(szPath, "media/");
		strcat(szPath, szUrl);
		pFile = GFile::LoadFileToBuffer(szPath, &nSize);
	}
	if(!pFile)
		GameEngine::ThrowError("Failed to load image: %s", szUrl);
	Holder<char*> hFile(pFile);
	if(!pImage->LoadPNGFile((const unsigned char*)pFile, nSize))
		GameEngine::ThrowError("The image %s appears to be corrupt", szUrl);
	return hVH.Drop();
}

void MScriptEngine::MRectToGRect(ObjectObject* pMRect, GRect* pGRect)
{
	pGRect->x = ((IntObject*)pMRect->arrFields[0])->m_value;
	pGRect->y = ((IntObject*)pMRect->arrFields[1])->m_value;
	pGRect->w = ((IntObject*)pMRect->arrFields[2])->m_value;
	pGRect->h = ((IntObject*)pMRect->arrFields[3])->m_value;
}

void MScriptEngine::GRectToMRect(GRect* pGRect, ObjectObject* pMRect)
{
	((IntObject*)pMRect->arrFields[0])->m_value = pGRect->x;
	((IntObject*)pMRect->arrFields[1])->m_value = pGRect->y;
	((IntObject*)pMRect->arrFields[2])->m_value = pGRect->w;
	((IntObject*)pMRect->arrFields[3])->m_value = pGRect->h;
}

#define MAV_FIELD_DX MOB_FIELD_COUNT + 0
#define MAV_FIELD_DY MOB_FIELD_COUNT + 1
#define MAV_FIELD_SPEED MOB_FIELD_COUNT + 2
#define MAV_FIELD_STEPRATE MOB_FIELD_COUNT + 3
#define MAV_FIELD_REACH MOB_FIELD_COUNT + 4
#define MAV_FIELD_ACTIONTIME MOB_FIELD_COUNT + 5
#define MAV_FIELD_ANIMATION MOB_FIELD_COUNT + 6
#define MAV_FIELD_ANIMACTION MOB_FIELD_COUNT + 7

/*static*/ void MScriptEngine::DoAvatarActionAnimation(MObject* pAvatar)
{
	ObjectObject* pOb = pAvatar->GetGObject();
	((GaspFloat*)pOb->arrFields[MAV_FIELD_ACTIONTIME])->m_value = .3;
}

/*static*/ void MScriptEngine::SetAvatarVelocity(float dx, float dy, ObjectObject* pAvatar)
{
	// Set the velocity
	float speed = (float)((GaspFloat*)pAvatar->arrFields[MAV_FIELD_SPEED])->m_value;
	dx *= speed;
	dy *= speed;
	((GaspFloat*)pAvatar->arrFields[MAV_FIELD_DX])->m_value = dx;
	((GaspFloat*)pAvatar->arrFields[MAV_FIELD_DY])->m_value = dy;

	// Calculate the direction
	if(dx != 0 || dy != 0)
	{
		MAnimation* pAnimation = (MAnimation*)pAvatar->arrFields[MAV_FIELD_ANIMATION];
		MAnimation* pAnimAction = (MAnimation*)pAvatar->arrFields[MAV_FIELD_ANIMACTION];
		if(!pAnimation || !pAnimAction)
			GameEngine::ThrowError("The avatar's animation should not be null");
		float d = (float)atan2(dy, dx); // range = -PI to PI
		d += (float)1.9634954; // todo: unmagic 5 * PI / 8  (pi/2 because the animation starts facing forward + pi/8 to maximize margins between columns)
		pAnimation->SetDirection(d);
		pAnimAction->SetDirection(d);
	}
}

/*static*/ float MScriptEngine::GetAvatarReach(MObject* pAvatar)
{
	ObjectObject* pOb = pAvatar->GetGObject();
	return (float)((GaspFloat*)pOb->arrFields[MAV_FIELD_REACH])->m_value;
}

/*static*/ const char* MScriptEngine::GetChatCloudText(MObject* pChatCloud)
{
	ObjectObject* pOb = pChatCloud->GetGObject();
	MGameImage* pGameImage = (MGameImage*)pOb->arrFields[MOB_FIELD_COUNT];
	return pGameImage->GetID();
}

// -------------------------------------------------------------------------

MGameMachine::MGameMachine(Engine* pEngine)
: WrapperObject(pEngine, "GameMachine")
{
	
}

MGameMachine::~MGameMachine()
{
}

void MGameMachine::getAnimation(Engine* pEngine, EVar* pOutAnimation, EVar* pID)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(!pGameClient)
		return;
	MAnimationStore* pStore = pGameClient->GetAnimations();
	GString* pIDString = &pID->pStringObject->m_value;
	char* szID = (char*)alloca(pIDString->GetLength() + 1);
	pIDString->GetAnsi(szID);
	int idAnimation = pStore->GetIndex(szID);
	if(idAnimation < 0)
		GameEngine::ThrowError("There is no animation with the ID: %s", szID);
	pEngine->SetVar(pOutAnimation, pStore->GetVarHolder(idAnimation)->GetGObject());
}

void MGameMachine::getImage(Engine* pEngine, EVar* pOutImage, EVar* pID)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(!pGameClient)
		return;
	MImageStore* pStore = pGameClient->GetImages();
	GString* pIDString = &pID->pStringObject->m_value;
	char* szID = (char*)alloca(pIDString->GetLength() + 1);
	pIDString->GetAnsi(szID);
	int nImageIndex = pStore->GetIndex(szID);
	VarHolder* pVH = pStore->GetVarHolder(nImageIndex);
	if(!pVH)
		pEngine->ThrowEngineError(L"No such image"); // todo: throw custom error
	pEngine->SetVar(pOutImage, pVH ? pVH->GetGObject() : NULL);
}

void MGameMachine::getAvatar(Engine* pEngine, EVar* pOutAvatar)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(!pGameClient)
		return;
	MObject* pAvatar = pGameClient->GetAvatar();
	if(!pAvatar)
		pEngine->SetVar(pOutAvatar, NULL);
	else
	{
		GObject* pOb = pAvatar->GetGObject();
		if(!pOb)
			pEngine->ThrowEngineError(L"No such object"); // todo: throw a custom exception
		pEngine->SetVar(pOutAvatar, pOb);
	}
}

void MGameMachine::getObjectById(Engine* pEngine, EVar* pOutObject, EVar* pID)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(!pGameClient)
		return;
	int nID = pID->pIntObject->m_value;
	MObject* pObj = pGameClient->GetCurrentRealm()->GetObjectByID(nID);
	if(!pObj)
		GameEngine::ThrowError("There's no object with the ID %d", nID);
	GObject* pOb = pObj->GetGObject();
	GAssert(pOb, "invalid realm object");
	pEngine->SetVar(pOutObject, pOb);
}

void MGameMachine::getClosestObject(Engine* pEngine, EVar* pOutObject, EVar* pX, EVar* pY)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(!pGameClient)
		return;
	float x = (float)pX->pFloatObject->m_value;
	float y = (float)pY->pFloatObject->m_value;
	MObject* pOb = pGameClient->GetCurrentRealm()->GetClosestObject(x, y);
	pEngine->SetVar(pOutObject, pOb ? pOb->GetGObject() : NULL);
}

void MGameMachine::notifyServerAboutObjectUpdate(Engine* pEngine, EVar* pObj)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(!pGameClient)
		return;
	MRealm* pRealm = pGameClient->GetCurrentRealm();
	int uid = MObject::GetUid(pObj->pObjectObject);
	MObject* pOb = pRealm->GetObjectByID(uid);
	if(!pOb)
		return;
    pGameClient->NotifyServerAboutObjectUpdate(pOb);
}

const char* GetExtension(const char* szUrl)
{
	int n;
	for(n = strlen(szUrl) - 1; n > 0; n--)
	{
		if(szUrl[n] == '.')
			return szUrl + n;
	}
	return szUrl;
}

void MGameMachine::followLink(Engine* pEngine, EVar* pURL)
{
	GString* pUrlString = &pURL->pStringObject->m_value;
	char* szUrl = (char*)alloca(pUrlString->GetLength() + 1);
	pUrlString->GetAnsi(szUrl);
	const char* szExt = GetExtension(szUrl);
	if(strnicmp(szExt, ".realm", 6) == 0) // todo: unhard-code the extension
		((MVM*)pEngine)->m_pController->FollowLink(szUrl);
	else
	{
		// Let the OS open with the default program
#ifdef WIN32
		SHELLEXECUTEINFO i;
		memset(&i, '\0', sizeof(SHELLEXECUTEINFO));
		i.cbSize = sizeof(SHELLEXECUTEINFO);
		i.nShow = SW_SHOW;
		i.lpFile = szUrl;
		ShellExecuteEx(&i);
#else
		GAssert(false, "not implemented for Linux yet");
#endif // !WIN32
	}
}

void MGameMachine::getUid(Engine* pEngine, EVar* pOutID)
{
	pOutID->pIntObject->m_value = GameEngine::GetUid();
}

void MGameMachine::getTime(Engine* pEngine, EVar* pOutTime)
{
	pOutTime->pFloatObject->m_value = GameEngine::GetTime();
}

void MGameMachine::addObject(Engine* pEngine, EVar* pObject)
{
	if(!pObject->pOb)
		GameEngine::ThrowError("The object is null");
	MScriptEngine* pScriptEngine = ((MVM*)pEngine)->m_pScriptEngine;
	MObject* pNewMObject = new MObject(pScriptEngine);
	pNewMObject->SetGObject(pObject->pOb);

	// todo: this is a hack.  Think about it
	if(pNewMObject->GetGhostPos()->z > 100)
		pNewMObject->SetTangible(false);

	MRealm* pRealm = pScriptEngine->GetRealm();
	pRealm->ReplaceObject(0, pNewMObject);
}

void MGameMachine::removeObject(Engine* pEngine, EVar* pID)
{
	MRealm* pRealm = ((MVM*)pEngine)->m_pGameClient->GetCurrentRealm();
	pRealm->RemoveObject(0, pID->pIntObject->m_value);
}

void MGameMachine::playSound(Engine* pEngine, EVar* pID)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(!pGameClient)
		return;
	MSoundStore* pSounds = pGameClient->GetSounds();
	GString* pIDString = &pID->pStringObject->m_value;
	char* szID = (char*)alloca(pIDString->GetLength() + 1);
	pIDString->GetAnsi(szID);
	int index = pSounds->GetIndex(szID);
	if(index < 0)
		GameEngine::ThrowError("There was no sound with the ID \"%s\" in the store", szID);
	MSound* pSound = pSounds->GetSound(index);
	VAudioPlayer* pPlayer = pGameClient->GetWavePlayer();
	pPlayer->Play(pSound);
}

void MGameMachine::sendToClient(Engine* pEngine, EVar* pObj, EVar* pConnection)
{
	((MVM*)pEngine)->m_pController->SendToClient(pObj->pOb, pConnection->pIntObject->m_value);
}

void MGameMachine::sendToServer(Engine* pEngine, EVar* pObj)
{
	((MVM*)pEngine)->m_pController->SendToServer(pObj->pOb);
}

void MGameMachine::addInventoryItem(Engine* pEngine, EVar* pUrl)
{
	// Get the client model
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	Controller* pController = ((MVM*)pEngine)->m_pController;
	if(!pGameClient)
	{
		GAssert(false, "Why is the server trying to add an inventory item?");
		return;
	}

	// Download the file
	GString* pUrlString = &pUrl->pStringObject->m_value;
	int nLen = pUrlString->GetLength();
	Holder<char*> hAnsi(new char[nLen + 1]);
	char* szAnsi = hAnsi.Get();
	pUrlString->GetAnsi(szAnsi);
	int nFileSize;
	Holder<char*> hFile(pController->LoadFileFromUrl(pGameClient->GetRemoteFolder(), szAnsi, &nFileSize));
	char* szFile = hFile.Get();

	// Parse the XML
	const char* szErrorMessage;
	int nErrorLine;
	Holder<GXMLTag*> hTag(GXMLTag::FromString(szFile, nFileSize, &szErrorMessage, NULL, &nErrorLine, NULL));
	GXMLTag* pTag = hTag.Get();
	if(!pTag)
		GameEngine::ThrowError("Error parsing item XML string at line %d: %s", nErrorLine, szErrorMessage);

	// Add it to the inventory
	pGameClient->AddInventoryItem(hTag.Drop());
}

void MGameMachine::checkForSolidObject(Engine* pEngine, EVar* pBool, EVar* pX, EVar* pY)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(!pGameClient)
	{
		GAssert(false, "Sorry, not implemented for the server yet");
		return;
	}
	MRealm* pRealm = pGameClient->GetCurrentRealm();
	pBool->pIntObject->m_value = pRealm->CheckForSolidObject((float)pX->pFloatObject->m_value, (float)pY->pFloatObject->m_value);
}

void MGameMachine::setAccountVar(Engine* pEngine, EVar* pName, EVar* pValue)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	GString* pNameString = &pName->pStringObject->m_value;
	char* szName = (char*)alloca(pNameString->GetLength() + 1);
	pNameString->GetAnsi(szName);
	GString* pValueString = &pValue->pStringObject->m_value;
	char* szValue = (char*)alloca(pValueString->GetLength() + 1);
	pValueString->GetAnsi(szValue);
	pGameClient->SetAccountVar(szName, szValue);
}

void MGameMachine::getAccountVar(Engine* pEngine, EVar* pValue, EVar* pName)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	GString* pNameString = &pName->pStringObject->m_value;
	char* szName = (char*)alloca(pNameString->GetLength() + 1);
	pNameString->GetAnsi(szName);
	const char* szValue = pGameClient->GetAccountVar(szName);
	GString* pValueString = &pValue->pStringObject->m_value;
	pValueString->Copy(szValue);
}

void MGameMachine::reportStats(Engine* pEngine, EVar* pNameValuePairs)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	MArray* pArray = (MArray*)pNameValuePairs->pWrapperObject;
	pGameClient->ReportStats(pArray->m_pArray);
}

void MGameMachine::setSkyImage(Engine* pEngine, EVar* pID)
{
	Controller* pController = ((MVM*)pEngine)->m_pController;
	pController->SetSkyImage(&pID->pStringObject->m_value);
}

void MGameMachine::setGroundImage(Engine* pEngine, EVar* pID)
{
	Controller* pController = ((MVM*)pEngine)->m_pController;
	pController->SetGroundImage(&pID->pStringObject->m_value);
}

void MGameMachine::setPivotHeight(Engine* pEngine, EVar* pID, EVar* pHeight)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	int nID = pID->pIntObject->m_value;
	MObject* pObj = pGameClient->GetCurrentRealm()->GetObjectByID(nID);
	if(!pObj)
		GameEngine::ThrowError("There's no object with the ID %d", nID);
	pObj->SetPivotHeight(pHeight->pIntObject->m_value);
}

void MGameMachine::addMerit(Engine* pEngine, EVar* pSkill, EVar* pCorrect, EVar* pAbilityLevel, EVar* pAmount)
{
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	const wchar_t* wszSkill = pSkill->pStringObject->m_value.GetString();
	bool bCorrect = pCorrect->pIntObject->m_value ? true : false;
	double dAbilityLevel = pAbilityLevel->pFloatObject->m_value;
	double dAmount = pAmount->pFloatObject->m_value;
	pGameClient->AddMerit(wszSkill, bCorrect, dAbilityLevel, dAmount);
}
