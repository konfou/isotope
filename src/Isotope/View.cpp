/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "View.h"
#include "ViewPort.h"
#include "../GClasses/GArray.h"
#include "GameEngine.h"

View::View()
{
	m_nScreenWidth = 800;
	m_nScreenHeight = 600;
	SetScreenSize(m_nScreenWidth, m_nScreenHeight);
	m_pViewPorts = new GPointerArray(8);
	m_dLastFullRefreshTime = 0;
}

View::~View()
{
	while(m_pViewPorts->GetSize() > 0)
		delete(PopViewPort());
	delete(m_pViewPorts);
}

void View::SetScreenSize(int x, int y)
{
#ifdef WIN32
	m_bFullScreen = false;
#else // WIN32
	m_bFullScreen = false;
#endif // !WIN32
	unsigned int flags = 
		SDL_HWSURFACE |
		SDL_ANYFORMAT;
	if(m_bFullScreen)
		flags |= SDL_FULLSCREEN;
#ifdef WIN32
	else
#endif // WIN32
		flags |= SDL_DOUBLEBUF; // There's a bug in SDL where double-buffering doesn't work with full screen mode on Windows
	m_pScreen = SDL_SetVideoMode(x, y, 32, flags);
	if(!m_pScreen)
	{
		GAssert(false, SDL_GetError());
		GameEngine::ThrowError("failed to create SDL screen");
	}
	m_screenRect.x = 5;
	m_screenRect.y = 5;
	m_screenRect.w = m_pScreen->w - 10;
	m_screenRect.h = m_pScreen->h - 10;
}

void View::MakeScreenSmaller()
{
	m_nScreenWidth = (int)(m_nScreenWidth / 1.25);
	m_nScreenHeight = (int)(m_nScreenHeight / 1.25);
	SetScreenSize(m_nScreenWidth, m_nScreenHeight);
}

void View::MakeScreenBigger()
{
	m_nScreenWidth = (int)(m_nScreenWidth * 1.25);
	m_nScreenHeight = (int)(m_nScreenHeight* 1.25);
	SetScreenSize(m_nScreenWidth, m_nScreenHeight);
}

void View::PushViewPort(ViewPort* pViewPort)
{
	m_pViewPorts->AddPointer(pViewPort);
}

int View::GetViewPortCount()
{
	return m_pViewPorts->GetSize();
}

ViewPort* View::PopViewPort()
{
	ViewPort* pVP = (ViewPort*)m_pViewPorts->GetPointer(m_pViewPorts->GetSize() - 1);
	m_pViewPorts->DeleteCell(m_pViewPorts->GetSize() - 1);
	return pVP;
}

GRect* View::GetScreenRect()
{
    return &m_screenRect;
}

void View::Refresh()
{
	// Lock the screen for direct access to the pixels
    SDL_Surface *pScreen = m_pScreen;
	if ( SDL_MUSTLOCK(pScreen) )
	{
		if ( SDL_LockSurface(pScreen) < 0 )
		{
			if(m_bFullScreen)
			{
				SDL_WM_ToggleFullScreen(pScreen);
				m_bFullScreen = false;
			}
			GameEngine::ThrowError(SDL_GetError()); // failed to lock the surface
			return;
		}
	}

	// Clear the screen
    /*SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = m_screenRect.w;
	r.h = m_screenRect.h;*/
	//SDL_FillRect(pScreen, NULL/*&r*/, 0x000000);

	// Draw all the view ports
	int n;
	int nCount = m_pViewPorts->GetSize();

	// Decide whether to refresh all view ports or just the top-most one
	n = nCount - 1;
	double d = GameEngine::GetTime();
	if(d - m_dLastFullRefreshTime > .3)
	{
		m_dLastFullRefreshTime = d;
		n = 0;
	}

	// Refresh the view ports
	for(; n < nCount; n++)
	{
		ViewPort* pViewPort = (ViewPort*)m_pViewPorts->GetPointer(n);
		pViewPort->Draw(pScreen);
	}

	// Unlock the screen
	if ( SDL_MUSTLOCK(pScreen) )
		SDL_UnlockSurface(pScreen);

	// Update the whole screen
	SDL_UpdateRect(pScreen, m_screenRect.x, m_screenRect.y, m_screenRect.w, m_screenRect.h);
}

