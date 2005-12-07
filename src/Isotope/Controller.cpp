/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include <stdlib.h>
#include "Controller.h"
#include "GameEngine.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GMacros.h"
#include "../GClasses/GHashTable.h"
#include "../GClasses/GBillboardCamera.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GHttp.h"
#include "../GClasses/GWindows.h"
#include "../GClasses/GFile.h"
#include "../GClasses/GSDL.h"
#include "../GClasses/sha2.h"
#include <math.h>
#include "VGame.h"
#include "MGameClient.h"
#include "MRealm.h"
#include "MScriptEngine.h"
#include "MObject.h"
#include "VMainMenu.h"
#include "VEntropyCollector.h"
#include "View.h"
#include "VServer.h"
#include "MRealmServer.h"
#include "MKeyPair.h"
#include "VCharSelect.h"
#include "VCharMake.h"
#include "VLoading.h"
#include "MGameImage.h"
#include "MAnimation.h"
#include "MStore.h"
#ifdef WIN32
#include <direct.h>
#else // WIN32
#include <unistd.h>
#endif // !WIN32

//#define SERVER_HAS_VIEW

Controller::Controller(Controller::RunModes eRunMode, const char* szParam)
{
	// Init the keyboard
	int n;
	for(n = 0; n < SDLK_LAST; n++)
		m_keyboard[n] = 0;
	m_nTypeBufferPos = 0;
	LoadKeyControlValues();

	// Init the mouse
	m_mouse[1] = 0;
	m_mouse[2] = 0;
	m_mouse[3] = 0;
	m_mouseX = -1;
	m_mouseY = -1;
	m_mouseDownX = -1;
	m_mouseDownY = -1;
	m_prevMouseX = -1;
	m_prevMouseY = -1;
	m_bMouseDown = false;

	m_pGameView = NULL;
	m_pLoadingView = NULL;
	m_mode = NOTHING;
	m_goalX = 0;
	m_goalY = 0;
	m_pMainMenu = NULL;
	m_pCharSelect = NULL;
	m_pMakeNewChar = NULL;
	m_szNewUrl = NULL;
	m_pGameClient = NULL;
	m_bQuit = false;
	m_downloadProgressMode = DPM_IGNORE;
	m_pErrorHandler = new IsotopeErrorHandler();
	m_pHttpClient = new GHttpClient();

	if(eRunMode == SERVER)
#ifdef SERVER_HAS_VIEW
		m_pView = new View();
#else // SERVER_HAS_VIEW
		m_pView = NULL;
#endif // !SERVER_HAS_VIEW
	else
		m_pView = new View();
	m_pModel = NULL;
	switch(eRunMode)
	{
		case SERVER:
			m_pModel = new MGameServer(szParam, this);
#ifdef SERVER_HAS_VIEW
			MakeServerView((MGameServer*)m_pModel);
#endif // SERVER_HAS_VIEW
			break;

		case CLIENT:
			m_pModel = new NoModel();
			MakeCharSelectView();
			break;

		case KEYPAIR:
			m_pModel = new MKeyPair(this, szParam);
			MakeEntropyCollectorView();
			break;
	}
}

Controller::~Controller()
{
	delete(m_szNewUrl);
	PopAllViewPorts();
	delete(m_pModel);
	delete(m_pView);
	delete(m_pErrorHandler);
	delete(m_pHttpClient);
}

int GetAttrValue(GConstStringHashTable* pKeyMappings, GXMLTag* pTag, const char* szAttrName)
{
	GXMLAttribute* pAttr = pTag->GetAttribute(szAttrName);
	if(!pAttr)
		GameEngine::ThrowError("Expected a '%s' attribute in the '%s' tag in the config file", szAttrName, pTag->GetName());
	int nSdlKeySym = SDLK_DOWN;
	bool bOk = pKeyMappings->Get(pAttr->GetValue(), (void**)&nSdlKeySym);
	if(!bOk)
		GameEngine::ThrowError("There is no key named \"%s\"", pAttr->GetValue());
	return nSdlKeySym;
}

void AddKey(GConstStringHashTable* pHT, const char* szLabel, int nSdlKeySym)
{
	pHT->Add(szLabel, (const void*)nSdlKeySym);
}

