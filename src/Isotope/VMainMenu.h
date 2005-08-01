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

class GWidgetStyle;
class GWidgetButton;
class GWidget;
class Controller;
struct GRect;

// This shows a view of server status
class VMainMenu : public ViewPort
{
protected:
	GRect* m_pClippingRect;
	GImage* m_pImage;
	GWidgetStyle* m_pWidgetStyle;
	GWidgetButton* m_pViewMapButton;
	GWidgetButton* m_pViewScriptButton;
	GWidgetButton* m_pScreenBiggerButton;
	GWidgetButton* m_pScreenSmallerButton;
	GWidgetButton* m_pTerrainButton;
	GWidgetButton* m_pAddObjectButton;
	GWidgetButton* m_pSaveMapButton;
	GWidget* m_pClickWidget;

public:
	VMainMenu(GRect* pRect, GRect* pClippingRect);
	virtual ~VMainMenu();

	virtual void Draw(SDL_Surface *pScreen);
	void RefreshEntireImage();

	void OnMouseDown(Controller* pController, int x, int y);
	void OnMouseUp(Controller* pController, int x, int y);
	void SetRect(GRect* pRect) { m_rect = *pRect; }

protected:
	void PressButton(GWidgetButton* pButton);
	void ReleaseButton(Controller* pController, GWidgetButton* pButton);
	void AddObject(Controller* pController);
};

#endif // __VMAINMENU_H__
