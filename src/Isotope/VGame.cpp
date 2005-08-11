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
#include "../GClasses/GWidgets.h"
#include "View.h"
#include "../GClasses/GBillboardCamera.h"
#include "GameEngine.h"
#include "MStore.h"
#include "MGameClient.h"
#include "MAnimation.h"
#include "MGameImage.h"

#define ALPHA_BLENDING

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

VGame::VGame(GRect* pRect, MGameClient* pGameClient)
: ViewPort(pRect)
{
	m_pGameClient = pGameClient;
	m_pCamera = pGameClient->GetCamera();
	MImageStore* pImageStore = pGameClient->GetImages();
	m_pImageSky = &((MGameImage*)pImageStore->GetVarHolder("sky")->GetGObject())->m_value;
	MAnimationStore* pAnimStore = pGameClient->GetAnimations();
	m_pImageGround = &((MGameImage*)pImageStore->GetVarHolder("ground")->GetGObject())->m_value;
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

void VGame::SetRect(GRect* pRect)
{
	m_rect = *pRect;
	delete(m_pVertGroundParams);
	m_pVertGroundParams = new struct VVertGroundParams[(int)(pRect->h * TERRAIN_EXTRA_RATIO)];
	delete(m_pHorizGroundParams);
	m_pHorizGroundParams = new struct VHorizGroundParams[pRect->w];
}

void VGame::DrawGroundAndSkyNoTerrain(SDL_Surface* pScreen)
{
	float nMapXMin = m_nMapXMin;
	float nMapXMax = m_nMapXMax;
	float nMapYMin = m_nMapYMin;
	float nMapYMax = m_nMapYMax;

	// Precalculate some map positions
	GRect* pScreenRect = &m_rect;
	int x = pScreenRect->x;
	int y = pScreenRect->y;
	int xn;
	int yn = 0;
	GColor pix;
	int nBytesPerPixel = pScreen->format->BytesPerPixel;
	for(yn = 0; yn < pScreenRect->h; yn++)
	{
		m_pCamera->ScreenToMap(&m_pVertGroundParams[yn].x, &m_pVertGroundParams[yn].y, &m_pVertGroundParams[yn].size, &m_pVertGroundParams[yn].sky, x, y, pScreenRect);
		m_pCamera->ScreenToMap(&m_pVertGroundParams[yn].dx, &m_pVertGroundParams[yn].dy, &m_pVertGroundParams[yn].size, &m_pVertGroundParams[yn].sky, x + pScreenRect->w, y, pScreenRect);
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
					xImage = ((int)fMapX + 1342177480) % m_pImageSky->GetWidth();
					yImage = ((int)fMapY) % m_pImageSky->GetHeight();
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
						xImage = (((int)fMapX >> 2) + 1342177280) % m_pImageGround->GetWidth();
						yImage = (((int)fMapY >> 2) + 1342177480) % m_pImageGround->GetHeight();
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
					xImage = ((int)fMapX + 1342177480) % m_pImageSky->GetWidth();
					yImage = ((int)fMapY + 1342177480) % m_pImageSky->GetHeight();
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
						xImage = (((int)fMapX >> 2) + 1342177280) % m_pImageGround->GetWidth();
						yImage = (((int)fMapY >> 2) + 1342177480) % m_pImageGround->GetHeight();
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
	// Get the source image
	GRect srcRect;
	GImage* pImage = pSprite->GetFrame(&srcRect, m_pCamera);
	if(!pImage)
		return;
	GAssert(srcRect.x >= 0 && srcRect.y >= 0 && srcRect.x + srcRect.w <= (int)pImage->GetWidth() && srcRect.y + srcRect.h <= (int)pImage->GetHeight(), "Out of range");
	if(srcRect.w <= 0 && srcRect.h <= 0)
		return;

	// Calculate the destination values
	GPosSize* pGhostPos = pSprite->GetGhostPos();
	if(pSprite->IsPanel())
		DrawPanel(pScreen, pImage, &srcRect, pGhostPos);
	else
		DrawBillboard(pScreen, pImage, &srcRect, pGhostPos);
}

void VGame::DrawPanel(SDL_Surface* pScreen, GImage* pImage, GRect* pSrcRect, GPosSize* pGhostPos)
{
	// Precalculate stuff
	GRect* pScreenRect = &m_rect;
	GPanelPos destTrap;
	m_pCamera->CalcPanelTrapezoid(&destTrap, pGhostPos, pScreenRect);
	float fWidth = destTrap.w;
	float fHeight = destTrap.h;
	float fY = destTrap.y;
	float fSrcDX = pSrcRect->w / destTrap.w;
	float fSrcX = (float)pSrcRect->x;
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
	if((int)(fSrcX + fSrcDX * nWid) > pSrcRect->x + pSrcRect->w)
		fSrcDX = ((float)pSrcRect->w - 1 - fSrcX) / nWid;
	int nBytesPerPixel = pScreen->format->BytesPerPixel;

	// Blit it
	int nY;
	int yEnd;
	float fSrcY, fSrcDY;
	GColor col;
#ifdef ALPHA_BLENDING
	GColor colOld;
	int a;
	Uint32* pPix;
#endif // ALPHA_BLENDING
	while(nWid > 0)
	{
		fSrcY = (float)pSrcRect->y;
		fSrcDY = pSrcRect->h / fHeight;
		yEnd = (int)(fY + fHeight);
		if(yEnd > pScreenRect->y + pScreenRect->h)
			yEnd = pScreenRect->y + pScreenRect->h;
		nY = (int)fY;
		if(nY < pScreenRect->y)
		{
			fSrcY += fSrcDY * (pScreenRect->y - nY);
			nY = pScreenRect->y;
		}
		if((int)(fSrcY + fSrcDY * (yEnd - nY)) > pSrcRect->y + pSrcRect->h) // todo: is this check necessary?
			fSrcDY = ((float)pSrcRect->h - 1 - fSrcY) / (yEnd - nY);
		if(nBytesPerPixel == 4)
		{
			while(nY < yEnd)
			{
				col = pImage->GetPixel((int)fSrcX, (int)fSrcY);
#ifdef ALPHA_BLENDING
				pPix = getPixMem32(pScreen, nX, nY);
				a = gAlpha(col);
				colOld = *pPix;
				*pPix = gRGB((a * gRed(col) + (256 - a) * gRed(colOld)) >> 8,
							(a * gGreen(col) + (256 - a) * gGreen(colOld)) >> 8,
							(a * gBlue(col) + (256 - a) * gBlue(colOld)) >> 8);
#else // ALPHA_BLENDING
				if(col != 0x00ff00) // todo: use sprite's transparent background color
					*getPixMem32(pScreen, nX, nY) = col;
#endif // ALPHA_BLENDING
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

void VGame::DrawBillboard(SDL_Surface* pScreen, GImage* pImage, GRect* pSrcRect, GPosSize* pGhostPos)
{
	GRect* pScreenRect = &m_rect;
	GRect destRect;
	m_pCamera->CalcBillboardRect(&destRect, pGhostPos, pScreenRect);
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

	// Calculate stepping values and make sure the bounds of the source image won't be exceeded by the blitting loops
	float dSrcDeltaX = (float)pSrcRect->w / ((float)destRect.w + 1);
	while((destRect.w - 1) * dSrcDeltaX >= pSrcRect->w)
		nDestXFinish--;
	float dSrcDeltaY = (float)pSrcRect->h / ((float)destRect.h + 1);
	while((destRect.h - 1) * dSrcDeltaY >= pSrcRect->h)
		nDestYFinish--;

	// stretch-blt the image
	float dSrcX, dSrcY;
	dSrcY = pSrcRect->y + dSrcDeltaY * (nDestYStart - destRect.y);
	float dStartX = pSrcRect->x + dSrcDeltaX * (nDestXStart - destRect.x);
	int x, y;
#ifdef ALPHA_BLENDING
	int a;
	GColor pixOld;
#endif // ALPHA_BLENDING
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
#ifdef ALPHA_BLENDING
				a = gAlpha(pix);
				pixOld = *pPix;
				*pPix = gRGB((a * gRed(pix) + (256 - a) * gRed(pixOld)) >> 8,
							(a * gGreen(pix) + (256 - a) * gGreen(pixOld)) >> 8,
							(a * gBlue(pix) + (256 - a) * gBlue(pixOld)) >> 8);
#else // ALPHA_BLENDING
				if(pix != 0x00ff00) // todo: use sprite's transparent background color
					*pPix = pix;
#endif // ALPHA_BLENDING
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
				if(pix != 0x00ff00) // todo: use sprite's transparent background color
					*pPix = pix;
				pPix++;
				dSrcX += dSrcDeltaX;
			}
		}
		dSrcY += dSrcDeltaY;
	}
}

void VGame::DrawSpritesNoTerrain(SDL_Surface* pScreen)
{
	MObject* pSprite;
	MRealm* pRealm = m_pGameClient->GetCurrentRealm();
	for(pSprite = pRealm->GetFirstObject(); pSprite; pSprite = pSprite->GetNext())
		DrawSprite(pScreen, pSprite);
}

GColor MixColors(GColor a, GColor b, int n, int d)
{
	int n2 = d - n;
	return gRGB(
		(gRed(a) * n + gRed(b) * n2) / d,
		(gGreen(a) * n + gGreen(b) * n2) / d,
		(gBlue(a) * n + gBlue(b) * n2) / d
		);
}

void VGame::DrawEverythingWithTerrain(SDL_Surface* pScreen)
{
	float nMapXMin = m_nMapXMin;
	float nMapXMax = m_nMapXMax;
	float nMapYMin = m_nMapYMin;
	float nMapYMax = m_nMapYMax;

	// Precalculate some map positions
	GRect* pScreenRect = &m_rect;
	int x = pScreenRect->x;
	int y = pScreenRect->y;
	int xn;
	int yn = 0;
	int nVertHeight = (int)(pScreenRect->h * TERRAIN_EXTRA_RATIO);
	for(yn = 0; yn < nVertHeight; yn++)
	{
		m_pCamera->ScreenToMap(&m_pVertGroundParams[yn].x, &m_pVertGroundParams[yn].y, &m_pVertGroundParams[yn].size, &m_pVertGroundParams[yn].sky, x, y, pScreenRect);
		m_pCamera->ScreenToMap(&m_pVertGroundParams[yn].dx, &m_pVertGroundParams[yn].dy, &m_pVertGroundParams[yn].size, &m_pVertGroundParams[yn].sky, x + pScreenRect->w, y, pScreenRect);
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
	int nSkyImageWidth = m_pImageSky->GetWidth();
	int nSkyImageHeight = m_pImageSky->GetHeight();
	Uint32* pPix;
	y = pScreenRect->y;
	for(yn = 0; ;yn++)
	{
		pMapParams = &m_pVertGroundParams[yn];
		if(!pMapParams->sky)
			break;
		fMapX = pMapParams->x;// + 1342177; // todo: does adding this big number really guarantee it will never be negative?  Especially when close to horizon
		fMapY = pMapParams->y;// + 1342177; // todo: does adding this big number really guarantee it will never be negative?  Especially when close to horizon
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
	int nGroundImageWidth = pImageGround->GetWidth();
	int nGroundImageHeight = pImageGround->GetHeight();
	float fMapZ;
	int z;
	GColor c, ter;
	MObject* pSprite = pRealm->GetFirstObject();
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
			pSprite = pSprite->GetNext();
			if(pSprite)
			{
				pSprite->GetPos(&fSpriteX, &fSpriteY);
				fSpriteZ = m_pCamera->CalculateDistanceFromCamera(fSpriteX, fSpriteY);
			}
			else
				fSpriteZ = (float)-1e20; // somewhere way behind the camera
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
	{
		MObject* pGoalFlag = m_pGameClient->GetGoalFlag();
		if(pGoalFlag)
		{
			float fOldX, fOldY;
			GPosSize* pPosSize = pGoalFlag->GetGhostPos();
			fOldX = pPosSize->x;
			fOldY = pPosSize->y;
			pGoalFlag->SetPos(m_fSelectionX1, m_fSelectionY1, 0);
			pGoalFlag->SetGhostPos(m_fSelectionX1, m_fSelectionY1);
			DrawSprite(pScreen, pGoalFlag);
			pGoalFlag->SetPos(m_fSelectionX1, m_fSelectionY2, 0);
			pGoalFlag->SetGhostPos(m_fSelectionX1, m_fSelectionY2);
			DrawSprite(pScreen, pGoalFlag);
			pGoalFlag->SetPos(m_fSelectionX2, m_fSelectionY1, 0);
			pGoalFlag->SetGhostPos(m_fSelectionX2, m_fSelectionY1);
			DrawSprite(pScreen, pGoalFlag);
			pGoalFlag->SetPos(m_fSelectionX2, m_fSelectionY2, 0);
			pGoalFlag->SetGhostPos(m_fSelectionX2, m_fSelectionY2);
			DrawSprite(pScreen, pGoalFlag);
			pGoalFlag->SetPos(fOldX, fOldY, -10000);
			pGoalFlag->SetGhostPos(fOldX, fOldY);
		}
	}
}

void VGame::ScreenToMap(float* px, float* py, bool* pbSky, int x, int y)
{
	float size;
	m_pCamera->ScreenToMap(px, py, &size, pbSky, x, y, &m_rect);
}

void VGame::SetSelectionRect(float x1, float y1, float x2, float y2)
{
	m_fSelectionX1 = x1;
	m_fSelectionY1 = y1;
	m_fSelectionX2 = x2;
	m_fSelectionY2 = y2;
}
