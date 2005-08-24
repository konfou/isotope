/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "../GClasses/GArray.h"
#include "GameEngine.h"
#include "MGameClient.h"
#include "NRealmProtocol.h"
#include "MRealm.h"
#include "VWave.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GHttp.h"
#include "../GClasses/GBillboardCamera.h"
#include "../GClasses/GWindows.h"
#include "../GClasses/GSocket.h"
#include "../GClasses/GFile.h"
#include "Controller.h"
#include "MScriptEngine.h"
#include "MStore.h"
#include "MObject.h"
#include "MGameImage.h"
#include "MSpot.h"
#ifdef WIN32
#include <direct.h>
#else // WIN32
#include <unistd.h>
#endif // WIN32

MGameClient::MGameClient(const char* szAccountFilename, GXMLTag* pAccountTag, GXMLTag* pAccountRefTag)
: Model()
{
	m_szAccountFilename = new char[strlen(szAccountFilename) + 1];
	strcpy(m_szAccountFilename, szAccountFilename);
	m_pAccountTag = pAccountTag;
	m_pAccountRefTag = pAccountRefTag;
	m_pCamera = NULL;
	m_pConnection = NULL;
	m_pCurrentRealm = NULL;
	m_pCurrentRealm = NULL;
	m_nextUpdateTime = 0;
	m_pClosestObject = NULL;
//#ifdef _DEBUG
	m_dFrameRateTime = 0;
	m_nFrames = 0;
//#endif // _DEBUG
	m_pErrorHandler = new IsotopeErrorHandler();
	m_pScriptEngine = NULL;
	m_pGoalFlag = NULL;
	m_pInfoCloud = NULL;
	m_pPlayer = new VWavePlayer();
	m_pMap = NULL;
	m_szScript = NULL;
	m_szRemoteFolder = NULL;

	m_pImageStore = NULL;
	m_pAnimationStore = NULL;
	m_pSoundStore = NULL;
	m_pSpotStore = NULL;
	m_bFirstPerson = false;
	m_pSelectedObjects = new GPointerArray(64);

	// Check for the Mode attribute
	m_bLoner = false;
	GXMLTag* pConfigTag = GameEngine::GetConfig();
	GXMLAttribute* pAttrMode = pConfigTag->GetAttribute("Mode");
	if(pAttrMode)
	{
		if(stricmp(pAttrMode->GetValue(), "loner") == 0)
			m_bLoner = true;
	}
}

/*virtual*/ MGameClient::~MGameClient()
{
	delete(m_pAccountTag);
	delete(m_szAccountFilename);
	UnloadRealm();
	delete(m_pErrorHandler);
	delete(m_pPlayer);
	delete(m_pMap);
	delete(m_szScript);
	delete(m_szRemoteFolder);
	delete(m_pSelectedObjects);
}

void MGameClient::UnloadRealm()
{
	// Unload the current realm
	delete(m_pCamera);
	m_pCamera = NULL;
	delete(m_pCurrentRealm);
	m_pAvatar = NULL;
	m_pClosestObject = NULL;
	m_pGoalFlag = NULL;
	m_pInfoCloud = NULL;
	UnloadMedia();
	delete(m_pScriptEngine);
	m_pScriptEngine = NULL;
	delete(m_pConnection);
	m_pConnection = NULL;
	delete(m_szScript);
	m_szScript = NULL;
	delete(m_pMap);
	m_pMap = NULL;
	delete(m_szRemoteFolder);
	m_szRemoteFolder = NULL;
}

void MGameClient::LoadScript(Controller* pController, const char* szUrl, GXMLTag* pObjectsTag)
{
	int nBufSize;
	delete(m_szScript);
	m_szScript = GameEngine::LoadFileFromUrl(m_szRemoteFolder, szUrl, &nBufSize);
	m_pScriptEngine = new MScriptEngine(m_szScript, nBufSize, m_pErrorHandler, pObjectsTag, this, pController);
}

