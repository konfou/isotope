/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "SdlFrame.h"
#include "SdlWindow.h"
#include "../../GClasses/GHashTable.h"
#include "../MachineObjects/MImage.h"
#include <stdlib.h>

void RegisterSdlFrame(GConstStringHashTable* pTable)
{
	pTable->Add("method &drawStars()", new EMethodPointerHolder((MachineMethod0)&SdlFrame::drawStars));
	pTable->Add("method &stretchBlit(Rect, Image, Rect)", new EMethodPointerHolder((MachineMethod3)&SdlFrame::stretchBlit));
}


SdlFrame::SdlFrame(Engine* pEngine, SdlWindow* pWindow)
	: WrapperObject(pEngine, "SdlFrame")
{
	m_pWindow = pWindow;
	m_frameRect = pWindow->m_screenRect;
	m_pScreen = pWindow->m_pScreen;
}

SdlFrame::~SdlFrame()
{
	m_pWindow->update();
}

inline void putpixel32(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
    *(Uint32*)((Uint8*)surface->pixels + y * surface->pitch + (x << 2)) = pixel;
}

void SdlFrame::drawStars(Engine* pEngine)
{
	// Draw stars
	int n;
	for(n = 0; n < 200; n++)
	{
		int x = rand() % m_frameRect.w;
		int y = rand() % m_frameRect.h;
		GColor pix = (rand() * rand()) % 0xffffff;
		putpixel32(m_pScreen, x, y, pix);
	}
}

void SdlFrame::stretchBlit(Engine* pEngine, EVar* pDestRect, EVar* pSourceImage, EVar* pSourceRect)
{
	SDL_Surface* pSurface = m_pWindow->m_pScreen;
	GImage* pSrcImage = &((MImage*)pSourceImage->pOb)->m_value;
	int nDestX = ((IntObject*)pDestRect->pObjectObject->arrFields[0])->m_value;
	int nDestY = ((IntObject*)pDestRect->pObjectObject->arrFields[1])->m_value;
	int nDestW = ((IntObject*)pDestRect->pObjectObject->arrFields[2])->m_value;
	int nDestH = ((IntObject*)pDestRect->pObjectObject->arrFields[3])->m_value;
	int nSourceX = ((IntObject*)pSourceRect->pObjectObject->arrFields[0])->m_value;
	int nSourceY = ((IntObject*)pSourceRect->pObjectObject->arrFields[1])->m_value;
	int nSourceW = ((IntObject*)pSourceRect->pObjectObject->arrFields[2])->m_value;
	int nSourceH = ((IntObject*)pSourceRect->pObjectObject->arrFields[3])->m_value;
	float fDeltaY = (float)nSourceH / (float)nDestH;
	float fDeltaX = (float)nSourceW / (float)nDestW;
	float fSrcY = (float)nSourceY;
	float fSrcX;
	float fStartX = (float)nSourceX;
	int nFinishY = nDestY + nDestH;
	int nFinishX = nDestX + nDestW;
	int x, y;
	GColor pix;
	for(y = nDestY; y < nFinishY; y++)
	{
		fSrcX = fStartX;
		for(x = nDestX; x < nFinishX; x++)
		{
			pix = pSrcImage->GetPixel((int)fSrcX, (int)fSrcY);
			if(pix) // todo: use sprite's transparent background color
				putpixel32(pSurface, x, y, pix);
			fSrcX += fDeltaX;
		}
		fSrcY += fDeltaY;
	}
}

void SdlFrame::blit(Engine* pEngine, EVar* pX, EVar* pY, EVar* pSourceImage, EVar* pSourceRect)
{
	GImage* pSrcImage = &((MImage*)pSourceImage->pOb)->m_value;
	int nDestX = pX->pIntObject->m_value;
	int nDestY = pY->pIntObject->m_value;
	int nSourceX = ((IntObject*)pSourceRect->pObjectObject->arrFields[0])->m_value;
	int nSourceY = ((IntObject*)pSourceRect->pObjectObject->arrFields[1])->m_value;
	int nSourceW = ((IntObject*)pSourceRect->pObjectObject->arrFields[2])->m_value;
	int nSourceH = ((IntObject*)pSourceRect->pObjectObject->arrFields[3])->m_value;
	blit(nDestX, nDestY, pSrcImage, nSourceX, nSourceY, nSourceW, nSourceH);
}

void SdlFrame::blit(int nDestX, int nDestY, GImage* pSrcImage, int nSourceX, int nSourceY, int nSourceW, int nSourceH)
{
	SDL_Surface* pSurface = m_pWindow->m_pScreen;
	int nSourceStart = nSourceX;
	int nDestStart = nDestX;
	int nFinishY = nSourceY + nSourceH;
	int nFinishX = nSourceX + nSourceW;
	while(nSourceY < nFinishY)
	{
		nSourceX = nSourceStart;
		nDestX = nDestStart;
		while(nSourceX < nFinishX)
		{
			putpixel32(pSurface, nDestX, nDestY, pSrcImage->GetPixel(nSourceX, nSourceY));
			nSourceX++;
			nDestX++;
		}
		nSourceY++;
		nDestY++;
	}
}
