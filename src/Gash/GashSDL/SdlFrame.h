/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __SDLFRAME_H__
#define __SDLFRAME_H__

#include "../Include/GashEngine.h"
#include "../../GClasses/GRayTrace.h"
#include "../../SDL/SDL.h"
#include <wchar.h>

class SdlWindow;

class SdlFrame : public WrapperObject
{
public:
	SdlWindow* m_pWindow;
	GRect m_frameRect;
    SDL_Surface *m_pScreen;

	SdlFrame(Engine* pEngine, SdlWindow* pWindow);

public:
	virtual ~SdlFrame();

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
		wcscpy(pBuf, L"SDL Frame");
	}

	void drawStars(Engine* pEngine);
	void stretchBlit(Engine* pEngine, EVar* pDestRect, EVar* pSourceImage, EVar* pSourceRect);
	void blit(Engine* pEngine, EVar* pX, EVar* pY, EVar* pSourceImage, EVar* pSourceRect);

	void blit(int nDestX, int nDestY, GImage* pSrcImage, int nSourceX, int nSourceY, int nSourceW, int nSourceH);

};

#endif // __SDLFRAME_H__
