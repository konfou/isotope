/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "GMacros.h"
#include "GDDrawWindow.h"
#include "GnuSDK.h"
#include "GDDraw.h"
#include <windowsx.h>

#define RELEASE(x) if(x != NULL) { x->Release(); x = NULL; }

void GDDrawWindow::DDError(HRESULT hErr)
{
	const char* szMessage = NULL;
	switch (hErr)
	{
		case DDERR_ALREADYINITIALIZED: szMessage = "DDERR_ALREADYINITIALIZED"; break;
		case DDERR_CANNOTATTACHSURFACE: szMessage = "DDERR_CANNOTATTACHSURFACE"; break;
		case DDERR_CANNOTDETACHSURFACE: szMessage = "DDERR_CANNOTDETACHSURFACE"; break;
		case DDERR_CURRENTLYNOTAVAIL: szMessage = "DDERR_CURRENTLYNOTAVAIL"; break;
		case DDERR_EXCEPTION: szMessage = "DDERR_EXCEPTION"; break;
		case DDERR_GENERIC: szMessage = "DDERR_GENERIC"; break;
		case DDERR_HEIGHTALIGN: szMessage = "DDERR_HEIGHTALIGN"; break;
		case DDERR_INCOMPATIBLEPRIMARY: szMessage = "DDERR_INCOMPATIBLEPRIMARY"; break;
		case DDERR_INVALIDCAPS: szMessage = "DDERR_INVALIDCAPS"; break;
		case DDERR_INVALIDCLIPLIST: szMessage = "DDERR_INVALIDCLIPLIST"; break;
		case DDERR_INVALIDMODE: szMessage = "DDERR_INVALIDMODE"; break;
		case DDERR_INVALIDOBJECT: szMessage = "DDERR_INVALIDOBJECT"; break;
		case DDERR_INVALIDPARAMS: szMessage = "DDERR_INVALIDPARAMS"; break;
		case DDERR_INVALIDPIXELFORMAT: szMessage = "DDERR_INVALIDPIXELFORMAT"; break;
		case DDERR_INVALIDRECT: szMessage = "DDERR_INVALIDRECT"; break;
		case DDERR_LOCKEDSURFACES: szMessage = "DDERR_LOCKEDSURFACES"; break;
		case DDERR_NO3D: szMessage = "DDERR_NO3D"; break;
		case DDERR_NOALPHAHW: szMessage = "DDERR_NOALPHAHW"; break;
		case DDERR_NOCLIPLIST: szMessage = "DDERR_NOCLIPLIST"; break;
		case DDERR_NOCOLORCONVHW: szMessage = "DDERR_NOCOLORCONVHW"; break;
		case DDERR_NOCOOPERATIVELEVELSET: szMessage = "DDERR_NOCOOPERATIVELEVELSET"; break;
		case DDERR_NOCOLORKEY: szMessage = "DDERR_NOCOLORKEY"; break;
		case DDERR_NOCOLORKEYHW: szMessage = "DDERR_NOCOLORKEYHW"; break;
		case DDERR_NODIRECTDRAWSUPPORT: szMessage = "DDERR_NODIRECTDRAWSUPPORT"; break;
		case DDERR_NOEXCLUSIVEMODE: szMessage = "DDERR_NOEXCLUSIVEMODE"; break;
		case DDERR_NOFLIPHW: szMessage = "DDERR_NOFLIPHW"; break;
		case DDERR_NOGDI: szMessage = "DDERR_NOGDI"; break;
		case DDERR_NOMIRRORHW: szMessage = "DDERR_NOMIRRORHW"; break;
		case DDERR_NOTFOUND: szMessage = "DDERR_NOTFOUND"; break;
		case DDERR_NOOVERLAYHW: szMessage = "DDERR_NOOVERLAYHW"; break;
		case DDERR_NORASTEROPHW: szMessage = "DDERR_NORASTEROPHW"; break;
		case DDERR_NOROTATIONHW: szMessage = "DDERR_NOROTATIONHW"; break;
		case DDERR_NOSTRETCHHW: szMessage = "DDERR_NOSTRETCHHW"; break;
		case DDERR_NOT4BITCOLOR: szMessage = "DDERR_NOT4BITCOLOR"; break;
		case DDERR_NOT4BITCOLORINDEX: szMessage = "DDERR_NOT4BITCOLORINDEX"; break;
		case DDERR_NOT8BITCOLOR: szMessage = "DDERR_NOT8BITCOLOR"; break;
		case DDERR_NOTEXTUREHW: szMessage = "DDERR_NOTEXTUREHW"; break;
		case DDERR_NOVSYNCHW: szMessage = "DDERR_NOVSYNCHW"; break;
		case DDERR_NOZBUFFERHW: szMessage = "DDERR_NOZBUFFERHW"; break;
		case DDERR_NOZOVERLAYHW: szMessage = "DDERR_NOZOVERLAYHW"; break;
		case DDERR_OUTOFCAPS: szMessage = "DDERR_OUTOFCAPS"; break;
		case DDERR_OUTOFMEMORY: szMessage = "DDERR_OUTOFMEMORY"; break;
		case DDERR_OUTOFVIDEOMEMORY: szMessage = "DDERR_OUTOFVIDEOMEMORY"; break;
		case DDERR_OVERLAYCANTCLIP: szMessage = "DDERR_OVERLAYCANTCLIP"; break;
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE: szMessage = "DDERR_OVERLAYCOLORKEYONLYONEACTIVE"; break;
		case DDERR_PALETTEBUSY: szMessage = "DDERR_PALETTEBUSY"; break;
		case DDERR_COLORKEYNOTSET: szMessage = "DDERR_COLORKEYNOTSET"; break;
		case DDERR_SURFACEALREADYATTACHED: szMessage = "DDERR_SURFACEALREADYATTACHED"; break;
		case DDERR_SURFACEALREADYDEPENDENT: szMessage = "DDERR_SURFACEALREADYDEPENDENT"; break;
		case DDERR_SURFACEBUSY: szMessage = "DDERR_SURFACEBUSY"; break;
		case DDERR_CANTLOCKSURFACE: szMessage = "DDERR_CANTLOCKSURFACE"; break;
		case DDERR_SURFACEISOBSCURED: szMessage = "DDERR_SURFACEISOBSCURED"; break;
		case DDERR_SURFACELOST: szMessage = "DDERR_SURFACELOST"; break;
		case DDERR_SURFACENOTATTACHED: szMessage = "DDERR_SURFACENOTATTACHED"; break;
		case DDERR_TOOBIGHEIGHT: szMessage = "DDERR_TOOBIGHEIGHT"; break;
		case DDERR_TOOBIGSIZE: szMessage = "DDERR_TOOBIGSIZE"; break;
		case DDERR_TOOBIGWIDTH: szMessage = "DDERR_TOOBIGWIDTH"; break;
		case DDERR_UNSUPPORTED: szMessage = "DDERR_UNSUPPORTED"; break;
		case DDERR_UNSUPPORTEDFORMAT: szMessage = "DDERR_UNSUPPORTEDFORMAT"; break;
		case DDERR_UNSUPPORTEDMASK: szMessage = "DDERR_UNSUPPORTEDMASK"; break;
		case DDERR_VERTICALBLANKINPROGRESS: szMessage = "DDERR_VERTICALBLANKINPROGRESS"; break;
		case DDERR_WASSTILLDRAWING: szMessage = "DDERR_WASSTILLDRAWING"; break;
		case DDERR_XALIGN: szMessage = "DDERR_XALIGN"; break;
		case DDERR_INVALIDDIRECTDRAWGUID: szMessage = "DDERR_INVALIDDIRECTDRAWGUID"; break;
		case DDERR_DIRECTDRAWALREADYCREATED: szMessage = "DDERR_DIRECTDRAWALREADYCREATED"; break;
		case DDERR_NODIRECTDRAWHW: szMessage = "DDERR_NODIRECTDRAWHW"; break;
		case DDERR_PRIMARYSURFACEALREADYEXISTS: szMessage = "DDERR_PRIMARYSURFACEALREADYEXISTS"; break;
		case DDERR_NOEMULATION: szMessage = "DDERR_NOEMULATION"; break;
		case DDERR_REGIONTOOSMALL: szMessage = "DDERR_REGIONTOOSMALL"; break;
		case DDERR_CLIPPERISUSINGHWND: szMessage = "DDERR_CLIPPERISUSINGHWND"; break;
		case DDERR_NOCLIPPERATTACHED: szMessage = "DDERR_NOCLIPPERATTACHED"; break;
		case DDERR_NOHWND: szMessage = "DDERR_NOHWND"; break;
		case DDERR_HWNDSUBCLASSED: szMessage = "DDERR_HWNDSUBCLASSED"; break;
		case DDERR_HWNDALREADYSET: szMessage = "DDERR_HWNDALREADYSET"; break;
		case DDERR_NOPALETTEATTACHED: szMessage = "DDERR_NOPALETTEATTACHED"; break;
		case DDERR_NOPALETTEHW: szMessage = "DDERR_NOPALETTEHW"; break;
		case DDERR_BLTFASTCANTCLIP: szMessage = "DDERR_BLTFASTCANTCLIP"; break;
		case DDERR_NOBLTHW: szMessage = "DDERR_NOBLTHW"; break;
		case DDERR_NODDROPSHW: szMessage = "DDERR_NODDROPSHW"; break;
		case DDERR_OVERLAYNOTVISIBLE: szMessage = "DDERR_OVERLAYNOTVISIBLE"; break;
		case DDERR_NOOVERLAYDEST: szMessage = "DDERR_NOOVERLAYDEST"; break;
		case DDERR_INVALIDPOSITION: szMessage = "DDERR_INVALIDPOSITION"; break;
		case DDERR_NOTAOVERLAYSURFACE: szMessage = "DDERR_NOTAOVERLAYSURFACE"; break;
		case DDERR_EXCLUSIVEMODEALREADYSET: szMessage = "DDERR_EXCLUSIVEMODEALREADYSET"; break;
		case DDERR_NOTFLIPPABLE: szMessage = "DDERR_NOTFLIPPABLE"; break;
		case DDERR_CANTDUPLICATE: szMessage = "DDERR_CANTDUPLICATE"; break;
		case DDERR_NOTLOCKED: szMessage = "DDERR_NOTLOCKED"; break;
		case DDERR_CANTCREATEDC: szMessage = "DDERR_CANTCREATEDC"; break;
		case DDERR_NODC: szMessage = "DDERR_NODC"; break;
		case DDERR_WRONGMODE: szMessage = "DDERR_WRONGMODE"; break;
		case DDERR_IMPLICITLYCREATED: szMessage = "DDERR_IMPLICITLYCREATED"; break;
		case DDERR_NOTPALETTIZED: szMessage = "DDERR_NOTPALETTIZED"; break;
		case DDERR_UNSUPPORTEDMODE: szMessage = "DDERR_UNSUPPORTEDMODE"; break;
		case DDERR_NOMIPMAPHW: szMessage = "DDERR_NOMIPMAPHW"; break;
		case DDERR_INVALIDSURFACETYPE: szMessage = "DDERR_INVALIDSURFACETYPE"; break;
		case DDERR_DCALREADYCREATED: szMessage = "DDERR_DCALREADYCREATED"; break;
		case DDERR_CANTPAGELOCK: szMessage = "DDERR_CANTPAGELOCK"; break;
		case DDERR_CANTPAGEUNLOCK: szMessage = "DDERR_CANTPAGEUNLOCK"; break;
		case DDERR_NOTPAGELOCKED: szMessage = "DDERR_NOTPAGELOCKED"; break;
		case DDERR_NOTINITIALIZED: szMessage = "DDERR_NOTINITIALIZED"; break;

		default: szMessage = "Unknown Error"; break;
	}

    GAssert(false, szMessage);
	PostQuitMessage(0);
}

