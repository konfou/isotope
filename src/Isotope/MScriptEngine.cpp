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
#include "../Gash/BuiltIns/GashStream.h"
#include "../Gash/BuiltIns/GashFloat.h"
#include "../Gash/Engine/Error.h"
#include "../Gash/CodeObjects/Project.h"
#include "../Gash/Engine/GCompiler.h"
#include "GameEngine.h"
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
	const char* szErrorMessage = GameEngine::SetErrorMessage(pBuf);
	GameEngine::ThrowError(szErrorMessage);
}

// -------------------------------------------------------------------------------

GConstStringHashTable* g_pIsotopeMachineObjects = NULL;


void RegisterMAnimation(GConstStringHashTable* pTable)
{
	pTable->Add("method getFrame(!GImage, &Rect)", new EMethodPointerHolder((MachineMethod2)&MAnimation::getFrame));
	pTable->Add("method getColumnFrame(!GImage, &Rect, Float)", new EMethodPointerHolder((MachineMethod3)&MAnimation::getColumnFrame));
	pTable->Add("method !newCopy(Animation)", new EMethodPointerHolder((MachineMethod1)&MAnimation::newCopy));
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&MAnimation::toStream));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&MAnimation::fromStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&MAnimation::setRefs));
	pTable->Add("method &advanceTime(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&MAnimation::advanceTime));
}

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
}

// Declared in MGameImage.cpp
void RegisterMGameImage(GConstStringHashTable* pTable);

void RegisterIsotopeMachineClasses()
{
	WrapperObject::RegisterMachineClass(&g_pIsotopeMachineObjects, "GameMachine", RegisterMGameMachine);
	WrapperObject::RegisterMachineClass(&g_pIsotopeMachineObjects, "Animation", RegisterMAnimation);
	WrapperObject::RegisterMachineClass(&g_pIsotopeMachineObjects, "GImage", RegisterMGameImage);
}

// -------------------------------------------------------------------------------


