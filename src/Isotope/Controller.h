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
class VLoading;
class View;
class VCharSelect;
class VCharMake;
class MGameServer;
class MRealm;
class GXMLTag;
class GObject;
class IsotopeErrorHandler;
class GHttpClient;
class GString;
class MImageStore;
class MAnimationStore;

#define TYPE_BUFFER_SIZE 128

typedef void (*DownloadFileProgressCallback)(void* pThis, float fProgress);

// This class takes input from the mouse and keyboard and makes appropriate changes to the model
class Controller
{
public:
	enum RunModes
	{
		SERVER,
		CLIENT,
		KEYPAIR,
		PUZGEN,
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

	enum DownloadProgressModes
	{
		DPM_IGNORE,
		DPM_LOADINGPAGE,
	};

protected:
	bool m_bQuit;
	View* m_pView;
	Model* m_pModel;
	int m_keyboard[SDLK_LAST];
	char m_szTypeBuffer[TYPE_BUFFER_SIZE];
	int m_nTypeBufferPos;
	int m_mouse[8];
	int m_mouseX;
	int m_mouseY;
	int m_prevMouseX;
	int m_prevMouseY;
	int m_mouseDownX;
	int m_mouseDownY;

	// Keyboard Controls
	int m_keyMenu1, m_keyMenu2, m_keyAction1, m_keyAction2;
	int m_ktpYawRight, m_ktpYawLeft, m_ktpZoomIn, m_ktpZoomOut, m_ktpPitchUp, m_ktpPitchDown;
	int m_kfpYawRight, m_kfpYawLeft, m_kfpZoomIn, m_kfpZoomOut, m_kfpPitchUp, m_kfpPitchDown, m_kfpTrackRight, m_kfpTrackLeft, m_kfpTrackUp, m_kfpTrackDown;
	bool m_bFpsControls;

	ControlModes m_mode; // Tells what you are currently controlling
	DownloadProgressModes m_downloadProgressMode;
	float m_goalX, m_goalY;
	int m_goalSignVector;
	VGame* m_pGameView;
	MGameClient* m_pGameClient;
	VMainMenu* m_pMainMenu;
	VCharSelect* m_pCharSelect;
	VCharMake* m_pMakeNewChar;
	VLoading* m_pLoadingView;
	double m_dMenuSlideInAnimTime;
	double m_dMenuSlideOutAnimTime;
	bool m_bMouseDown;
	char* m_szNewUrl;
	IsotopeErrorHandler* m_pErrorHandler;
	GHttpClient* m_pHttpClient;

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

	// This downloads a file from a URL or loads it from disk.  You must delete the
	// buffer that it returns.  It will call ThrowError if there are any problems.
	// (If pnSize is NULL, then instead of returning a pointer to the file loaded into memory,
	// it will just return the cached filename.  You must still delete the buffer it returns,
	// and it will still download the file if it doesn't exist in the cache yet.)
	char* LoadFileFromUrl(const char* szRemotePath, const char* szUrl, int* pnSize);

	// Downloads the file at the specified URL.  If it fails and bThrow is true,
	// it will throw an execption, otherwise just return NULL.  dTimout is measured in
	// seconds.  pProgressCallback will report progress periodically if it's not NULL.
	static char* DownloadFile(GHttpClient* pSocket, const char* szUrl, int* pnSize, bool bThrow, double dTimeout, DownloadFileProgressCallback pProgressCallback, void* pThis);

	void MakeServerView(MGameServer* pServer);
	void MakeNewCharView();
	void CancelMakeNewChar();
	void AddObject(const char* szFilename);
	void ClearTypeBuffer();
	void CreateNewCharacter(const char* szAvatarID, const char* szPassword);
	void LogIn(GXMLTag* pAccountRefTag, const char* szPassword);
	void SendToClient(GObject* pObj, int nConnection);
	void SendToServer(GObject* pObj);
	void OnBeginDownloadingFile(const char* szUrl);
	void OnDownloadFileProgress(float fProgress);
	void SetSkyImage(GString* pID);
	void SetGroundImage(GString* pID);

protected:
	//void GetArrowKeyVector(float* pdx, float* pdy);

	void DoControl(double dTimeDelta);
	void ControlThirdPerson(double dTimeDelta);
	void ControlFirstPerson(double dTimeDelta);
	void ControlMainMenu(double dTimeDelta);
	void ControlCharSelect(double dTimeDelta);
	void ControlMakeNewChar(double dTimeDelta);
	void ControlEntropyCollector(double dTimeDelta);
	void GoToRealm(const char* szUrl);
	void PopAllViewPorts();
	void LoadKeyControlValues();

	// Helper for ControlThirdPerson
	void SetAvatarVelocity(MObject* pAvatar, float dx, float dy);

	// Helper for ControlThirdPerson
	void SetGoalSpot(float x, float y);

	// Helper for ControlMainMenu
	void SlideMenu(float fac);

	// Helper used by LoadFileFromUrl
	char* DownloadAndCacheFile(const char* szUrl, int* pnSize, char* szCacheName);

	void OnKeyDown(SDLKey key, SDLMod mod);

	void MakeEntropyCollectorView();
	void MakeCharSelectView();
	void BringUpMainMenu();
	void RemoveMainMenu();
	void ShutDown();
	void ReduceImageFootprint(MImageStore* pImages, MAnimationStore* pAnimations, int nAcceptableFootprint);
};

#endif // __CONTROLLER_H__
