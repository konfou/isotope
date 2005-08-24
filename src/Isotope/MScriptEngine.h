/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MSCRIPTENGINE_H__
#define __MSCRIPTENGINE_H__

// --------------------------------
//
//  The classes in this file constitue the glue between C++ and the Gash scripting language
//
// --------------------------------


#include "../GClasses/GImage.h"
#include "../Gash/Include/GashSdl.h"

class GVM;
class Library;
class CallBackGetter;
class ErrorHandler;
class VarHolder;
class GImage;
class MObject;
class MGameClient;
class Controller;
class MImageStore;
class MScriptEngine;
class COClass;


// This class just provids a callback function to use when an error occurs.
class IsotopeErrorHandler : public ErrorHandler
{
public:
	virtual void OnError(ErrorHolder* pErrorHolder);
};




extern GConstStringHashTable* g_pIsotopeMachineObjects;

// This method stuffs a hash table with information about which C++ methods are
// available to be called from script code.
void RegisterIsotopeMachineClasses();


// The purpose of this class is to link calls from a Gash script to a C++ method.
class IsotopeCallBackGetter : public GashSdlCallBackGetter
{
public:
	IsotopeCallBackGetter() : GashSdlCallBackGetter() {}
	virtual ~IsotopeCallBackGetter() {}

	// When the Gash linker wants to link to a C++ method, it will pass the
	// class name and method signature as parameters to GetCallBack, which should
	// return a pointer to the C++ method with that class name and method signature.
	virtual EMethodPointerHolder* GetCallBack(const char* szClassName, EMethodSignature* pMethodSignature)
	{
		if(!g_pIsotopeMachineObjects)
			RegisterIsotopeMachineClasses();
		EMethodPointerHolder* pMeth = WrapperObject::FindMachineMethod(g_pIsotopeMachineObjects, szClassName, pMethodSignature);
		if(pMeth)
			return pMeth;
		return GashSdlCallBackGetter::GetCallBack(szClassName, pMethodSignature);
	}
};



// This class wraps a Gash Virtual Machine.  The only purpose of this class is to contain a
// pointer to the MGameClient, so that when you call from a Gash script into a C++ method,
// you can get a pointer to the MGameClient.
class MVM : public GVM
{
public:
	MGameClient* m_pGameClient;
	Controller* m_pController;
	MScriptEngine* m_pScriptEngine;

	MVM(Library* pLibrary, CallBackGetter* pCBG, ErrorHandler* pErrorHandler, const char* szChrootJail, MGameClient* pGameClient, Controller* pController, MScriptEngine* pScriptEngine)
		: GVM(pLibrary, pCBG, pErrorHandler, szChrootJail)
	{
		m_pGameClient = pGameClient;
		m_pController = pController;
		m_pScriptEngine = pScriptEngine;
	}

	virtual ~MVM()
	{
	}
};



#define MAX_PARAMS 8

// This class wraps the virtual machine that runs Gash scripts, and contains methods that
// let you call from C++ into Gash functions.
class MScriptEngine
{
protected:
	MVM* m_pVM;
	CallBackGetter* m_pCBG;
	ErrorHandler* m_pErrorHandler;
	Library* m_pLibrary;

	struct MethodRef m_mrUpdate;
	struct MethodRef m_mrVerify;
	struct MethodRef m_mrGetFrame;
	struct MethodRef m_mrDoAction;
	struct MethodRef m_mrOnGetFocus;
	struct MethodRef m_mrOnLoseFocus;
	VarHolder* m_pTempVar;
	VarHolder* m_pTempVar2;
	VarHolder* m_pIntVar;
	VarHolder* m_pIntVar2;
	VarHolder* m_pRectVar;
	VarHolder* m_pImageVar;
	VarHolder* m_pFloatVar;
	VarHolder* m_pFloatVar2;
	VarHolder* m_pStreamVar;
	VarHolder* m_pStringVars[MAX_PARAMS];
	VarHolder* m_pParams[MAX_PARAMS + 1];

public:
	MScriptEngine(const char* szScript, int nScriptSize, ErrorHandler* pErrorHandler, GXMLTag* pObjectsTag, MGameClient* pGameClient, Controller* pController);
	virtual ~MScriptEngine();

	// Copies the values from a GRect structure into a Gash Rect object
	static void GRectToMRect(GRect* pGRect, ObjectObject* pMRect);

