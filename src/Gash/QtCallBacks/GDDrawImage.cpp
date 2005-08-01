/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "GDDrawImage.h"
#include <math.h>
#include "../../GClasses/GDDrawWindow.h"
#include "../../GClasses/GDDraw.h"
#include <qimage.h>
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GImage.h"

GDDrawImage::GDDrawImage(GDDrawWindow *pWindow)
{
    m_pWindow = pWindow;
    m_pCanvas = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nTransparentColor = -1;
	m_DC = NULL;
	m_Font = NULL;
}

GDDrawImage::~GDDrawImage()
{
	if(m_pCanvas)
	{
		m_pCanvas->Release();
		m_pCanvas = NULL;
	}
}

bool GDDrawImage::Init(HBITMAP hBM)
{
	// Get size of the bitmap
	BITMAP bm;
	GetObject(hBM, sizeof(bm), &bm);      // get size of bitmap

	// Create a DirectDrawSurface for this bitmap
	ZeroMemory(&m_DDSD, sizeof(m_DDSD));
	m_DDSD.dwSize = sizeof(m_DDSD);
	m_DDSD.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	m_DDSD.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	m_DDSD.dwWidth = bm.bmWidth;
	m_DDSD.dwHeight = bm.bmHeight;

	int nRes = m_pWindow->GetDD()->CreateSurface(&m_DDSD, &m_pCanvas, NULL);
	if (nRes != DD_OK)
	{
		GAssert(false, "Error creating surface");
		ShowDDError(nRes);
		return false;
	}
	DDCopyBitmap(m_pCanvas, hBM, 0, 0, 0, 0);
    m_nWidth = bm.bmWidth;
    m_nHeight = bm.bmHeight;
    m_bltFlags = DDBLTFAST_WAIT;
    m_bltScaledFlags = DDBLT_WAIT;
	return true;
}

bool GDDrawImage::Init(int Width, int Height)
{
	HRESULT rval;

	ZeroMemory(&m_DDSD, sizeof(m_DDSD));
	m_DDSD.dwSize = sizeof(m_DDSD);
	m_DDSD.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	m_DDSD.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	m_DDSD.dwWidth = Width;
	m_DDSD.dwHeight = Height;

	rval = m_pWindow->GetDD()->CreateSurface(&m_DDSD, &m_pCanvas, NULL);
	if(rval != DD_OK)
	{
		ShowDDError(rval);
		return false;
	}
	m_nWidth = Width;
    m_nHeight = Height;
	return true;
}

bool GDDrawImage::Init(GImage* pImage)
{
	// Calculate widths and heights and stuff
	int nWidth = pImage->GetWidth();
	int nHeight = pImage->GetHeight();
	int nDispRectDataWidth = nWidth * 3;
	if(nDispRectDataWidth % 4 > 0) // Bitmaps are alligned with each row on a 4-byte boundary
		nDispRectDataWidth += (4 - nDispRectDataWidth % 4);
	int nDispRectDataSize = nDispRectDataWidth * nHeight;

	// Make a bitmap
	unsigned char* pData = new unsigned char[nDispRectDataSize + sizeof(BITMAPINFO) + nDispRectDataWidth]; // there is an extra line for padding
	GAssert(pData, "Failed to allocate");
	BITMAPINFO* pbi = (BITMAPINFO*)pData;
	pbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbi->bmiHeader.biWidth = nWidth;
	pbi->bmiHeader.biHeight = nHeight;
	pbi->bmiHeader.biPlanes = 1;
	pbi->bmiHeader.biBitCount = 24;
	pbi->bmiHeader.biCompression = 0;
	pbi->bmiHeader.biSizeImage = nDispRectDataSize;
	pbi->bmiHeader.biXPelsPerMeter = 3780;
	pbi->bmiHeader.biYPelsPerMeter = 3780;
	pbi->bmiHeader.biClrUsed = 0;
	pbi->bmiHeader.biClrImportant = 0;
	unsigned char* pRGBQuads = pImage->GetRGBQuads();
	unsigned char* pRGBDest = pData + sizeof(BITMAPINFO);
	int x, y;
	int nSourcePos;
	int nDestPos;
	for(y = 0; y < nHeight; y++)
	{
		for(x = 0; x < nWidth; x++)
		{
			nSourcePos = (y * nWidth + x) << 2;
			nDestPos = (nHeight - 1 - y) * nDispRectDataWidth + (nWidth + x) * 3;
			pRGBDest[nDestPos] = pRGBQuads[nSourcePos + 2];
			pRGBDest[nDestPos + 1] = pRGBQuads[nSourcePos + 1];
			pRGBDest[nDestPos + 2] = pRGBQuads[nSourcePos];
		}
	}
    HDC tmpDC = ::GetDC(NULL);
    HBITMAP hBitmap = CreateDIBitmap(tmpDC, &pbi->bmiHeader, CBM_INIT, pData + sizeof(BITMAPINFO), pbi, DIB_RGB_COLORS);
	if(SetDIBits(NULL, hBitmap, 0, nHeight, pData + sizeof(BITMAPINFO), pbi, DIB_RGB_COLORS) < nHeight)
	{
		GAssert(false, "wrong number of scan lines copied");
	}

    // Initialize the image with the bitmap
    bool bRet = Init(hBitmap);

	// Clean up
	DeleteObject(hBitmap);
	delete [] pData;
	return bRet;
}