void MakeKeyMappingsHashTable(GConstStringHashTable* pHT)
{
	AddKey(pHT, "BACKSPACE", 8);
	AddKey(pHT, "TAB", 9);
	AddKey(pHT, "CLEAR", 12);
	AddKey(pHT, "RETURN", 13);
	AddKey(pHT, "PAUSE", 19);
	AddKey(pHT, "ESCAPE", 27);
	AddKey(pHT, "SPACE", 32);
	AddKey(pHT, "EXCLAIM", 33);
	AddKey(pHT, "QUOTEDBL", 34);
	AddKey(pHT, "HASH", 35);
	AddKey(pHT, "DOLLAR", 36);
	AddKey(pHT, "AMPERSAND", 38);
	AddKey(pHT, "QUOTE", 39);
	AddKey(pHT, "LEFTPAREN", 40);
	AddKey(pHT, "RIGHTPAREN", 41);
	AddKey(pHT, "ASTERISK", 42);
	AddKey(pHT, "PLUS", 43);
	AddKey(pHT, "COMMA", 44);
	AddKey(pHT, "MINUS", 45);
	AddKey(pHT, "PERIOD", 46);
	AddKey(pHT, "SLASH", 47);
	AddKey(pHT, "0", 48);
	AddKey(pHT, "1", 49);
	AddKey(pHT, "2", 50);
	AddKey(pHT, "3", 51);
	AddKey(pHT, "4", 52);
	AddKey(pHT, "5", 53);
	AddKey(pHT, "6", 54);
	AddKey(pHT, "7", 55);
	AddKey(pHT, "8", 56);
	AddKey(pHT, "9", 57);
	AddKey(pHT, "COLON", 58);
	AddKey(pHT, "SEMICOLON", 59);
	AddKey(pHT, "LESS", 60);
	AddKey(pHT, "EQUALS", 61);
	AddKey(pHT, "GREATER", 62);
	AddKey(pHT, "QUESTION", 63);
	AddKey(pHT, "AT", 64);

	AddKey(pHT, "LEFTBRACKET", 91);
	AddKey(pHT, "BACKSLASH", 92);
	AddKey(pHT, "RIGHTBRACKET", 93);
	AddKey(pHT, "CARET", 94);
	AddKey(pHT, "UNDERSCORE", 95);
	AddKey(pHT, "BACKQUOTE", 96);

	// Alphabet
	AddKey(pHT, "a", 97);
	AddKey(pHT, "b", 98);
	AddKey(pHT, "c", 99);
	AddKey(pHT, "d", 100);
	AddKey(pHT, "e", 101);
	AddKey(pHT, "f", 102);
	AddKey(pHT, "g", 103);
	AddKey(pHT, "h", 104);
	AddKey(pHT, "i", 105);
	AddKey(pHT, "j", 106);
	AddKey(pHT, "k", 107);
	AddKey(pHT, "l", 108);
	AddKey(pHT, "m", 109);
	AddKey(pHT, "n", 110);
	AddKey(pHT, "o", 111);
	AddKey(pHT, "p", 112);
	AddKey(pHT, "q", 113);
	AddKey(pHT, "r", 114);
	AddKey(pHT, "s", 115);
	AddKey(pHT, "t", 116);
	AddKey(pHT, "u", 117);
	AddKey(pHT, "v", 118);
	AddKey(pHT, "w", 119);
	AddKey(pHT, "x", 120);
	AddKey(pHT, "y", 121);
	AddKey(pHT, "z", 122);
	AddKey(pHT, "DELETE", 127);
	// End of ASCII mapped keysyms

	// Numeric keypad
	AddKey(pHT, "KP0", 256);
	AddKey(pHT, "KP1", 257);
	AddKey(pHT, "KP2", 258);
	AddKey(pHT, "KP3", 259);
	AddKey(pHT, "KP4", 260);
	AddKey(pHT, "KP5", 261);
	AddKey(pHT, "KP6", 262);
	AddKey(pHT, "KP7", 263);
	AddKey(pHT, "KP8", 264);
	AddKey(pHT, "KP9", 265);
	AddKey(pHT, "KP_PERIOD", 266);
	AddKey(pHT, "KP_DIVIDE", 267);
	AddKey(pHT, "KP_MULTIPLY", 268);
	AddKey(pHT, "KP_MINUS", 269);
	AddKey(pHT, "KP_PLUS", 270);
	AddKey(pHT, "KP_ENTER", 271);
	AddKey(pHT, "KP_EQUALS", 272);

	// Arrows + Home/End pad
	AddKey(pHT, "UP", 273);
	AddKey(pHT, "DOWN", 274);
	AddKey(pHT, "RIGHT", 275);
	AddKey(pHT, "LEFT", 276);
	AddKey(pHT, "INSERT", 277);
	AddKey(pHT, "HOME", 278);
	AddKey(pHT, "END", 279);
	AddKey(pHT, "PAGEUP", 280);
	AddKey(pHT, "PAGEDOWN", 281);

	// Function keys
	AddKey(pHT, "F1", 282);
	AddKey(pHT, "F2", 283);
	AddKey(pHT, "F3", 284);
	AddKey(pHT, "F4", 285);
	AddKey(pHT, "F5", 286);
	AddKey(pHT, "F6", 287);
	AddKey(pHT, "F7", 288);
	AddKey(pHT, "F8", 289);
	AddKey(pHT, "F9", 290);
	AddKey(pHT, "F10", 291);
	AddKey(pHT, "F11", 292);
	AddKey(pHT, "F12", 293);
	AddKey(pHT, "F13", 294);
	AddKey(pHT, "F14", 295);
	AddKey(pHT, "F15", 296);

	// Key state modifier keys
	AddKey(pHT, "NUMLOCK", 300);
	AddKey(pHT, "CAPSLOCK", 301);
	AddKey(pHT, "SCROLLOCK", 302);
	AddKey(pHT, "RSHIFT", 303);
	AddKey(pHT, "LSHIFT", 304);
	AddKey(pHT, "RCTRL", 305);
	AddKey(pHT, "LCTRL", 306);
	AddKey(pHT, "RALT", 307);
	AddKey(pHT, "LALT", 308);
	AddKey(pHT, "RMETA", 309);
	AddKey(pHT, "LMETA", 310);
	AddKey(pHT, "LSUPER", 311);		// Left "Windows" key
	AddKey(pHT, "RSUPER", 312);		// Right "Windows" key
	AddKey(pHT, "MODE", 313);		// "Alt Gr" key
	AddKey(pHT, "COMPOSE", 314);		// Multi-key compose key

	// Miscellaneous function keys
	AddKey(pHT, "HELP", 315);
	AddKey(pHT, "PRINT", 316);
	AddKey(pHT, "SYSREQ", 317);
	AddKey(pHT, "BREAK", 318);
	AddKey(pHT, "MENU", 319);
	AddKey(pHT, "POWER", 320);		// Power Macintosh power key
	AddKey(pHT, "EURO", 321);		// Some european keyboards
	AddKey(pHT, "UNDO", 322);		// Atari keyboard has Undo
}

void Controller::LoadKeyControlValues()
{
	GConstStringHashTable ht(307, false);
	MakeKeyMappingsHashTable(&ht);

	// Global key mappings
	GXMLTag* pConfigTag = GameEngine::GetConfig();
	GXMLTag* pKeyMappingsTag = pConfigTag->GetChildTag("KeyMappings");
	if(!pKeyMappingsTag)
		GameEngine::ThrowError("Expected a 'KeyMappings' tag in the config file");
	m_keyMenu1 = GetAttrValue(&ht, pKeyMappingsTag, "menu1");
	m_keyMenu2 = GetAttrValue(&ht, pKeyMappingsTag, "menu2");
	m_keyAction1 = GetAttrValue(&ht, pKeyMappingsTag, "action1");
	m_keyAction2 = GetAttrValue(&ht, pKeyMappingsTag, "action2");

	// Third person mappings
	GXMLTag* pThirdPersonTag = pKeyMappingsTag->GetChildTag("ThirdPerson");
	if(!pThirdPersonTag)
		GameEngine::ThrowError("Expected a 'ThirdPerson' tag in the config file");
	m_ktpYawRight = GetAttrValue(&ht, pThirdPersonTag, "yawright");
	m_ktpYawLeft = GetAttrValue(&ht, pThirdPersonTag, "yawleft");
	m_ktpZoomIn = GetAttrValue(&ht, pThirdPersonTag, "zoomin");
	m_ktpZoomOut = GetAttrValue(&ht, pThirdPersonTag, "zoomout");
	m_ktpPitchUp = GetAttrValue(&ht, pThirdPersonTag, "pitchup");
	m_ktpPitchDown = GetAttrValue(&ht, pThirdPersonTag, "pitchdown");
	m_bFpsControls = false;
	GXMLAttribute* pFpsControlsAttr = pThirdPersonTag->GetAttribute("fpscontrols");
	if(pFpsControlsAttr && stricmp(pFpsControlsAttr->GetValue(), "true") == 0)
		m_bFpsControls = true;

	// First person mappings
	GXMLTag* pFirstPersonTag = pKeyMappingsTag->GetChildTag("FirstPerson");
	if(!pFirstPersonTag)
		GameEngine::ThrowError("Expected a 'FirstPerson' tag in the config file");
	m_kfpYawRight = GetAttrValue(&ht, pFirstPersonTag, "yawright");
	m_kfpYawLeft = GetAttrValue(&ht, pFirstPersonTag, "yawleft");
	m_kfpZoomIn = GetAttrValue(&ht, pFirstPersonTag, "zoomin");
	m_kfpZoomOut = GetAttrValue(&ht, pFirstPersonTag, "zoomout");
	m_kfpPitchUp = GetAttrValue(&ht, pFirstPersonTag, "pitchup");
	m_kfpPitchDown = GetAttrValue(&ht, pFirstPersonTag, "pitchdown");
	m_kfpTrackRight = GetAttrValue(&ht, pFirstPersonTag, "trackright");
	m_kfpTrackLeft = GetAttrValue(&ht, pFirstPersonTag, "trackleft");
	m_kfpTrackUp = GetAttrValue(&ht, pFirstPersonTag, "trackup");
	m_kfpTrackDown = GetAttrValue(&ht, pFirstPersonTag, "trackdown");
}

