/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VMainMenu.h"
#include "Main.h"
#include "../GClasses/GWidgets.h"
#include "Controller.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GWindows.h"
#include "../GClasses/GXML.h"

const wchar_t* g_wszColumnLabels[] =
{
	L"X",
	L"Item Name",
	L"About",
};

#define CHECK_BOX_WIDTH 25
#define ITEM_NAME_WIDTH 150
#define ABOUT_BUTTON_WIDTH 80

#define BACKGROUND_COLOR 0x445566

class MMainMenuDialog : public GWidgetDialog
{
protected:
	Controller* m_pController;
	VMainMenu* m_pView;
	GXMLTag* m_pAccountTag;
	GWidgetTextButton* m_pViewMapButton;
	GWidgetTextButton* m_pViewScriptButton;
	GWidgetTextButton* m_pToggleFullScreenButton;
	GPointerArray* m_pInventoryItems;
	GWidgetGrid* m_pInventoryWidget;

public:
	MMainMenuDialog(VMainMenu* pView, Controller* pController, int w, int h, GXMLTag* pAccountTag)
		: GWidgetDialog(w, h, BACKGROUND_COLOR)
	{
		m_pView = pView;
		m_pController = pController;
		m_pAccountTag = pAccountTag;

		// Make the buttons
		m_pViewMapButton = MakeNewButton(10, 10, 100, 24, L"View Map");
		m_pViewScriptButton = MakeNewButton(10, 40, 100, 24, L"View Script");
		m_pToggleFullScreenButton = MakeNewButton(10, 70, 100, 24, L"Toggle Full Screen");

		m_pInventoryItems = new GPointerArray(32);
		m_pInventoryWidget = new GWidgetGrid(this, m_pInventoryItems, 3, 200, 50, 300, 300);
		m_pInventoryWidget->SetColumnWidth(0, CHECK_BOX_WIDTH);
		m_pInventoryWidget->SetColumnWidth(1, ITEM_NAME_WIDTH);
		m_pInventoryWidget->SetColumnWidth(2, ABOUT_BUTTON_WIDTH);
		InitInventoryWidget();
	}

	virtual ~MMainMenuDialog()
	{
		delete(m_pInventoryItems);
	}

	/*static*/ GWidgetTextButton* MMainMenuDialog::MakeNewButton(int x, int y, int w, int h, const wchar_t* wszText)
	{
		GString sText(wszText);
		GWidgetTextButton* pNewButton = new GWidgetTextButton(this, x, y, w, h, &sText);
		return pNewButton;
	}

	void InitInventoryWidget()
	{
		// Make the column headers
		int nCols = sizeof(g_wszColumnLabels) / sizeof(const wchar_t*);
		int x, y;
		for(x = 0; x < nCols; x++)
		{
			GString s;
			s.Add(g_wszColumnLabels[x]);
			GWidgetTextButton* pNewButton = new GWidgetTextButton(m_pInventoryWidget, 0, 0, m_pInventoryWidget->GetColumnWidth(x), 20, &s);
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
				m_pInventoryWidget->SetWidget(0, y, pNewCheckBox);

				// Add a label
				GString s;
				GXMLAttribute* pAttrName = pItem->GetAttribute("Name");
				s.Add(pAttrName ? pAttrName->GetValue() : "Mysterious Object");
				GWidgetTextLabel* pNewLabel = new GWidgetTextLabel(m_pInventoryWidget, 0, 0, ITEM_NAME_WIDTH, 20, &s, 0xff8888ff);
				m_pInventoryWidget->SetWidget(1, y, pNewLabel);

				// Add a button
				s.Clear();
				s.Add(L"Details");
				GWidgetTextButton* pNewButton = new GWidgetTextButton(m_pInventoryWidget, 0, 0, ABOUT_BUTTON_WIDTH, 20, &s);
				m_pInventoryWidget->SetWidget(2, y, pNewButton);
				y++;
			}
		}
	}

	virtual void OnReleaseTextButton(GWidgetTextButton* pButton)
	{
		if(pButton == m_pToggleFullScreenButton)
			m_pController->ToggleFullScreen();
		else if(pButton == m_pViewScriptButton)
			m_pController->ViewScript();
		else if(pButton == m_pViewMapButton)
			m_pController->ViewMap();
		else
			GAssert(false, "Unrecognized button");
	}
};



// --------------------------------------------------------------------

VMainMenu::VMainMenu(GRect* pRect, GRect* pClippingRect, GXMLTag* pAccountTag, Controller* pController)
: ViewPort(pRect)
{
	m_pClippingRect = pClippingRect;
	m_pDialog = new MMainMenuDialog(this, pController, pClippingRect->w - 100, pClippingRect->h - 100, pAccountTag);

	RefreshEntireImage();
}

/*virtual*/VMainMenu::~VMainMenu()
{
	delete(m_pDialog);
}

void VMainMenu::RefreshEntireImage()
{
	GRect r;
	GImage* pImage = m_pDialog->GetImage(&r);
	pImage->DrawBox(0, 0, pImage->GetWidth() - 1, pImage->GetHeight() - 1, 0xffffff, false);
}

/*virtual*/ void VMainMenu::Draw(SDL_Surface *pScreen)
{
	GRect r;
	StretchClipAndBlitImage(pScreen, &m_rect, m_pClippingRect, m_pDialog->GetImage(&r));
}

void VMainMenu::OnMouseDown(int x, int y)
{
	x -= m_rect.x;
	y -= m_rect.y;
	GWidgetAtomic* pNewWidget = m_pDialog->FindAtomicWidget(x, y);
	m_pDialog->GrabWidget(pNewWidget, x, y);
}

void VMainMenu::OnMouseUp(int x, int y)
{
	m_pDialog->ReleaseWidget();
}

void VMainMenu::OnMousePos(int x, int y)
{
	m_pDialog->HandleMousePos(x - m_rect.x, y - m_rect.y);
}
