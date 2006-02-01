/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VGame.h"
#include "MObject.h"
#include "MRealm.h"
#include "../GClasses/GRayTrace.h"
#include "View.h"
#include "../GClasses/GBillboardCamera.h"
#include "Main.h"
#include "MStore.h"
#include "MGameClient.h"
#include "MAnimation.h"
#include "MGameImage.h"
#include "Controller.h"
#include "VPanel.h"

struct VVertGroundParams
{
	float x, y, dx, dy, size;
	bool sky;
	Uint32* pPix;
};

struct VHorizGroundParams
{
	int z;
	Uint32* pPix;
	GColor c;
};

// ------------------------------------------------------------------

#define TERRAIN_EXTRA_RATIO 1.3

VGame::VGame(GRect* pRect, MGameClient* pGameClient, GImage* pSkyImage, GImage* pGroundImage, VOnScreenPanel* pPanel)
: ViewPort(pRect)
{
	m_pPanel = pPanel;
	m_WorldRect = *pRect;
	m_WorldRect.h -= PANEL_HEIGHT;
	m_pGameClient = pGameClient;
	m_pCamera = pGameClient->GetCamera();
	m_pImageSky = pSkyImage;
	m_pImageGround = pGroundImage;
	m_pVertGroundParams = new struct VVertGroundParams[(int)(pRect->h * TERRAIN_EXTRA_RATIO)];
	m_pHorizGroundParams = new struct VHorizGroundParams[pRect->w];
	m_dTime = 0;
	m_bTerrain = false;

	m_fSelectionX1 = 0;
	m_fSelectionY1 = 0;
	m_fSelectionX2 = 0;
	m_fSelectionY2 = 0;

	m_pGameClient->GetCurrentRealm()->GetMapRect(&m_nMapXMin, &m_nMapXMax, &m_nMapYMin, &m_nMapYMax);
}