void Controller::Run()
{
#ifndef SERVER_HAS_VIEW
	if(m_pModel->GetType() == Model::Server)
	{
		while(!m_bQuit)
			m_pModel->Update(GameEngine::GetTime());
		return;
	}
#endif // SERVER_HAS_VIEW
	double timeOld = GameEngine::GetTime();
	double time;
	while(!m_bQuit)
	{
		time = GameEngine::GetTime();
		Update(time - timeOld);
		m_pModel->Update(time);
		m_pView->Refresh();
		timeOld = time;
	}
}

void Controller::Update(double dTimeDelta)
{
	// Follow the link if there is one set
	if(m_szNewUrl)
	{
		GAssert(m_pGameClient, "Only the client should try to follow a URL");
		char* szNewUrl = m_szNewUrl;
		m_szNewUrl = NULL;
		GoToRealm(szNewUrl);
		delete(szNewUrl);
	}

	// Check for events
	int mouseButtonClearers[8];
	mouseButtonClearers[1] = 1;
	mouseButtonClearers[3] = 1;
	int nCount = 0;
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		nCount++;
		switch(event.type)
		{
			case SDL_KEYDOWN:
				m_keyboard[event.key.keysym.sym] = 1;
				OnKeyDown(event.key.keysym.sym, event.key.keysym.mod);
				break;

			case SDL_KEYUP:
				m_keyboard[event.key.keysym.sym] = 0;
				break;

			case SDL_MOUSEBUTTONDOWN:
				m_mouse[event.button.button] = 1;
				m_mouseDownX = event.button.x;
				m_mouseDownY = event.button.y;
				break;

			case SDL_MOUSEBUTTONUP:
				mouseButtonClearers[event.button.button] = 0;
				//m_mouse[event.button.button] = 0;
				break;

			case SDL_MOUSEMOTION:
				m_mouseX = event.motion.x;
				m_mouseY = event.motion.y;
				break;

			case SDL_QUIT:
				ShutDown();
				break;

			default:
				break;
		}
	}
	DoControl(dTimeDelta);
	m_mouse[1] &= mouseButtonClearers[1];
	m_mouse[3] &= mouseButtonClearers[3];
	mouseButtonClearers[3] = 1;
}

void Controller::ShutDown()
{
	m_bQuit = true;
	if(m_pModel->GetType() == Model::Client)
	{
		m_pGameClient->SaveState();
	}
}

void Controller::SetMode(ControlModes newMode)
{
	m_mode = newMode;
}

void Controller::FollowLink(const char* szUrl)
{
	GAssert(m_pModel->GetType() == Model::Client, "Only the client should follow links");
	delete(m_szNewUrl);
	if(strnicmp(szUrl, "http://", 7) == 0)
	{
		m_szNewUrl = new char[strlen(szUrl) + 1];
		strcpy(m_szNewUrl, szUrl);
	}
	else
	{
		const char* szRemoteFolder = m_pGameClient->GetRemoteFolder();
		m_szNewUrl = new char[strlen(szRemoteFolder) + strlen(szUrl) + 10];
		strcpy(m_szNewUrl, szRemoteFolder);
		strcat(m_szNewUrl, szUrl);
	}
}

void Controller::OnKeyDown(SDLKey key, SDLMod mod)
{
	if(key > SDLK_z)
		return;
	else if(key < SDLK_SPACE)
	{
		if(key == SDLK_BACKSPACE)
		{
			if(m_nTypeBufferPos > 0)
			{
				m_szTypeBuffer[--m_nTypeBufferPos] = '\0';
				return;
			}
		}
		else if(key == SDLK_RETURN)
		{
			if(m_mode == THIRDPERSON)
			{
				// Make the chat cloud
				if(m_nTypeBufferPos <= 0)
					return;
				if(!m_pGameClient)
					return;
				m_pGameClient->MakeChatCloud(m_szTypeBuffer);
				ClearTypeBuffer();
			}
		}
		else if(key == SDLK_ESCAPE)
			ShutDown();
		return;
	}
	char c = (char)key;
	if(c > 126)
		return;
	if(m_nTypeBufferPos >= TYPE_BUFFER_SIZE - 1)
		return;

	// Capitalize if shift is down
	if(mod & KMOD_SHIFT)
		c = GSDL::ShiftKey(c);

	// Record the key
	m_szTypeBuffer[m_nTypeBufferPos++] = c;
	m_szTypeBuffer[m_nTypeBufferPos] = '\0';
}

void Controller::ClearTypeBuffer()
{
	m_nTypeBufferPos = 0;
	m_szTypeBuffer[0] = '\0';
}

inline int CalculateSignVector(float x, float y)
{
	int n = 0;
	if(x >= 0)
		n |= 1;
	if(y >= 0)
		n |= 2;
	return n;
}

void Controller::DoControl(double dTimeDelta)
{
	switch(m_mode)
	{
		case NOTHING:
			return;
		case THIRDPERSON:
			ControlThirdPerson(dTimeDelta);
			break;
		case FIRSTPERSON:
			ControlFirstPerson(dTimeDelta);
			break;
		case MAINMENU:
			ControlMainMenu(dTimeDelta);
			break;
		case ENTROPYCOLLECTOR:
			ControlEntropyCollector(dTimeDelta);
			break;
		case SELECTCHAR:
			ControlCharSelect(dTimeDelta);
			break;
		case MAKENEWCHAR:
			ControlMakeNewChar(dTimeDelta);
			break;
		default:
			GAssert(false, "unexpected control mode");
	}
}

void Controller::SetAvatarVelocity(MObject* pAvatar, float dx, float dy)
{
	// Change avatar's velocity
	MScriptEngine::SetAvatarVelocity(dx, dy, pAvatar->GetGObject());

	// Report the change to the server
	GAssert(m_pGameClient, "Why is the server changing the avatar's velocity?");
	m_pGameClient->NotifyServerAboutObjectUpdate(pAvatar);
}

void Controller::SetGoalSpot(float x, float y)
{
	// Check boundaries
	float xmin, xmax, ymin, ymax;
	m_pGameClient->GetCurrentRealm()->GetMapRect(&xmin, &xmax, &ymin, &ymax);
	if(x < xmin || x > xmax || y < ymin || y > ymax)
		return;

	// Move the flag
	float xAvatar, yAvatar;
	MObject* pAvatar = m_pGameClient->GetAvatar();
	pAvatar->GetPos(&xAvatar, &yAvatar);
	m_pGameClient->MoveGoalFlag(x, y, true);
	m_goalX = x;
	m_goalY = y;
	m_goalSignVector = CalculateSignVector(x - xAvatar, y - yAvatar);

	// Calculate the direction from the avatar to the goal spot
	float dx = x - xAvatar;
	float dy = y - yAvatar;
	float mag = dx * dx + dy * dy;
	if(mag > .0001)
	{
		mag = (float)sqrt(mag);
		dx /= mag;
		dy /= mag;
	}
	else
	{
		dx = 0;
		dy = 0;
	}

	// Set the avatar's velocity
	SetAvatarVelocity(pAvatar, dx, dy);
}

// This returns 1 if n is negative and 0 if n is positive
inline int IsNegative(int n)
{
	return ((unsigned int)n & 0x80000000) >> 31;
}