bool GDDrawImage::Init(QImage* pImage)
{
	// Calculate widths and heights and stuff
	int nWidth = pImage->width();
	int nHeight = pImage->height();
	int nDispRectDataWidth = nWidth * 3;
	if(nDispRectDataWidth % 4 > 0) // Bitmaps are alligned with each row on a 4-byte boundary
		nDispRectDataWidth += (4 - nDispRectDataWidth % 4);
	int nDispRectDataSize = nDispRectDataWidth * nHeight;

	// Make a bitmap
	unsigned char* pData = new unsigned char[nDispRectDataSize + sizeof(BITMAPINFO) + nDispRectDataWidth]; // there is an extra line for padding
	GAssert(pData, "Failed to allocate");
	BITMAPINFO* pbi = (BITMAPINFO*)pData;
	pbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbi->bmiHeader.biWidth = nWidth;
	pbi->bmiHeader.biHeight = nHeight;
	pbi->bmiHeader.biPlanes = 1;
	pbi->bmiHeader.biBitCount = 24;
	pbi->bmiHeader.biCompression = 0;
	pbi->bmiHeader.biSizeImage = nDispRectDataSize;
	pbi->bmiHeader.biXPelsPerMeter = 3780;
	pbi->bmiHeader.biYPelsPerMeter = 3780;
	pbi->bmiHeader.biClrUsed = 0;
	pbi->bmiHeader.biClrImportant = 0;
	unsigned char* pRGBDest = pData + sizeof(BITMAPINFO);
	int x, y;
	int nSourcePos;
	int nDestPos;
	for(y = 0; y < nHeight; y++)
	{
		for(x = 0; x < nWidth; x++)
		{
			nSourcePos = (y * nWidth + x) << 2;
			nDestPos = (nHeight - 1 - y) * nDispRectDataWidth + (nWidth + x) * 3;
			pRGBDest[nDestPos] = qBlue(pImage->pixel(x, y));
			pRGBDest[nDestPos + 1] = qGreen(pImage->pixel(x, y));
			pRGBDest[nDestPos + 2] = qRed(pImage->pixel(x, y));
		}
	}
    HDC tmpDC = ::GetDC(NULL);
    HBITMAP hBitmap = CreateDIBitmap(tmpDC, &pbi->bmiHeader, CBM_INIT, pData + sizeof(BITMAPINFO), pbi, DIB_RGB_COLORS);
	if(SetDIBits(NULL, hBitmap, 0, nHeight, pData + sizeof(BITMAPINFO), pbi, DIB_RGB_COLORS) < nHeight)
	{
		GAssert(false, "wrong number of scan lines copied");
	}

    // Initialize the image with the bitmap
    bool bRet = Init(hBitmap);

	// Clean up
	DeleteObject(hBitmap);
	delete [] pData;
	return bRet;
}

HRESULT GDDrawImage::Blt(int nSourceX, int nSourceY, int nSourceWidth, int nSourceHeight, GDDrawImage* pDestImage, int nDestX, int nDestY)
{
	HRESULT hr;
    RECT SrcRect;
    SrcRect.left = nSourceX;
    SrcRect.top = nSourceY;
    SrcRect.right = nSourceX + nSourceWidth - 1;
    SrcRect.bottom = nSourceY + nSourceHeight - 1;
	hr = pDestImage->m_pCanvas->BltFast(nDestX, nDestY, m_pCanvas, &SrcRect, m_bltFlags);
	if(hr == DDERR_SURFACELOST)
    	m_pCanvas->Restore();
	return hr;
}

