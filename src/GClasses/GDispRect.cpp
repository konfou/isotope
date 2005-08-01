/*
	Copyright (C) 1999, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include <math.h>
#include "GDDraw.h"
#include "GMacros.h"

GDispRect::GDispRect()
{
	m_lpDDS = NULL;
	SetInitialValues(0, 0);
}

GDispRect::GDispRect(GWindow *pScreen, int Width, int Height)
{
	Init(pScreen, Width, Height);
}

GDispRect::~GDispRect()
{
	if(m_lpDDS)
	{
		m_lpDDS->Release();
		m_lpDDS = NULL;
	}
	delete(m_pFilename);
}

bool GDispRect::Init(GWindow *pScreen, HBITMAP hBM)
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

	int nRes = pScreen->GetDD()->CreateSurface(&m_DDSD, &m_lpDDS, NULL);
	if (nRes != DD_OK)
	{
		GAssert(false, "Error creating surface");
		ShowDDError(nRes);
		return false;
	}
	DDCopyBitmap(m_lpDDS, hBM, 0, 0, 0, 0);
	SetFilename(NULL);
	SetInitialValues(bm.bmWidth, bm.bmHeight);
	return true;
}

bool GDispRect::Init(GWindow *pScreen, int Width, int Height)
{
	HRESULT rval;

	ZeroMemory(&m_DDSD, sizeof(m_DDSD));
	m_DDSD.dwSize = sizeof(m_DDSD);
	m_DDSD.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	m_DDSD.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	m_DDSD.dwWidth = Width;
	m_DDSD.dwHeight = Height;

	rval = pScreen->GetDD()->CreateSurface(&m_DDSD, &m_lpDDS, NULL);
	if(rval != DD_OK)
	{
		ShowDDError(rval);
		return false;
	}
	SetFilename(NULL);
	SetInitialValues(Width, Height);
	return true;
}

void GDispRect::SetInitialValues(int nWidth, int nHeight)
{
	m_DC = NULL;
	m_Font = NULL;
	m_pFilename = NULL;
	m_ColorKey = -1;

	m_PixelWidth = nWidth;
	m_PixelHeight = nHeight;

	DestRect.top = 0;
	DestRect.left = 0;
	DestRect.bottom = m_PixelHeight;
	DestRect.right = m_PixelWidth;

	SrcRect.top = 0;
	SrcRect.left = 0;
	SrcRect.bottom = m_PixelHeight;
	SrcRect.right = m_PixelWidth;
}

void GDispRect::SetFilename(const char* szFilename)
{
	delete(m_pFilename);
	if(szFilename)
	{
		m_pFilename = new char[strlen(szFilename) + 1];
		strcpy(m_pFilename, szFilename);
	}
	else
		m_pFilename = NULL;
}

HRESULT GDispRect::Draw(GDispRect* lpDDest)
{
	HRESULT rval;

	rval = lpDDest->m_lpDDS->Blt(&DestRect, m_lpDDS, &SrcRect, DDBLT_WAIT, NULL);
	if(rval == DDERR_SURFACELOST)
		Restore();

	return rval;
}

HRESULT GDispRect::DrawFast(int X, int Y, GDispRect* lpDDest)
{
	HRESULT rval;

	rval = lpDDest->m_lpDDS->BltFast(X, Y, m_lpDDS, &SrcRect, DDBLTFAST_WAIT);
	if(rval == DDERR_SURFACELOST)
		Restore();

	return rval;
}

HRESULT GDispRect::DrawTrans(int X, int Y, GDispRect* lpDDest)
{
	HRESULT rval;

	rval = lpDDest->m_lpDDS->BltFast(X, Y, m_lpDDS, &SrcRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	if(rval == DDERR_SURFACELOST)
		Restore();

	return rval;
}



HRESULT GDispRect::DrawClipped(int X, int Y, GDispRect* lpDDest, LPRECT ClipRect)
{
	HRESULT rval;
	RECT ModSrc = SrcRect;
	Clip(&X, &Y, &ModSrc, ClipRect);

	rval = lpDDest->m_lpDDS->BltFast(X, Y, m_lpDDS, &ModSrc, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	if(rval == DDERR_SURFACELOST)
		Restore();

	return rval;
}

HRESULT GDispRect::DrawWindowed(GWindow* pScreen, GDispRect* lpDDest)
{
	HRESULT rval;
	RECT Window;
	GetClientRect((HWND__*)pScreen->GetHWnd(), &Window);
	ClientToScreen((HWND__*)pScreen->GetHWnd(), (LPPOINT)&Window);
	ClientToScreen((HWND__*)pScreen->GetHWnd(), (LPPOINT)&Window+1);

	rval = lpDDest->m_lpDDS->Blt(&Window, m_lpDDS, NULL, DDBLT_WAIT, NULL);
	if(rval == DDERR_SURFACELOST)
		Restore();

	return rval;
}

HRESULT GDispRect::DrawScaled(int X, int Y, float Factor, GDispRect* lpDDS)
{
	HRESULT rval;

	DestRect.top = Y;
	DestRect.left = X;
	DestRect.bottom = (long)((Y + m_PixelHeight) * Factor);
	DestRect.right = (long)((X + m_PixelWidth) * Factor);

	rval = lpDDS->m_lpDDS->Blt(&DestRect, m_lpDDS, &SrcRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	if(rval == DDERR_SURFACELOST)
		Restore();

	return rval;
}

HRESULT GDispRect::DrawHFlip(int X, int Y, GDispRect* Dest)
{
	DDBLTFX ddBltFx;

	ddBltFx.dwSize = sizeof(DDBLTFX);
	ddBltFx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
	return m_lpDDS->Blt(NULL, NULL, NULL, DDBLT_DDFX | DDBLT_WAIT, &ddBltFx);
}


// GDispRect DrawVFlip

HRESULT GDispRect::DrawVFlip(int X, int Y, GDispRect* Dest)
{
	DDBLTFX ddBltFx;

	ddBltFx.dwSize = sizeof(DDBLTFX);
	ddBltFx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
	return m_lpDDS->Blt(NULL, NULL, NULL, DDBLT_DDFX | DDBLT_WAIT, &ddBltFx);
}

void GDispRect::SetDest(int t, int l, int b, int r)
{
	DestRect.top = t;
	DestRect.left = l;
	DestRect.bottom = b;
	DestRect.right = r;
}

void GDispRect::SetSrc(int t, int l, int b, int r)
{
	SrcRect.top = t;
	SrcRect.left = l;
	SrcRect.bottom = b;
	SrcRect.right = r;
}

void GDispRect::SetBackGroundCol(int col)
{
	DDCOLORKEY ddck;

	m_ColorKey = col;
	ddck.dwColorSpaceLowValue = col;
	ddck.dwColorSpaceHighValue = col;

	m_lpDDS->SetColorKey(DDCKEY_SRCBLT, &ddck);
}

void GDispRect::Restore(void)
{
	m_lpDDS->Restore();
	GAssert(false, "having to relaod from file");
	DDReLoadBitmap(m_lpDDS, m_pFilename);
}

HRESULT GDispRect::Lock(void)
{
	ZeroMemory(&m_DDSD, sizeof(m_DDSD));
	m_DDSD.dwSize = sizeof(m_DDSD);
	return m_lpDDS->Lock(NULL, &m_DDSD, DDLOCK_WAIT, NULL);
}

HRESULT GDispRect::UnLock(void)
{
	return m_lpDDS->Unlock(&m_DDSD);
}

void GDispRect::GetDC()
{
	m_lpDDS->GetDC(&m_DC);
}

void GDispRect::ReleaseDC()
{
	m_lpDDS->ReleaseDC(m_DC);
}

void GDispRect::ChangeFont(const char* FontName, int Width, int Height, int Attributes)
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



void GDispRect::SetFont(void)
{
	SelectObject(m_DC, m_Font);
}



void GDispRect::TextXY(int x, int y, COLORREF col, LPCTSTR pString)
{
	if(x < 0 || y < 0 || x > 639 || y > 479)
		return;
	SetBkMode(m_DC, TRANSPARENT);
	SetTextColor(m_DC, col);
	TextOut(m_DC, x, y, pString, strlen(pString));
}


void GDispRect::PutPix(int X, int Y, int Col)
{
/*   switch(Screen->GetBPP())
   {
   case 8:
      {*/