long PASCAL WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_SETCURSOR:
			SetCursor(NULL);
			return(TRUE);
		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_ESCAPE:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
                default:
				    break;
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND MakeWindow(const char* szTitle, int nWidth, int nHeight, bool bFullScreen)
{
	HWND hWnd;

	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WinProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = NULL;
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH__ *)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = 0;
	WndClass.lpszClassName = "Gash";
	RegisterClass(&WndClass);
	hWnd = CreateWindowEx(WS_EX_TOPMOST, "Gash", szTitle, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, NULL, NULL);
	if (!hWnd)
		return NULL;

	if(!bFullScreen)
	{
		RECT rc;
		DWORD dwStyle;

		dwStyle = GetWindowStyle(hWnd);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX;
		SetWindowLong(hWnd, GWL_STYLE, dwStyle);

		SetRect(&rc, 0, 0, nWidth, nHeight);

		AdjustWindowRectEx(&rc,
		   GetWindowStyle(hWnd),
		   GetMenu(hWnd) != NULL,
		   GetWindowExStyle(hWnd));

		SetWindowPos(hWnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top,
		   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
		   SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	return(hWnd);
}

GDDrawWindow::GDDrawWindow(int Width, int Height, bool bFullScreen, DWORD bpp, GObject* pProvidedBackCanvas, Engine* pEngine)
{
	m_lpDD = NULL;
	m_pCanvasFront = NULL;
	m_pCanvasBack = NULL;
	m_lpDDPalette = NULL;
	m_lpClipper = NULL;
	m_bFlipped = false;
	m_dwPixelWidth = Width;
	m_dwPixelHeight = Height;
    m_hWnd = MakeWindow("", Width, Height, bFullScreen);

	HRESULT rval;
	DWORD dwFlags;
	DDSURFACEDESC ddsd;
	HDC hDC;
	DDSCAPS ddscaps;

	m_lpDD = NULL;
	m_bFullScreen = bFullScreen ? TRUE : FALSE;
	m_lpDDPalette = NULL;
	m_dwPixelWidth = Width;
	m_dwPixelHeight = Height;
	m_BPP = bpp;

	if(bFullScreen)
		dwFlags = DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_ALLOWMODEX;
	else
	{
		hDC = GetDC(NULL);
		m_BPP = GetDeviceCaps(hDC, PLANES) * GetDeviceCaps(hDC, BITSPIXEL);
		ReleaseDC(NULL, hDC);

		dwFlags = DDSCL_NORMAL;
	}

	LPDIRECTDRAW lpDD;

	rval = DirectDrawCreate(NULL, &lpDD, NULL);
	if(rval != DD_OK)
		DDError(rval);

	rval = lpDD->QueryInterface(IID_IDirectDraw2, (LPVOID*)&m_lpDD);
	if(rval != DD_OK)
		DDError(rval);

	RELEASE(lpDD);

	rval = m_lpDD->SetCooperativeLevel((HWND__*)m_hWnd, dwFlags);
	if(rval != DD_OK)
		DDError(rval);

	if(bFullScreen)
	{
		rval = m_lpDD->SetDisplayMode(m_dwPixelWidth, m_dwPixelHeight, m_BPP, 0, 0);
		if(rval != DD_OK)
			DDError(rval);
	}

	ddsd.dwSize = sizeof(ddsd);
	if(bFullScreen)
	{
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
	}
	else
	{
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}

	// Make the front canvas
	m_pCanvasFront = new GDDrawImage(this);

	// Make the back canvas
	if(pProvidedBackCanvas && pEngine)
	{
		m_pBackCanvasVarHolder = new VarHolder(pEngine);
		m_pBackCanvasVarHolder->SetGObject(pProvidedBackCanvas);
		GAssert(pProvidedBackCanvas->object.arrFields[0]->header.nClassID == Library::CLASS_ID_WRAPPER, "expected a wrapper of a GDDrawImage object");
		m_pCanvasBack = (GDDrawImage*)pProvidedBackCanvas->object.arrFields[0]->wrapper.pObject;		
	}
	else
	{
		m_pBackCanvasVarHolder = NULL;
		m_pCanvasBack = new GDDrawImage(this);
	}

	m_pCanvasBack->m_nWidth = Width;
    m_pCanvasBack->m_nHeight = Height;
    m_pCanvasFront->m_nWidth = Width;
	m_pCanvasFront->m_nHeight = Height;

	rval = m_lpDD->CreateSurface(&ddsd, &m_pCanvasFront->m_pCanvas, NULL);
	if(rval != DD_OK)
		DDError(rval);

	if(bFullScreen)
	{
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

		rval = m_pCanvasFront->m_pCanvas->GetAttachedSurface(&ddscaps, &m_pCanvasBack->m_pCanvas);
		if(rval != DD_OK)
			DDError(rval);
	}
	else
	{
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;    
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = Width;
		ddsd.dwHeight = Height;

		rval = m_lpDD->CreateSurface( &ddsd, &m_pCanvasBack->m_pCanvas, NULL );
		if(rval != DD_OK)
			DDError(rval);

		// Create the window clipper
		rval = m_lpDD->CreateClipper(0, &m_lpClipper, NULL);
		if(rval != DD_OK)
			DDError(rval);

		rval = m_lpClipper->SetHWnd(0, m_hWnd);
		if(rval != DD_OK)
			DDError(rval);

		rval = m_pCanvasFront->m_pCanvas->SetClipper(m_lpClipper);
		if(rval != DD_OK)
			DDError(rval);
	}
}

GDDrawWindow::~GDDrawWindow(void)
{
	delete(m_pImageHolder);
	if(m_lpDD != NULL)
		m_lpDD->RestoreDisplayMode();
	if(m_pBackCanvasVarHolder)
		delete(m_pBackCanvasVarHolder);
	else
		delete(m_pCanvasBack);
	delete(m_pCanvasFront);
	RELEASE(m_lpClipper);
	RELEASE(m_lpDDPalette);
	RELEASE(m_lpDD);
}

HWND GDDrawWindow::GetSafeHWnd()
{
	if(!m_bFlipped)
		Update();
	return(m_hWnd);
}

// brings the back-image to the front (GDDrawWindows are double buffered)
HRESULT GDDrawWindow::Update()
{
	HRESULT rval;

	if(m_bFullScreen)
	{
		rval = m_pCanvasFront->m_pCanvas->Flip(NULL, DDFLIP_WAIT);
		if(rval == DDERR_SURFACELOST) Restore();
	}
	else
	{
		RECT Window;
		POINT pt;

		GetClientRect((HWND__*)m_hWnd, &Window);
		pt.x = pt.y = 0;
		ClientToScreen((HWND__*)m_hWnd, &pt);
		OffsetRect(&Window, pt.x, pt.y);

		rval = m_pCanvasFront->m_pCanvas->Blt(&Window, m_pCanvasBack->m_pCanvas, NULL, DDBLT_WAIT, NULL);
		if(rval == DDERR_SURFACELOST)
			Restore();
	}
   
	m_bFlipped = !m_bFlipped;
	return rval;
}

BOOL GDDrawWindow::LoadBitmap(const char* szFilename)
{
	if(szFilename == NULL) return FALSE;
    if(DDReLoadBitmap(m_pCanvasBack->m_pCanvas, szFilename) != DD_OK) return FALSE;

	return TRUE;
}

// GDDrawWindow LoadPalette - loads a palette from a bitmap file
BOOL GDDrawWindow::LoadPalette(const char* szFilename)
{
	if(szFilename == NULL) return FALSE;
	m_lpDDPalette = DDLoadPalette(m_lpDD, szFilename);
	if(m_lpDDPalette == NULL) return FALSE;
    m_pCanvasFront->m_pCanvas->SetPalette(m_lpDDPalette);

	return TRUE;
}

// GDDrawWindow Restore - restores the front buffer if it is lost
void GDDrawWindow::Restore(void)
{
    if(m_pCanvasFront->m_pCanvas != NULL && m_pCanvasFront->m_pCanvas->IsLost() != DD_OK)
    {
        m_pCanvasFront->m_pCanvas->Restore();
    }
}

// GDDrawWindow SetColor - sets the r,g,b value of a single color
void GDDrawWindow::SetColor(int col, GColor* pCol)
{
    int r = pCol->GetRed();
    int g = pCol->GetGreen();
    int b = pCol->GetBlue();

    PALETTEENTRY pe[1];

	m_lpDDPalette->GetEntries(0, col, 1, pe);
	pe[0].peRed = r;
	pe[0].peGreen = g;
	pe[0].peBlue = b;
	m_lpDDPalette->SetEntries(0, col, 1, pe);
}

// GDDrawWindow GetColor - returns the r,g,b values of a single color
void GDDrawWindow::GetColor(int col, GColor* pCol)
{
	PALETTEENTRY pe[1];

	m_lpDDPalette->GetEntries(0, col, 1, pe);
   pCol->SetRed(pe[0].peRed);
   pCol->SetGreen(pe[0].peGreen);
   pCol->SetBlue(pe[0].peBlue);
}


// GDDrawWindow SetPalette - sets the palette
void GDDrawWindow::SetPalette(int Start, int Count, LPPALETTEENTRY lpPE)
{
	m_lpDDPalette->SetEntries(0, Start, Count, lpPE);
}

// GDDrawWindow GetPalette - returns the palette
void GDDrawWindow::GetPalette(int Start, int Count, LPPALETTEENTRY lpPE)
{
	m_lpDDPalette->GetEntries(0, Start, Count, lpPE);
}

// GDDrawWindow FadeTo - fades to a specified color
void GDDrawWindow::Fade(GColor* pCol, int delay)
{
   int r = pCol->GetRed();
   int g = pCol->GetGreen();
   int b = pCol->GetBlue();

   PALETTEENTRY pe[256];

	m_lpDDPalette->GetEntries(0, 0, 256, pe);

	for(int j = 1; j < 84; j++)
	{
		for(int i = 1; i < 256; i++)
		{
			// Red
			if(pe[i].peRed > r+5) pe[i].peRed -= 3;
			else if(pe[i].peRed < r-5) pe[i].peRed += 3;
			else pe[i].peRed = r;

			// Green
			if(pe[i].peGreen > g+5) pe[i].peGreen -= 3;
			else if(pe[i].peGreen < g-5) pe[i].peGreen += 3;
			else pe[i].peGreen = g;

			// Blue
			if(pe[i].peBlue > b+5) pe[i].peBlue -= 3;
			else if(pe[i].peBlue < b-5) pe[i].peBlue += 3;
			else pe[i].peBlue = b;

			// Add a delay here for slower fades
			for(long d = 0; d < delay*100; d++) ;
		}
		m_lpDDPalette->SetEntries(0, 0, 256, pe);
	}
}

GObject* GDDrawWindow::GetCanvasAsGObject(Library* pLibrary, int nImageClassID)
{
	GAssert(m_pBackCanvasVarHolder, "Back canvas wasn't supplied to constructor");
	return m_pBackCanvasVarHolder->GetGObject();
}
