/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "../../GClasses/GImage.h"
#include "../../SDL/SDL.h"

class GWidgetListBox;
class EditorList;
class COProject;
class GWidgetStyle;
class ErrorHandler;
class EditorController;


void EditFile(const char* szFilename, const char* szAppPath, ErrorHandler* pErrorHandler);


inline Uint32* getPixMem(SDL_Surface *surface, int x, int y)
{
    return (Uint32*)((Uint8*)surface->pixels + y * surface->pitch + (x << 2));
}







#define EDITOR_VIEW_MAX_LIST_COUNT 32
#define EDITOR_VIEW_LIST_WIDTH 160
#define EDITOR_VIEW_MIN_FULL_SIZE_LISTS 3

class EditorView
{
protected:
	int m_nScreenWidth;
	int m_nScreenHeight;
	SDL_Surface* m_pScreen;
	GRect m_screenRect;
	GWidgetStyle* m_pWidgetStyle;
	EditorList* m_pLists[EDITOR_VIEW_MAX_LIST_COUNT];
	int m_nLists;
	int m_nCursorCol;
	int m_nCursorRow;

public:
	EditorView(COProject* pModel);
	~EditorView();

	void MakeRootList(EditorController* pController);
	void Update();
	void MoveRow(int dy);
	void MoveCol(int dx);
	GWidgetStyle* GetWidgetStyle() { return m_pWidgetStyle; }
	bool OnChar(char c);

protected:
	/*static*/ void BlitImage(SDL_Surface* pScreen, int x, int y, GImage* pImage);
	/*static*/ void StretchClipAndBlitImage(SDL_Surface* pScreen, GRect* pDestRect, GRect* pClipRect, GImage* pImage);
	void AjustListPositions();
	void MakeNextListBox();
	void SetScreenSize(int x, int y);
	void MakeScreenSmaller();
	void MakeScreenBigger();
	void OnSelectionChange();
};


#define KEY_REPEAT_DELAY .3
#define KEY_REPEAT_RATE .01

class EditorController
{
protected:
	enum KeyState
	{
		Normal,
		Holding,
		Repeating,
	};

	bool m_bKeepRunning;
	COProject* m_pModel;
	EditorView* m_pView;
	int m_keyboard[SDLK_LAST];
	int m_mouse[4];
	int m_mouseX;
	int m_mouseY;
	KeyState m_eKeyState;
	SDLKey m_lastPressedKey;
	double m_dKeyRepeatTimer;

public:
	EditorController(COProject* pProject);
	~EditorController();

	void Run();
	bool Update(double dTimeDelta);
	void HandleKeyPress(SDLKey eKey);
	void HandleMouseClick();
	GWidgetStyle* GetWidgetStyle() { return m_pView->GetWidgetStyle(); }
	COProject* GetProject() { return m_pModel; }
};




#endif // __EDITOR_H__