//         BYTE *Bitmap = (BYTE*)m_DDSD.lpSurface;
if(X >= 0 && X < m_PixelWidth && Y > 0 && Y < m_PixelHeight) // For Safety
{
		unsigned short *Bitmap = (unsigned short*)m_DDSD.lpSurface;
		Bitmap[Y * m_DDSD.lPitch + X] = Col;
}
/*      }
      break;
   case 16: 
      {
         short *Bitmap = (short*)m_DDSD.lpSurface;
	      Bitmap[Y * m_DDSD.lPitch + X] = Col;
      }
      break;
   case 32:
      {
         long *Bitmap = (long*)m_DDSD.lpSurface;
	      Bitmap[Y * m_DDSD.lPitch + X] = Col;
      }
      break;
   }*/
}

void GDispRect::SafePutPix(int X, int Y, int Col)
{
	if(X >= 0 && X <= 1279 && Y > 0 && Y <= 479)
		PutPix(X, Y, Col);
}

void GDispRect::PutPixel(int X, int Y, int Col)
{
	PutPix(X, Y, Col);
}

int GDispRect::GetPixel(int X, int Y)
{
//   BYTE *Bitmap = (BYTE*)m_DDSD.lpSurface;
	unsigned short *Bitmap = (unsigned short*)m_DDSD.lpSurface;
	return Bitmap[Y * m_DDSD.lPitch + X];
}