void MGameClient::LoadRealm(Controller* pController, const char* szUrl, double time, int nScreenVerticalCenter)
{
	UnloadRealm();

	// Download the file into memory
	GAssert(m_pCamera == NULL && m_pAvatar == NULL && m_pConnection == NULL && m_pMap == NULL, "The realm was not unloaded");
	if(strnicmp(szUrl, "http://", 7) != 0)
		GameEngine::ThrowError("the URL should begin with \"http://\"");
	int nLine, nCol;
	const char* szError;
	int nBufSize;
	Holder<char*> hMap(GameEngine::LoadFileFromUrl("", szUrl, &nBufSize));
	delete(m_pMap);
	m_pMap = GXMLTag::FromString(hMap.Get(), nBufSize, &szError, NULL, &nLine, &nCol);
	if(!m_pMap)
		GameEngine::ThrowError("Failed to parse XML file \"%s\". %s", szUrl, szError);

	// Parse the URL
	int nLen = strlen(szUrl);
	GTEMPBUF(pHost, nLen + 1);
	GTEMPBUF(pProtocol, nLen + 1);
	GTEMPBUF(pParams, nLen + 1);
	GSocket::ParseURL(szUrl, pProtocol, pHost, NULL, NULL, pParams);
	delete(m_szRemoteFolder);
	m_szRemoteFolder = new char[nLen + 1];
	strcpy(m_szRemoteFolder, szUrl);
	int n;
	for(n = nLen; n > 0; n--)
	{
		if(m_szRemoteFolder[n] == '/' || m_szRemoteFolder[n] == '\\')
			break;
		m_szRemoteFolder[n] = '\0';
	}
	char* szSpot = NULL;

	// Look for a URL parameter specifying the spot
	if(strlen(pParams) > 0)
	{
		char* szName;
		int nNameLen;
		char* szValue;
		int nValueLen;
		if(GSocket::ParseUrlParams(pParams, 1, &szName, &nNameLen, &szValue, &nValueLen) > 0)
		{
			if(nNameLen == 4 && strnicmp(szName, "spot", 4) == 0)
			{
				szSpot = (char*)alloca(nValueLen + 1);
				memcpy(szSpot, szValue, nValueLen);
				szSpot[nValueLen] = '\0';
			}
		}
	}

	// Set the point of view
	m_bFirstPerson = false;
	GXMLTag* pCameraTag = m_pMap->GetChildTag("Camera");
	if(pCameraTag)
	{
		GXMLAttribute* pAttrPointOfView = pCameraTag->GetAttribute("PointOfView");
		if(pAttrPointOfView)
		{
			if(stricmp(pAttrPointOfView->GetValue(), "FirstPerson") == 0)
				m_bFirstPerson = true;
			else if(stricmp(pAttrPointOfView->GetValue(), "ThirdPerson") == 0)
			{
			}
			else
				GameEngine::ThrowError("Unrecognized point of view: %s", pAttrPointOfView->GetValue());
		}
	}

	// Load the script
	GXMLAttribute* pAttrScript = m_pMap->GetAttribute("Script");
	if(!pAttrScript)
		GameEngine::ThrowError("Expected a \"Script\" attribute in file: %s", szUrl);
	GXMLTag* pObjectsTag = m_pMap->GetChildTag("Objects");
	LoadScript(pController, pAttrScript->GetValue(), pObjectsTag);

	// Connect to the server
	if(!m_bLoner)
	{
		m_pConnection = new NRealmClientConnection(this, pHost);
		NSetPathPacket packet(szUrl);
		m_pConnection->SendPacket(&packet);
	}

	// Load the media
	LoadMedia(m_pMap);

	// Make the new realm
	m_pCurrentRealm = new MRealm(this);
	m_pCurrentRealm->FromXml(m_pMap, m_pImageStore);

	// Find the starting spot
	float x = 0;
	float y = 0;
	int nSpotIndex = m_pSpotStore->GetIndex(szSpot ? szSpot : "Default");
	if(nSpotIndex >= 0)
	{
		MSpot* pSpot = m_pSpotStore->GetSpot(nSpotIndex);
		pSpot->GetPos(&x, &y);
	}

	// Load the static objects
	LoadObjects(m_pCurrentRealm, m_pMap);

	// Make the goal flag
	m_pGoalFlag = MakeIntangibleGlobalObject("flag", 0, 0, -10000);

	if(!m_bFirstPerson)
	{
		// Copy the animations for your avatar into the local animation store
		GXMLAttribute* pAttrAnim = m_pAccountRefTag->GetAttribute("Anim");
		if(!pAttrAnim)
			GameEngine::ThrowError("Expected an \"Anim\" attribute in the account tag in the config file");
		const char* szAvatarAnimID = pAttrAnim->GetValue();
		m_pAnimationStore->AddAnimation(m_pScriptEngine, m_pImageStore, szAvatarAnimID);
		char* szAvatarAnimActionID = (char*)alloca(strlen(szAvatarAnimID) + 10);
		strcpy(szAvatarAnimActionID, szAvatarAnimID);
		strcat(szAvatarAnimActionID, "action");
		m_pAnimationStore->AddAnimation(m_pScriptEngine, m_pImageStore, szAvatarAnimActionID);

		// Construct the avatar object
		const char* pParams[2];
		pParams[0] = szAvatarAnimID;
		pParams[1] = szAvatarAnimActionID;
		m_pAvatar = m_pScriptEngine->NewObject("Avatar", x, y, 0, 250, 250, 250, pParams, 2);
		m_pAvatar->SetTime(time);
		m_pAvatar->SetTangible(false);
		m_pCurrentRealm->ReplaceObject(0, m_pAvatar);

		// Make the info cloud
		m_pInfoCloud = MakeIntangibleGlobalObject("cloud", 0, -5000, 200);
	}

	// Make the camera
	m_pCamera = new GBillboardCamera(nScreenVerticalCenter);
	if(pCameraTag)
	{
		// Yaw
		GXMLAttribute* pAttrYaw = pCameraTag->GetAttribute("Yaw");
		if(pAttrYaw)
			m_pCamera->SetDirection((float)atof(pAttrYaw->GetValue()));

		// Pitch
		GXMLAttribute* pAttrPitch = pCameraTag->GetAttribute("Pitch");
		if(pAttrPitch)
			m_pCamera->AjustHorizonHeight((float)atof(pAttrPitch->GetValue()), nScreenVerticalCenter);

		// Zoom
		GXMLAttribute* pAttrZoom = pCameraTag->GetAttribute("Zoom");
		if(pAttrZoom)
			m_pCamera->AjustZoom((float)atof(pAttrZoom->GetValue()), nScreenVerticalCenter);

		// Position
		if(m_bFirstPerson)
			m_pCamera->SetPos(x, y);
	}

	// Record URL in the account
	GXMLTag* pStartTag = m_pAccountTag->GetChildTag("Start");
	if(!pStartTag)
	{
		pStartTag = new GXMLTag("Start");
		m_pAccountTag->AddChildTag(pStartTag);
	}
	GXMLAttribute* pAttrUrl = pStartTag->GetAttribute("url");
	if(pAttrUrl)
		pAttrUrl->SetValue(szUrl);
	else
		pStartTag->AddAttribute(new GXMLAttribute("url", szUrl));
}