void Controller::ControlThirdPerson(double dTimeDelta)
{
	// Stop the avatar if he/she has reached the goal spot
	if(m_pGameClient)
	{
		float xAvatar, yAvatar;
		MObject* pAvatar = m_pGameClient->GetAvatar();
		if(pAvatar)
		{
			pAvatar->GetPos(&xAvatar, &yAvatar);
			if(CalculateSignVector(m_goalX - xAvatar, m_goalY - yAvatar) != m_goalSignVector)
			{
				SetAvatarVelocity(pAvatar, 0, 0);
				m_pGameClient->MoveGoalFlag(xAvatar, yAvatar, false);
				m_goalSignVector = CalculateSignVector(m_goalX - xAvatar, m_goalY - yAvatar);
			}
		}
	}

	GBillboardCamera* pCamera = m_pGameView->GetCamera();

	// Mouse Click
	if(m_mouse[1]) // left button
	{
		m_mouse[1] = 0;
		float mapX, mapY;
		bool bSky;
		m_pGameView->ScreenToMap(&mapX, &mapY, &bSky, m_mouseDownX, m_mouseDownY);
		if(bSky)
		{
			int nSafety = 50;
			while(bSky && nSafety--)
			{
				m_mouseDownY += 4;
				m_pGameView->ScreenToMap(&mapX, &mapY, &bSky, m_mouseDownX, m_mouseDownY);
			}
		}
		if(!bSky)
			SetGoalSpot(mapX, mapY);
	}
	if(m_keyboard[m_keyAction1] | m_keyboard[m_keyAction2])
	{
		// Find the nearest object and tell it to do its action
		m_keyboard[m_keyAction1] = 0;
		m_keyboard[m_keyAction2] = 0;
		m_mouse[3] = 0;
		MRealm* pRealm = m_pGameClient->GetCurrentRealm();
		MObject* pNearestOb = pRealm->GetClosestObject();
		if(pNearestOb)
		{
			MObject* pAvatar = m_pGameClient->GetAvatar();
			float avatarX, avatarY;
			pAvatar->GetPos(&avatarX, &avatarY);
			float distsquared = pAvatar->GetDistanceSquared(pNearestOb);
			float reach = MScriptEngine::GetAvatarReach(pAvatar);
			MScriptEngine* pScriptEngine = m_pGameClient->GetScriptEngine();
			pScriptEngine->DoAvatarActionAnimation(pAvatar);
			if(distsquared <= reach * reach)
				pScriptEngine->CallDoAction(pNearestOb, avatarX, avatarY);
		}
	}

	// Camera Yaw
	float dx = (float)(m_keyboard[m_ktpYawLeft] - m_keyboard[m_ktpYawRight]);
	if(dx != 0)
	{
		dx *= (float)dTimeDelta * 3;
		float fDirection = pCamera->GetDirection();
		fDirection += dx;
		pCamera->SetDirection(fDirection);
	}

	// Zoom
	float dy = (float)(m_keyboard[m_ktpZoomOut] - m_keyboard[m_ktpZoomIn]);
	if(dy != 0)
	{
		dy *= (float)dTimeDelta;
		dy = 1 - dy;
		pCamera->AjustZoom(dy, m_pGameView->GetRect()->h / 2 - 75);
	}

	// Camera Pitch (sort of)
	float dPitch = (float)(m_keyboard[m_ktpPitchUp] - m_keyboard[m_ktpPitchDown]);
	if(dPitch != 0)
	{
		dPitch *= (float)dTimeDelta;
		dPitch += 1;
		pCamera->AjustHorizonHeight(dPitch, m_pGameView->GetRect()->h / 2 - 75);
	}

	// Menu Key
	if(m_keyboard[m_keyMenu1] | m_keyboard[m_keyMenu2])
		BringUpMainMenu();
}

void Controller::ControlFirstPerson(double dTimeDelta)
{
	GBillboardCamera* pCamera = m_pGameView->GetCamera();

	// Mouse Click
	if(m_mouse[1]) // left button
	{
		// Calculate the selection rect so the view can draw it
		bool bSky;
		float mapX, mapY;
		m_pGameView->ScreenToMap(&mapX, &mapY, &bSky, m_mouseX, m_mouseY);
		if(!bSky)
		{
			if(!m_bMouseDown)
			{
				m_goalX = mapX;
				m_goalY = mapY;
			}
			m_bMouseDown = true;
			m_pGameView->SetSelectionRect(m_goalX, m_goalY, mapX, mapY);
		}
	}
	else if(m_bMouseDown)
	{
		// Clear the selection rect
		m_bMouseDown = false;
		m_pGameView->SetSelectionRect(0, 0, 0, 0);

		// Calculate map position
		bool bSky;
		float mapX, mapY;
		m_pGameView->ScreenToMap(&mapX, &mapY, &bSky, m_mouseX, m_mouseY);
		if(!bSky)
		{
			// Select the objects
			m_pGameClient->SelectObjects(m_goalX, m_goalY, mapX, mapY);
		}
	}
	if(m_mouse[3] | m_keyboard[m_keyAction1] | m_keyboard[m_keyAction2])
	{
		// Tell selected objects to do their action
		m_mouse[3] = 0;
		m_keyboard[m_keyAction1] = 0;
		m_keyboard[m_keyAction2] = 0;
		bool bSky;
		float mapX, mapY;
		m_pGameView->ScreenToMap(&mapX, &mapY, &bSky, m_mouseX, m_mouseY);
		if(!bSky)
			m_pGameClient->DoActionOnSelectedObjects(mapX, mapY);
	}

	// Camera Yaw
	float dx = (float)(m_keyboard[m_kfpYawLeft] - m_keyboard[m_kfpYawRight]);
	if(dx != 0)
	{
		dx *= (float)dTimeDelta * 3;
		float fDirection = pCamera->GetDirection();
		fDirection += dx;
		pCamera->SetDirection(fDirection);
	}

	// Zoom
	float dy = (float)(m_keyboard[m_kfpZoomOut] - m_keyboard[m_kfpZoomIn]);
	if(dy != 0)
	{
		dy *= (float)dTimeDelta;
		dy = 1 - dy;
		pCamera->AjustZoom(dy, m_pGameView->GetRect()->h / 2 - 75);
	}

	// Camera Pitch (sort of)
	float dPitch = (float)(m_keyboard[m_kfpPitchUp] - m_keyboard[m_kfpPitchDown]);
	if(dPitch != 0)
	{
		dPitch *= (float)dTimeDelta;
		dPitch += 1;
		pCamera->AjustHorizonHeight(dPitch, m_pGameView->GetRect()->h / 2 - 75);
	}

	// Arrow Keys
	dx = (float)(m_keyboard[m_kfpTrackRight] - m_keyboard[m_kfpTrackLeft]) * 1500 * (float)dTimeDelta;
	dy = (float)(m_keyboard[m_kfpTrackUp] - m_keyboard[m_kfpTrackDown]) * 1500 * (float)dTimeDelta;
	pCamera->Move(dx, dy);

	// Menu Key
	if(m_keyboard[m_keyMenu1] | m_keyboard[m_keyMenu2])
		BringUpMainMenu();
}

