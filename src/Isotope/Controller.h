/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "../SDL/SDL.h"

class GameEngine;
class GPointerArray;
class MService;
class MAvatar;
class GBillboardCamera;
class VGame;
class MGameClient;
class MObject;
class VMainMenu;
class Model;
class View;
class VCharSelect;
class VCharMake;
class MGameServer;
class MRealm;
class GXMLTag;

#define TYPE_BUFFER_SIZE 128

// This class takes input from the mouse and keyboard and makes appropriate changes to the model
class Controller
{
public:
	enum RunModes
	{
		SERVER,
		CLIENT,
		LONER,
		KEYPAIR,
	};

	enum ControlModes
	{
		NOTHING,
		THIRDPERSON,
		FIRSTPERSON,
		MAINMENU,
		ENTROPYCOLLECTOR,
		SELECTCHAR,
		MAKENEWCHAR,
	};

protected:
	bool m_bQuit;
	View* m_pView;
	Model* m_pModel;
	int m_keyboard[SDLK_LAST];
	char m_szTypeBuffer[TYPE_BUFFER_SIZE];
	int m_nTypeBufferPos;
	int m_mouse[4];
	int m_mouseX;
	int m_mouseY;
	int m_prevMouseX;
	int m_prevMouseY;
	int m_mouseDownX;
	int m_mouseDownY;
	ControlModes m_mode; // Tells what you are currently controlling
	float m_goalX, m_goalY;
	int m_goalSignVector;
	VGame* m_pGameView;
	MGameClient* m_pGameClient;
	VMainMenu* m_pMainMenu;
	VCharSelect* m_pCharSelect;
	VCharMake* m_pMakeNewChar;
	double m_dMenuSlideInAnimTime;
	double m_dMenuSlideOutAnimTime;
	bool m_bMouseDown;
	char* m_szNewUrl;
	bool m_bLoner;

public:
	Controller(RunModes eRunMode, const char* szParam);
	~Controller();

	// The main loop
	void Run();

	// This method tells the controller to check for events and handle them
	void Update(double dTimeDelta);

	// Specify what the user is controlling
	void SetMode(ControlModes newMode);

	// Main Menu Function
	void ToggleTerrain();

	// Main Menu Function
	void MakeScreenSmaller();

	// Main Menu Function
	void MakeScreenBigger();

	// Main Menu Function
	void ViewScript();

	// Main Menu Function
	void ViewMap();

	// This tells the controller to go to the specified URL next time Controller::Update is called
	void FollowLink(const char* szUrl);

	void MakeServerView(MGameServer* pServer);
	void MakeNewCharView();
	void CancelMakeNewChar();
	void AddObject(const char* szFilename);
	void ClearTypeBuffer();
	void CreateNewCharacter(const char* szAvatarID, const char* szPassword);
	void LogIn(GXMLTag* pAccountRefTag, const char* szPassword);

protected:
	//void GetArrowKeyVector(float* pdx, float* pdy);

	void DoControl(double dTimeDelta);
	void ControlThirdPerson(double dTimeDelta);
	void ControlFirstPerson(double dTimeDelta);
	void ControlMainMenu(double dTimeDelta);
	void ControlCharSelect(double dTimeDelta);
	void ControlMakeNewChar(double dTimeDelta);
	void ControlEntropyCollector(double dTimeDelta);
	void GoToRealm(const char* szURL);
	void PopAllViewPorts();

	// Helper for ControlThirdPerson
	void SetAvatarVelocity(MObject* pAvatar, float dx, float dy);

	// Helper for ControlThirdPerson
	void SetGoalSpot(float x, float y);

	// Helper for ControlMainMenu
	void SlideMenu(float fac);

	void OnKeyDown(SDLKey key, SDLMod mod);

	void MakeEntropyCollectorView();
	void MakeCharSelectView();
	void BringUpMainMenu();
};

#endif // __CONTROLLER_H__
