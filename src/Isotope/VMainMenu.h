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

class GWidgetContainer;
class GWidgetTextButton;
class GWidget;
class GWidgetAtomic;
class Controller;
struct GRect;
class GPointerArray;
class GWidgetGrid;
class GXMLTag;

// This shows a view of server status
class VMainMenu : public ViewPort
{
protected:
	GRect* m_pClippingRect;
	GImage* m_pImage;
	GXMLTag* m_pAccountTag;
	GWidgetContainer* m_pWidgetContainer;
	GWidgetTextButton* m_pViewMapButton;
	GWidgetTextButton* m_pViewScriptButton;
	GWidgetTextButton* m_pScreenBiggerButton;
	GWidgetTextButton* m_pScreenSmallerButton;
	GWidgetTextButton* m_pTerrainButton;
	GWidgetTextButton* m_pAddObjectButton;
	GWidgetTextButton* m_pSaveMapButton;
	
	GPointerArray* m_pInventoryItems;
	GWidgetGrid* m_pInventoryWidget;

public:
	VMainMenu(GRect* pRect, GRect* pClippingRect, GXMLTag* pAccountTag);
	virtual ~VMainMenu();

	virtual void Draw(SDL_Surface *pScreen);
	void RefreshEntireImage();

	void OnMouseDown(Controller* pController, int x, int y);
	void OnMouseUp(Controller* pController, int x, int y);
	void SetRect(GRect* pRect) { m_rect = *pRect; }

protected:
	void ReleaseButton(Controller* pController, GWidgetTextButton* pButton);
	void AddObject(Controller* pController);
	void InitInventoryWidget();
};

#endif // __VMAINMENU_H__
