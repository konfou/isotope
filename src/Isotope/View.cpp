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
#include "../GClasses/GXML.h"
#include "Main.h"

View::View()
{
	m_nScreenWidth = 800;
	m_nScreenHeight = 600;
	SetScreenSize(m_nScreenWidth, m_nScreenHeight);
	m_pViewPorts = new GPointerArray(8);
	m_dLastFullRefreshTime = 0;
	m_pTopView = NULL;
}

View::~View()
{
	while(m_pViewPorts->GetSize() > 0)
		delete(PopViewPort());
	delete(m_pViewPorts);
}

void View::SetScreenSize(int x, int y)
{
	// Determine whether to use full screen or windowed mode
#ifdef _DEBUG
	m_bFullScreen = false;
#else // _DEBUG
	m_bFullScreen = true;
#endif // !_DEBUG
	GXMLTag* pConfigTag = GameEngine::GetConfig();
	GXMLTag* pStartTag = pConfigTag->GetChildTag("Start");
	if(pStartTag)
	{
		GXMLAttribute* pFullScreenAttr = pStartTag->GetAttribute("fullscreen");
		if(pFullScreenAttr && stricmp(pFullScreenAttr->GetValue(), "true") != 0)
			m_bFullScreen = false;
	}

	// Make the display flags
	unsigned int flags = 
//      * Don't add SDL_HWSURFACE to these flags. Why? It may seem counter-intuitive, but
//      * it's actually faster without it because we draw each pixel individually and it
//      * takes a long time to push each pixel over the bus directly into video ram, and
//      * it's faster to let SDL just push the whole back-buffer into video ram in one shot.
//      * Besides, this flag seems to cause bad flicker problems in Windows in full-screen
//      * mode because it draw to the front-buffer instead of the back buffer. I think this
//      * is due to a bug in NVidia's driver that fails to sync up with the vertical
//      * refresh rate properly, but I'm not sure.
//		SDL_HWSURFACE |
		SDL_SWSURFACE |
//      * This flag seems to make the screen go black in Windows. I haven't yet determined
//      * whether it has any beneficial effect on Linux so it's commented out for now.
//		SDL_DOUBLEBUF |
//      * I don't know what this flag does
		SDL_ANYFORMAT;
	if(m_bFullScreen)
		flags |= SDL_FULLSCREEN;

	// Make the display
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
	m_pTopView = pViewPort;
}

int View::GetViewPortCount()
{
	return m_pViewPorts->GetSize();
}

ViewPort* View::PopViewPort()
{
	int nCount = m_pViewPorts->GetSize();
	ViewPort* pVP = (ViewPort*)m_pViewPorts->GetPointer(nCount - 1);
	m_pViewPorts->DeleteCell(nCount - 1);
	if(nCount > 1)
		m_pTopView = (ViewPort*)m_pViewPorts->GetPointer(nCount - 2);
	else
		m_pTopView = NULL;
	return pVP;
}

GRect* View::GetScreenRect()
{
    return &m_screenRect;
}

void View::OnChar(char c)
{
	m_pTopView->OnChar(c);
}

void View::OnMouseDown(int x, int y)
{
	m_pTopView->OnMouseDown(x, y);
}

void View::OnMouseUp(int x, int y)
{
	m_pTopView->OnMouseUp(x, y);
}

bool View::OnMousePos(int x, int y)
{
	m_pTopView->OnMousePos(x, y);
	return false;
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