void MGameClient::SaveState()
{
	m_pAccountTag->ToFile(m_szAccountFilename);
}

MObject* MGameClient::MakeIntangibleGlobalObject(const char* szID, float x, float y, float z)
{
	m_pImageStore->AddImage(m_pScriptEngine, szID);
	VarHolder* pVH = m_pImageStore->GetVarHolder(szID);
	MGameImage* pImage = (MGameImage*)pVH->GetGObject();
	MObject* pNewObject = m_pScriptEngine->NewObject("Scenery", x, y, z, (float)pImage->m_value.GetWidth(), (float)pImage->m_value.GetWidth(), (float)pImage->m_value.GetHeight(), &szID, 1);
	pNewObject->SetTangible(false);
	m_pCurrentRealm->ReplaceObject(0, pNewObject);
	return pNewObject;
}

// Whenever the MRealm updates an object, it will call this method
// to inform the model about the change.  If the model is holding any
// pointers to old objects then it needs to update them to point to
// the new objects.
/*virtual*/ bool MGameClient::OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew)
{
	if(m_pClosestObject == pOld)
		m_pClosestObject = pNew;
	int nCount = m_pSelectedObjects->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
	{
		if((MObject*)m_pSelectedObjects->GetPointer(n) == pOld)
			m_pSelectedObjects->SetPointer(n, pNew);
	}

	// m_pAvatar, m_pGoalFlag, and m_pInfoCloud won't be updated so we don't worry about those here,
	// but if we ever change the code so that those are updated, we'll need to check for them here

	// todo: Is there a security vulnerability if someone replaces an avatar in a realm in order to
	//       crash other people's clients?

	return true;
}

