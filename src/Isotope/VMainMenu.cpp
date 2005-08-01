/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VMainMenu.h"
#include "GameEngine.h"
#include "../GClasses/GWidgets.h"
#include "Controller.h"
#include "../GClasses/GWindows.h"

GWidgetButton* MakeNewButton(GWidgetStyle* pStyle, int x, int y, int w, int h, const wchar_t* wszText)
{
	GString sText(wszText);
	GWidgetButton* pNewButton = new GWidgetButton(pStyle, x, y, w, h, &sText);
	pStyle->AddWidget(pNewButton);
	return pNewButton;
}

VMainMenu::VMainMenu(GRect* pRect, GRect* pClippingRect)
: ViewPort(pRect)
{
	m_pClippingRect = pClippingRect;
	m_pImage = new GImage();
	m_pImage->SetSize(m_pClippingRect->w - 100, m_pClippingRect->h - 100);
	m_pWidgetStyle = new GWidgetStyle();
	m_pViewMapButton = MakeNewButton(m_pWidgetStyle, 10, 10, 100, 24, L"View Map");
	m_pViewScriptButton = MakeNewButton(m_pWidgetStyle, 10, 40, 100, 24, L"View Script");
	m_pScreenBiggerButton = MakeNewButton(m_pWidgetStyle, 10, 70, 100, 24, L"Screen Bigger");
	m_pScreenSmallerButton = MakeNewButton(m_pWidgetStyle, 10, 100, 100, 24, L"Screen Smaller");
	m_pTerrainButton = MakeNewButton(m_pWidgetStyle, 10, 130, 100, 24, L"Terrain On");
	m_pAddObjectButton = MakeNewButton(m_pWidgetStyle, 10, 160, 100, 24, L"Add Object");
	m_pSaveMapButton = MakeNewButton(m_pWidgetStyle, 10, 190, 100, 25, L"Save Map");
	m_pViewMapButton->Update();
	m_pViewScriptButton->Update();
	m_pScreenBiggerButton->Update();
	m_pScreenSmallerButton->Update();
	m_pTerrainButton->Update();
	m_pAddObjectButton->Update();
	m_pSaveMapButton->Update();
	RefreshEntireImage();
	m_pClickWidget = NULL;
}

/*virtual*/VMainMenu::~VMainMenu()
{
	delete(m_pWidgetStyle);
	delete(m_pImage);
}

void VMainMenu::RefreshEntireImage()
{
	m_pImage->Clear(0x445566);
	m_pImage->DrawBox(0, 0, m_pImage->GetWidth() - 1, m_pImage->GetHeight() - 1, 0xffffff, false);
	m_pViewMapButton->Draw(m_pImage);
	m_pViewScriptButton->Draw(m_pImage);
	m_pScreenBiggerButton->Draw(m_pImage);
	m_pScreenSmallerButton->Draw(m_pImage);
	m_pTerrainButton->Draw(m_pImage);
	m_pAddObjectButton->Draw(m_pImage);
	m_pSaveMapButton->Draw(m_pImage);
}

/*virtual*/ void VMainMenu::Draw(SDL_Surface *pScreen)
{
	StretchClipAndBlitImage(pScreen, &m_rect, m_pClippingRect, m_pImage);
}

void VMainMenu::PressButton(GWidgetButton* pButton)
{
	pButton->SetPressed(true);
	pButton->Update();
	pButton->Draw(m_pImage);
	// todo: Refresh this portion of the view port now so that the reaction time
	//       will feel snappy.  Currently it waits until the next call to View::Update
}

void VMainMenu::AddObject(Controller* pController)
{
#ifdef WIN32
	char szBuf[512];
	if(GWindows::GetOpenFilename(NULL, "Select an image", "*.bmp", szBuf) != IDOK)
		return;
	pController->AddObject(szBuf);
#else // WIN32
	GAssert(false, "Sorry, this feature not supported in Linux yet");
#endif // !WIN32
}

void VMainMenu::ReleaseButton(Controller* pController, GWidgetButton* pButton)
{
	if(!pButton->IsPressed())
		return; // The user moved the mouse to another button while holding down the mouse button

	// Unpress the button
	pButton->SetPressed(false);
	pButton->Update();
	pButton->Draw(m_pImage);

	// Do the action
	if(pButton == m_pTerrainButton)
		pController->ToggleTerrain();
	else if(pButton == m_pScreenSmallerButton)
		pController->MakeScreenSmaller();
	else if(pButton == m_pScreenBiggerButton)
		pController->MakeScreenBigger();
	else if(pButton == m_pViewScriptButton)
		pController->ViewScript();
	else if(pButton == m_pViewMapButton)
		pController->ViewMap();
	else if(pButton == m_pAddObjectButton)
		AddObject(pController);
	else if(pButton == m_pSaveMapButton)
	{
	}
	else
		GAssert(false, "Unrecognized button");
}

void VMainMenu::OnMouseDown(Controller* pController, int x, int y)
{
	x -= m_rect.x;
	y -= m_rect.y;
	m_pClickWidget = m_pWidgetStyle->FindWidget(x, y);
	if(!m_pClickWidget)
		return;
	switch(m_pClickWidget->GetType())
	{
		case GWidget::Button:
			PressButton((GWidgetButton*)m_pClickWidget);
			break;
	}
}

void VMainMenu::OnMouseUp(Controller* pController, int x, int y)
{
	x -= m_rect.x;
	y -= m_rect.y;
	m_pClickWidget = m_pWidgetStyle->FindWidget(x, y);
	if(!m_pClickWidget)
		return;
	switch(m_pClickWidget->GetType())
	{
		case GWidget::Button:
			ReleaseButton(pController, (GWidgetButton*)m_pClickWidget);
			break;
	}
}