HRESULT GDDrawImage::SafeBlt(int nSourceX, int nSourceY, int nSourceWidth, int nSourceHeight, GDDrawImage* pDestImage, int nDestX, int nDestY)
{
	HRESULT hr;
    RECT SrcRect;
    SrcRect.left = nSourceX;
    SrcRect.top = nSourceY;
    SrcRect.right = nSourceX + nSourceWidth - 1;
    SrcRect.bottom = nSourceY + nSourceHeight - 1;
    if(nDestX < 0)
    {
        SrcRect.left -= nDestX;
        nDestX = 0;
        if(SrcRect.left > SrcRect.right)
            return S_OK;
    }
    if(nDestY < 0)
    {
        SrcRect.top -= nDestY;
        nDestY = 0;
        if(SrcRect.top > SrcRect.bottom)
            return S_OK;
    }
    if(nDestX + SrcRect.right - SrcRect.left > pDestImage->m_nWidth)
    {
        SrcRect.right = pDestImage->m_nWidth - SrcRect.left - 1;
        if(SrcRect.right < SrcRect.left)
            return S_OK;
    }
    if(nDestY + SrcRect.bottom - SrcRect.top > pDestImage->m_nHeight)
    {
        SrcRect.bottom = pDestImage->m_nHeight - SrcRect.top - 1;
        if(SrcRect.bottom < SrcRect.top)
            return S_OK;
    }
    hr = pDestImage->m_pCanvas->BltFast(nDestX, nDestY, m_pCanvas, &SrcRect, m_bltFlags);
	if(hr == DDERR_SURFACELOST)
    	m_pCanvas->Restore();
	return hr;
}

HRESULT GDDrawImage::BltScaled(int nSourceX, int nSourceY, int nSourceWidth, int nSourceHeight, GDDrawImage* pDestImage, int nDestX, int nDestY, int nDestWidth, int nDestHeight)
{
	HRESULT hr;
    RECT DestRect;
    RECT SrcRect;
    DestRect.left = nDestX;
    DestRect.top = nDestY;
    DestRect.right = nDestX + nDestWidth - 1;
    DestRect.bottom = nDestY + nDestHeight - 1;
    SrcRect.left = nSourceX;
    SrcRect.top = nSourceY;
    SrcRect.right = nSourceX + nSourceWidth - 1;
    SrcRect.bottom = nSourceY + nSourceHeight - 1;
	hr = pDestImage->m_pCanvas->Blt(&DestRect, m_pCanvas, &SrcRect, m_bltScaledFlags, NULL);
	if(hr == DDERR_SURFACELOST)
    	m_pCanvas->Restore();
	return hr;
}

void GDDrawImage::SetTransparentColor(int col)
{
	DDCOLORKEY ddck;
	ddck.dwColorSpaceLowValue = col;
	ddck.dwColorSpaceHighValue = col;
	m_pCanvas->SetColorKey(DDCKEY_SRCBLT, &ddck);
    if(col >= 0)
    {
        m_bltFlags |= DDBLTFAST_SRCCOLORKEY;
        m_bltScaledFlags |= DDBLT_KEYSRC;
    }
    else
    {
        m_bltFlags &= (~DDBLTFAST_SRCCOLORKEY);
        m_bltScaledFlags &= (~DDBLT_KEYSRC);
    }
}

HRESULT GDDrawImage::Lock(void)
{
	ZeroMemory(&m_DDSD, sizeof(m_DDSD));
	m_DDSD.dwSize = sizeof(m_DDSD);
	return m_pCanvas->Lock(NULL, &m_DDSD, DDLOCK_WAIT, NULL);
}

HRESULT GDDrawImage::UnLock(void)
{
	return m_pCanvas->Unlock(&m_DDSD);
}

void GDDrawImage::GetDC()
{
	m_pCanvas->GetDC(&m_DC);
}

void GDDrawImage::ReleaseDC()
{
	m_pCanvas->ReleaseDC(m_DC);
}

void GDDrawImage::ChangeFont(const char* FontName, int Width, int Height, int Attributes)
{
	m_Font = CreateFont(Height, Width,
											0, 0,
											Attributes,
											FALSE,
											FALSE,
											FALSE,
											ANSI_CHARSET,
											OUT_DEFAULT_PRECIS,
											CLIP_DEFAULT_PRECIS,
											NONANTIALIASED_QUALITY,
											VARIABLE_PITCH,
											FontName);
}

void GDDrawImage::SetFont(void)
{
	SelectObject(m_DC, m_Font);
}

void GDDrawImage::TextXY(int x, int y, COLORREF col, LPCTSTR pString)
{
	if(x < 0 || y < 0 || x > 639 || y > 479)
		return;
	SetBkMode(m_DC, TRANSPARENT);
	SetTextColor(m_DC, col);
	TextOut(m_DC, x, y, pString, strlen(pString));
}