VGame::~VGame()
{
	delete [] m_pVertGroundParams;
	delete [] m_pHorizGroundParams;
}
/*
void VGame::SetRect(GRect* pRect)
{
	m_rect = *pRect;
	delete(m_pVertGroundParams);
	m_pVertGroundParams = new struct VVertGroundParams[(int)(pRect->h * TERRAIN_EXTRA_RATIO)];
	delete(m_pHorizGroundParams);
	m_pHorizGroundParams = new struct VHorizGroundParams[pRect->w];
}
*/
void VGame::DrawGroundAndSkyNoTerrain(SDL_Surface* pScreen)
{
	float nMapXMin = m_nMapXMin;
	float nMapXMax = m_nMapXMax;
	float nMapYMin = m_nMapYMin;
	float nMapYMax = m_nMapYMax;

	// Precalculate some map positions
	GRect* pScreenRect = &m_WorldRect;
	int x = pScreenRect->x;
	int y = pScreenRect->y;
	int xn;
	int yn = 0;
	GColor pix;
	int nBytesPerPixel = pScreen->format->BytesPerPixel;
	unsigned int nSkyImageWidth = m_pImageSky->GetWidth();
	unsigned int nSkyImageHeight = m_pImageSky->GetHeight();
	for(yn = 0; yn < pScreenRect->h; yn++)
	{
		m_pCamera->ScreenToMap(&m_pVertGroundParams[yn].x, &m_pVertGroundParams[yn].y, &m_pVertGroundParams[yn].size, &m_pVertGroundParams[yn].sky, x, y, pScreenRect, nSkyImageWidth);
		m_pCamera->ScreenToMap(&m_pVertGroundParams[yn].dx, &m_pVertGroundParams[yn].dy, &m_pVertGroundParams[yn].size, &m_pVertGroundParams[yn].sky, x + pScreenRect->w, y, pScreenRect, nSkyImageWidth);
		m_pVertGroundParams[yn].dx -= m_pVertGroundParams[yn].x;
		m_pVertGroundParams[yn].dy -= m_pVertGroundParams[yn].y;
		m_pVertGroundParams[yn].dx /= pScreenRect->w;
		m_pVertGroundParams[yn].dy /= pScreenRect->w;
		if(nBytesPerPixel == 4)
			m_pVertGroundParams[yn].pPix = getPixMem32(pScreen, x, y);
		else
			m_pVertGroundParams[yn].pPix = (Uint32*)getPixMem16(pScreen, x, y);
		y++;
	}

	// Draw the ground and sky
	GImage* pImageGround = m_pImageGround;
	unsigned int nGroundImageWidth = pImageGround->GetWidth();
	unsigned int nGroundImageHeight = pImageGround->GetHeight();
	VVertGroundParams* pMapParams;
	float fMapX, fMapY, fMapDeltaX, fMapDeltaY;
	int xImage, yImage;
	y = pScreenRect->y;
	Uint32* pPix32;
	for(yn = 0; yn < pScreenRect->h; yn++)
	{
		pMapParams = &m_pVertGroundParams[yn];
		fMapX = pMapParams->x;
		fMapY = pMapParams->y;
		fMapDeltaX = pMapParams->dx;
		fMapDeltaY = pMapParams->dy;
		bool bSky = pMapParams->sky;
		x = pScreenRect->x;
		if(nBytesPerPixel == 4)
		{
			pPix32 = getPixMem32(pScreen, x, y);
			if(bSky)
			{
				for(xn = 0; xn < pScreenRect->w; xn++)
				{
					xImage = ((unsigned int)fMapX + 1342177) % nSkyImageWidth;
					yImage = (nSkyImageHeight - (unsigned int)fMapY) % nSkyImageHeight;
					*(pPix32++) = m_pImageSky->GetPixel(xImage, yImage);
					fMapX += fMapDeltaX;
					fMapY += fMapDeltaY;
				}
			}
			else
			{
				for(xn = 0; xn < pScreenRect->w; xn++)
				{
					if(fMapX < nMapXMin || fMapX > nMapXMax || fMapY < nMapYMin || fMapY > nMapYMax)
						pix = 0;
					else
					{
						xImage = ((unsigned int)(fMapX - nMapXMin)) % nGroundImageWidth;
						yImage = nGroundImageHeight - 1 - (((unsigned int)(fMapY - nMapYMin)) % nGroundImageHeight);
						pix = pImageGround->GetPixel(xImage, yImage);
					}
					*(pPix32++) = pix;
					fMapX += fMapDeltaX;
					fMapY += fMapDeltaY;
				}
			}
		}
		else
		{
			Uint16* pPix16 = getPixMem16(pScreen, x, y);
			if(bSky)
			{
				for(xn = 0; xn < pScreenRect->w; xn++)
				{
					xImage = (unsigned int)fMapX % nSkyImageWidth;
					yImage = (unsigned int)fMapY % nSkyImageHeight;
					pix = m_pImageSky->GetPixel(xImage, yImage);
					*pPix16 = SDL_MapRGB(pScreen->format, gRed(pix), gGreen(pix), gBlue(pix));
					pPix16++;
					fMapX += fMapDeltaX;
					fMapY += fMapDeltaY;
				}
			}
			else
			{
				for(xn = 0; xn < pScreenRect->w; xn++)
				{
					if(fMapX < nMapXMin || fMapX > nMapXMax || fMapY < nMapYMin || fMapY > nMapYMax)
						pix = 0;
					else
					{
						xImage = ((unsigned int)fMapX >> 2) % nGroundImageWidth;
						yImage = ((unsigned int)fMapY >> 2) % nGroundImageHeight;
						pix = pImageGround->GetPixel(xImage, yImage);
					}
					*pPix16 = SDL_MapRGB(pScreen->format, gRed(pix), gGreen(pix), gBlue(pix));
					pPix16++;
					fMapX += fMapDeltaX;
					fMapY += fMapDeltaY;
				}
			}
		}
		y++;
	}
}

void VGame::DrawSprite(SDL_Surface* pScreen, MObject* pSprite)
{
	// Calculate the destination values
	if(pSprite->IsPanel())
		DrawPanel(pScreen, pSprite);
	else
		DrawBillboard(pScreen, pSprite);
}

