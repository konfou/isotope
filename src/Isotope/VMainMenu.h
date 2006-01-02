/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VMAINMENU_H__
#define __VMAINMENU_H__

#include "ViewPort.h"

class MMainMenuDialog;
class Controller;
struct GRect;
class GXMLTag;

// This shows a view of server status
class VMainMenu : public ViewPort
{
protected:
	GRect* m_pClippingRect;
	MMainMenuDialog* m_pDialog;

public:
	VMainMenu(GRect* pRect, GRect* pClippingRect, GXMLTag* pAccountTag, Controller* pController);
	virtual ~VMainMenu();

	virtual void Draw(SDL_Surface *pScreen);
	virtual void OnChar(char c) {}
	virtual void OnMouseDown(int x, int y);
	virtual void OnMouseUp(int x, int y);
	virtual void OnMousePos(int x, int y);

	void RefreshEntireImage();
	void SetRect(GRect* pRect) { m_rect = *pRect; }
	void AddObject(Controller* pController);
};

#endif // __VMAINMENU_H__
