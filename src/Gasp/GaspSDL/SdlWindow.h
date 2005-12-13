/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __SDLWINDOW_H__
#define __SDLWINDOW_H__

#include "../Include/GaspEngine.h"
#include "../../GClasses/GRayTrace.h"
#include "../../SDL/SDL.h"
#include <wchar.h>

class SdlWindow : public WrapperObject
{
friend class SdlFrame;
protected:
	int m_keyboard[SDLK_LAST];
    SDL_Surface* m_pScreen;
	GRect m_screenRect;
	bool m_bLocked;

	SdlWindow(Engine* pEngine);

public:
	virtual ~SdlWindow();

	void toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
	{
		GAssert(false, "todo: write me");
	}

	void fromStream(Engine* pEngine, EVar* pStream)
	{
		GAssert(false, "todo: write me");
	}

	void setRefs(Engine* pEngine, EVar* pRefs)
	{
		GAssert(false, "todo: write me");
	}

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		GAssert(nSize > 32, "Buffer too small");
		wcscpy(pBuf, L"SDL Window");
	}

	void allocate(Engine* pEngine)
	{
		pEngine->SetThis(new SdlWindow(pEngine));
	}

	void getNextFrame(Engine* pEngine, EVar* pFrame);

	void update();

};

#endif // __SDLWINDOW_H__