void MGameClient::Update(double time)
{
	// Synchronize with server
	if(!m_bLoner)
		SynchronizeWithServer(time);

	// Update all the objects
	m_pCurrentRealm->Update(time, m_pCamera, m_pAvatar);

	// Move the camera
	float xAvatar, yAvatar;
	if(m_pAvatar)
	{	
		m_pAvatar->GetPos(&xAvatar, &yAvatar);
		m_pCamera->SetPos(xAvatar, yAvatar);
	}

	// Ajust thought cloud
	MObject* pClosestObject = m_pCurrentRealm->GetClosestObject();
	if(pClosestObject)
	{
		float reach = MScriptEngine::GetAvatarReach(m_pAvatar);
		if(m_pAvatar->GetDistanceSquared(pClosestObject) <= reach * reach)
		{
			if(pClosestObject != m_pClosestObject)
			{
				if(m_pClosestObject)
					m_pScriptEngine->CallOnLoseFocus(m_pClosestObject);
				m_pScriptEngine->CallOnGetFocus(pClosestObject);
				m_pClosestObject = pClosestObject;
			}
		}
		else
		{
			if(m_pClosestObject)
			{
				m_pScriptEngine->CallOnLoseFocus(m_pClosestObject);
				m_pClosestObject = NULL;
			}
		}
	}

//#ifdef _DEBUG
	// Measure the frame rate
	m_nFrames++;
	if(time - m_dFrameRateTime > 10)
	{
		fprintf(stderr, "Frame Rate: %f\n", (double)m_nFrames / (time - m_dFrameRateTime));
		m_dFrameRateTime = time;
		m_nFrames = 0;
	}
//#endif // _DEBUG
}

void MGameClient::SelectObjects(float x1, float y1, float x2, float y2)
{
	// Drop previous selection
	int nCount = m_pSelectedObjects->GetSize();
	MObject* pOb;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pOb = (MObject*)m_pSelectedObjects->GetPointer(n);
		if(pOb)
			m_pScriptEngine->CallOnLoseFocus(pOb);
	}
	m_pSelectedObjects->Clear();

	// Make sure the mins are less than the maxes
	float tmp;
	if(x2 < x1)
	{
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if(y2 < y1)
	{
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}

	// Select new objects
	m_pCurrentRealm->GetObjectsWithinBox(m_pSelectedObjects, x1, y1, x2, y2);
	nCount = m_pSelectedObjects->GetSize();
	for(n = 0; n < nCount; n++)
	{
		pOb = (MObject*)m_pSelectedObjects->GetPointer(n);
		if(pOb)
			m_pScriptEngine->CallOnGetFocus(pOb);
	}
}

