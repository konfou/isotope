/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "SdlWindow.h"
#include "SdlFrame.h"
#include "../../GClasses/GHashTable.h"

void RegisterSdlWindow(GConstStringHashTable* pTable)
{
	pTable->Add("method !new()", new EMethodPointerHolder((MachineMethod0)&SdlWindow::allocate));
	pTable->Add("method getNextFrame(!SdlFrame)", new EMethodPointerHolder((MachineMethod1)&SdlWindow::getNextFrame));
}


SdlWindow::SdlWindow(Engine* pEngine)
	: WrapperObject(pEngine, "SdlWindow")
{
	// Keyboard
	memset(m_keyboard, '\0', sizeof(int) * SDLK_LAST);

	// Screen
	m_pScreen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE|SDL_ANYFORMAT|SDL_DOUBLEBUF);
	if(!m_pScreen)
	{
		const char* szError = SDL_GetError();
		GAssert(false, szError);
		throw "failed to create SDL screen";
	}
	m_screenRect.x = 0;
	m_screenRect.y = 0;
	m_screenRect.w = m_pScreen->w - 5;
	m_screenRect.h = m_pScreen->h - 5;

	// Misc
	m_bLocked = false;
}

SdlWindow::~SdlWindow()
{
}

void SdlWindow::getNextFrame(Engine* pEngine, EVar* pFrame)
{
	// Lock the screen
	if(m_bLocked)
		pEngine->ThrowSdlError(L"You're still referencing another frame.  You can only have one at a time.");
	if(SDL_MUSTLOCK(m_pScreen))
	{
		if(SDL_LockSurface(m_pScreen) < 0)
			pEngine->ThrowSdlError(L"Failed to lock the surface.");
	}
	m_bLocked = true;

	// Allocate the frame
	pEngine->SetVar(pFrame, new SdlFrame(pEngine, this));
}

void SdlWindow::update()
{
	// Unlock the screen
	m_bLocked = false;
	if ( SDL_MUSTLOCK(m_pScreen) )
		SDL_UnlockSurface(m_pScreen);

	// Update the whole screen
	SDL_UpdateRect(m_pScreen, m_screenRect.x, m_screenRect.y, m_screenRect.w, m_screenRect.h);
}
