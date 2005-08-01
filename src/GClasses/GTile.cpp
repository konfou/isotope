/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GDDraw.h"
#include "GMacros.h"
#include "GImage.h"

#define DISP_RECT_WIDTH 256
#define DISP_RECT_HEIGHT 256

GTile::GTile()
{
	m_pArrDispRects = NULL;
	m_nDispRectCount = 0;
	m_nTileCount = 0;
	m_nTilesPerDispRect = 0;
	m_nHorizTilesPerDispRect = 0;
	m_nTileWidth = 0;
	m_nTileHeight = 0;
	m_pScreen = NULL;
	m_nSelectedDispRect = -1;
}

GTile::~GTile()
{
	int n;
	for(n = 0; n < m_nDispRectCount; n++)
		delete(m_pArrDispRects[n]);
	delete(m_pArrDispRects);
}

bool GTile::LoadTiles(GWindow* pScreen, const char* szFilename, int nTileWidth, int nTileHeight)
{
	// Load the file
	GImage image;
	if(!image.LoadBMPFile(szFilename))
		return NULL;
	return LoadTiles(pScreen, &image, nTileWidth, nTileHeight);
}

bool GTile::LoadTiles(GWindow* pScreen, GImage* pImage, int nTileWidth, int nTileHeight)
{
	// Calculate widths and heights and stuff
	m_nTileWidth = nTileWidth;
	m_nTileHeight = nTileHeight;
	int nWidth = pImage->GetWidth();
	int nHeight = pImage->GetHeight();
	GAssert(nWidth % nTileWidth == 0, "Bitmap width not a multiple of nTileWidth");
	GAssert(nHeight % nTileHeight == 0, "Bitmap height not a multiple of nTileHeight");
	int nHorizTilesPerDispRect;
	int nVertTilesPerDispRect;
	if(nWidth <= DISP_RECT_WIDTH && nHeight < DISP_RECT_HEIGHT)
	{
		// we don't need to break the image down into lots of parts because it's small enough
		nHorizTilesPerDispRect = 1;
		nVertTilesPerDispRect = 1;
	}
	else
	{
		nHorizTilesPerDispRect = DISP_RECT_WIDTH / nTileWidth;
		nVertTilesPerDispRect = DISP_RECT_HEIGHT / nTileHeight;
	}
	m_nTilesPerDispRect = nHorizTilesPerDispRect * nVertTilesPerDispRect;
	m_nHorizTilesPerDispRect = nHorizTilesPerDispRect;
	int nDispRectSize = m_nTilesPerDispRect * nTileWidth * nTileHeight;
	if(nHorizTilesPerDispRect < 1 || nVertTilesPerDispRect < 1)
	{
		GAssert(false, "Tile size bigger than DispRect capability");
		return false;
	}
	int nDispRectWidth = nTileWidth * nHorizTilesPerDispRect;
	int nDispRectDataWidth = nDispRectWidth * 3;
	if(nDispRectDataWidth % 4 > 0) // Bitmaps are alligned with each row on a 4-byte boundary
		nDispRectDataWidth += (4 - nDispRectDataWidth % 4);
	int nDispRectDataSize = nDispRectDataWidth * nVertTilesPerDispRect * nTileHeight;
	int nDispRectHeight = nTileHeight * nVertTilesPerDispRect;
	int nHorizTiles = nWidth / nTileWidth;
	int nVertTiles = nHeight / nTileHeight;
	m_nTileCount = nHorizTiles * nVertTiles;

	// Allocate the disp rect array
	m_nDispRectCount = ((m_nTileCount - 1) / m_nTilesPerDispRect) + 1;
	m_pArrDispRects = new GDispRect*[m_nDispRectCount];
	GAssert(m_pArrDispRects, "Failed to allocate");

	// Make each disp rect
	int nDispRect;
	for(nDispRect = 0; nDispRect < m_nDispRectCount; nDispRect++)
	{
		// Make a bitmap of tiles for that disp rect
		unsigned char* pData = new unsigned char[nDispRectDataSize + sizeof(BITMAPINFO)];
		GAssert(pData, "Failed to allocate");
		BITMAPINFO* pbi = (BITMAPINFO*)pData;
		pbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbi->bmiHeader.biWidth = nDispRectWidth;
		pbi->bmiHeader.biHeight = nDispRectHeight;
		pbi->bmiHeader.biPlanes = 1;
		pbi->bmiHeader.biBitCount = 24;
		pbi->bmiHeader.biCompression = 0;
		pbi->bmiHeader.biSizeImage = nDispRectDataSize;
		pbi->bmiHeader.biXPelsPerMeter = 3780;
		pbi->bmiHeader.biYPelsPerMeter = 3780;
		pbi->bmiHeader.biClrUsed = 0;
		pbi->bmiHeader.biClrImportant = 0;
		GColor* pRGBQuads = pImage->GetRGBQuads();
		unsigned char* pRGBDest = pData + sizeof(BITMAPINFO);
		int n;
		for(n = 0; n < m_nTilesPerDispRect; n++)
		{
			int nTile = m_nTilesPerDispRect * nDispRect + n;
			if(nTile >= m_nTileCount)
			{
				GAssert(nDispRect + 1 == m_nDispRectCount, "tsnh");
				break;
			}
			int nBitmapTileX = (nTile % nHorizTiles) * nTileWidth;
			int nBitmapTileY = (nTile / nHorizTiles) * nTileHeight;
			int nDispRectTileX = (n % nHorizTilesPerDispRect) * nTileWidth;
			int nDispRectTileY = (n / nHorizTilesPerDispRect) * nTileHeight;
			int x, y;
			int nSourcePos;
			int nDestPos;
			for(y = 0; y < nTileHeight; y++)
			{
				for(x = 0; x < nTileWidth; x++)
				{
					nSourcePos = (nBitmapTileY + y) * nWidth + (nBitmapTileX + x);
					nDestPos = ((nDispRectHeight - 1 - (nDispRectTileY + y)) * nDispRectDataWidth + (nDispRectTileX + x) * 3);
					GColor col = pRGBQuads[nSourcePos];
					pRGBDest[nDestPos] = gBlue(col);
					pRGBDest[nDestPos + 1] = gGreen(col);
					pRGBDest[nDestPos + 2] = gRed(col);
				}
			}
		}

//		memset(pRGBDest, 255, nDispRectSize * 3);

		HBITMAP hBitmap;
//		HDC hTmpDC = CreateCompatibleDC(NULL/*dc.m_hDC*/);
//		hBitmap = CreateDIBitmap(hTmpDC, &pbi->bmiHeader, CBM_INIT, pData + sizeof(BITMAPINFO), pbi, DIB_RGB_COLORS);
//		hBitmap = CreateDIBitmap(NULL/*hTmpDC*/, &pbi->bmiHeader, NULL, NULL, NULL, DIB_RGB_COLORS);
//		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hTmpDC, hBitmap);
hBitmap = (HBITMAP)LoadImage(NULL, "GameStarterData\\blank.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION);
GAssert(hBitmap, "Failed to load blank.bmp.  It is required that this bitmap file be in the directory.");
//		BITMAP bm;
//		GetObject(hBitmap, sizeof(bm), &bm);    // get size of bitmap

		if(SetDIBits(NULL/*hTmpDC*/, hBitmap, 0, nDispRectHeight, pData + sizeof(BITMAPINFO), pbi, DIB_RGB_COLORS) < nDispRectHeight)
		{
			GAssert(false, "wrong number of scan lines copied");
		}

		// Allocate the disp rect
		m_pArrDispRects[nDispRect] = new GDispRect();
		m_pArrDispRects[nDispRect]->Init(pScreen, hBitmap);
		m_pArrDispRects[nDispRect]->SetBackGroundCol(0);

		// Clean up
//		SelectObject(hTmpDC, hOldBitmap);
		DeleteObject(hBitmap);	
//		DeleteDC(hTmpDC);
		delete(pData);
	}
	m_pScreen = pScreen;
	return true;
}

int GTile::GetTileCount()
{
	return m_nTileCount;
}

void GTile::SetTile(int nTile)
{
	GAssert(nTile >= 0 && nTile < m_nTileCount, "Tile out of range");
	m_nSelectedDispRect = nTile / m_nTilesPerDispRect;
	GAssert(m_nSelectedDispRect >= 0 && m_nSelectedDispRect < m_nDispRectCount, "Invalid disp rect");
	int nRelativeTile = nTile % m_nTilesPerDispRect;
	int nX = (nRelativeTile % m_nHorizTilesPerDispRect) * m_nTileWidth;
	int nY = (nRelativeTile / m_nHorizTilesPerDispRect) * m_nTileHeight;
	m_pArrDispRects[m_nSelectedDispRect]->SetSrc(nY, nX, nY + m_nTileHeight - 1, nX + m_nTileWidth - 1);
}

void GTile::Draw(int nX, int nY)
{
	GAssert(m_nSelectedDispRect >= 0, "You didn't call SetTile yet");
	int rval = m_pScreen->GetDispRect()->m_lpDDS->BltFast(nX, nY, m_pArrDispRects[m_nSelectedDispRect]->m_lpDDS, &m_pArrDispRects[m_nSelectedDispRect]->SrcRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	if(rval != DD_OK)
	{
		ShowDDError(rval);	
	}
	GAssert(rval != DDERR_SURFACELOST, "Surface was lost!");
}

void GTile::DrawScaled(int nX, int nY, float fScale)
{
	int rval;
	GDispRect* pDR = m_pArrDispRects[m_nSelectedDispRect];
	pDR->DestRect.top = nY;
	pDR->DestRect.left = nX;
	pDR->DestRect.bottom = (int)((nY + m_nTileWidth) * fScale);
	pDR->DestRect.right = (int)((nX + m_nTileHeight) * fScale);

	rval = m_pScreen->GetDispRect()->m_lpDDS->Blt(&pDR->DestRect, pDR->m_lpDDS, &pDR->SrcRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	if(rval != DD_OK)
	{
		ShowDDError(rval);	
	}
	GAssert(rval != DDERR_SURFACELOST, "Surface was lost!");

	
//	GLog("Not implemented yet");
}
