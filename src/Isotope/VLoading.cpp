/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VLoading.h"
#include "MStore.h"
#include "GameEngine.h"
#include "MGameImage.h"
#include "../GClasses/GString.h"
#include "../GClasses/GWidgets.h"

class MLoadingDialog : public GWidgetDialog
{
public:
	GWidgetTextLabel* m_pCurrentFile;
	GWidgetProgressBar* m_pProgressBar;

public:
	MLoadingDialog(int w, int h, const char* szUrl)
		: GWidgetDialog(w, h, 0xff008800)
	{
		GString s;
		s.Copy(szUrl);
		m_pCurrentFile = new GWidgetTextLabel(this, 2, 2, w - 4, 20, &s, false);
		m_pProgressBar = new GWidgetProgressBar(this, 2, 24, w - 4, 15);
	}

	virtual ~MLoadingDialog()
	{
	}
};







#define FUDGE_AMOUNT .1
#define FUDGE_RATE .02

VLoading::VLoading(GRect* pRect, const char* szUrl)
: ViewPort(pRect)
{
	m_pImage = new GImage();
	GAssert(pRect->w >= 620 && pRect->h >= 460, "Screen not big enough to hold this view");
	m_pImage->SetSize(630, 470);
	m_nLeft = (pRect->w - 630) / 2 + pRect->x;
	m_nTop = (pRect->h - 470) / 2 + pRect->y;

	m_pDialog = new MLoadingDialog(570, 41, szUrl);
	m_dirty = true;
	m_fudgeFactor = 0;
	m_dPrevProgressTime = 0;

	RefreshEntireImage();
}

/*virtual*/ VLoading::~VLoading()
{
	delete(m_pDialog);
	delete(m_pImage);
}

/*virtual*/ void VLoading::Draw(SDL_Surface *pScreen)
{
	if(m_dirty)
	{
		m_dirty = false;
		GRect r;
		GImage* pCanvas = m_pDialog->GetImage(&r);
		m_pImage->Blit(30, 100, pCanvas, &r);
	}
	BlitImage(pScreen, m_nLeft, m_nTop, m_pImage);
}

void VLoading::SetUrl(const char* szUrl)
{
	m_pDialog->m_pCurrentFile->SetText(szUrl);
	m_pDialog->m_pProgressBar->SetProgress(0);
	m_fudgeFactor = 0;
	m_dPrevProgressTime = 0;
	m_dirty = true;
}

void VLoading::SetProgress(float f)
{
	float fPrevProgress = m_pDialog->m_pProgressBar->GetProgress();
	double dTime = GameEngine::GetTime();
	if(f > fPrevProgress)
	{
		m_fudgeFactor -= (f - fPrevProgress);
		if(m_fudgeFactor < 0)
			m_fudgeFactor = 0;
	}
	else
	{
		if(m_dPrevProgressTime > 0)
		{
			m_fudgeFactor += ((float)FUDGE_RATE * (float)(dTime - m_dPrevProgressTime));
			if(m_fudgeFactor > (float)FUDGE_AMOUNT)
				m_fudgeFactor = (float)FUDGE_AMOUNT;
		}
	}
	m_dPrevProgressTime = dTime;
	m_pDialog->m_pProgressBar->SetProgress(MIN(f + m_fudgeFactor, (float)1));
	m_dirty = true;
}

void VLoading::RefreshEntireImage()
{
	m_pImage->Clear(0x002244);

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

	m_dirty = true;
}