void VGame::DrawPanel(SDL_Surface* pScreen, MObject* pSprite)
{
	// Get the source image
	GRect srcRect;
	GImage* pImage = pSprite->GetFrame(&srcRect, m_pCamera);
	if(!pImage)
		return;
	GAssert(srcRect.x >= 0 && srcRect.y >= 0 && srcRect.x + srcRect.w <= (int)pImage->GetWidth() && srcRect.y + srcRect.h <= (int)pImage->GetHeight(), "Out of range");
	if(srcRect.w <= 0 && srcRect.h <= 0)
		return;

	// Precalculate stuff
	GPosSize* pGhostPos = pSprite->GetGhostPos();
	GRect* pScreenRect = &m_WorldRect;
	GPanelPos destTrap;
	m_pCamera->CalcPanelTrapezoid(&destTrap, pGhostPos, pScreenRect);
	float fWidth = destTrap.w;
	float fHeight = destTrap.h;
	float fY = destTrap.y;
	float fSrcDX = srcRect.w / destTrap.w;
	float fSrcX = (float)srcRect.x;
	int nX = (int)destTrap.x;
	if(nX < pScreenRect->x)
	{
		float tmp = (pScreenRect->x - destTrap.x);
		nX = pScreenRect->x;
		fWidth -= tmp;
		fHeight += destTrap.dh * tmp;
		fY += destTrap.dy * tmp;
		fSrcX += fSrcDX * tmp;
	}
	int nWid = (int)fWidth;
	if(nWid <= 0 || fHeight <= 0)
		return;
	if(nX + nWid > pScreenRect->x + pScreenRect->w)
		nWid = pScreenRect->x + pScreenRect->w - nX;
	if((int)(fSrcX + fSrcDX * nWid) > srcRect.x + srcRect.w)
		fSrcDX = ((float)srcRect.w - 1 - fSrcX) / nWid;
	int nBytesPerPixel = pScreen->format->BytesPerPixel;

	// Blit it
	int nY;
	int yEnd;
	float fSrcY, fSrcDY;
	GColor col;
	GColor colOld;
	int a;
	Uint32* pPix;
	while(nWid > 0)
	{
		fSrcY = (float)srcRect.y;
		fSrcDY = srcRect.h / fHeight;
		yEnd = (int)(fY + fHeight);

		// Clip with bottom of screen
		if(yEnd > pScreenRect->y + pScreenRect->h)
			yEnd = pScreenRect->y + pScreenRect->h;
		nY = (int)fY;

		// Clip with top of screen
		if(nY < pScreenRect->y)
		{
			fSrcY += fSrcDY * (pScreenRect->y - nY);
			nY = pScreenRect->y;
		}

		// Bounds check (just to make sure we don't exceed the edges of the source image)
		if((int)(fSrcY + fSrcDY * (yEnd - nY - 1)) >= srcRect.y + srcRect.h)
			fSrcDY = ((float)(srcRect.y + srcRect.h) - fSrcY) / (yEnd - nY);

		// Draw it
		if(nBytesPerPixel == 4)
		{
			while(nY < yEnd)
			{
				col = pImage->GetPixel((int)fSrcX, (int)fSrcY);
				pPix = getPixMem32(pScreen, nX, nY);
				a = gAlpha(col);
				colOld = *pPix;
				*pPix = gRGB((a * gRed(col) + (256 - a) * gRed(colOld)) >> 8,
							(a * gGreen(col) + (256 - a) * gGreen(colOld)) >> 8,
							(a * gBlue(col) + (256 - a) * gBlue(colOld)) >> 8);
				fSrcY += fSrcDY;
				nY++;
			}
		}
		else
		{
			GAssert(false, "todo: implement 16-bit support here");
		}
		nWid--;
		nX++;
		fSrcX += fSrcDX;
		fY += destTrap.dy;
		fHeight += destTrap.dh;
	}
}

