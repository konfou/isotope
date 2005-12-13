/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GDDrawImage_H__
#define __GDDrawImage_H__

#include <ddraw.h>

class GDDrawWindow;
class GImage;
class QImage;

class GDDrawImage
{
friend GDDrawWindow;
protected:
	int m_nWidth;
	int m_nHeight;
	int m_nTransparentColor;
	HDC m_DC;
	HFONT m_Font;
	DDSURFACEDESC m_DDSD;
	LPDIRECTDRAWSURFACE m_pCanvas;
    GDDrawWindow* m_pWindow;
    DWORD m_bltFlags;
    DWORD m_bltScaledFlags;

public:
	GDDrawImage(GDDrawWindow *pScreen);
	virtual ~GDDrawImage();

	bool Init(HBITMAP hBM);
	bool Init(int Width, int Height); // currently doesn't work
    bool Init(GImage* pImage);
    bool Init(QImage* pImage);
	void SetDest(int t, int l, int b, int r);
 	void SetSrc(int t, int l, int b, int r);
    void SetTransparentColor(int col);
	void Fill(DWORD FillColor);
	void ChangeFont(const char* FontName, int Width, int Height, int Attributes = FW_NORMAL);
	void SetFont(void);
	void TextXY(int X, int Y, COLORREF Col, LPCTSTR pString);
    void SetPixel(int x, int y, int Col);
    void SafeSetPixel(int x, int y, int Col);
	int  GetPixel(int X, int Y);
	void Box(int X1,int Y1,int X2,int Y2,int Col, bool bFill);
	void Line(int X1, int Y1, int X2, int Y2, int Col);
	void Circle(int X, int Y, int Radius, int Col);
    HRESULT Blt(int nSourceX, int nSourceY, int nSourceWidth, int nSourceHeight, GDDrawImage* pDestImage, int nDestX, int nDestY);
    HRESULT SafeBlt(int nSourceX, int nSourceY, int nSourceWidth, int nSourceHeight, GDDrawImage* pDestImage, int nDestX, int nDestY);
    HRESULT BltScaled(int nSourceX, int nSourceY, int nSourceWidth, int nSourceHeight, GDDrawImage* pDestImage, int nDestX, int nDestY, int nDestWidth, int nDestHeight);
	void GetDC();
	void ReleaseDC();
	HRESULT Lock();
	HRESULT UnLock();

protected:
	void VLine(int Y1, int Y2, int X, int Col);
	void HLine(int X1, int X2, int Y, int Col);
};

#endif __GDDrawImage_H__