void MGameClient::DoActionOnSelectedObjects(float x, float y)
{
	int nCount = m_pSelectedObjects->GetSize();
	MObject* pOb;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pOb = (MObject*)m_pSelectedObjects->GetPointer(n);
		if(pOb)
			m_pScriptEngine->CallDoAction(pOb, x, y);
	}
}

void MGameClient::MakeChatCloud(const char* szText)
{
	float x, y;
	m_pAvatar->GetPos(&x, &y);
	MObject* pChatCloud = m_pScriptEngine->NewObject("ChatCloud", x, y, 250, 0, 0, 0, &szText, 1);
	GAssert(pChatCloud, "Failed to create chat cloud");
	pChatCloud->SetTangible(false);
	m_pCurrentRealm->ReplaceObject(0, pChatCloud);
	NotifyServerAboutObjectUpdate(pChatCloud);
}

void MGameClient::NotifyServerAboutObjectUpdate(MObject* pOb)
{
	if(m_bLoner)
		return;
	NUpdateObjectPacket packet;
	packet.SetObject(pOb);
	m_pConnection->SendPacket(&packet);
	packet.SetObject(NULL);
}

void MGameClient::SynchronizeWithServer(double time)
{
	// Accept updates from the server
	bool bGotUpdates = false;
	while(true)
	{
		NRealmPacket* pPacket = m_pConnection->GetNextPacket();
		if(!pPacket)
			break;
		bGotUpdates = true;
		ProcessPacket(pPacket);
		delete(pPacket);
	}

	// Ask for more updates
	if(time - m_nextUpdateTime > .1) // todo: unmagic this time interval
	{
		NSendMeUpdatesPacket packet;
		packet.SetTime(m_nextUpdateTime);
		m_pConnection->SendPacket(&packet);
		//m_nextUpdateTime = time;
	}
}

void MGameClient::ProcessPacket(NRealmPacket* pPacket)
{
	switch(pPacket->GetPacketType())
	{
		case NRealmPacket::SEND_ME_UPDATES:
			GAssert(false, "Why is the server asking the client for updates?");
			break;

		case NRealmPacket::UPDATE_OBJECT:
			UpdateObject((NUpdateObjectPacket*)pPacket);
			break;

		default:
			GAssert(false, "Unrecognized packet type");
	}
}

void MGameClient::UpdateObject(NUpdateObjectPacket* pPacket)
{
	MObject* pOb = pPacket->GetMObject();
	if(pOb->GetTime() > m_nextUpdateTime)
		m_nextUpdateTime = pOb->GetTime();
	if(pOb->GetUid() == m_pAvatar->GetUid())
	{
		// todo: determine whether it's necessary to update the avatar
	}
	else
	{
		pPacket->SetObject(NULL);
		m_pCurrentRealm->ReplaceObject(pPacket->GetConnection(), pOb);
	}
}

void MGameClient::MoveGoalFlag(float x, float y, bool bVisible)
{
	if(m_pGoalFlag)
	{
		if(bVisible)
		{
			m_pGoalFlag->SetPos(x, y, 0);
			m_pGoalFlag->SetGhostPos(x, y);
		}
		else
			m_pGoalFlag->SetPos(x, y, -10000);
	}
}

void MGameClient::UnloadMedia()
{
	delete(m_pImageStore);
	m_pImageStore = NULL;
	delete(m_pAnimationStore);
	m_pAnimationStore = NULL;
	delete(m_pSoundStore);
	m_pSoundStore = NULL;
	delete(m_pSpotStore);
	m_pSpotStore = NULL;
}

