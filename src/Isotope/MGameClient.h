/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MGAMECLIENT_H__
#define __MGAMECLIENT_H__

#include "Model.h"

class VGame;
class View;
class Controller;
class MScriptEngine;
class IsotopeErrorHandler;
class VWavePlayer;
class GBillboardCamera;
class MImageStore;
class MAnimationStore;
class MSoundStore;
class MSpotStore;
class GPointerArray;

// This class represents the model (all the internal data including maps, players, objects, scripts, etc)
// when the application is running as the client (as opposed to the server, which uses a different model).
class MGameClient : public Model
{
protected:
	MImageStore* m_pImageStore;
	MAnimationStore* m_pAnimationStore;
	MSoundStore* m_pSoundStore;
	MSpotStore* m_pSpotStore;

	char* m_szRemoteFolder;
	GXMLTag* m_pMap;
	char* m_szScript;
	GBillboardCamera* m_pCamera;
	VWavePlayer* m_pPlayer;
	MObject* m_pClosestObject;
	MRealm* m_pCurrentRealm;
	NRealmClientConnection* m_pConnection;
	double m_nextUpdateTime;
//#ifdef _DEBUG // todo: uncomment this line
	double m_dFrameRateTime;
	int m_nFrames;
//#endif // _DEBUG
	IsotopeErrorHandler* m_pErrorHandler;
	MScriptEngine* m_pScriptEngine;
	MObject* m_pAvatar;
	MObject* m_pGoalFlag;
	MObject* m_pInfoCloud;
	bool m_bLoner;
	bool m_bFirstPerson;
	GPointerArray* m_pSelectedObjects;
	char* m_szAccountFilename;
	GXMLTag* m_pAccountTag;
	GXMLTag* m_pAccountRefTag;

public:
	MGameClient(const char* szAccountFilename, GXMLTag* pAccountTag, GXMLTag* pAccountRefTag, bool loner);
	virtual ~MGameClient();

	MImageStore* GetImages() { return m_pImageStore; }
	MAnimationStore* GetAnimations() { return m_pAnimationStore; }
	MSoundStore* GetSounds() { return m_pSoundStore; }
	MSpotStore* GetSpots() { return m_pSpotStore; }

	// Loads the script that contains logic for the objects
	void LoadScript(Controller* pController, const char* szUrl);

	// Load the XML realm file
	void LoadRealm(Controller* pController, const char* szUrl, double time, int nScreenVerticalCenter);

	const char* GetRemoteFolder() { return m_szRemoteFolder; }

	// Tell the model to update itself (including synchronizing with the server).  You should
	// call this method in the main loop
	virtual void Update(double time);

	// This is called when an object in the realm is replaced.  It's primary purpose is
	// to give you a chance to update any pointers that you may be holding to old objects
	virtual bool OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew);

	// Always returns false because only the client will instantiate this class
	virtual ModelType GetType() { return Client; }

	// Returns the current realm
	MRealm* GetCurrentRealm() { return m_pCurrentRealm; }

	// Tells the server that we changed an object
	void NotifyServerAboutObjectUpdate(MObject* pOb);

	// Moves the avatar's goal marker
	void MoveGoalFlag(float x, float y, bool bVisible);

	// Returns a pointer to the scripting engine currently being used
	MScriptEngine* GetScriptEngine() { return m_pScriptEngine; }

	// Returns a pointer to the avatar object
	MObject* GetAvatar() { return m_pAvatar; }

	// Loads all the images, animations, sound effects, and spots
	void LoadMedia(GXMLTag* pModelTag);

	// Unloads the images, animations, sound effects, and spots
	void UnloadMedia();

	// Returns the sound effect player
	VWavePlayer* GetWavePlayer() { return m_pPlayer; }

	// Returns the original source to the map file
	GXMLTag* GetMap() { return m_pMap; }

	// Returns the original source to the script file
	const char* GetScript() { return m_szScript; }

	void MakeChatCloud(const char* szText);
	void AddObject(const char* szFilename);
	bool IsFirstPerson() { return m_bFirstPerson; }
	GBillboardCamera* GetCamera() { return m_pCamera; }
	void SelectObjects(float xMin, float yMin, float xMax, float yMax);
	void DoActionOnSelectedObjects(float x, float y);
	MObject* GetGoalFlag() { return m_pGoalFlag; }
	void SaveState();

protected:
	void SynchronizeWithServer(double time);
	void ProcessPacket(NRealmPacket* pPacket);
	void UpdateObject(NUpdateObjectPacket* pPacket);
	void LoadObjects(MRealm* pRealm, GXMLTag* pModelTag);

	// Unloads the current realm
	void UnloadRealm();

	// Makes an intangible scenery object from an image in the global cache
	MObject* MakeIntangibleGlobalObject(const char* szID, float x, float y, float z);
};

#endif // __MGAMECLIENT_H__