void VGame::DrawBillboard(SDL_Surface* pScreen, MObject* pSprite)
{
	GPosSize* pGhostPos = pSprite->GetGhostPos();
	GRect* pScreenRect = &m_WorldRect;
	GRect destRect;
	m_pCamera->CalcBillboardRect(&destRect, pGhostPos, pScreenRect, pSprite->GetPivotHeight());
	int nDestXStart = destRect.x;
	int nDestYStart = destRect.y;
	int nDestXFinish = destRect.x + destRect.w;
	int nDestYFinish = destRect.y + destRect.h;
	if(nDestXStart < pScreenRect->x)
		nDestXStart = pScreenRect->x;
	if(nDestXFinish > pScreenRect->x + pScreenRect->w)
		nDestXFinish = pScreenRect->x + pScreenRect->w;
	if(nDestYStart < pScreenRect->y)
		nDestYStart = pScreenRect->y;
	if(nDestYFinish > pScreenRect->y + pScreenRect->h)
		nDestYFinish = pScreenRect->y + pScreenRect->h;
	if(nDestXFinish <= nDestXStart)
		return;
	if(nDestYFinish <= nDestYStart)
		return;

	// Get the source image
	GRect srcRect;
	GImage* pImage = pSprite->GetFrame(&srcRect, m_pCamera);
	if(!pImage)
		return;
	GAssert(srcRect.x >= 0 && srcRect.y >= 0 && srcRect.x + srcRect.w <= (int)pImage->GetWidth() && srcRect.y + srcRect.h <= (int)pImage->GetHeight(), "Out of range");
	if(srcRect.w <= 0 && srcRect.h <= 0)
		return;

	// Calculate stepping values and make sure the bounds of the source image won't be exceeded by the blitting loops
	float dSrcDeltaX = (float)srcRect.w / ((float)destRect.w + 1);
	while((nDestXFinish - nDestXStart - 1) * dSrcDeltaX >= srcRect.w)
		nDestXFinish--;
	float dSrcDeltaY = (float)srcRect.h / ((float)destRect.h + 1);
	while((nDestYFinish - nDestYStart - 1) * dSrcDeltaY >= srcRect.h)
		nDestYFinish--;

	// stretch-blt the image
	float dSrcX, dSrcY;
	dSrcY = srcRect.y + dSrcDeltaY * (nDestYStart - destRect.y);
	float dStartX = srcRect.x + dSrcDeltaX * (nDestXStart - destRect.x);
	int x, y;
	int a;
	GColor pixOld;
	GColor pix;
	int nBytesPerPixel = pScreen->format->BytesPerPixel;
	for(y = nDestYStart; y < nDestYFinish; y++)
	{
		dSrcX = dStartX;
		if(nBytesPerPixel == 4)
		{
			Uint32* pPix = getPixMem32(pScreen, nDestXStart, y);
			for(x = nDestXStart; x < nDestXFinish; x++)
			{
				pix = pImage->GetPixel((int)dSrcX, (int)dSrcY);
				a = gAlpha(pix);
				pixOld = *pPix;
				*pPix = gRGB((a * gRed(pix) + (256 - a) * gRed(pixOld)) >> 8,
							(a * gGreen(pix) + (256 - a) * gGreen(pixOld)) >> 8,
							(a * gBlue(pix) + (256 - a) * gBlue(pixOld)) >> 8);
				pPix++;
				dSrcX += dSrcDeltaX;
			}
		}
		else
		{
			Uint16* pPix = getPixMem16(pScreen, nDestXStart, y);
			for(x = nDestXStart; x < nDestXFinish; x++)
			{
				pix = pImage->GetPixel((int)dSrcX, (int)dSrcY);
				a = gAlpha(pix);
				pixOld = *pPix;
				*pPix = SDL_MapRGB(pScreen->format,
							(a * gRed(pix) + (256 - a) * gRed(pixOld)) >> 8,
							(a * gGreen(pix) + (256 - a) * gGreen(pixOld)) >> 8,
							(a * gBlue(pix) + (256 - a) * gBlue(pixOld)) >> 8
						);
				pPix++;
				dSrcX += dSrcDeltaX;
			}
		}
		dSrcY += dSrcDeltaY;
	}
}

void VGame::DrawSpritesNoTerrain(SDL_Surface* pScreen)
{
	MObject* pObj;
	MRealm* pRealm = m_pGameClient->GetCurrentRealm();
	int n;
	for(n = pRealm->GetObjectCount() - 1; n >= 0; n--)
	{
		pObj = pRealm->GetObj(n);
		DrawSprite(pScreen, pObj);
	}
}