inline void Swap(int *a, int *b)
{
	int Temp = *a;
	*a = *b;
	*b = Temp;
}

void GDispRect::Box(int x1, int y1, int x2, int y2, int Col, bool bFill)
{
   if(bFill)
   {
      DDBLTFX ddbltfx;
	   RECT Rect = { x1, y1, x2, y2 };
	   
	   ddbltfx.dwSize = sizeof(ddbltfx);
	   ddbltfx.dwFillColor = Col;

	   m_lpDDS->Blt(&Rect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
   }
   else
   {
      Line(x1, y1, x2, y1, Col);
      Line(x1, y2, x2, y2, Col);
      Line(x1, y1, x1, y2, Col);
      Line(x2, y1, x2, y2, Col);
   }
}


void GDispRect::Line(int X1, int Y1, int X2, int Y2, int Col)
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

	bool bSafe = true;
	if(X1 < 0 || X1 > 1279 || X2 < 0 || X2 > 1279)
		bSafe = false;
	if(Y1 < 0 || Y1 > 479 || Y2 < 0 || Y2 > 479)
		bSafe = false;

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
			if(bSafe)
            PutPix(xCount, (int)Y, Col);
			else
            SafePutPix(xCount, (int)Y, Col);
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
			if(bSafe)
            PutPix((int)X, yCount, Col);
         else
            SafePutPix((int)X, yCount, Col);
			X += xStep;
		}
	}
}


void GDispRect::VLine(int Y1, int Y2, int X, int Col)
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


void GDispRect::HLine(int X1, int X2, int Y, int Col)
{
   if(Y < 0 || Y > 479)
      return;
   if(X1 < 0 || X1 > 1279 || X2 < 0 || X2 > 1279)
   {
      for(int i = X1; i < X2; i++)
		   SafePutPix(i, Y, Col);
   }
   else
   {
      for(int i = X1; i < X2; i++)
		   PutPix(i, Y, Col);
   }
}

void GDispRect::Circle(int X, int Y, int Radius, int Col)
{
	int iX, iY;
	float Angle;

	for (Angle = 0; Angle < 2 * (float)PI; Angle += (float)PI / 180) 
	{
		iX = X + (int)((float)Radius * cos(Angle));
		iY = Y + (int)((float)Radius * sin(Angle));
		PutPix(iX, iY, Col);
	}
}

void GDispRect::Fill(DWORD FillColor)
{
	DDBLTFX ddBltFx;

	ddBltFx.dwSize = sizeof(DDBLTFX);
	ddBltFx.dwFillColor = FillColor;
	m_lpDDS->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddBltFx);
}

// Clip - clips a destination rectange and modifys X,Y coords appropriatly
void GDispRect::Clip(int *DestX, int *DestY, RECT *SrcRect, RECT *DestRect)
{
	// If it's partly off the right side of the screen
	if(*DestX + (SrcRect->right - SrcRect->left) > DestRect->right)
		SrcRect->right -= *DestX + (SrcRect->right-SrcRect->left) - DestRect->right;

	// Partly off the left side of the screen
	if(*DestX < DestRect->left)
	{
		SrcRect->left += DestRect->left - *DestX;
		*DestX = DestRect->left;
	}

	// Partly off the top of the screen
	if(*DestY < DestRect->top)
	{
		SrcRect->top += DestRect->top - *DestY;
		*DestY = DestRect->top;
	}

	// If it's partly off the bottom side of the screen
	if(*DestY + (SrcRect->bottom - SrcRect->top) > DestRect->bottom)
	SrcRect->bottom -= ((SrcRect->bottom-SrcRect->top)+*DestY) - DestRect->bottom;

	return;
}