	// Copies the values from a Gash Rect object into a GRect structure
	static void MRectToGRect(ObjectObject* pMRect, GRect* pGRect);

	// Call Gash code to set the avatar's velocity
	static void SetAvatarVelocity(float dx, float dy, ObjectObject* pAvatar);

	// Returns how far the avatar can reach
	static float GetAvatarReach(MObject* pAvatar);

	GVM* GetEngine() { return m_pVM; }

	// Calls "getFrame" on the specified object and returns the results in C++ objects
	GImage* CallGetFrame(ObjectObject* pObject, GRect* pRect, float fCameraDirection);

	// Calls "update" on the specified object
	void CallUpdate(ObjectObject* pObject, double time);

	// Calls "verify" on the specified object
	bool CallVerify(int nConnection, MObject* pOldObj, MObject* pNewObj);

	// Calls "doAction" on the specified object
	void CallDoAction(MObject* pObject, float x, float y);

	// Calls "getFocus" on the specified object
	void CallOnGetFocus(MObject* pObject);

	// Calls "loseFocus" on the specified object
	void CallOnLoseFocus(MObject* pObject);

	// Serializes the specified MObject
	int SerializeObject(MObject* pObject, unsigned char* pBuf, int nBufSize);

	// Deserializes a blob into an MObject
	MObject* DeserializeObject(const unsigned char* pBuf, int nBufSize);

	// Allocates a new MObject (including the Gash object that it wraps)
	MObject* NewObject(const char* szClass, float x, float y, float z, float sx, float sy, float sz, const char** szParams, int nParamCount);

	// loads a bitmap image into a Gash Image object
	VarHolder* LoadPNGImage(const char* szRemotePath, const char* szUrl, const char* szID);

	// Copies an image from the global image store and returns a VarHolder
	VarHolder* CopyGlobalImage(const char* szGlobalID);

	// Copies an animation from the global image store and returns a VarHolder
	VarHolder* CopyGlobalAnimation(MImageStore* pLocalImageStore, const char* szGlobalID);

	static void DoAvatarActionAnimation(MObject* pAvatar);

protected:
	void FindMethod(struct MethodRef* pMethodRef, const char* szType, const char* szSig);
	void ImportObjectDependencies(GCompiler* pCompiler, COProject* pProject, GXMLTag* pObjectsTag);
	void ImportMethodDependency(GCompiler* pCompiler, COClass* pClass, const char* szSig);
};


// This class contains C++ methods that you can call from within Gash scripts
class MGameMachine : public WrapperObject
{
public:
	MGameMachine(Engine* pEngine);
	virtual ~MGameMachine();

	// Finds an animation in the animation store with the specified ID and returns a reference
	// to it.  (Note that it returns the master copy, so you should probably copy it before
	// using it, or else all copies will be affected when you call advanceTime or make any other
	// changes.)
	void getAnimation(Engine* pEngine, EVar* pOutAnimation, EVar* pID);

	// Finds an image in the image store with the specified ID and returns a reference to it
	void getImage(Engine* pEngine, EVar* pOutImage, EVar* pID);

	// Follows a link to another realm
	void followLink(Engine* pEngine, EVar* pURL);

	// returns the avatar object
	void getAvatar(Engine* pEngine, EVar* pOutAvatar);

	// returns the object with the specified ID
	void getObjectById(Engine* pEngine, EVar* pOutObject, EVar* pID);

	// returns the object closest to the specified coordinates
	void getClosestObject(Engine* pEngine, EVar* pOutObject, EVar* pX, EVar* pY);

	// Gets a unique number to identify an object
	void getUid(Engine* pEngine, EVar* pOutID);

	// Gets the number of seconds since midnight (accurate to the milisecond)
	void getTime(Engine* pEngine, EVar* pOutTime);

	// Adds a new object to the realm.  This method makes it possible for your scripts
	// to create new objects and add them to map dynamically
	void addObject(Engine* pEngine, EVar* pObject);

	// Removes an object from the realm.  (The garbage collector will delete it when
	// nothing else references it.)
	void removeObject(Engine* pEngine, EVar* pID);

	// Plays a wave sound
	void playSound(Engine* pEngine, EVar* pID);

	// Call this method after you make changes to an object so the changes can be
	// sent to the server and it can propagate them to all other clients
	void notifyServerAboutObjectUpdate(Engine* pEngine, EVar* pObj);
};


#endif // __MSCRIPTENGINE_H__