void Controller::BringUpMainMenu()
{
	// Don't let the key stroke repeat
	m_keyboard[m_keyMenu1] = 0;
	m_keyboard[m_keyMenu2] = 0;

	// Create the main menu
	GAssert(!m_pMainMenu, "Main menu already exists");
	GRect* pScreenRect = m_pView->GetScreenRect();
	GRect r;
	r.x = pScreenRect->x + pScreenRect->w / 2;
	r.y = pScreenRect->y + pScreenRect->h;
	r.w = 50;
	r.h = 50;
	m_pMainMenu = new VMainMenu(&r, pScreenRect, m_pGameClient->GetAccountTag(), this);
	m_pView->PushViewPort(m_pMainMenu);
	SetMode(MAINMENU);
	m_dMenuSlideInAnimTime = 0;
	m_dMenuSlideOutAnimTime = 0;
}

void Controller::RemoveMainMenu()
{
	ViewPort* pPort = m_pView->PopViewPort();
	GAssert(pPort == m_pMainMenu, "unexpected view port");
	delete(pPort);
	m_pMainMenu = NULL;
	if(m_pGameClient->IsFirstPerson())
		SetMode(FIRSTPERSON);
	else
		SetMode(THIRDPERSON);
}

#define MENU_SLIDE_ANIMATION_TIME .3

void Controller::SlideMenu(float fac)
{
	if(fac < 0)
		fac = 0;
	if(fac > 1)
		fac = 1;
	float fac2 = 1 - fac;
	GRect r;
	GRect* pScreenRect = m_pView->GetScreenRect();
	r.x = (int)(fac * (pScreenRect->x + 50) + fac2 * (pScreenRect->x + pScreenRect->w / 2));
	r.y = (int)(fac * (pScreenRect->y + 50) + fac2 * (pScreenRect->y + pScreenRect->h));
	r.w = (int)(fac * (pScreenRect->w - 100) + fac2 * 50);
	r.h = (int)(fac * (pScreenRect->h - 100) + fac2 * 50);
	m_pMainMenu->SetRect(&r);
}

void Controller::ControlMainMenu(double dTimeDelta)
{
	// Slide in the menu
	if(m_dMenuSlideInAnimTime < MENU_SLIDE_ANIMATION_TIME)
	{
		m_dMenuSlideInAnimTime += dTimeDelta;
		float fac = (float)sqrt(m_dMenuSlideInAnimTime / MENU_SLIDE_ANIMATION_TIME);
		SlideMenu(fac);
	}
	else if(m_dMenuSlideOutAnimTime > 0)
	{
		m_dMenuSlideOutAnimTime -= dTimeDelta;
		float fac = (float)(m_dMenuSlideOutAnimTime / MENU_SLIDE_ANIMATION_TIME);
		fac *= fac;
		SlideMenu(fac);
		if(m_dMenuSlideOutAnimTime <= 0)
		{
			RemoveMainMenu();
			return;
		}
	}

	// Control key
	if(m_keyboard[m_keyMenu1] | m_keyboard[m_keyMenu2])
	{
		// Don't let the key stroke repeat
		m_keyboard[m_keyMenu1] = 0;
		m_keyboard[m_keyMenu2] = 0;
		if(m_dMenuSlideOutAnimTime <= 0)
			m_dMenuSlideOutAnimTime = MENU_SLIDE_ANIMATION_TIME;
		else
		{
			m_dMenuSlideInAnimTime = 0;
			m_dMenuSlideOutAnimTime = 0;
			SlideMenu(0);
		}
	}

	if(m_mouse[1]) // left button
	{
		if(!m_bMouseDown)
		{
			m_pMainMenu->OnMouseDown(m_mouseX, m_mouseY);
			m_bMouseDown = true;
		}
	}
	else
	{
		if(m_bMouseDown)
		{
			m_pMainMenu->OnMouseUp(m_mouseX, m_mouseY);
			m_bMouseDown = false;
		}
	}

	// Give the view mouse tracking information so it can do the scroll bars properly
	m_pMainMenu->OnMousePos(m_mouseX, m_mouseY);
}

void Controller::ControlCharSelect(double dTimeDelta)
{
	if(m_mouse[1]) // left button
	{
		if(!m_bMouseDown)
		{
			m_pCharSelect->OnMouseDown(this, m_mouseX, m_mouseY);
			m_bMouseDown = true;
		}
	}
	else
	{
		if(m_bMouseDown)
		{
			m_pCharSelect->OnMouseUp(this, m_mouseX, m_mouseY);
			m_bMouseDown = false;
		}
	}
}

void Controller::ControlMakeNewChar(double dTimeDelta)
{
	if(m_mouse[1]) // left button
	{
		if(!m_bMouseDown)
		{
			m_pMakeNewChar->OnMouseDown(this, m_mouseX, m_mouseY);
			m_bMouseDown = true;
		}
	}
	else
	{
		if(m_bMouseDown)
		{
			m_pMakeNewChar->OnMouseUp(this, m_mouseX, m_mouseY);
			m_bMouseDown = false;
		}
	}
}

void Controller::ToggleTerrain()
{
	m_pGameView->ToggleTerrain();
}

void Controller::MakeScreenSmaller()
{
	m_pView->MakeScreenSmaller();
	m_pGameView->SetRect(m_pView->GetScreenRect());
}

void Controller::MakeScreenBigger()
{
	m_pView->MakeScreenBigger();
	m_pGameView->SetRect(m_pView->GetScreenRect());
}

void Controller::ViewScript()
{
	const char* szScript = m_pGameClient->GetScript();
	FILE* pFile = fopen("script.txt", "wb");
	fputs(szScript, pFile);
	fclose(pFile);
#ifdef WIN32
	ShellExecute(NULL, NULL, "script.txt", NULL, NULL, SW_SHOW);
#else
	system("kate script.txt");
#endif // !WIN32
}

void Controller::ViewMap()
{
	GXMLTag* pMap = m_pGameClient->GetMap();
	Holder<char*> hScript(pMap->ToString());
	FileHolder hFile(fopen("map.txt", "wb"));
	FILE* pFile = hFile.Get();
	fputs(hScript.Get(), pFile);
#ifdef WIN32
	ShellExecute(NULL, NULL, "map.txt", NULL, NULL, SW_SHOW);
#else
	system("kate map.txt");
#endif // !WIN32
}

void Controller::PopAllViewPorts()
{
	if(!m_pView)
		return;
	while(m_pView->GetViewPortCount() > 0)
		delete(m_pView->PopViewPort());
	m_pGameView = NULL;
	m_pMainMenu = NULL;
	m_pCharSelect = NULL;
	m_pMakeNewChar = NULL;
}

void Controller::MakeServerView(MGameServer* pServer)
{
	ViewPort* pServerPort = new VServer(m_pView->GetScreenRect(), pServer);
	m_pView->PushViewPort(pServerPort);
	SetMode(NOTHING);
}

