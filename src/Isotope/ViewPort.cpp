/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "ViewPort.h"

/*static*/ void ViewPort::BlitImage(SDL_Surface* pScreen, int x, int y, GImage* pImage)
{
	if(pScreen->format->BytesPerPixel == 4)
	{
		// 32 bits per pixel
		GColor* pRGB = pImage->GetRGBQuads();
		int w = pImage->GetWidth();
		int h = pImage->GetHeight();
		int yy;
		Uint32* pPix;
		for(yy = 0; yy < h; yy++)
		{
			pPix = getPixMem32(pScreen, x, y);
			memcpy(pPix, &pRGB[yy * w], w * sizeof(GColor));
			y++;
		}
	}
	else
	{
		// 16 bits per pixel
		GAssert(pScreen->format->BytesPerPixel == 2, "Only 16 and 32 bit video modes are supported");
		int w = pImage->GetWidth();
		int h = pImage->GetHeight();
		int xx, yy, xxx;
		GColor colIn;
		Uint16* pPix;
		for(yy = 0; yy < h; yy++)
		{
			xxx = x;
			pPix = (Uint16*)pScreen->pixels + y * pScreen->pitch / 2 + x;
			for(xx = 0; xx < w; xx++)
			{
				colIn = pImage->GetPixel(xx, yy);
				*pPix = SDL_MapRGB(pScreen->format, gRed(colIn), gGreen(colIn), gBlue(colIn));
				xxx++;
				pPix++;
			}
			y++;
		}
	}
}

/*static*/ void ViewPort::StretchClipAndBlitImage(SDL_Surface* pScreen, GRect* pDestRect, GRect* pClipRect, GImage* pImage)
{
	float fSourceDX =  (float)(pImage->GetWidth() - 1) / (float)(pDestRect->w - 1);
	GAssert((int)((pDestRect->w - 1) * fSourceDX) < (int)pImage->GetWidth(), "Extends past source image width");
	float fSourceDY = (float)(pImage->GetHeight() - 1) / (float)(pDestRect->h - 1);
	GAssert((int)((pDestRect->h - 1) * fSourceDY) < (int)pImage->GetHeight(), "Extends past source image height");
	float fSourceX = 0;
	float fSourceY = 0;
	int xStart = pDestRect->x;
	int yStart = pDestRect->y;
	if(pClipRect->x > xStart)
	{
		xStart = pClipRect->x;
		fSourceX = (pClipRect->x * pDestRect->x) * fSourceDX;
	}
	if(pClipRect->y > yStart)
	{
		yStart = pClipRect->y;
		fSourceY = (pClipRect->y * pDestRect->y) * fSourceDY;
	}
	int xEnd = MIN(pDestRect->x + pDestRect->w, pClipRect->x + pClipRect->w);
	int yEnd = MIN(pDestRect->y + pDestRect->h, pClipRect->y + pClipRect->h);
	int x, y;
	float fSX;
	if(pScreen->format->BytesPerPixel == 4)
	{
		// 32 bits per pixel
		for(y = yStart; y < yEnd; y++)
		{
			fSX = fSourceX;
			for(x = xStart; x < xEnd; x++)
			{
				*getPixMem32(pScreen, x, y) = pImage->GetPixel((int)fSX, (int)fSourceY);
				fSX += fSourceDX;
			}
			fSourceY += fSourceDY;
		}
	}
	else
	{
		// 16 bits per pixel
		GAssert(pScreen->format->BytesPerPixel == 2, "Only 16 and 32 bit video modes are supported");
		GColor colIn;
		for(y = yStart; y < yEnd; y++)
		{
			fSX = fSourceX;
			for(x = xStart; x < xEnd; x++)
			{
				colIn = pImage->GetPixel((int)fSX, (int)fSourceY);
				*getPixMem16(pScreen, x, y) = SDL_MapRGB(pScreen->format, gRed(colIn), gGreen(colIn), gBlue(colIn));
				fSX += fSourceDX;
			}
			fSourceY += fSourceDY;
		}
	}
}