void GDDrawImage::SetPixel(int x, int y, int Col)
{
	unsigned short *Bitmap = (unsigned short*)m_DDSD.lpSurface;
	Bitmap[y * m_DDSD.lPitch + x] = Col;
}

void GDDrawImage::SafeSetPixel(int x, int y, int Col)
{
	if(x >= 0 && x < m_nWidth && y >= 0 && y < m_nHeight)
    {
	    unsigned short *Bitmap = (unsigned short*)m_DDSD.lpSurface;
	    Bitmap[y * m_DDSD.lPitch + x] = Col;
    }
}

int GDDrawImage::GetPixel(int x, int y)
{
	unsigned short *Bitmap = (unsigned short*)m_DDSD.lpSurface;
	return Bitmap[y * m_DDSD.lPitch + x];
}

inline void Swap(int *a, int *b)
{
	int Temp = *a;
	*a = *b;
	*b = Temp;
}

void GDDrawImage::Box(int x1, int y1, int x2, int y2, int Col, bool bFill)
{
   if(bFill)
   {
      DDBLTFX ddbltfx;
	   RECT Rect = { x1, y1, x2, y2 };
	   
	   ddbltfx.dwSize = sizeof(ddbltfx);
	   ddbltfx.dwFillColor = Col;

	   m_pCanvas->Blt(&Rect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
   }
   else
   {
      Line(x1, y1, x2, y1, Col);
      Line(x1, y2, x2, y2, Col);
      Line(x1, y1, x1, y2, Col);
      Line(x2, y1, x2, y2, Col);
   }
}


void GDDrawImage::Line(int X1, int Y1, int X2, int Y2, int Col)
{
	double xStep, yStep, X, Y;
	int xLength, yLength, xCount, yCount;

	xLength = abs(X2 - X1);
	yLength = abs(Y2 - Y1);

	if(xLength == 0)
	{
		VLine(Y1, Y2, X1, Col);
		return;
	}
	if(yLength == 0)
	{
		HLine(X1, X2, Y1, Col);
		return;
	}

	if(xLength > yLength)
	{
		if(X1 > X2)
		{
			Swap(&X1, &X2);
			Swap(&Y1, &Y2);
		}
		yStep = (double)(Y2 - Y1) / (double)(X2 - X1);
		Y = Y1;
		for(xCount = X1; xCount <= X2; xCount++)
		{
            SetPixel(xCount, (int)Y, Col);
            Y += yStep;
		}
	}
	else
	{
		if(Y1 > Y2)
		{
			Swap(&X1, &X2);
			Swap(&Y1, &Y2);
		}
		xStep = (double)(X2 - X1) / (double)(Y2 - Y1);
		X = X1;
		for(yCount = Y1; yCount <= Y2; yCount++)
		{
            SetPixel((int)X, yCount, Col);
			X += xStep;
		}
	}
}


void GDDrawImage::VLine(int Y1, int Y2, int X, int Col)
{
   if(X < 0 || X > 1279)
      return;
   BYTE *Bitmap = (BYTE*)m_DDSD.lpSurface;

   if(Y1 < 0 || Y1 > 479 || Y2 < 0 || Y2 > 479)
   {
	   for( int i = Y1; i < Y2; i++ )
		   if(i >= 0 && i <= 479)
            Bitmap[i * m_DDSD.lPitch + X] = Col;
   }
   else
   {
	   for( int i = Y1; i < Y2; i++ )
		   Bitmap[i * m_DDSD.lPitch + X] = Col;
   }
}


void GDDrawImage::HLine(int X1, int X2, int Y, int Col)
{
   if(Y < 0 || Y > 479)
      return;
   if(X1 < 0 || X1 > 1279 || X2 < 0 || X2 > 1279)
   {
      for(int i = X1; i < X2; i++)
		   SetPixel(i, Y, Col);
   }
   else
   {
      for(int i = X1; i < X2; i++)
		   SetPixel(i, Y, Col);
   }
}

void GDDrawImage::Circle(int X, int Y, int Radius, int Col)
{
	int iX, iY;
	float Angle;

	for (Angle = 0; Angle < 2 * (float)PI; Angle += (float)PI / 180) 
	{
		iX = X + (int)((float)Radius * cos(Angle));
		iY = Y + (int)((float)Radius * sin(Angle));
		SetPixel(iX, iY, Col);
	}
}

void GDDrawImage::Fill(DWORD FillColor)
{
	DDBLTFX ddBltFx;

	ddBltFx.dwSize = sizeof(DDBLTFX);
	ddBltFx.dwFillColor = FillColor;
	m_pCanvas->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddBltFx);
}