void Controller::ReduceImageFootprint(MImageStore* pImages, MAnimationStore* pAnimations, int nAcceptableFootprint)
{
	int nPrevBiggestIndex = -1;
	while(true)
	{
		// Find the biggest image
		int nFootprint = 0;
		int nGroundID = pImages->GetIndex("ground");
		int nSkyID = pImages->GetIndex("sky");
		int nBiggest = 0;
		int nBiggestIndex = -1;
		int n, nImageFootprint;
		int nCount = pImages->GetImageCount();
		VarHolder* pVH;
		MGameImage* pGameImage;
		GImage* pImage;
		for(n = 0; n < nCount; n++)
		{
			if(n == nGroundID || n == nSkyID)
				continue;
			if(n == nPrevBiggestIndex)
				continue; // Don't squish the same image twice in a row
			pVH = pImages->GetVarHolder(n);
			pGameImage = (MGameImage*)pVH->GetGObject();
			pImage = &pGameImage->m_value;
			nImageFootprint = (int)pImage->GetWidth() * (int)pImage->GetHeight() * sizeof(unsigned int);
			nFootprint += nImageFootprint;
			if(nImageFootprint > nBiggest)
			{
				nBiggest = nImageFootprint;
				nBiggestIndex = n;
			}
		}
printf("Total image footprint: %d\n", nFootprint);
		if(nBiggestIndex < 0)
			break;
		if(nFootprint < nAcceptableFootprint)
			break;
		if(nBiggest < 10000)
			break; // Don't try to squish images that are already rather small
		nPrevBiggestIndex = nBiggestIndex;

		// Scale the image
printf("Squishing the biggest image...\n");
		pVH = pImages->GetVarHolder(nBiggestIndex);
		pGameImage = (MGameImage*)pVH->GetGObject();
		pImage = &pGameImage->m_value;
		pImage->Scale(pImage->GetWidth() / 2, pImage->GetHeight() / 2);

		// Scale the frames of any animations that use this image
		nCount = pAnimations->GetAnimationCount();
		VarHolder* pVHAnim;
		MAnimation* pAnim;
		for(n = 0; n < nCount; n++)
		{
			pVHAnim = pAnimations->GetVarHolder(n);
			pAnim = (MAnimation*)pVHAnim->GetGObject();
			if(pAnim->GetImage() == pVH)
				pAnim->Scale(.5);
		}
	}
}

void Controller::GoToRealm(const char* szUrl)
{
	// Show the "loading" page
	SetMode(Controller::NOTHING);
	PopAllViewPorts();
	m_pLoadingView = new VLoading(m_pView->GetScreenRect(), szUrl);
	m_pView->PushViewPort(m_pLoadingView);
	m_pView->Refresh(); // Redraw the loading page
	m_downloadProgressMode = DPM_LOADINGPAGE;

	// Download the realm file and parse into an XML doc
	if(strnicmp(szUrl, "http://", 7) != 0)
		GameEngine::ThrowError("the URL should begin with \"http://\"");
	int nLine, nCol;
	const char* szError;
	int nBufSize;
	Holder<char*> hMap(LoadFileFromUrl("", szUrl, &nBufSize));
	GXMLTag* pMap = GXMLTag::FromString(hMap.Get(), nBufSize, &szError, NULL, &nLine, &nCol);
	if(!pMap)
		GameEngine::ThrowError("Failed to parse XML file \"%s\" at line %d. %s", szUrl, nLine, szError);

	// Load realm phase 1
	m_pGameClient->UnloadRealm();
	m_pGameClient->LoadRealmPhase1(pMap, szUrl);

	// Load the script
	GXMLAttribute* pAttrScript = pMap->GetAttribute("Script");
	if(!pAttrScript)
		GameEngine::ThrowError("Expected a \"Script\" attribute in file: %s", szUrl);
	int nScriptSize;
	char* szScript = LoadFileFromUrl(m_pGameClient->GetRemoteFolder(), pAttrScript->GetValue(), &nScriptSize);
	MScriptEngine* pScriptEngine = new MScriptEngine(pAttrScript->GetValue(), szScript, nScriptSize, m_pErrorHandler, pMap, m_pGameClient, this, m_pGameClient->GetCurrentRealm());

	// Load the image store
	GXMLTag* pImages = pMap->GetChildTag("Images");
	if(!pImages)
		GameEngine::ThrowError("Expected an 'Images' tag");
	Holder<MImageStore*> hImageStore(new MImageStore());
	hImageStore.Get()->FromXml(m_pGameClient->GetRemoteFolder(), pImages, pScriptEngine);

	// Load the animation store
	GXMLTag* pAnimations = pMap->GetChildTag("Animations");
	if(!pAnimations)
		GameEngine::ThrowError("Expected an 'Animations' tag");
	Holder<MAnimationStore*> hAnimationStore(new MAnimationStore(pScriptEngine));
	hAnimationStore.Get()->FromXml(pAnimations, hImageStore.Get());

	// Squish big images until we have an acceptable footprint
	ReduceImageFootprint(hImageStore.Get(), hAnimationStore.Get(), 50331648 /*48 MB*/);

	// Load the sound store
	GXMLTag* pSounds = pMap->GetChildTag("Sounds");
	if(!pSounds)
		GameEngine::ThrowError("Expected a 'Sounds' tag");
	Holder<MSoundStore*> hSoundStore(new MSoundStore());
	hSoundStore.Get()->FromXml(this, m_pGameClient->GetRemoteFolder(), pSounds);

	// Load the spot store
	GXMLTag* pSpots = pMap->GetChildTag("Spots");
	if(!pSpots)
		GameEngine::ThrowError("Expected a 'Spots' tag");
	Holder<MSpotStore*> hSpotStore(new MSpotStore());
	hSpotStore.Get()->FromXml(pSpots);

	// Load realm phase 2
	m_pGameClient->LoadRealmPhase2(szUrl, szScript, pScriptEngine, GameEngine::GetTime(), m_pView->GetScreenRect()->h / 2 - 75, hImageStore.Drop(), hAnimationStore.Drop(), hSoundStore.Drop(), hSpotStore.Drop());
	if(m_pGameClient->IsFirstPerson())
		SetMode(Controller::FIRSTPERSON);
	else
		SetMode(Controller::THIRDPERSON);
	m_downloadProgressMode = DPM_IGNORE;

	// Record URL in the account
	GXMLTag* pAccountTag = m_pGameClient->GetAccountTag();
	GXMLTag* pStartTag = pAccountTag->GetChildTag("Start");
	if(!pStartTag)
	{
		pStartTag = new GXMLTag("Start");
		pAccountTag->AddChildTag(pStartTag);
	}
	GXMLAttribute* pAttrUrl = pStartTag->GetAttribute("url");
	if(pAttrUrl)
		pAttrUrl->SetValue(szUrl);
	else
		pStartTag->AddAttribute(new GXMLAttribute("url", szUrl));

	// Get the ground and sky images
	MImageStore* pImageStore = m_pGameClient->GetImages();
	int nImageIndex = pImageStore->GetIndex("sky");
	VarHolder* pVH = pImageStore->GetVarHolder(nImageIndex);
	if(!pVH)
		GameEngine::ThrowError("Expected an image with the ID \"sky\"");
	MGameImage* pSkyImage = (MGameImage*)pVH->GetGObject();
	nImageIndex = pImageStore->GetIndex("ground");
	pVH = pImageStore->GetVarHolder(nImageIndex);
	if(!pVH)
		GameEngine::ThrowError("Expected an image with the ID \"ground\"");
	MGameImage* pGroundImage = (MGameImage*)pVH->GetGObject();

	// Show the game view
	ViewPort* pViewPort = m_pView->PopViewPort();
	GAssert(pViewPort == m_pLoadingView, "Unexpected view port");
	delete(pViewPort);
	m_pLoadingView = NULL;
	GAssert(!m_pGameView, "The game view already exists");
	m_pGameView = new VGame(m_pView->GetScreenRect(), m_pGameClient, &pSkyImage->m_value, &pGroundImage->m_value);
	m_pView->PushViewPort(m_pGameView);
}

