/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VError.h"
#include "GameEngine.h"
#include "../GClasses/GString.h"
#include "../GClasses/GWidgets.h"

class VErrorDialog : public GWidgetDialog
{
protected:
	GWidgetTextLabel* m_pErrorMessage;

public:
	VErrorDialog(int w, int h, const char* szError)
		: GWidgetDialog(w, h, 0xffffffaa)
	{
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
	}

	virtual ~VErrorDialog()
	{
	}
};







VError::VError(GRect* pRect, const char* szErrorMessage)
: ViewPort(pRect)
{
	m_pImage = new GImage();
	GAssert(pRect->w >= 630 && pRect->h >= 470, "Screen not big enough to hold this view");
	m_pImage->SetSize(630, 470);
	m_nLeft = (pRect->w - 630) / 2 + pRect->x;
	m_nTop = (pRect->h - 470) / 2 + pRect->y;

	m_pDialog = new VErrorDialog(pRect->w, pRect->h, szErrorMessage);
	m_dirty = true;

	RefreshEntireImage();
}

/*virtual*/ VError::~VError()
{
	delete(m_pDialog);
	delete(m_pImage);
}

/*virtual*/ void VError::Draw(SDL_Surface *pScreen)
{
	if(m_dirty)
	{
		m_dirty = false;
		GRect r;
		GImage* pCanvas = m_pDialog->GetImage(&r);
		m_pImage->Blit(0, 0, pCanvas, &r);
	}
	BlitImage(pScreen, m_nLeft, m_nTop, m_pImage);
}

void VError::RefreshEntireImage()
{
/*
	m_pImage->Clear(0xffffffaa);

	// Draw the background image
	MImageStore* pGlobalImageStore = GameEngine::GetGlobalImageStore();
	int nIndex = pGlobalImageStore->GetIndex("loading");
	VarHolder* pVH = pGlobalImageStore->GetVarHolder(nIndex);
	if(pVH)
	{
		MGameImage* pGameImage = (MGameImage*)pVH->GetGObject();
		GRect r;
		r.x = 0;
		r.y = 0;
		r.w = 630;
		r.h = 470;
		m_pImage->Blit(0, 0, &pGameImage->m_value, &r);
	}
*/
	m_dirty = true;
}
