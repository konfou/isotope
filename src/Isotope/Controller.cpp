/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "Controller.h"
#include "GameEngine.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GMacros.h"
#include "../GClasses/GBillboardCamera.h"
#include "../GClasses/GXML.h"
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
	m_mode = NOTHING;
	m_goalX = 0;
	m_goalY = 0;
	m_pMainMenu = NULL;
	m_pCharSelect = NULL;
	m_pMakeNewChar = NULL;
	m_szNewUrl = NULL;
	m_pGameClient = NULL;
	m_bQuit = false;
	if(eRunMode == SERVER)
#ifdef SERVER_HAS_VIEW
		m_pView = new View();
#else // SERVER_HAS_VIEW
		m_pView = NULL;
#endif // !SERVER_HAS_VIEW
	else
		m_pView = new View();
	switch(eRunMode)
	{
		case SERVER:
			m_pModel = new MGameServer(szParam);
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
		GoToRealm(m_szNewUrl);
		delete(m_szNewUrl);
		m_szNewUrl = NULL;
	}

	// Check for events
	int mouseButtonClearers[4];
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
#ifdef WIN32
				m_mouseX = event.motion.x;
				m_mouseY = event.motion.y;
#else // WIN32
				// This is a workaround for a packing bug in SDL on Linux
				m_mouseX = event.motion.y;
				m_mouseY = event.motion.xrel;
#endif // !WIN32
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

char g_shiftTable[] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
	33, 34, 35, 36, 37, 38, '"', 40, 41, 42, 43,
	'<', '_', '>', '?',
	')', '!', '@', '#', '$', '%', '^', '&', '*', '(',
	58, ':', 60, 61, 62, 63, 64,
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'{', '|', '}', 94, 95, '~', 
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	123, 124, 125, 126, 127
};

void Controller::OnKeyDown(SDLKey key, SDLMod mod)
{
	if(key > SDLK_z)
		return;
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
	else if(key < SDLK_SPACE)
		return;
	char c = (char)key;
	if(c > 126)
		return;
	if(m_nTypeBufferPos >= TYPE_BUFFER_SIZE - 1)
		return;

	// Capitalize if shift is down
	if(mod & KMOD_SHIFT)
		c = g_shiftTable[c];

	// Record the key
	m_szTypeBuffer[m_nTypeBufferPos++] = c;
	m_szTypeBuffer[m_nTypeBufferPos] = '\0';
}

void Controller::ClearTypeBuffer()
{
	m_nTypeBufferPos = 0;
	m_szTypeBuffer[0] = '\0';
}

/*
void Controller::GetArrowKeyVector(float* pdx, float* pdy)
{
	float dx = (float)(m_keyboard[SDLK_RIGHT] - m_keyboard[SDLK_LEFT]);
	float dy = (float)(m_keyboard[SDLK_UP] - m_keyboard[SDLK_DOWN]);
	float mag = dx * dx + dy * dy;
	if(mag > .0001)
	{
		mag = sqrt(mag);
		*pdx = dx / mag;
		*pdy = dy / mag;
	}
	else
	{
		*pdx = 0;
		*pdy = 0;
	}
}
*/

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
	if(m_keyboard[SDLK_LALT] | m_keyboard[SDLK_RALT])
	{
		// Find the nearest object and tell it to do its action
		m_keyboard[SDLK_LALT] = 0;
		m_keyboard[SDLK_RALT] = 0;
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
	float dx = (float)(m_keyboard[SDLK_LEFT] - m_keyboard[SDLK_RIGHT]);
	if(dx != 0)
	{
		dx *= (float)dTimeDelta * 3;
		float fDirection = pCamera->GetDirection();
		fDirection += dx;
		pCamera->SetDirection(fDirection);
	}

	// Zoom
	float dy = (float)(m_keyboard[SDLK_DOWN] - m_keyboard[SDLK_UP]);
	if(dy != 0)
	{
		dy *= (float)dTimeDelta;
		dy = 1 - dy;
		pCamera->AjustZoom(dy, m_pGameView->GetRect()->h / 2 - 75);
	}

	// Camera Pitch (sort of)
	float dPitch = (float)(m_keyboard[SDLK_KP8] - m_keyboard[SDLK_KP5]);
	if(dPitch != 0)
	{
		dPitch *= (float)dTimeDelta;
		dPitch += 1;
		pCamera->AjustHorizonHeight(dPitch, m_pGameView->GetRect()->h / 2 - 75);
	}

	// Menu Key
	if(m_keyboard[SDLK_RCTRL] | m_keyboard[SDLK_LCTRL])
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
	if(m_mouse[3] | m_keyboard[SDLK_LALT] | m_keyboard[SDLK_RALT])
	{
		// Tell selected objects to do their action
		m_mouse[3] = 0;
		m_keyboard[SDLK_LALT] = 0;
		m_keyboard[SDLK_RALT] = 0;
		bool bSky;
		float mapX, mapY;
		m_pGameView->ScreenToMap(&mapX, &mapY, &bSky, m_mouseX, m_mouseY);
		if(!bSky)
			m_pGameClient->DoActionOnSelectedObjects(mapX, mapY);
	}

	// Camera Yaw
	float dx = (float)(m_keyboard[SDLK_KP4] - m_keyboard[SDLK_KP6]);
	if(dx != 0)
	{
		dx *= (float)dTimeDelta * 3;
		float fDirection = pCamera->GetDirection();
		fDirection += dx;
		pCamera->SetDirection(fDirection);
	}

	// Zoom
	float dy = (float)(m_keyboard[SDLK_KP1] - m_keyboard[SDLK_KP7]);
	if(dy != 0)
	{
		dy *= (float)dTimeDelta;
		dy = 1 - dy;
		pCamera->AjustZoom(dy, m_pGameView->GetRect()->h / 2 - 75);
	}

	// Camera Pitch (sort of)
	float dPitch = (float)(m_keyboard[SDLK_KP8] - m_keyboard[SDLK_KP5]);
	if(dPitch != 0)
	{
		dPitch *= (float)dTimeDelta;
		dPitch += 1;
		pCamera->AjustHorizonHeight(dPitch, m_pGameView->GetRect()->h / 2 - 75);
	}

	// Arrow Keys
	dx = (float)(m_keyboard[SDLK_RIGHT] - m_keyboard[SDLK_LEFT]) * 1500 * (float)dTimeDelta;
	dy = (float)(m_keyboard[SDLK_UP] - m_keyboard[SDLK_DOWN]) * 1500 * (float)dTimeDelta;
	pCamera->Move(dx, dy);

	// Menu Key
	if(m_keyboard[SDLK_RCTRL] | m_keyboard[SDLK_LCTRL])
		BringUpMainMenu();
}

