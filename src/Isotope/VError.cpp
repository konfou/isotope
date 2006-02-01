/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VError.h"
#include "Main.h"
#include "../GClasses/GString.h"
#include "../GClasses/GWidgets.h"
#include "Controller.h"

class VErrorDialog : public GWidgetDialog
{
protected:
	GWidgetTextLabel* m_pErrorMessage;
	GWidgetTextButton* m_pBackButton;
	Controller* m_pController;

public:
	VErrorDialog(Controller* pController, int w, int h, const char* szError)
		: GWidgetDialog(w, h, 0xffffffaa)
	{
		m_pController = pController;
		int nMaxLineLength = 70;
		char* szLine = (char*)alloca(nMaxLineLength + 1);
		int nStart;
		int nEnd = 0;
		int y = 2;
		int nLineHeight = 24;
		GString sLine;
		while(true)
		{
			// Move forward nMaxLineLength characters
			nStart = nEnd;
			while(szError[nStart] == ' ')
				nStart++;
			for(nEnd = nStart; nEnd - nStart < nMaxLineLength && szError[nEnd] != '\0'; nEnd++)
			{
			}

			// Back up until we find a word break
			int nLongEnd = nEnd;
			while(nEnd > nStart + nMaxLineLength / 2 && szError[nEnd] > ' ')
				nEnd--;
			if(nEnd <= nStart + nMaxLineLength / 2)
				nEnd = nLongEnd;
			GAssert(nEnd - nStart <= nMaxLineLength, "out of range");

			// Extract the substring
			memcpy(szLine, &szError[nStart], nEnd - nStart);
			szLine[nEnd - nStart] = '\0';
			sLine.Copy(szLine);

			// Print it
			GWidgetTextLabel* pLine = new GWidgetTextLabel(this, 2, y, w - 38, nLineHeight, &sLine, 0xff331100);
			y += nLineHeight;
			if(szError[nEnd] == '\0')
				break;
		}

		GString s;
		s.Copy(L"Back");
		m_pBackButton = new GWidgetTextButton(this, (w - 80) / 2, h - 40, 80, 20, &s);
	}

	virtual ~VErrorDialog()
	{
	}

	virtual void OnReleaseTextButton(GWidgetTextButton* pButton)
	{
		if(pButton == m_pBackButton)
			m_pController->GoBack();
		else
			GAssert(false, "Unexpected button");
	}
};





// ----------------------------------------------------------------------

VError::VError(Controller* pController, GRect* pRect, const char* szErrorMessage)
: ViewPort(pRect)
{
	GAssert(pRect->w >= 630 && pRect->h >= 470, "Screen not big enough to hold this view");
	m_nLeft = (pRect->w - 630) / 2 + pRect->x;
	m_nTop = (pRect->h - 470) / 2 + pRect->y;

	m_pDialog = new VErrorDialog(pController, 630, 470, szErrorMessage);

	RefreshEntireImage();
}

/*virtual*/ VError::~VError()
{
	delete(m_pDialog);
}

/*virtual*/ void VError::Draw(SDL_Surface *pScreen)
{
	GRect r;
	GImage* pCanvas = m_pDialog->GetImage(&r);
	BlitImage(pScreen, m_nLeft, m_nTop, pCanvas);
}

void VError::RefreshEntireImage()
{
	GRect r;
	GImage* pCanvas = m_pDialog->GetImage(&r);
	pCanvas->DrawBox(1, 1, pCanvas->GetWidth() - 2, pCanvas->GetHeight() - 2, 0xff886600, false);
}

void VError::OnMouseDown(int x, int y)
{
	x -= m_nLeft;
	y -= m_nTop;
	GWidgetAtomic* pNewWidget = m_pDialog->FindAtomicWidget(x, y);
	m_pDialog->GrabWidget(pNewWidget, x, y);
}

void VError::OnMouseUp(int x, int y)
{
	m_pDialog->ReleaseWidget();
}

void VError::OnMousePos(int x, int y)
{
	m_pDialog->HandleMousePos(x - m_nLeft, y - m_nTop);
}