void Controller::SetSkyImage(GString* pID)
{
	MImageStore* pImageStore = m_pGameClient->GetImages();
	char* szID = (char*)alloca(pID->GetLength() + 1);
	pID->GetAnsi(szID);
	int nImageIndex = pImageStore->GetIndex(szID);
	VarHolder* pVH = pImageStore->GetVarHolder(nImageIndex);
	if(!pVH)
		GameEngine::ThrowError("There is no image with the ID \"%s\"", szID);
	MGameImage* pImage = (MGameImage*)pVH->GetGObject();
	m_pGameView->SetSkyImage(&pImage->m_value);
}

void Controller::SetGroundImage(GString* pID)
{
	MImageStore* pImageStore = m_pGameClient->GetImages();
	char* szID = (char*)alloca(pID->GetLength() + 1);
	pID->GetAnsi(szID);
	int nImageIndex = pImageStore->GetIndex(szID);
	VarHolder* pVH = pImageStore->GetVarHolder(nImageIndex);
	if(!pVH)
		GameEngine::ThrowError("There is no image with the ID \"%s\"", szID);
	MGameImage* pImage = (MGameImage*)pVH->GetGObject();
	m_pGameView->SetGroundImage(&pImage->m_value);
}

void Controller::MakeEntropyCollectorView()
{
	ViewPort* pViewPort = new VEntropyCollector(m_pView->GetScreenRect(), (MKeyPair*)m_pModel);
	m_pView->PushViewPort(pViewPort);
	SetMode(ENTROPYCOLLECTOR);
}

void Controller::MakeCharSelectView()
{
	m_pCharSelect = new VCharSelect(m_pView->GetScreenRect(), m_szTypeBuffer, this);
	m_pView->PushViewPort(m_pCharSelect);
	SetMode(SELECTCHAR);
}

void Controller::MakeNewCharView()
{
	m_pMakeNewChar = new VCharMake(m_pView->GetScreenRect(), m_szTypeBuffer, this);
	m_pView->PushViewPort(m_pMakeNewChar);
	SetMode(MAKENEWCHAR);
	ClearTypeBuffer();
}

void Controller::CancelMakeNewChar()
{
	ViewPort* pViewPort = m_pView->PopViewPort();
	GAssert(pViewPort == m_pMakeNewChar, "unexpected view port");
	m_pMakeNewChar = NULL;
	delete(pViewPort);
	SetMode(SELECTCHAR);
	m_pCharSelect->ReloadAccounts();
}