void Controller::BringUpMainMenu()
{
	// Don't let the key stroke repeat
	m_keyboard[SDLK_RCTRL] = 0;
	m_keyboard[SDLK_LCTRL] = 0;

	// Create the main menu
	GAssert(!m_pMainMenu, "Main menu already exists");
	GRect* pScreenRect = m_pView->GetScreenRect();
	GRect r;
	r.x = pScreenRect->x + pScreenRect->w / 2;
	r.y = pScreenRect->y + pScreenRect->h;
	r.w = 50;
	r.h = 50;
	m_pMainMenu = new VMainMenu(&r, pScreenRect);
	m_pView->PushViewPort(m_pMainMenu);
	SetMode(MAINMENU);
	m_dMenuSlideInAnimTime = 0;
	m_dMenuSlideOutAnimTime = 0;
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
			// Remove the main menu and switch back to Avatar mode
			ViewPort* pPort = m_pView->PopViewPort();
			GAssert(pPort == m_pMainMenu, "unexpected view port");
			delete(pPort);
			m_pMainMenu = NULL;
			if(m_pGameClient->IsFirstPerson())
				SetMode(FIRSTPERSON);
			else
				SetMode(THIRDPERSON);
			return;
		}
	}

	// Control key
	if(m_keyboard[SDLK_RCTRL] | m_keyboard[SDLK_LCTRL])
	{
		// Don't let the key stroke repeat
		m_keyboard[SDLK_RCTRL] = 0;
		m_keyboard[SDLK_LCTRL] = 0;
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
			m_pMainMenu->OnMouseDown(this, m_mouseX, m_mouseY);
			m_bMouseDown = true;
		}
	}
	else
	{
		if(m_bMouseDown)
		{
			m_pMainMenu->OnMouseUp(this, m_mouseX, m_mouseY);
			m_bMouseDown = false;
		}
	}
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
	GAssert(false, "todo: view script not implemented for Linux yet");
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
	GAssert(false, "todo: view map not implemented for Linux yet");
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

void Controller::GoToRealm(const char* szURL)
{
	// Show the "loading" page
	SetMode(Controller::NOTHING);
	PopAllViewPorts();
	ViewPort* pLoadingView = new VLoading(m_pView->GetScreenRect());
	m_pView->PushViewPort(pLoadingView);
	m_pView->Refresh(); // Redraw the loading page

	// Load the new realm
	m_pGameClient->LoadRealm(this, szURL, GameEngine::GetTime(), m_pView->GetScreenRect()->h / 2 - 75);
	if(m_pGameClient->IsFirstPerson())
		SetMode(Controller::FIRSTPERSON);
	else
		SetMode(Controller::THIRDPERSON);

	// Show the game view
	ViewPort* pViewPort = m_pView->PopViewPort();
	GAssert(pViewPort == pLoadingView, "Unexpected view port");
	delete(pViewPort);
	GAssert(!m_pGameView, "The game view already exists");
	m_pGameView = new VGame(m_pView->GetScreenRect(), m_pGameClient);
	m_pView->PushViewPort(m_pGameView);
}

void Controller::MakeEntropyCollectorView()
{
	ViewPort* pViewPort = new VEntropyCollector(m_pView->GetScreenRect(), (MKeyPair*)m_pModel);
	m_pView->PushViewPort(pViewPort);
	SetMode(ENTROPYCOLLECTOR);
}

void Controller::MakeCharSelectView()
{
	m_pCharSelect = new VCharSelect(m_pView->GetScreenRect(), m_szTypeBuffer);
	m_pView->PushViewPort(m_pCharSelect);
	SetMode(SELECTCHAR);
}

void Controller::MakeNewCharView()
{
	m_pMakeNewChar = new VCharMake(m_pView->GetScreenRect(), m_szTypeBuffer);
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
		GameEngine::ThrowError("Expected an Accounts tag in config.xml");
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
		GameEngine::ThrowError("Error loading account %s, %s", szFilename, szErrorMessage);

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