inline GColor MixColors(GColor a, GColor b, int n, int d)
{
	int n2 = d - n;
	return gRGB(
		(gRed(a) * n + gRed(b) * n2) / d,
		(gGreen(a) * n + gGreen(b) * n2) / d,
		(gBlue(a) * n + gBlue(b) * n2) / d
		);
}

void VGame::SetSkyImage(GImage* pImage)
{
	m_pImageSky = pImage;
}

void VGame::SetGroundImage(GImage* pImage)
{
	m_pImageGround = pImage;
}

void VGame::DrawEverythingWithTerrain(SDL_Surface* pScreen)
{
	float nMapXMin = m_nMapXMin;
	float nMapXMax = m_nMapXMax;
	float nMapYMin = m_nMapYMin;
	float nMapYMax = m_nMapYMax;

	// Precalculate some map positions
	GRect* pScreenRect = &m_WorldRect;
	int x = pScreenRect->x;
	int y = pScreenRect->y;
	int xn;
	int yn = 0;
	int nVertHeight = (int)(pScreenRect->h * TERRAIN_EXTRA_RATIO);
	unsigned int nSkyImageWidth = m_pImageSky->GetWidth();
	unsigned int nSkyImageHeight = m_pImageSky->GetHeight();
	for(yn = 0; yn < nVertHeight; yn++)
	{
		m_pCamera->ScreenToMap(&m_pVertGroundParams[yn].x, &m_pVertGroundParams[yn].y, &m_pVertGroundParams[yn].size, &m_pVertGroundParams[yn].sky, x, y, pScreenRect, nSkyImageWidth);
		m_pCamera->ScreenToMap(&m_pVertGroundParams[yn].dx, &m_pVertGroundParams[yn].dy, &m_pVertGroundParams[yn].size, &m_pVertGroundParams[yn].sky, x + pScreenRect->w, y, pScreenRect, nSkyImageWidth);
		m_pVertGroundParams[yn].dx -= m_pVertGroundParams[yn].x;
		m_pVertGroundParams[yn].dy -= m_pVertGroundParams[yn].y;
		m_pVertGroundParams[yn].dx /= pScreenRect->w;
		m_pVertGroundParams[yn].dy /= pScreenRect->w;
		m_pVertGroundParams[yn].pPix = getPixMem32(pScreen, x, y);
		y++;
	}

	// Draw the sky
	VVertGroundParams* pMapParams;
	float fMapX, fMapY, fMapDeltaX, fMapDeltaY;
	Uint32* pPix;
	y = pScreenRect->y;
	for(yn = 0; ;yn++)
	{
		pMapParams = &m_pVertGroundParams[yn];
		if(!pMapParams->sky)
			break;
		fMapX = pMapParams->x;
		fMapY = pMapParams->y;
		fMapDeltaX = pMapParams->dx;
		fMapDeltaY = pMapParams->dy;
		x = pScreenRect->x;
		pPix = getPixMem32(pScreen, x, y);
		for(xn = 0; xn < pScreenRect->w; xn++)
		{
			*pPix = m_pImageSky->GetPixel(((unsigned int)fMapX) % nSkyImageWidth, ((unsigned int)fMapY) % nSkyImageHeight);
			pPix++;
			fMapX += fMapDeltaX;
			fMapY += fMapDeltaY;
			x++;
		}
		y++;
	}

	// Init the horizontal parameters
	VHorizGroundParams* pHorizParams;
	x = pScreenRect->x;
	for(xn = 0; xn < pScreenRect->w; xn++)
	{
		pHorizParams = &m_pHorizGroundParams[xn];
		pHorizParams->z = y;
		pHorizParams->pPix = getPixMem32(pScreen, x, y);
		pHorizParams->c = 0;
		x++;
	}
	int nVertPixDelta = getPixMem32(pScreen, 0, 1) - getPixMem32(pScreen, 0, 0);

	// Draw the terrain and sprites
	MRealm* pRealm = m_pGameClient->GetCurrentRealm();
	GImage* pTerrainMap = pRealm->GetTerrainMap();
	float fTerXFac = (pTerrainMap->GetWidth() - 1) / (nMapXMax - nMapXMin);
	float fTerYFac = (pTerrainMap->GetHeight() - 1) / (nMapYMax - nMapYMin);
	GImage* pImageGround = m_pImageGround;
	unsigned int nGroundImageWidth = pImageGround->GetWidth();
	unsigned int nGroundImageHeight = pImageGround->GetHeight();
	float fMapZ;
	int z;
	GColor c, ter;
	int nObject = pRealm->GetObjectCount() - 1;
	MObject* pSprite = NULL;
	if(nObject >= 0)
		pSprite = pRealm->GetObj(nObject--);
	float fSpriteX, fSpriteY, fSpriteZ, currentZDepth;
	if(pSprite)
	{
		pSprite->GetPos(&fSpriteX, &fSpriteY);
		fSpriteZ = m_pCamera->CalculateDistanceFromCamera(fSpriteX, fSpriteY);
	}
	else
		fSpriteZ = (float)-1e20; // somewhere way behind the camera
	int nMaxScreenPos = pScreenRect->y + pScreenRect->h - 1;
	for( ; yn < nVertHeight; yn++) // todo: find a better way to tell when we're done with this loop
	{
		// Draw the terrain
		pMapParams = &m_pVertGroundParams[yn];
		fMapX = pMapParams->x;
		fMapY = pMapParams->y;
		fMapDeltaX = pMapParams->dx;
		fMapDeltaY = pMapParams->dy;
		x = pScreenRect->x;
		int zStretch;
		for(xn = 0; xn < pScreenRect->w; xn++)
		{
			if(fMapX < nMapXMin || fMapX > nMapXMax || fMapY < nMapYMin || fMapY > nMapYMax)
			{
				if(pHorizParams->z <= nMaxScreenPos)
				{
					*pHorizParams->pPix = 0;
					pHorizParams->pPix += nVertPixDelta;
					pHorizParams->z++;
				}
			}
			else
			{
				ter = pTerrainMap->GetPixel((int)((fMapX - nMapXMin) * fTerXFac), (int)((fMapY - nMapYMin) * fTerYFac));
				fMapZ = (float)(ter & 0xffff) / 32 - 1024;
				z = y - (int)(pMapParams->size * fMapZ);
				if(z >= pScreenRect->y && z < nMaxScreenPos) // todo: can this check be avoided somehow?
				{
					pHorizParams = &m_pHorizGroundParams[xn];
					zStretch = z - pHorizParams->z;
					c = pImageGround->GetPixel(((unsigned int)fMapX) % nGroundImageWidth, ((unsigned int)fMapY) % nGroundImageHeight);
					while(pHorizParams->z < z)
					{
						*pHorizParams->pPix = MixColors(pHorizParams->c, c, z - pHorizParams->z, zStretch);
						pHorizParams->pPix += nVertPixDelta;
						pHorizParams->z++;
					}
					while(pHorizParams->z > z)
					{
						pHorizParams->pPix -= nVertPixDelta;
						pHorizParams->z--;
					}
					c &= ((ter >> 8) | 0xff);
					pHorizParams->c = c;
					*pHorizParams->pPix = c;
					pHorizParams->pPix += nVertPixDelta;
					pHorizParams->z++;
				}
			}
			fMapX += fMapDeltaX;
			fMapY += fMapDeltaY;
			x++;
		}

		// Draw the sprites
		currentZDepth = m_pCamera->CalculateDistanceFromCamera(pMapParams->x + pScreenRect->w * fMapDeltaX / 2, pMapParams->y + pScreenRect->w * fMapDeltaY / 2);
		while(fSpriteZ >= currentZDepth && pSprite)
		{
			DrawSprite(pScreen, pSprite);
			if(nObject >= 0)
			{
				pSprite = pRealm->GetObj(nObject--);
				pSprite->GetPos(&fSpriteX, &fSpriteY);
				fSpriteZ = m_pCamera->CalculateDistanceFromCamera(fSpriteX, fSpriteY);
			}
			else
			{
				pSprite = NULL;
				fSpriteZ = (float)-1e20; // somewhere way behind the camera
			}
		}

		y++;
	}

	// Blank out any remaining pixels
	x = pScreenRect->x;
	for(xn = 0; xn < pScreenRect->w; xn++)
	{
		pHorizParams = &m_pHorizGroundParams[xn];
		while(pHorizParams->z++ <= nMaxScreenPos)
		{
			*pHorizParams->pPix = 0;
			pHorizParams->pPix += nVertPixDelta;
		}
		x++;
	}
}