MScriptEngine::MScriptEngine(const char* szScript, int nScriptSize, ErrorHandler* pErrorHandler, MGameClient* pGameClient, Controller* pController)
{
	// Parse the script into a project
	COProject proj("*bogus*");
	char szGashLibPath[512];
	strcpy(szGashLibPath, GameEngine::GetAppPath());
	strcat(szGashLibPath, "../src/Gash/xlib");
	if(!proj.LoadLibraries(szGashLibPath, pErrorHandler))
	{
		GAssert(false, "The error handler should have thrown");
		GameEngine::ThrowError("Failed to load Gash libraries from: %s", szGashLibPath);
	}
	const char* szFilename = "*The Script*";
	if(!proj.LoadSources(&szScript, &szFilename, 1, pErrorHandler))
	{
		GAssert(false, "The error handler should have thrown");
		GameEngine::ThrowError("Failed to parse the script file");
	}

	// Build the project into a library
	CompileError errorHolder;
	m_pLibrary = GCompiler::Compile(&proj, &errorHolder);
	if(!m_pLibrary)
	{
		pErrorHandler->OnError(&errorHolder);
		GAssert(false, "The error handler should have thrown");
		GameEngine::ThrowError("Failed to compile the script");
	}
/*
// todo: remove this code:	
if(pGameClient)
m_pLibrary->GetLibraryTag()->ToFile("client.xlib");
else
m_pLibrary->GetLibraryTag()->ToFile("server.xlib");
*/

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
		GameEngine::ThrowError("Failed to allocate a Gash object");

	// Preallocate some machine objects
	m_pFloatVar->SetGObject(new GashFloat(m_pVM));
	m_pFloatVar2->SetGObject(new GashFloat(m_pVM));
	m_pStreamVar->SetGObject(new GashStream(m_pVM));
	for(n = 0; n < MAX_PARAMS; n++)
		m_pStringVars[n]->SetGObject(new GashString(m_pVM));
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

int MScriptEngine::SerializeObject(MObject* pObject, unsigned char* pBuf, int nBufSize)
{
	GashStream* pStream = (GashStream*)m_pStreamVar->GetGObject();
	GQueue* pQ = &pStream->m_value;
	pQ->Flush();
	m_pVM->SerializeObject(pObject->GetGObject(), m_pStreamVar->GetVariable());
	int nSize = pQ->GetSize();
	if(nSize > nBufSize)
		GameEngine::ThrowError("Object serializes too big for buffer");
	pQ->DumpToExistingBuffer((char*)pBuf);
	return nSize;
}

MObject* MScriptEngine::DeserializeObject(const unsigned char* pBuf, int nBufSize)
{
	GashStream* pStream = (GashStream*)m_pStreamVar->GetGObject();
	GQueue* pQ = &pStream->m_value;
	pQ->Flush();
	pQ->Push(pBuf, nBufSize);
	GObject* pOb = m_pVM->DeserializeObject(m_pStreamVar->GetVariable());
	MObject* pObject = new MObject(this);
	pObject->SetGObject(pOb);
	return pObject;
}

GImage* MScriptEngine::CallGetFrame(ObjectObject* pObject, GRect* pRect, float fCameraDirection)
{
	m_pTempVar->SetGObject(pObject);
	m_pParams[0] = m_pTempVar;
	m_pParams[1] = m_pImageVar;
	m_pParams[2] = m_pRectVar;
	m_pFloatVar->GetVariable()->pFloatObject->m_value = fCameraDirection;
	m_pParams[3] = m_pFloatVar;
	if(!m_pVM->Call(&m_mrGetFrame, m_pParams, 4))
		GameEngine::ThrowError("Failed to call a Gash method");
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
	((GashFloat*)m_pFloatVar->GetGObject())->m_value = time;
	m_pParams[0] = m_pTempVar;
	m_pParams[1] = m_pFloatVar;
	if(!m_pVM->Call(&m_mrUpdate, m_pParams, 2))
		GameEngine::ThrowError("Failed to call a Gash method");
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
		GameEngine::ThrowError("Failed to call a Gash method");
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
		GameEngine::ThrowError("Failed to call a Gash method");
}

void MScriptEngine::CallOnGetFocus(MObject* pObject)
{
	m_pTempVar->SetGObject(pObject->GetGObject());
	m_pParams[0] = m_pTempVar;
	if(!m_pVM->Call(&m_mrOnGetFocus, m_pParams, 1))
		GameEngine::ThrowError("Failed to call a Gash method");
}

void MScriptEngine::CallOnLoseFocus(MObject* pObject)
{
	m_pTempVar->SetGObject(pObject->GetGObject());
	m_pParams[0] = m_pTempVar;
	if(!m_pVM->Call(&m_mrOnLoseFocus, m_pParams, 1))
		GameEngine::ThrowError("Failed to call a Gash method");
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
		((GashString*)m_pStringVars[n]->GetGObject())->m_value.Copy(szParams[n]);
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

VarHolder* MScriptEngine::CopyGlobalImage(const char* szGlobalID)
{
	Holder<VarHolder*> hVHLocal(new VarHolder(m_pVM));
	VarHolder* pVHLocal = hVHLocal.Get();
	GAssert(pVHLocal->m_szID = "local copy of global image", "");
	MGameImage* pMImage = new MGameImage(m_pVM, MGameImage::GlobalId, szGlobalID);
	pVHLocal->SetGObject(pMImage);
	MImageStore* pGlobalImageStore = GameEngine::GetGlobalImageStore();
	VarHolder* pVHGlobal = pGlobalImageStore->GetVarHolder(szGlobalID);
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
	VarHolder* pVHImageLocal = pLocalImageStore->GetVarHolder(szImageGlobalID);
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
		pFile = GameEngine::LoadFileFromUrl(szRemotePath, szUrl, &nSize);
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
	((GashFloat*)pOb->arrFields[MAV_FIELD_ACTIONTIME])->m_value = .3;
}

/*static*/ void MScriptEngine::SetAvatarVelocity(float dx, float dy, ObjectObject* pAvatar)
{
	// Set the velocity
	float speed = (float)((GashFloat*)pAvatar->arrFields[MAV_FIELD_SPEED])->m_value;
	dx *= speed;
	dy *= speed;
	((GashFloat*)pAvatar->arrFields[MAV_FIELD_DX])->m_value = dx;
	((GashFloat*)pAvatar->arrFields[MAV_FIELD_DY])->m_value = dy;

	// Calculate the direction
	if(dx != 0 || dy != 0)
	{
		MAnimation* pAnimation = (MAnimation*)pAvatar->arrFields[MAV_FIELD_ANIMATION];
		MAnimation* pAnimAction = (MAnimation*)pAvatar->arrFields[MAV_FIELD_ANIMACTION];
		if(!pAnimation || !pAnimAction)
			GameEngine::ThrowError("The avatar's animation should not be null");
		float d = (float)atan2(dy, dx); // range = -PI to PI
		d += (float)1.570795; // todo: unmagic PI/2
		pAnimation->SetDirection(d);
		pAnimAction->SetDirection(d);
	}
}

/*static*/ float MScriptEngine::GetAvatarReach(MObject* pAvatar)
{
	ObjectObject* pOb = pAvatar->GetGObject();
	return (float)((GashFloat*)pOb->arrFields[MAV_FIELD_REACH])->m_value;
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
	VarHolder* pVH = pStore->GetVarHolder(szID);
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
	GObject* pOb = pObj->GetGObject();
	if(!pOb)
		pEngine->ThrowEngineError(L"No such object"); // todo: throw a custom exception
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
	GAssert(((MVM*)pEngine)->m_pController, "The server should never call this method");
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
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	MObject* pNewMObject = new MObject(pGameClient->GetScriptEngine());
	if(!pObject->pOb)
		GameEngine::ThrowError("The object is null");
	pNewMObject->SetGObject(pObject->pOb);

	// todo: this is a hack.  Think about it
	if(pNewMObject->GetGhostPos()->z > 100)
		pNewMObject->SetTangible(false);

	MRealm* pRealm = pGameClient->GetCurrentRealm();
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
	VWavePlayer* pPlayer = pGameClient->GetWavePlayer();
	pPlayer->Play(pSound);
}
