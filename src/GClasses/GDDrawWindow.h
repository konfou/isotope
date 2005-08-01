/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GDDRAWWINDOW_H__
#define __GDDRAWWINDOW_H__

#include <ddraw.h>
#include <vfw.h>

class GDDrawImage;
class Library;
class VarHolder;
class Engine;
struct _GObject;
typedef struct _GObject GObject;

class GDDrawWindow
{
protected:
	int m_dwPixelWidth;
	int m_dwPixelHeight;
	LPDIRECTDRAW2 m_lpDD;
	GDDrawImage* m_pCanvasFront;
	GDDrawImage* m_pCanvasBack;
	LPDIRECTDRAWPALETTE m_lpDDPalette;
	LPDIRECTDRAWCLIPPER m_lpClipper;
	DWORD m_BPP;
	HWND m_hWnd;
	bool m_bFullScreen;
	bool m_bFlipped;
	unsigned char* m_pImageHolder;
	VarHolder* m_pBackCanvasVarHolder;

public:
	GDDrawWindow(int nWidth, int nHeight, bool bFullScreen, DWORD bpp = 16, GObject* pProvidedBackCanvas = NULL, Engine* pEngine = NULL);
	virtual ~GDDrawWindow();

	HWND GetHWnd() { return m_hWnd; }
	HWND GetSafeHWnd();
	DWORD GetWidth() { return m_dwPixelWidth; }
	DWORD GetHeight() { return m_dwPixelHeight; }
	BOOL LoadBitmap(const char* szFilename);
//	void GetColor(int col, GColor* pCol);
//	void SetColor(int col, GColor* pCol);
	BOOL LoadPalette(const char* szFilename);
	void GetPalette(int Start, int Count, LPPALETTEENTRY lpPE);
	void SetPalette(int Start, int Count, LPPALETTEENTRY lpPE);
//	void Fade(GColor* pCol, int delay);
	LPDIRECTDRAW2 GetDD() { return m_lpDD; }
	int GetBPP() { return m_BPP; }
	GDDrawImage* GetCanvas() { return m_pCanvasBack; }
	GObject* GetCanvasAsGObject(Library* pLibrary, int nImageClassID);
	HRESULT Update();

protected:
	LPDIRECTDRAWPALETTE GetPalette(void) { return m_lpDDPalette; }
	GDDrawImage* GetFront(void) { return m_pCanvasFront; }
	BOOL CreateFullScreen(DWORD nWidth, DWORD nHeight, DWORD BitsPerPix, GObject* pProvidedBackCanvas);
	BOOL CreateWindowed(int nWidth, int nHeight, GObject* pProvidedBackCanvas);
	void Restore();
    static void DDError(HRESULT hErr);
};

#endif // __GDDRAWWINDOW_H__