void VGame::DrawSelectedRegion(SDL_Surface* pScreen)
{
	MObject* pSelObj = m_pGameClient->GetSelectionBorder();
	if(!pSelObj)
		return;
	float fOldX, fOldY, x, y, dx, dy;
	GPosSize* pPosSize = pSelObj->GetGhostPos();
	fOldX = pPosSize->x;
	fOldY = pPosSize->y;
	dx = 1;
	dy = 1;
	if(m_fSelectionX2 < m_fSelectionX1)
		dx = -1;
	if(m_fSelectionY2 < m_fSelectionY1)
		dy = -1;
	x = m_fSelectionX1;
	while(x * dx < m_fSelectionX2 * dx)
	{
		pSelObj->SetPos(x, m_fSelectionY1, 0);
		pSelObj->SetGhostPos(x, m_fSelectionY1);
		DrawSprite(pScreen, pSelObj);
		pSelObj->SetPos(x, m_fSelectionY2, 0);
		pSelObj->SetGhostPos(x, m_fSelectionY2);
		DrawSprite(pScreen, pSelObj);
		x += 50 * dx;
	}
	y = m_fSelectionY1;
	while(y * dy < m_fSelectionY2 * dy)
	{
		pSelObj->SetPos(m_fSelectionX1, y, 0);
		pSelObj->SetGhostPos(m_fSelectionX1, y);
		DrawSprite(pScreen, pSelObj);
		pSelObj->SetPos(m_fSelectionX2, y, 0);
		pSelObj->SetGhostPos(m_fSelectionX2, y);
		DrawSprite(pScreen, pSelObj);
		y += 50 * dy;
	}
	pSelObj->SetPos(fOldX, fOldY, -10000);
	pSelObj->SetGhostPos(fOldX, fOldY);
}