void Controller::CreateNewCharacter(const char* szAvatarID, const char* szPassword)
{
	// Make the account file
	char szUsername[64];
	itoa(rand(), szUsername, 10); // todo: use the username instead of a random number
	const char* szAppPath = GameEngine::GetAppPath();
	char* szAccountFilename = (char*)alloca(strlen(szAppPath) + 50 + strlen(szUsername));
	strcpy(szAccountFilename, szAppPath);
	strcat(szAccountFilename, "accounts");
#ifdef WIN32
	mkdir(szAccountFilename);
#else // WIN32
	mkdir(szAccountFilename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif // !WIN32
	strcat(szAccountFilename, "/");
	strcat(szAccountFilename, szUsername);
	strcat(szAccountFilename, ".xml");
	Holder<GXMLTag*> hRootTag(new GXMLTag("Account"));
	hRootTag.Get()->ToFile(szAccountFilename);

	// Add an account ref to the config file
	GXMLTag* pConfigTag = GameEngine::GetConfig();
	GXMLTag* pAccountsTag = pConfigTag->GetChildTag("Accounts");
	if(!pAccountsTag)
	{
		pAccountsTag = new GXMLTag("Accounts");
		pConfigTag->AddChildTag(pAccountsTag);
	}
	GXMLTag* pNewAccountTag = new GXMLTag("Account");
	pAccountsTag->AddChildTag(pNewAccountTag);
	pNewAccountTag->AddAttribute(new GXMLAttribute("Anim", szAvatarID));
	char szPasswordHash[2 * SHA512_DIGEST_LENGTH + 1];
	GameEngine::MakePasswordHash(szPasswordHash, szPassword);
	pNewAccountTag->AddAttribute(new GXMLAttribute("Password", szPasswordHash));
	pNewAccountTag->AddAttribute(new GXMLAttribute("File", szAccountFilename + strlen(szAppPath)));
	GameEngine::SaveConfig();

	CancelMakeNewChar();
}

void Controller::LogIn(GXMLTag* pAccountRefTag, const char* szPassword)
{
	// Unload the character selection view
	ViewPort* pViewPort = m_pView->PopViewPort();
	GAssert(pViewPort == m_pCharSelect, "unexpected view port");
	m_pCharSelect = NULL;
	delete(pViewPort);

	// Load the account
	GXMLAttribute* pFileAttr = pAccountRefTag->GetAttribute("File");
	if(!pFileAttr)
		GameEngine::ThrowError("Expected a \"File\" attribute in the account tag");
	const char* szAppPath = GameEngine::GetAppPath();
	char* szFilename = (char*)alloca(strlen(szAppPath) + strlen(pFileAttr->GetValue()) + 10);
	strcpy(szFilename, szAppPath);
	strcat(szFilename, pFileAttr->GetValue());
	const char* szErrorMessage;
	int nErrorLine;
	GXMLTag* pAccountTag = GXMLTag::FromFile(szFilename, &szErrorMessage, NULL, &nErrorLine, NULL);
	if(!pAccountTag)
		GameEngine::ThrowError("Error loading account %s at line %d, %s", szFilename, nErrorLine, szErrorMessage);

	// Find the start URL
	const char* szStartUrl = NULL;
	GXMLTag* pStartTag = pAccountTag->GetChildTag("Start");
	if(pStartTag)
	{
		GXMLAttribute* pUrlAttr = pStartTag->GetAttribute("url");
		if(pUrlAttr)
			szStartUrl = pUrlAttr->GetValue();
	}
	if(!szStartUrl)
		szStartUrl = GameEngine::GetStartUrl();

	// Start the game
	delete(m_pModel);
	m_pGameClient = new MGameClient(szFilename, pAccountTag, pAccountRefTag);
	m_pModel = m_pGameClient;
	GoToRealm(szStartUrl);
}

void Controller::AddObject(const char* szFilename)
{
	m_pGameClient->AddObject(szFilename);
}

void Controller::ControlEntropyCollector(double dTimeDelta)
{
	if(m_mouseX == m_prevMouseX && m_mouseY == m_prevMouseY)
		return;
	m_prevMouseX = m_mouseX;
	m_prevMouseY = m_mouseY;
	MKeyPair* pModel = (MKeyPair*)m_pModel;
	pModel->AddEntropy(m_mouseX);
	pModel->AddEntropy(rand());
	pModel->AddEntropy(m_mouseY);
	pModel->AddEntropy((int)(GameEngine::GetTime() * 10000));
	if(pModel->GetPercent() >= 1)
	{
		m_pView->Refresh();
		pModel->SaveKeyPair();
		m_bQuit = true;
	}
}

void Controller::SendToClient(GObject* pObj, int nConnection)
{
	GAssert(m_pModel->GetType() == Model::Server, "Only the server should send to the client");
	m_pModel->SendObject(pObj, nConnection);
}

void Controller::SendToServer(GObject* pObj)
{
	GAssert(m_pModel->GetType() == Model::Client, "Only the client should send to the server");
	m_pModel->SendObject(pObj, 0);
}

// todo: this method probably doesn't handle all URL's properly.  For example, if the host is
// specified but the protocol is not, I think it chokes.  Also if the URL begins with a '/', I
// don't think it really looks in the host's root folder as expected
char* Controller::LoadFileFromUrl(const char* szRemotePath, const char* szUrl, int* pnSize)
{
	OnBeginDownloadingFile(szUrl);

	// Cut off the protocol specification
	const char* pUrlWithoutProtocol = szUrl;
	bool bProtocolSpecified = false;
	if(strnicmp(szUrl, "http://", 7) == 0)
	{
		bProtocolSpecified = true;
		pUrlWithoutProtocol = szUrl + 7;
	}

	// Try loading from the cache
	const char* szCachePath = GameEngine::GetCachePath();
	GTEMPBUF(szBuf, strlen(szCachePath) + strlen(szRemotePath) + strlen(pUrlWithoutProtocol) + 10);
	strcpy(szBuf, szCachePath);
	if(!bProtocolSpecified)
	{
		if(strnicmp(szRemotePath, "http://", 7) != 0)
			GameEngine::ThrowError("remote path should start with protocol specifier");
		strcat(szBuf, szRemotePath + 7);
	}
	strcat(szBuf, pUrlWithoutProtocol);
	int n;
	for(n = 0; szBuf[n] != '\0' && szBuf[n] != '?'; n++)
	{
	}
	if(szBuf[n] == '?')
		szBuf[n] = '\0';
	if(GFile::DoesFileExist(szBuf))
	{
		if(pnSize)
		{
			char* pFile = GFile::LoadFileToBuffer(szBuf, pnSize);
			if(!pFile)
				GameEngine::ThrowError("Failed to load existing file from cache: %s", szBuf);
			return pFile;
		}
		else
		{
			char* szFilename = new char[strlen(szBuf) + 1];
			strcpy(szFilename, szBuf);
			return szFilename;
		}
	}

	// Download and cache the file
	if(bProtocolSpecified)
		return DownloadAndCacheFile(szUrl, pnSize, szBuf);
	else
	{
		GTEMPBUF(szFullUrl, strlen(szRemotePath) + strlen(szUrl) + 1);
		strcpy(szFullUrl, szRemotePath);
		strcat(szFullUrl, szUrl);
		return DownloadAndCacheFile(szFullUrl, pnSize, szBuf);
	}
}


/*static*/ char* Controller::DownloadFile(GHttpClient* pSocket, const char* szUrl, int* pnSize, bool bThrow, double dTimeout, DownloadFileProgressCallback pProgressCallback, void* pThis)
{
	// Download from URL
#ifdef _DEBUG
	int nAttempts = 1;
#else // _DEBUG
	int nAttempts = 2;
#endif // !_DEBUG
	for( ; nAttempts > 0; nAttempts--)
	{
		if(!pSocket->Get(szUrl, 80))
			GameEngine::ThrowError("Failed to connect to url: %s", szUrl);
		float fProgress = 0;
		float fPrevProgress = 0;
		double dTime;
		double dLastReportProgressTime = GameEngine::GetTime();
		double dLastMakeProgressTime = dLastReportProgressTime;
		while(pSocket->CheckStatus(&fProgress) == GHttpClient::Downloading)
		{
#ifdef WIN32
			GWindows::YieldToWindows();
			Sleep(0);
#else // WIN32
			usleep(0);
#endif // else WIN32
			dTime = GameEngine::GetTime();
			if(dTime - dLastReportProgressTime > .15)
			{
				if(fProgress > fPrevProgress)
				{
					fPrevProgress = fProgress;
					dLastMakeProgressTime = dTime;
				}
				else if(dTime - dLastMakeProgressTime > dTimeout)
					break;
				if(pProgressCallback)
					pProgressCallback(pThis, fProgress);
				dLastReportProgressTime = dTime;
			}
		}
		int nSize;
		char* pFile = (char*)pSocket->DropData(&nSize);
		if(!pFile)
		{
			if(nAttempts > 1)
				continue;
			if(bThrow)
			{
				char* szSocketStatus;
				switch(pSocket->CheckStatus(NULL))
				{
					case GHttpClient::Downloading: szSocketStatus = "Timed out"; break;
					case GHttpClient::Error: szSocketStatus = "An error occurred while downloading"; break;
					case GHttpClient::NotFound: szSocketStatus = "404- Not Found"; break;
					case GHttpClient::Done: szSocketStatus = "Successful"; break;
					default: szSocketStatus = "Unexpected status enumeration"; break;
				}
				GameEngine::ThrowError("Failed to download URL \"%s\".  Status=%s", szUrl, szSocketStatus);
			}
		}
		if(pnSize)
			*pnSize = nSize;
		return pFile;
	}
	GAssert(false, "shouldn't get here");
	GameEngine::ThrowError("Failed to download URL \"%s\".", szUrl);
	return NULL;
}

void Controller::OnBeginDownloadingFile(const char* szUrl)
{
	GAssert(!m_szNewUrl, "this will be problematic");
	Update(0);
	if(m_bQuit)
		GameEngine::ThrowError("User aborted download");
	if(m_downloadProgressMode == DPM_LOADINGPAGE)
	{
		m_pLoadingView->SetUrl(szUrl);
		m_pLoadingView->SetProgress(0);
		m_pView->Refresh();
	}
}

void Controller::OnDownloadFileProgress(float fProgress)
{
	GAssert(!m_szNewUrl, "this will be problematic");
	Update(0);
	if(m_bQuit)
		GameEngine::ThrowError("User aborted download");
	if(m_downloadProgressMode == DPM_LOADINGPAGE)
	{
		m_pLoadingView->SetProgress(fProgress);
		m_pView->Refresh();
	}
}

void ControllerDownloadFileProgressCallback(void* pThis, float fProgress)
{
	((Controller*)pThis)->OnDownloadFileProgress(fProgress);
}

char* Controller::DownloadAndCacheFile(const char* szUrl, int* pnSize, char* szCacheName)
{
	// Download it
	int nSize;
	Holder<char*> hFile(DownloadFile(m_pHttpClient, szUrl, &nSize, true, 30, ControllerDownloadFileProgressCallback, this));
	char* pFile = hFile.Get();

	// Cache the file
	int n;
	for(n = strlen(szCacheName) - 1; n > 0; n--)
	{
		if(szCacheName[n] == '/' || szCacheName[n] == '\\')
		{
			char cTmp = szCacheName[n];
			szCacheName[n] = '\0';
			GFile::MakeDir(szCacheName);
			szCacheName[n] = cTmp;
			break;
		}
	}
	{
		FileHolder fh(fopen(szCacheName, "wb"));
		FILE* pFH = fh.Get();
		if(pFH)
			fwrite(pFile, nSize, 1, pFH);
		else
		{
			GAssert(false, "Failed to save file in cache");
			if(!pnSize)
				GameEngine::ThrowError("Failed to save file in cache: %s", szCacheName);
		}
	}

	// Return the requested data (either the file loaded in memory, or the filename in the cache)
	if(pnSize)
	{
		*pnSize = nSize;
		return hFile.Drop();
	}
	else
	{
		char* szFilename = new char[strlen(szCacheName) + 1];
		strcpy(szFilename, szCacheName);
		return szFilename;
	}
}

