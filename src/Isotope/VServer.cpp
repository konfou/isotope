/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VServer.h"
#include "MRealmServer.h"
#include "Main.h"

VServer::VServer(GRect* pRect, MGameServer* pServer)
: ViewPort(pRect)
{
	m_pImage = new GImage();
	GAssert(pRect->w >= 620 && pRect->h >= 460, "Screen not big enough to hold this view");
	m_pImage->SetSize(620, 460);
	m_pServer = pServer;
	int nWidth = m_pImage->GetWidth();
	m_pHistory = new float[nWidth];
	int n;
	for(n = 0; n < nWidth; n++)
		m_pHistory[n] = 0;
	m_nHistoryPos = 0;
	m_time = 0;
}

/*virtual*/VServer::~VServer()
{
	delete(m_pHistory);
}

/*virtual*/ void VServer::Draw(SDL_Surface *pScreen)
{
	// See if it's time to refresh yet
	double time = GameEngine::GetTime();
	if(time - m_time < 4) // todo: unmagic this time interval
		return;
	m_time = time;

	// Check the server load
	m_pHistory[m_nHistoryPos] = m_pServer->MeasureLoad();
	m_nHistoryPos++;
	int nWidth = m_pImage->GetWidth();
	int nHeight = m_pImage->GetHeight();
	if(m_nHistoryPos >= nWidth)
		m_nHistoryPos = 0;

	// Clear the screen
	m_pImage->Clear(0x000000);

	// Draw the load chart
	float loadPrevPrev = 0;
	float loadPrev = 0;
	float load = 0;
	int nPos = m_nHistoryPos;
	int x, height;
	for(x = 0; x < nWidth; x++)
	{
		loadPrevPrev = loadPrev;
		loadPrev = load;
		load = m_pHistory[nPos];
		height = (int)((load + loadPrev + loadPrevPrev) * (nHeight - 1) / 3);
		int bot = m_rect.y + m_rect.h - 1;
		m_pImage->DrawLine(x, nHeight - 1, x, nHeight - 1 - height, 0xffff);
		nPos++;
		if(nPos >= nWidth)
			nPos = 0;
	}

	BlitImage(pScreen, m_rect.x, m_rect.y, m_pImage);
}
