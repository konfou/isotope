/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VEntropyCollector.h"
#include "MKeyPair.h"

VEntropyCollector::VEntropyCollector(GRect* pRect, MKeyPair* pModel)
: ViewPort(pRect)
{
	m_pModel = pModel;
	m_pImage = new GImage();
	GAssert(pRect->w >= 620 && pRect->h >= 460, "Screen not big enough to hold this view");
	m_pImage->SetSize(620, 460);
	GRect r;
	r.x = 10;
	r.y = 10;
	r.w = 600;
	r.h = 25;
	m_pImage->DrawHardText(&r, "Please wiggle the mouse for a while", 0x00ffff, 1);
	r.y = 50;
	r.h = 20;
	m_pImage->DrawHardText(&r, "(We're collecting entropy needed to generate a symmetric key pair so you can digitally sign things.)", 0x00ffff, 1);
	m_pImage->DrawBox(10, 100, 10 + 1 * 600, 120, 0x00ffff, false);
}

/*virtual*/ VEntropyCollector::~VEntropyCollector()
{
	delete(m_pImage);
}

/*virtual*/ void VEntropyCollector::Draw(SDL_Surface *pScreen)
{
	double dPercent = m_pModel->GetPercent();
	m_pImage->DrawBox(11, 101, 11 + (int)(dPercent * 598), 119, 0x0000ff, true);
	if(dPercent >= 1)
	{
		GRect r;
		r.x = 10;
		r.y = 140;
		r.w = 600;
		r.h = 25;
		m_pImage->DrawHardText(&r, "Generating Keys.  Pease wait...", 0x00ffff, 1);
	}
	BlitImage(pScreen, m_rect.x, m_rect.y, m_pImage);
}
