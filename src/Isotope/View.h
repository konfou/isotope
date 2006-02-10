/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VIEW_H__
#define __VIEW_H__

#include <SDL/SDL.h>
#include "../GClasses/GRayTrace.h"

class ViewPort;

// This class draws the main view window.  It holds a collection of view ports, which are
// all blended together like transparency slides to produce the final view that is drawn
// to the screen.
class View
{
protected:
	int m_nScreenWidth;
	int m_nScreenHeight;
	SDL_Surface* m_pScreen;
	bool m_bFullScreen;
	GRect m_screenRect;
	GPointerArray* m_pViewPorts;
	double m_dLastFullRefreshTime;
	ViewPort* m_pTopView;

public:
	View();
	~View();

	void Refresh();
	void PushViewPort(ViewPort* pViewPort);
	int GetViewPortCount();
	ViewPort* PopViewPort();
	GRect* GetScreenRect();
	void OnChar(char c);
	void OnMouseDown(int x, int y);
	void OnMouseUp(int x, int y);
	bool OnMousePos(int x, int y);
	void SetScreenSize();
	void ToggleFullScreen();
	void MakeWindowed();

protected:
	void SetVideoMode();
};

#endif // __VIEW_H__