/*virtual*/ void VGame::Draw(SDL_Surface* pScreen)
{
	// Draw the sky, ground, and sprites
	if(m_bTerrain)
		DrawEverythingWithTerrain(pScreen);
	else
	{
		DrawGroundAndSkyNoTerrain(pScreen);
		DrawSpritesNoTerrain(pScreen);
	}

	// Draw the selection corners
	if(m_fSelectionX1 != m_fSelectionX2)
		DrawSelectedRegion(pScreen);

	// Draw the on-screen panel
	if(m_pPanel->IsDirty() || m_pPanel->GetGrabbedWidget())
		BlitImage(pScreen, m_WorldRect.x, m_WorldRect.y + m_WorldRect.h, m_pPanel->GetCanvas());
}

void VGame::ScreenToMap(float* px, float* py, bool* pbPanel, bool* pbSky, int x, int y)
{
	*pbPanel = false;
	if(y > m_WorldRect.y + m_WorldRect.h)
	{
		*pbPanel = true;
		return;
	}
	float size;
	m_pCamera->ScreenToMap(px, py, &size, pbSky, x, y, &m_WorldRect, 1);
}

void VGame::SetSelectionRect(float x1, float y1, float x2, float y2)
{
	m_fSelectionX1 = x1;
	m_fSelectionY1 = y1;
	m_fSelectionX2 = x2;
	m_fSelectionY2 = y2;
}

/*virtual*/ void VGame::OnChar(char c)
{
	m_pPanel->HandleChar(c);
}

/*virtual*/ void VGame::OnMouseDown(int x, int y)
{
	y -= (m_WorldRect.y + m_WorldRect.h);
	if(y >= 0)
	{
		x -= m_WorldRect.x;
		GWidgetAtomic* pNewWidget = m_pPanel->FindAtomicWidget(x, y);
		m_pPanel->GrabWidget(pNewWidget, x, y);
		m_pPanel->SetDirty();
	}
}

/*virtual*/ void VGame::OnMouseUp(int x, int y)
{
	m_pPanel->ReleaseWidget();
	m_pPanel->SetDirty();
}

/*virtual*/ void VGame::OnMousePos(int x, int y)
{
	m_pPanel->HandleMousePos(x, y);
}
