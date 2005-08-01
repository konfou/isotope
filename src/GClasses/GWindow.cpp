/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GDDraw.h"

#define RELEASE(x) if(x != NULL) { x->Release(); x = NULL; }

void GLogDDError(HRESULT hErr)
{
	char dderr[256];
	char err[1024];

	switch (hErr)
	{
		case DDERR_ALREADYINITIALIZED : sprintf(dderr, "DDERR_ALREADYINITIALIZED"); break;
		case DDERR_CANNOTATTACHSURFACE : sprintf(dderr, "DDERR_CANNOTATTACHSURFACE"); break;
		case DDERR_CANNOTDETACHSURFACE : sprintf(dderr, "DDERR_CANNOTDETACHSURFACE"); break;
		case DDERR_CURRENTLYNOTAVAIL : sprintf(dderr, "DDERR_CURRENTLYNOTAVAIL"); break;
		case DDERR_EXCEPTION : sprintf(dderr, "DDERR_EXCEPTION"); break;
		case DDERR_GENERIC : sprintf(dderr, "DDERR_GENERIC"); break;
		case DDERR_HEIGHTALIGN : sprintf(dderr, "DDERR_HEIGHTALIGN"); break;
		case DDERR_INCOMPATIBLEPRIMARY : sprintf(dderr, "DDERR_INCOMPATIBLEPRIMARY"); break;
		case DDERR_INVALIDCAPS : sprintf(dderr, "DDERR_INVALIDCAPS"); break;
		case DDERR_INVALIDCLIPLIST : sprintf(dderr, "DDERR_INVALIDCLIPLIST"); break;
		case DDERR_INVALIDMODE : sprintf(dderr, "DDERR_INVALIDMODE"); break;
		case DDERR_INVALIDOBJECT : sprintf(dderr, "DDERR_INVALIDOBJECT"); break;
		case DDERR_INVALIDPARAMS : sprintf(dderr, "DDERR_INVALIDPARAMS"); break;
		case DDERR_INVALIDPIXELFORMAT : sprintf(dderr, "DDERR_INVALIDPIXELFORMAT"); break;
		case DDERR_INVALIDRECT : sprintf(dderr, "DDERR_INVALIDRECT"); break;
		case DDERR_LOCKEDSURFACES : sprintf(dderr, "DDERR_LOCKEDSURFACES"); break;
		case DDERR_NO3D : sprintf(dderr, "DDERR_NO3D"); break;
		case DDERR_NOALPHAHW : sprintf(dderr, "DDERR_NOALPHAHW"); break;
		case DDERR_NOCLIPLIST : sprintf(dderr, "DDERR_NOCLIPLIST"); break;
		case DDERR_NOCOLORCONVHW : sprintf(dderr, "DDERR_NOCOLORCONVHW"); break;
		case DDERR_NOCOOPERATIVELEVELSET : sprintf(dderr, "DDERR_NOCOOPERATIVELEVELSET"); break;
		case DDERR_NOCOLORKEY : sprintf(dderr, "DDERR_NOCOLORKEY"); break;
		case DDERR_NOCOLORKEYHW : sprintf(dderr, "DDERR_NOCOLORKEYHW"); break;
		case DDERR_NODIRECTDRAWSUPPORT : sprintf(dderr, "DDERR_NODIRECTDRAWSUPPORT"); break;
		case DDERR_NOEXCLUSIVEMODE : sprintf(dderr, "DDERR_NOEXCLUSIVEMODE"); break;
		case DDERR_NOFLIPHW : sprintf(dderr, "DDERR_NOFLIPHW"); break;
		case DDERR_NOGDI : sprintf(dderr, "DDERR_NOGDI"); break;
		case DDERR_NOMIRRORHW : sprintf(dderr, "DDERR_NOMIRRORHW"); break;
		case DDERR_NOTFOUND : sprintf(dderr, "DDERR_NOTFOUND"); break;
		case DDERR_NOOVERLAYHW : sprintf(dderr, "DDERR_NOOVERLAYHW"); break;
		case DDERR_NORASTEROPHW : sprintf(dderr, "DDERR_NORASTEROPHW"); break;
		case DDERR_NOROTATIONHW : sprintf(dderr, "DDERR_NOROTATIONHW"); break;
		case DDERR_NOSTRETCHHW : sprintf(dderr, "DDERR_NOSTRETCHHW"); break;
		case DDERR_NOT4BITCOLOR : sprintf(dderr, "DDERR_NOT4BITCOLOR"); break;
		case DDERR_NOT4BITCOLORINDEX : sprintf(dderr, "DDERR_NOT4BITCOLORINDEX"); break;
		case DDERR_NOT8BITCOLOR : sprintf(dderr, "DDERR_NOT8BITCOLOR"); break;
		case DDERR_NOTEXTUREHW : sprintf(dderr, "DDERR_NOTEXTUREHW"); break;
		case DDERR_NOVSYNCHW : sprintf(dderr, "DDERR_NOVSYNCHW"); break;
		case DDERR_NOZBUFFERHW : sprintf(dderr, "DDERR_NOZBUFFERHW"); break;
		case DDERR_NOZOVERLAYHW : sprintf(dderr, "DDERR_NOZOVERLAYHW"); break;
		case DDERR_OUTOFCAPS : sprintf(dderr, "DDERR_OUTOFCAPS"); break;
		case DDERR_OUTOFMEMORY : sprintf(dderr, "DDERR_OUTOFMEMORY"); break;
		case DDERR_OUTOFVIDEOMEMORY : sprintf(dderr, "DDERR_OUTOFVIDEOMEMORY"); break;
		case DDERR_OVERLAYCANTCLIP : sprintf(dderr, "DDERR_OVERLAYCANTCLIP"); break;
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE : sprintf(dderr, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE"); break;
		case DDERR_PALETTEBUSY : sprintf(dderr, "DDERR_PALETTEBUSY"); break;
		case DDERR_COLORKEYNOTSET : sprintf(dderr, "DDERR_COLORKEYNOTSET"); break;
		case DDERR_SURFACEALREADYATTACHED : sprintf(dderr, "DDERR_SURFACEALREADYATTACHED"); break;
		case DDERR_SURFACEALREADYDEPENDENT : sprintf(dderr, "DDERR_SURFACEALREADYDEPENDENT"); break;
		case DDERR_SURFACEBUSY : sprintf(dderr, "DDERR_SURFACEBUSY"); break;
		case DDERR_CANTLOCKSURFACE : sprintf(dderr, "DDERR_CANTLOCKSURFACE"); break;
		case DDERR_SURFACEISOBSCURED : sprintf(dderr, "DDERR_SURFACEISOBSCURED"); break;
		case DDERR_SURFACELOST : sprintf(dderr, "DDERR_SURFACELOST"); break;
		case DDERR_SURFACENOTATTACHED : sprintf(dderr, "DDERR_SURFACENOTATTACHED"); break;
		case DDERR_TOOBIGHEIGHT : sprintf(dderr, "DDERR_TOOBIGHEIGHT"); break;
		case DDERR_TOOBIGSIZE : sprintf(dderr, "DDERR_TOOBIGSIZE"); break;
		case DDERR_TOOBIGWIDTH : sprintf(dderr, "DDERR_TOOBIGWIDTH"); break;
		case DDERR_UNSUPPORTED : sprintf(dderr, "DDERR_UNSUPPORTED"); break;
		case DDERR_UNSUPPORTEDFORMAT : sprintf(dderr, "DDERR_UNSUPPORTEDFORMAT"); break;
		case DDERR_UNSUPPORTEDMASK : sprintf(dderr, "DDERR_UNSUPPORTEDMASK"); break;
		case DDERR_VERTICALBLANKINPROGRESS : sprintf(dderr, "DDERR_VERTICALBLANKINPROGRESS"); break;
		case DDERR_WASSTILLDRAWING : sprintf(dderr, "DDERR_WASSTILLDRAWING"); break;
		case DDERR_XALIGN : sprintf(dderr, "DDERR_XALIGN"); break;
		case DDERR_INVALIDDIRECTDRAWGUID : sprintf(dderr, "DDERR_INVALIDDIRECTDRAWGUID"); break;
		case DDERR_DIRECTDRAWALREADYCREATED : sprintf(dderr, "DDERR_DIRECTDRAWALREADYCREATED"); break;
		case DDERR_NODIRECTDRAWHW : sprintf(dderr, "DDERR_NODIRECTDRAWHW"); break;
		case DDERR_PRIMARYSURFACEALREADYEXISTS : sprintf(dderr, "DDERR_PRIMARYSURFACEALREADYEXISTS"); break;
		case DDERR_NOEMULATION : sprintf(dderr, "DDERR_NOEMULATION"); break;
		case DDERR_REGIONTOOSMALL : sprintf(dderr, "DDERR_REGIONTOOSMALL"); break;
		case DDERR_CLIPPERISUSINGHWND : sprintf(dderr, "DDERR_CLIPPERISUSINGHWND"); break;
		case DDERR_NOCLIPPERATTACHED : sprintf(dderr, "DDERR_NOCLIPPERATTACHED"); break;
		case DDERR_NOHWND : sprintf(dderr, "DDERR_NOHWND"); break;
		case DDERR_HWNDSUBCLASSED : sprintf(dderr, "DDERR_HWNDSUBCLASSED"); break;
		case DDERR_HWNDALREADYSET : sprintf(dderr, "DDERR_HWNDALREADYSET"); break;
		case DDERR_NOPALETTEATTACHED : sprintf(dderr, "DDERR_NOPALETTEATTACHED"); break;
		case DDERR_NOPALETTEHW : sprintf(dderr, "DDERR_NOPALETTEHW"); break;
		case DDERR_BLTFASTCANTCLIP : sprintf(dderr, "DDERR_BLTFASTCANTCLIP"); break;
		case DDERR_NOBLTHW : sprintf(dderr, "DDERR_NOBLTHW"); break;
		case DDERR_NODDROPSHW : sprintf(dderr, "DDERR_NODDROPSHW"); break;
		case DDERR_OVERLAYNOTVISIBLE : sprintf(dderr, "DDERR_OVERLAYNOTVISIBLE"); break;
		case DDERR_NOOVERLAYDEST : sprintf(dderr, "DDERR_NOOVERLAYDEST"); break;
		case DDERR_INVALIDPOSITION : sprintf(dderr, "DDERR_INVALIDPOSITION"); break;
		case DDERR_NOTAOVERLAYSURFACE : sprintf(dderr, "DDERR_NOTAOVERLAYSURFACE"); break;
		case DDERR_EXCLUSIVEMODEALREADYSET : sprintf(dderr, "DDERR_EXCLUSIVEMODEALREADYSET"); break;
		case DDERR_NOTFLIPPABLE : sprintf(dderr, "DDERR_NOTFLIPPABLE"); break;
		case DDERR_CANTDUPLICATE : sprintf(dderr, "DDERR_CANTDUPLICATE"); break;
		case DDERR_NOTLOCKED : sprintf(dderr, "DDERR_NOTLOCKED"); break;
		case DDERR_CANTCREATEDC : sprintf(dderr, "DDERR_CANTCREATEDC"); break;
		case DDERR_NODC : sprintf(dderr, "DDERR_NODC"); break;
		case DDERR_WRONGMODE : sprintf(dderr, "DDERR_WRONGMODE"); break;
		case DDERR_IMPLICITLYCREATED : sprintf(dderr, "DDERR_IMPLICITLYCREATED"); break;
		case DDERR_NOTPALETTIZED : sprintf(dderr, "DDERR_NOTPALETTIZED"); break;
		case DDERR_UNSUPPORTEDMODE : sprintf(dderr, "DDERR_UNSUPPORTEDMODE"); break;
		case DDERR_NOMIPMAPHW : sprintf(dderr, "DDERR_NOMIPMAPHW"); break;
		case DDERR_INVALIDSURFACETYPE : sprintf(dderr, "DDERR_INVALIDSURFACETYPE"); break;
		case DDERR_DCALREADYCREATED : sprintf(dderr, "DDERR_DCALREADYCREATED"); break;
		case DDERR_CANTPAGELOCK : sprintf(dderr, "DDERR_CANTPAGELOCK"); break;
		case DDERR_CANTPAGEUNLOCK : sprintf(dderr, "DDERR_CANTPAGEUNLOCK"); break;
		case DDERR_NOTPAGELOCKED : sprintf(dderr, "DDERR_NOTPAGELOCKED"); break;
		case DDERR_NOTINITIALIZED : sprintf(dderr, "DDERR_NOTINITIALIZED"); break;

		default : sprintf(dderr, "Unknown Error"); break;
	}

	sprintf(err, "DirectDraw Error %s", dderr);
	GAssert(false, err);
	PostQuitMessage(0);
}

GWindow::GWindow()
{
	m_lpDD = NULL;
	m_lpDDSFront = NULL;
	m_lpDDSBack = NULL;
	m_lpDDPalette = NULL;
	m_lpClipper = NULL;
	m_bFlipped = false;
}

GWindow::GWindow(HWND hWnd, int Width, int Height, bool bFullScreen, DWORD bpp)
{
	m_lpDD = NULL;
	m_lpDDSFront = NULL;
	m_lpDDSBack = NULL;
	m_lpDDPalette = NULL;
	m_lpClipper = NULL;
	m_bFlipped = false;
	m_dwPixelWidth = Width;
	m_dwPixelHeight = Height;
	if(bFullScreen)
		CreateFullScreen(hWnd, Width, Height, bpp);
	else
		CreateWindowed(hWnd, Width, Height);
}

GWindow::~GWindow(void)
{
	if(m_lpDD != NULL)
		m_lpDD->RestoreDisplayMode();
	delete m_lpDDSBack;
	delete m_lpDDSFront;
	RELEASE(m_lpClipper);
	RELEASE(m_lpDDPalette);
	RELEASE(m_lpDD);
}

HWND GWindow::GetSafeHWnd()
{
	if(!m_bFlipped)
		Update();
	return(m_hWnd);
}

BOOL GWindow::CreateWindowed(HWND hWnd, int Width, int Height)
{
	HRESULT rval;
	DWORD dwFlags;
	DDSURFACEDESC ddsd;
	HDC hDC;

	m_lpDD = NULL;
	m_bFullScreen = FALSE;
	m_lpDDPalette = NULL;
	m_dwPixelWidth = Width;
	m_dwPixelHeight = Height;
	m_hWnd = hWnd;

	hDC = GetDC(NULL);
	m_BPP = GetDeviceCaps(hDC, PLANES) * GetDeviceCaps(hDC, BITSPIXEL);
	ReleaseDC(NULL, hDC);

	dwFlags = DDSCL_NORMAL;

	LPDIRECTDRAW lpDD;

	rval = DirectDrawCreate(NULL, &lpDD, NULL);
	if(rval != DD_OK) GLogDDError(rval);

	rval = lpDD->QueryInterface(IID_IDirectDraw2, (LPVOID*)&m_lpDD);
	if(rval != DD_OK) GLogDDError(rval);

	RELEASE(lpDD);

	rval = m_lpDD->SetCooperativeLevel((HWND__*)hWnd, dwFlags);
	if(rval != DD_OK) GLogDDError(rval);

	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	m_lpDDSFront = new GDispRect();
	m_lpDDSBack = new GDispRect();
	m_lpDDSFront->m_PixelWidth = m_dwPixelWidth;
	m_lpDDSFront->m_PixelHeight = m_dwPixelHeight;

	rval = m_lpDD->CreateSurface(&ddsd, &m_lpDDSFront->m_lpDDS, NULL);
	if(rval != DD_OK) GLogDDError(rval);

	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;    
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = Width;
	ddsd.dwHeight = Height;

	rval = m_lpDD->CreateSurface( &ddsd, &m_lpDDSBack->m_lpDDS, NULL );
	if(rval != DD_OK) GLogDDError(rval);

	// Create the window clipper
	rval = m_lpDD->CreateClipper(0, &m_lpClipper, NULL);
	if(rval != DD_OK) GLogDDError(rval);

	rval = m_lpClipper->SetHWnd(0, hWnd);
	if(rval != DD_OK) GLogDDError(rval);

	rval = m_lpDDSFront->m_lpDDS->SetClipper(m_lpClipper);
	if(rval != DD_OK) GLogDDError(rval);

	return TRUE;
}

// Acceptable values for BPP are 8, 16, 32
BOOL GWindow::CreateFullScreen(HWND hWnd, DWORD Width, DWORD Height, DWORD BPP)
{
	HRESULT rval;
	DWORD dwFlags;
	DDSURFACEDESC ddsd;
	DDSCAPS ddscaps;

	m_lpDD = NULL;
	m_bFullScreen = TRUE;
	m_lpDDPalette = NULL;
	m_dwPixelWidth = Width;
	m_dwPixelHeight = Height;
	m_BPP = BPP;
	m_hWnd = hWnd;

	dwFlags = DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_ALLOWMODEX;

	LPDIRECTDRAW lpDD;

	rval = DirectDrawCreate(NULL, &lpDD, NULL);
	if(rval != DD_OK) GLogDDError(rval);

	rval = lpDD->QueryInterface(IID_IDirectDraw2, (LPVOID*)&m_lpDD);
	if(rval != DD_OK) GLogDDError(rval);

	RELEASE(lpDD);

	rval = m_lpDD->SetCooperativeLevel((HWND__*)hWnd, dwFlags);
	if(rval != DD_OK) GLogDDError(rval);

	rval = m_lpDD->SetDisplayMode(m_dwPixelWidth, m_dwPixelHeight, m_BPP, 0, 0);
	if(rval != DD_OK) GLogDDError(rval);

	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
												DDSCAPS_FLIP |
												DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = 1;

	m_lpDDSFront = new GDispRect;
	m_lpDDSBack = new GDispRect;
	m_lpDDSFront->m_PixelWidth = m_dwPixelWidth;
	m_lpDDSFront->m_PixelHeight = m_dwPixelHeight;

	rval = m_lpDD->CreateSurface(&ddsd, &m_lpDDSFront->m_lpDDS, NULL);
	if(rval != DD_OK) GLogDDError(rval);

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

	rval = m_lpDDSFront->m_lpDDS->GetAttachedSurface(&ddscaps, &m_lpDDSBack->m_lpDDS);
	if(rval != DD_OK) GLogDDError(rval);

	return TRUE;
}


// GWindow Flip - swaps the front and back buffers
HRESULT GWindow::Update()
{
	HRESULT rval;

	if(m_bFullScreen)
	{
		rval = m_lpDDSFront->m_lpDDS->Flip(NULL, DDFLIP_WAIT);
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

		rval = m_lpDDSFront->m_lpDDS->Blt(&Window, m_lpDDSBack->m_lpDDS, NULL, DDBLT_WAIT, NULL);
		if(rval == DDERR_SURFACELOST) Restore();
	}
   
	m_bFlipped = !m_bFlipped;
	return rval;
}

/*

// GWindow Fill - fills the back buffer with a color
void GWindow::Fill(DWORD FillColor)
{
	DDBLTFX ddBltFx;

	ddBltFx.dwSize = sizeof(DDBLTFX);
	ddBltFx.dwFillColor = FillColor;
	m_lpDDSBack->m_lpDDS->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddBltFx);
}
*/

BOOL GWindow::LoadBitmap(const char* szFilename)
{
	if(szFilename == NULL) return FALSE;
  if(DDReLoadBitmap(m_lpDDSBack->m_lpDDS, szFilename) != DD_OK) return FALSE;

	return TRUE;
}

// GWindow LoadPalette - loads a palette from a bitmap file
BOOL GWindow::LoadPalette(const char* szFilename)
{
	if(szFilename == NULL) return FALSE;
	m_lpDDPalette = DDLoadPalette(m_lpDD, szFilename);
	if(m_lpDDPalette == NULL) return FALSE;
   m_lpDDSFront->m_lpDDS->SetPalette(m_lpDDPalette);

	return TRUE;
}

// GWindow Restore - restores the front buffer if it is lost
void GWindow::Restore(void)
{
  if(m_lpDDSFront->m_lpDDS != NULL && m_lpDDSFront->m_lpDDS->IsLost() != DD_OK)
  {
    m_lpDDSFront->m_lpDDS->Restore();
	}
}
/*
// GWindow SetColor - sets the r,g,b value of a single color
void GWindow::SetColor(int col, GColor* pCol)
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

// GWindow GetColor - returns the r,g,b values of a single color
void GWindow::GetColor(int col, GColor* pCol)
{
	PALETTEENTRY pe[1];

	m_lpDDPalette->GetEntries(0, col, 1, pe);
   pCol->SetRed(pe[0].peRed);
   pCol->SetGreen(pe[0].peGreen);
   pCol->SetBlue(pe[0].peBlue);
}
*/

// GWindow SetPalette - sets the palette
void GWindow::SetPalette(int Start, int Count, LPPALETTEENTRY lpPE)
{
	m_lpDDPalette->SetEntries(0, Start, Count, lpPE);
}

// GWindow GetPalette - returns the palette
void GWindow::GetPalette(int Start, int Count, LPPALETTEENTRY lpPE)
{
	m_lpDDPalette->GetEntries(0, Start, Count, lpPE);
}

/*
// GWindow FillPalette - sets the whole palette to one color
void GWindow::FillPalette(GColor* pCol)
{
   int r = pCol->GetRed();
   int g = pCol->GetGreen();
   int b = pCol->GetBlue();

   PALETTEENTRY pe[256];

	m_lpDDPalette->GetEntries(0, 0, 256, pe);

	for(int i = 1; i < 256; i++)
	{
		pe[i].peRed = r;
		pe[i].peGreen = g;
		pe[i].peBlue = b;
	}
	m_lpDDPalette->SetEntries(0, 0, 256, pe);
}

// GWindow GrayScale - converts the palette to monochrome
void GWindow::GreyScale(void)
{
	PALETTEENTRY pe[256];
	int total, grey;

	m_lpDDPalette->GetEntries(0, 0, 256, pe);

	for(int i = 1; i < 256; i++)
	{
		total = pe[i].peRed + pe[i].peGreen + pe[i].peBlue;
		grey = total / 3;

		pe[i].peRed = grey;
		pe[i].peGreen = grey;
		pe[i].peBlue = grey;
	}
	m_lpDDPalette->SetEntries(0, 0, 256, pe);
}

// GWindow FadeIn - smoothly fades to the specified palette
void GWindow::FadeIn(int delay, LPPALETTEENTRY lpPE)
{
	PALETTEENTRY pe[256];

	m_lpDDPalette->GetEntries(0, 0, 256, pe);

	for(int j = 1; j < 84; j++)
	{
		for(int i = 1; i < 256; i++)
		{
			// Red
			if(pe[i].peRed > (lpPE[i].peRed+5)) pe[i].peRed -= 3;
			else if(pe[i].peRed < (lpPE[i].peRed-5)) pe[i].peRed += 3;
			else pe[i].peRed = lpPE[i].peRed;

			// Green
			if(pe[i].peGreen > (lpPE[i].peGreen+5)) pe[i].peGreen -= 3;
			else if(pe[i].peGreen < (lpPE[i].peGreen-5)) pe[i].peGreen += 3;
			else pe[i].peGreen = lpPE[i].peGreen;

			// Blue
			if(pe[i].peBlue > (lpPE[i].peBlue+5)) pe[i].peBlue -= 3;
			else if(pe[i].peBlue < (lpPE[i].peBlue-5)) pe[i].peBlue += 3;
			else pe[i].peBlue = lpPE[i].peBlue;

			// Add a delay here for slower fades
			for(long d = 0; d < delay*100; d++) ;
		}
		m_lpDDPalette->SetEntries(0, 0, 256, pe);
	}
}

// GWindow FadeOut - fades to black, about 0-10 for the delay is usual
void GWindow::FadeOut(int delay)
{
	PALETTEENTRY pe[256];

	m_lpDDPalette->GetEntries(0, 0, 256, pe);

	for(int j = 1; j < 84; j++)
	{
		for(int i = 1; i < 256; i++)
		{
			// Red
			if(pe[i].peRed > 5) pe[i].peRed -= 3;
			else pe[i].peRed = 0;

			// Green
			if(pe[i].peGreen > 5) pe[i].peGreen -= 3;
			else pe[i].peGreen = 0;

			// Blue
			if(pe[i].peBlue > 5) pe[i].peBlue -= 3;
			else pe[i].peBlue = 0;

			// Add a delay here for slower fades
			for(long d = 0; d < delay*100; d++) ;
		}
		m_lpDDPalette->SetEntries(0, 0, 256, pe);
	}
}

// GWindow FadeTo - fades to a specified color
void GWindow::Fade(GColor* pCol, int delay)
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
*/
