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
#include "../GClasses/GArray.h"
#include "../GClasses/GWindows.h"
#include "../GClasses/GXML.h"

#define BACKGROUND_COLOR 0x445566

VMainMenu::VMainMenu(GRect* pRect, GRect* pClippingRect, GXMLTag* pAccountTag)
: ViewPort(pRect)
{
	m_pClippingRect = pClippingRect;
	m_pImage = new GImage();
	m_pAccountTag = pAccountTag;
	m_pImage->SetSize(m_pClippingRect->w - 100, m_pClippingRect->h - 100);
	m_pWidgetContainer = new GWidgetContainer(pRect->w, pRect->h);
	m_pViewMapButton = MakeNewButton(m_pWidgetContainer, 10, 10, 100, 24, L"View Map");
	m_pViewScriptButton = MakeNewButton(m_pWidgetContainer, 10, 40, 100, 24, L"View Script");
	m_pScreenBiggerButton = MakeNewButton(m_pWidgetContainer, 10, 70, 100, 24, L"Screen Bigger");
	m_pScreenSmallerButton = MakeNewButton(m_pWidgetContainer, 10, 100, 100, 24, L"Screen Smaller");
	m_pTerrainButton = MakeNewButton(m_pWidgetContainer, 10, 130, 100, 24, L"Terrain On");
	m_pAddObjectButton = MakeNewButton(m_pWidgetContainer, 10, 160, 100, 24, L"Add Object");
	m_pSaveMapButton = MakeNewButton(m_pWidgetContainer, 10, 190, 100, 25, L"Save Map");

	m_pInventoryItems = new GPointerArray(32);
	m_pInventoryWidget = new GWidgetGrid(m_pWidgetContainer, m_pInventoryItems, 3, 200, 50, 300, 300);
	m_pInventoryWidget->SetColumnWidth(0, 25);
	m_pInventoryWidget->SetColumnWidth(1, 150);
	InitInventoryWidget();

	RefreshEntireImage();
}

/*virtual*/VMainMenu::~VMainMenu()
{
	delete(m_pWidgetContainer);
	delete(m_pInventoryItems);
	delete(m_pImage);
}

const wchar_t* g_wszColumnLabels[] =
{
	L"X",
	L"Item Name",
	L"About",
};

void VMainMenu::InitInventoryWidget()
{
	// Make the column headers
	int nCols = sizeof(g_wszColumnLabels) / sizeof(const wchar_t*);
	int x, y;
	for(x = 0; x < nCols; x++)
	{
		GString s;
		s.Add(g_wszColumnLabels[x]);
		GWidgetTextButton* pNewButton = new GWidgetTextButton(m_pInventoryWidget, 0, 0, m_pInventoryWidget->GetColumnWidth(x), 20, &s);
		pNewButton->Update();
		m_pInventoryWidget->SetColumnHeader(x, pNewButton);
	}

	// Make the inventory items
	GXMLTag* pInventoryTag = m_pAccountTag->GetChildTag("Inventory");
	if(pInventoryTag)
	{
		GXMLTag* pItem;
		y = 0;
		for(pItem = pInventoryTag->GetFirstChildTag(); pItem; pItem = pInventoryTag->GetNextChildTag(pItem))
		{
			m_pInventoryWidget->AddBlankRow();

			// Add a check box
			GWidgetCheckBox* pNewCheckBox = new GWidgetCheckBox(m_pInventoryWidget, 0, 0, 17, 17);
			pNewCheckBox->Update();
			m_pInventoryWidget->SetWidget(0, y, pNewCheckBox);

			// Add a label
			GString s;
			GXMLAttribute* pAttrName = pItem->GetAttribute("Name");
			s.Add(pAttrName ? pAttrName->GetValue() : "Mysterious Object");
			GWidgetTextLabel* pNewLabel = new GWidgetTextLabel(m_pInventoryWidget, 0, 0, 80, 20, &s);
			pNewLabel->Update();
			m_pInventoryWidget->SetWidget(1, y, pNewLabel);

			// Add a button
			s.Clear();
			s.Add(L"Details");
			GWidgetTextButton* pNewButton = new GWidgetTextButton(m_pInventoryWidget, 0, 0, 80, 20, &s);
			pNewButton->Update();
			m_pInventoryWidget->SetWidget(2, y, pNewButton);
			y++;
		}
	}
	m_pInventoryWidget->UpdateAll();
}

void VMainMenu::RefreshEntireImage()
{
	m_pImage->Clear(BACKGROUND_COLOR);
	m_pImage->DrawBox(0, 0, m_pImage->GetWidth() - 1, m_pImage->GetHeight() - 1, 0xffffff, false);
	m_pViewMapButton->Draw(m_pImage);
	m_pViewScriptButton->Draw(m_pImage);
	m_pScreenBiggerButton->Draw(m_pImage);
	m_pScreenSmallerButton->Draw(m_pImage);
	m_pTerrainButton->Draw(m_pImage);
	m_pAddObjectButton->Draw(m_pImage);
	m_pSaveMapButton->Draw(m_pImage);
	m_pInventoryWidget->Draw(m_pImage);
}

/*virtual*/ void VMainMenu::Draw(SDL_Surface *pScreen)
{
	StretchClipAndBlitImage(pScreen, &m_rect, m_pClippingRect, m_pImage);
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

void VMainMenu::ReleaseButton(Controller* pController, GWidgetTextButton* pButton)
{
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
	//else
	//	GAssert(false, "Unrecognized button");
}

void VMainMenu::OnMouseDown(Controller* pController, int x, int y)
{
	x -= m_rect.x;
	y -= m_rect.y;
	GWidgetAtomic* pNewWidget = m_pWidgetContainer->FindAtomicWidget(x, y);
	if(!pNewWidget)
		return;
	GWidgetAtomic* pOldWidget = m_pWidgetContainer->GetGrabbedWidget();
	if(pOldWidget == pNewWidget)
		pOldWidget = NULL;
	m_pWidgetContainer->GrabWidget(pNewWidget);
	if(pOldWidget)
		pOldWidget->Draw(m_pImage);
	pNewWidget->Draw(m_pImage);
}

void VMainMenu::OnMouseUp(Controller* pController, int x, int y)
{
	x -= m_rect.x;
	y -= m_rect.y;
	GWidgetAtomic* pOldWidget = m_pWidgetContainer->GetGrabbedWidget();
	m_pWidgetContainer->ReleaseWidget();
	if(!pOldWidget)
		return;
	pOldWidget->Draw(m_pImage);
	switch(pOldWidget->GetType())
	{
		case GWidget::TextButton:
			ReleaseButton(pController, (GWidgetTextButton*)pOldWidget);
			break;
	}
}