void MGameClient::LoadMedia(GXMLTag* pModelTag)
{
	UnloadMedia();

	// Load the image store
	GXMLTag* pImages = pModelTag->GetChildTag("Images");
	if(!pImages)
		GameEngine::ThrowError("Expected an 'Images' tag");
	m_pImageStore = new MImageStore();
	m_pImageStore->FromXml(m_szRemoteFolder, pImages, m_pScriptEngine);

	// Load the animation store
	GXMLTag* pAnimations = pModelTag->GetChildTag("Animations");
	if(!pAnimations)
		GameEngine::ThrowError("Expected an 'Animations' tag");
	m_pAnimationStore = new MAnimationStore(m_pScriptEngine);
	m_pAnimationStore->FromXml(pAnimations, m_pImageStore);

	// Load the sound store
	GXMLTag* pSounds = pModelTag->GetChildTag("Sounds");
	if(!pSounds)
		GameEngine::ThrowError("Expected a 'Sounds' tag");
	m_pSoundStore = new MSoundStore();
	m_pSoundStore->FromXml(m_szRemoteFolder, pSounds);

	// Load the spot store
	GXMLTag* pSpots = pModelTag->GetChildTag("Spots");
	if(!pSpots)
		GameEngine::ThrowError("Expected a 'Spots' tag");
	m_pSpotStore = new MSpotStore();
	m_pSpotStore->FromXml(pSpots);
}

void MGameClient::LoadObjects(MRealm* pRealm, GXMLTag* pModelTag)
{
	GXMLTag* pObjectsTag = pModelTag->GetChildTag("Objects");
	GAssert(pObjectsTag, "Expected a 'Scenery' tag");
	if(!pObjectsTag)
		return;
	GXMLTag* pChildTag;
	for(pChildTag = pObjectsTag->GetFirstChildTag(); pChildTag; pChildTag = pObjectsTag->GetNextChildTag(pChildTag))
	{
		// Get a bunch of values from the attributes
		GXMLAttribute* pClassAttr = pChildTag->GetAttribute("class");
		if(!pClassAttr)
			GameEngine::ThrowError("Expected a \"class\" attribute");
		GXMLAttribute* pIDAttr = pChildTag->GetAttribute("id");
		if(!pIDAttr)
			GameEngine::ThrowError("Expected an \"id\" attribute");
		int nID = atoi(pIDAttr->GetValue());
		GXMLAttribute* pXAttr = pChildTag->GetAttribute("x");
		GXMLAttribute* pYAttr = pChildTag->GetAttribute("y");
		GXMLAttribute* pZAttr = pChildTag->GetAttribute("z");
		GXMLAttribute* pSXAttr = pChildTag->GetAttribute("sx");
		GXMLAttribute* pSYAttr = pChildTag->GetAttribute("sy");
		GXMLAttribute* pSZAttr = pChildTag->GetAttribute("sz");
		float x = pXAttr ? (float)atof(pXAttr->GetValue()) : 0;
		float y = pYAttr ? (float)atof(pYAttr->GetValue()) : 0;
		float z = pZAttr ? (float)atof(pZAttr->GetValue()) : 0;
		float sx = pSXAttr ? (float)atof(pSXAttr->GetValue()) : 100;
		float sy = pSYAttr ? (float)atof(pSYAttr->GetValue()) : 100;
		float sz = pSZAttr ? (float)atof(pSZAttr->GetValue()) : 100;

		// Count extra parameters
		char szTmp[64];
		strcpy(szTmp, "param1");
		int nParamCount;
		for(nParamCount = 0; nParamCount < MAX_PARAMS; nParamCount++)
		{
			itoa(nParamCount + 1, szTmp + 5 /*strlen("param")*/, 10);
			if(!pChildTag->GetAttribute(szTmp))
				break;
		}

		// Make pointers to extra parameters
		const char** pParams = NULL;
		if(nParamCount > 0)
		{
			pParams = (const char**)alloca(sizeof(const char*) * nParamCount);
			int n;
			for(n = 0; n < nParamCount; n++)
			{
				itoa(n + 1, szTmp + 5, 10);
				pParams[n] = pChildTag->GetAttribute(szTmp)->GetValue();
			}
		}

		// Allocate the object and add it to the realm
		MObject* pOb = m_pScriptEngine->NewObject(pClassAttr->GetValue(), x, y, z, sx, sy, sz, pParams, nParamCount);

		GXMLAttribute* pModeAttr = pChildTag->GetAttribute("mode");
		if(pModeAttr)
		{
			if(stricmp(pModeAttr->GetValue(), "panel") == 0)
				pOb->SetIsPanel(true);
		}

		pOb->SetUid(nID);
		pRealm->ReplaceObject(0, pOb);
	}
}

