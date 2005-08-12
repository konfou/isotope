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

VLoading::VLoading(GRect* pRect)
: ViewPort(pRect)
{
	m_pImage = new GImage();
	GAssert(pRect->w >= 620 && pRect->h >= 460, "Screen not big enough to hold this view");
	m_pImage->SetSize(620, 460);

	RefreshEntireImage();
}

/*virtual*/ VLoading::~VLoading()
{
	delete(m_pImage);
}

/*virtual*/ void VLoading::Draw(SDL_Surface *pScreen)
{
	BlitImage(pScreen, m_rect.x, m_rect.y, m_pImage);
}

void VLoading::RefreshEntireImage()
{
	m_pImage->Clear(0x002244);

	// Draw the background image
	MImageStore* pGlobalImageStore = GameEngine::GetGlobalImageStore();
	VarHolder* pVH = pGlobalImageStore->GetVarHolder("loading");
	if(pVH)
	{
		MGameImage* pGameImage = (MGameImage*)pVH->GetGObject();
		GRect r;
		r.x = 0;
		r.y = 0;
		r.w = 620;
		r.h = 460;
		m_pImage->Blit(0, 0, &pGameImage->m_value, &r);
	}
}