void MGameClient::AddObject(const char* szFilename)
{
	// Find the image in the store or add it
	szFilename += strlen("C:\\www\\isotope\\"); // todo: don't hard-code this path
	char szID[256];
	_splitpath(szFilename, NULL, NULL, szID, NULL);
	int n;
	for(n = 0; szFilename[n] != '\0'; n++)
	{
		if(szFilename[n] == '\\')
			((char*)szFilename)[n] = '/'; // todo: don't change a constant string
	}
	bool bAddID = false;
	if(m_pImageStore->GetIndex(szID) < 0)
	{
		m_pImageStore->AddImage(m_szRemoteFolder, m_pScriptEngine, szID, szFilename);
		bAddID = true;
	}
	VarHolder* pVH = m_pImageStore->GetVarHolder(szID);

	// Create the new object
	MObject* pAvatar = GetAvatar();
	float x, y, z;
	if(pAvatar)
		pAvatar->GetPos(&x, &y);
	else
		m_pCamera->GetPosition(&x, &y, &z);
	MGameImage* pGameImage = (MGameImage*)pVH->GetGObject();
	GImage* pImage = &pGameImage->m_value;
	int width = pImage->GetWidth() * 2;
	int height = pImage->GetHeight() * 2;
	char* pID = szID;
	MObject* pNewObject = m_pScriptEngine->NewObject("Scenery", x, y, 0, (float)width, (float)width, (float)height, (const char**)&pID, 1);

	// Add it to the realm
	m_pCurrentRealm->ReplaceObject(0, pNewObject);

	// Add it to the map
	if(bAddID)
	{
		GXMLTag* pImagesTag = m_pMap->GetChildTag("Images");
		GXMLTag* pNewIDTag = new GXMLTag("Image");
		pNewIDTag->AddAttribute(new GXMLAttribute("id", szID));
		pNewIDTag->AddAttribute(new GXMLAttribute("File", szFilename));
		pImagesTag->AddChildTag(pNewIDTag);
	}
	GXMLTag* pObjectsTag = m_pMap->GetChildTag("Objects");
	GXMLTag* pNewObjectTag = new GXMLTag("obj");
	pNewObjectTag->AddAttribute(new GXMLAttribute("id", "0"));
	pNewObjectTag->AddAttribute(new GXMLAttribute("class", "Scenery"));
	pNewObjectTag->AddAttribute(new GXMLAttribute("param1", szID));
	GPosSize* pPosSize = pNewObject->GetGhostPos();
	char szTmp[64];
	sprintf(szTmp, "%f", pPosSize->x);
	pNewObjectTag->AddAttribute(new GXMLAttribute("x", szTmp));
	sprintf(szTmp, "%f", pPosSize->y);
	pNewObjectTag->AddAttribute(new GXMLAttribute("y", szTmp));
	sprintf(szTmp, "%f", pPosSize->z);
	pNewObjectTag->AddAttribute(new GXMLAttribute("z", szTmp));
	sprintf(szTmp, "%f", pPosSize->sx);
	pNewObjectTag->AddAttribute(new GXMLAttribute("sx", szTmp));
	sprintf(szTmp, "%f", pPosSize->sy);
	pNewObjectTag->AddAttribute(new GXMLAttribute("sy", szTmp));
	sprintf(szTmp, "%f", pPosSize->sz);
	pNewObjectTag->AddAttribute(new GXMLAttribute("sz", szTmp));
	pObjectsTag->AddChildTag(pNewObjectTag);
}
