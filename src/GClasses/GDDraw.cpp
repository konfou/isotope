/*
	Copyright (C) 1999, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "GnuSDK.h"
#include "GDDraw.h"
#include "GMacros.h"

void ShowDDError(int nError)
{
	switch(nError)
	{
		case DDERR_ALREADYINITIALIZED: GAssert(false, "DDERR_ALREADYINITIALIZED"); break;
		case DDERR_CANNOTATTACHSURFACE: GAssert(false, "DDERR_CANNOTATTACHSURFACE"); break;
		case DDERR_CANNOTDETACHSURFACE: GAssert(false, "DDERR_CANNOTDETACHSURFACE"); break;
		case DDERR_CURRENTLYNOTAVAIL: GAssert(false, "DDERR_CURRENTLYNOTAVAIL"); break;
		case DDERR_EXCEPTION: GAssert(false, "DDERR_EXCEPTION"); break;
		case DDERR_GENERIC: GAssert(false, "DDERR_GENERIC"); break;
		case DDERR_HEIGHTALIGN: GAssert(false, "DDERR_HEIGHTALIGN"); break;
		case DDERR_INCOMPATIBLEPRIMARY: GAssert(false, "DDERR_INCOMPATIBLEPRIMARY"); break;
		case DDERR_INVALIDCAPS: GAssert(false, "DDERR_INVALIDCAPS"); break;
		case DDERR_INVALIDCLIPLIST: GAssert(false, "DDERR_INVALIDCLIPLIST"); break;
		case DDERR_INVALIDMODE: GAssert(false, "DDERR_INVALIDMODE"); break;
		case DDERR_INVALIDOBJECT: GAssert(false, "DDERR_INVALIDOBJECT"); break;
		case DDERR_INVALIDPARAMS: GAssert(false, "DDERR_INVALIDPARAMS"); break;
		case DDERR_INVALIDPIXELFORMAT: GAssert(false, "DDERR_INVALIDPIXELFORMAT"); break;
		case DDERR_INVALIDRECT: GAssert(false, "DDERR_INVALIDRECT"); break;
		case DDERR_LOCKEDSURFACES: GAssert(false, "DDERR_LOCKEDSURFACES"); break;
		case DDERR_NO3D: GAssert(false, "DDERR_NO3D"); break;
		case DDERR_NOALPHAHW: GAssert(false, "DDERR_NOALPHAHW"); break;
		case DDERR_NOCLIPLIST: GAssert(false, "DDERR_NOCLIPLIST"); break;
		case DDERR_NOCOLORCONVHW: GAssert(false, "DDERR_NOCOLORCONVHW"); break;
		case DDERR_NOCOOPERATIVELEVELSET: GAssert(false, "DDERR_NOCOOPERATIVELEVELSET"); break;
		case DDERR_NOCOLORKEY: GAssert(false, "DDERR_NOCOLORKEY"); break;
		case DDERR_NOCOLORKEYHW: GAssert(false, "DDERR_NOCOLORKEYHW"); break;
		case DDERR_NODIRECTDRAWSUPPORT: GAssert(false, "DDERR_NODIRECTDRAWSUPPORT"); break;
		case DDERR_NOEXCLUSIVEMODE: GAssert(false, "DDERR_NOEXCLUSIVEMODE"); break;
		case DDERR_NOFLIPHW: GAssert(false, "DDERR_NOFLIPHW"); break;
		case DDERR_NOGDI: GAssert(false, "DDERR_NOGDI"); break;
		case DDERR_NOMIRRORHW: GAssert(false, "DDERR_NOMIRRORHW"); break;
		case DDERR_NOTFOUND: GAssert(false, "DDERR_NOTFOUND"); break;
		case DDERR_NOOVERLAYHW: GAssert(false, "DDERR_NOOVERLAYHW"); break;
		case DDERR_NORASTEROPHW: GAssert(false, "DDERR_NORASTEROPHW"); break;
		case DDERR_NOROTATIONHW: GAssert(false, "DDERR_NOROTATIONHW"); break;
		case DDERR_NOSTRETCHHW: GAssert(false, "DDERR_NOSTRETCHHW"); break;
		case DDERR_NOT4BITCOLOR: GAssert(false, "DDERR_NOT4BITCOLOR"); break;
		case DDERR_NOT4BITCOLORINDEX: GAssert(false, "DDERR_NOT4BITCOLORINDEX"); break;
		case DDERR_NOT8BITCOLOR: GAssert(false, "DDERR_NOT8BITCOLOR"); break;
		case DDERR_NOTEXTUREHW: GAssert(false, "DDERR_NOTEXTUREHW"); break;
		case DDERR_NOVSYNCHW: GAssert(false, "DDERR_NOVSYNCHW"); break;
		case DDERR_NOZBUFFERHW: GAssert(false, "DDERR_NOZBUFFERHW"); break;
		case DDERR_NOZOVERLAYHW: GAssert(false, "DDERR_NOZOVERLAYHW"); break;
		case DDERR_OUTOFCAPS: GAssert(false, "DDERR_OUTOFCAPS"); break;
		case DDERR_OUTOFMEMORY: GAssert(false, "DDERR_OUTOFMEMORY"); break;
		case DDERR_OUTOFVIDEOMEMORY: GAssert(false, "DDERR_OUTOFVIDEOMEMORY"); break;
		case DDERR_OVERLAYCANTCLIP: GAssert(false, "DDERR_OVERLAYCANTCLIP"); break;
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE: GAssert(false, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE"); break;
		case DDERR_PALETTEBUSY: GAssert(false, "DDERR_PALETTEBUSY"); break;
		case DDERR_COLORKEYNOTSET: GAssert(false, "DDERR_COLORKEYNOTSET"); break;
		case DDERR_SURFACEALREADYATTACHED: GAssert(false, "DDERR_SURFACEALREADYATTACHED"); break;
		case DDERR_SURFACEALREADYDEPENDENT: GAssert(false, "DDERR_SURFACEALREADYDEPENDENT"); break;
		case DDERR_SURFACEBUSY: GAssert(false, "DDERR_SURFACEBUSY"); break;
		case DDERR_CANTLOCKSURFACE: GAssert(false, "DDERR_CANTLOCKSURFACE"); break;
		case DDERR_SURFACEISOBSCURED: GAssert(false, "DDERR_SURFACEISOBSCURED"); break;
		case DDERR_SURFACELOST: GAssert(false, "DDERR_SURFACELOST"); break;
		case DDERR_SURFACENOTATTACHED: GAssert(false, "DDERR_SURFACENOTATTACHED"); break;
		case DDERR_TOOBIGHEIGHT: GAssert(false, "DDERR_TOOBIGHEIGHT"); break;
		case DDERR_TOOBIGSIZE: GAssert(false, "DDERR_TOOBIGSIZE"); break;
		case DDERR_TOOBIGWIDTH: GAssert(false, "DDERR_TOOBIGWIDTH"); break;
		case DDERR_UNSUPPORTED: GAssert(false, "DDERR_UNSUPPORTED"); break;
		case DDERR_UNSUPPORTEDFORMAT: GAssert(false, "DDERR_UNSUPPORTEDFORMAT"); break;
		case DDERR_UNSUPPORTEDMASK: GAssert(false, "DDERR_UNSUPPORTEDMASK"); break;
		case DDERR_VERTICALBLANKINPROGRESS: GAssert(false, "DDERR_VERTICALBLANKINPROGRESS"); break;
		case DDERR_WASSTILLDRAWING: GAssert(false, "DDERR_WASSTILLDRAWING"); break;
		case DDERR_XALIGN: GAssert(false, "DDERR_XALIGN"); break;
		case DDERR_INVALIDDIRECTDRAWGUID: GAssert(false, "DDERR_INVALIDDIRECTDRAWGUID"); break;
		case DDERR_DIRECTDRAWALREADYCREATED: GAssert(false, "DDERR_DIRECTDRAWALREADYCREATED"); break;
		case DDERR_NODIRECTDRAWHW: GAssert(false, "DDERR_NODIRECTDRAWHW"); break;
		case DDERR_PRIMARYSURFACEALREADYEXISTS: GAssert(false, "DDERR_PRIMARYSURFACEALREADYEXISTS"); break;
		case DDERR_NOEMULATION: GAssert(false, "DDERR_NOEMULATION"); break;
		case DDERR_REGIONTOOSMALL: GAssert(false, "DDERR_REGIONTOOSMALL"); break;
		case DDERR_CLIPPERISUSINGHWND: GAssert(false, "DDERR_CLIPPERISUSINGHWND"); break;
		case DDERR_NOCLIPPERATTACHED: GAssert(false, "DDERR_NOCLIPPERATTACHED"); break;
		case DDERR_NOHWND: GAssert(false, "DDERR_NOHWND"); break;
		case DDERR_HWNDSUBCLASSED: GAssert(false, "DDERR_HWNDSUBCLASSED"); break;
		case DDERR_HWNDALREADYSET: GAssert(false, "DDERR_HWNDALREADYSET"); break;
		case DDERR_NOPALETTEATTACHED: GAssert(false, "DDERR_NOPALETTEATTACHED"); break;
		case DDERR_NOPALETTEHW: GAssert(false, "DDERR_NOPALETTEHW"); break;
		case DDERR_BLTFASTCANTCLIP: GAssert(false, "DDERR_BLTFASTCANTCLIP"); break;
		case DDERR_NOBLTHW: GAssert(false, "DDERR_NOBLTHW"); break;
		case DDERR_NODDROPSHW: GAssert(false, "DDERR_NODDROPSHW"); break;
		case DDERR_OVERLAYNOTVISIBLE: GAssert(false, "DDERR_OVERLAYNOTVISIBLE"); break;
		case DDERR_NOOVERLAYDEST: GAssert(false, "DDERR_NOOVERLAYDEST"); break;
		case DDERR_INVALIDPOSITION: GAssert(false, "DDERR_INVALIDPOSITION"); break;
		case DDERR_NOTAOVERLAYSURFACE: GAssert(false, "DDERR_NOTAOVERLAYSURFACE"); break;
		case DDERR_EXCLUSIVEMODEALREADYSET: GAssert(false, "DDERR_EXCLUSIVEMODEALREADYSET"); break;
		case DDERR_NOTFLIPPABLE: GAssert(false, "DDERR_NOTFLIPPABLE"); break;
		case DDERR_CANTDUPLICATE: GAssert(false, "DDERR_CANTDUPLICATE"); break;
		case DDERR_NOTLOCKED: GAssert(false, "DDERR_NOTLOCKED"); break;
		case DDERR_CANTCREATEDC: GAssert(false, "DDERR_CANTCREATEDC"); break;
		case DDERR_NODC: GAssert(false, "DDERR_NODC"); break;
		case DDERR_WRONGMODE: GAssert(false, "DDERR_WRONGMODE"); break;
		case DDERR_IMPLICITLYCREATED: GAssert(false, "DDERR_IMPLICITLYCREATED"); break;
		case DDERR_NOTPALETTIZED: GAssert(false, "DDERR_NOTPALETTIZED"); break;
		case DDERR_UNSUPPORTEDMODE: GAssert(false, "DDERR_UNSUPPORTEDMODE"); break;
		case DDERR_NOMIPMAPHW: GAssert(false, "DDERR_NOMIPMAPHW"); break;
		case DDERR_INVALIDSURFACETYPE: GAssert(false, "DDERR_INVALIDSURFACETYPE"); break;
		case DDERR_DCALREADYCREATED: GAssert(false, "DDERR_DCALREADYCREATED"); break;
		case DDERR_CANTPAGELOCK: GAssert(false, "DDERR_CANTPAGELOCK"); break;
		case DDERR_CANTPAGEUNLOCK: GAssert(false, "DDERR_CANTPAGEUNLOCK"); break;
		case DDERR_NOTPAGELOCKED: GAssert(false, "DDERR_NOTPAGELOCKED"); break;
		case DDERR_NOTINITIALIZED: GAssert(false, "DDERR_NOTINITIALIZED"); break;
		default:
			GAssert(false, "Unknown error");
	}
}

////////////////////////////////////////////////////////////////////////
//
//  DDLoadBitmap
//
//  Create a DirectDrawSurface from a bitmap resource.
//
////////////////////////////////////////////////////////////////////////
extern "C" IDirectDrawSurface * DDLoadBitmap(IDirectDraw2 *pdd, LPCSTR szBitmap, int dx, int dy)
{
    HBITMAP             hbm;
    BITMAP              bm;
    DDSURFACEDESC       ddsd;
    IDirectDrawSurface *pdds;

    //
    //  try to load the bitmap as a resource, if that fails, try it as a file
    //
    hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), szBitmap, IMAGE_BITMAP, dx, dy, LR_CREATEDIBSECTION);

    if (hbm == NULL)
        hbm = (HBITMAP)LoadImage(NULL, szBitmap, IMAGE_BITMAP, dx, dy, LR_LOADFROMFILE|LR_CREATEDIBSECTION);

    if (hbm == NULL)
        return NULL;

    //
    // get size of the bitmap
    //
    GetObject(hbm, sizeof(bm), &bm);      // get size of bitmap

    //
    // create a DirectDrawSurface for this bitmap
    //
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth = bm.bmWidth;
    ddsd.dwHeight = bm.bmHeight;

    if (pdd->CreateSurface(&ddsd, &pdds, NULL) != DD_OK)
        return NULL;

    DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);

    DeleteObject(hbm);

    return pdds;
}

IDirectDrawSurface * DDLoadSizeBitmap(IDirectDraw2 *pdd, LPCSTR szBitmap, int *dx, int *dy)
{
	HBITMAP             hbm;
	BITMAP              bm;
	DDSURFACEDESC       ddsd;
	IDirectDrawSurface	*pdds;

	hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), szBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (!hbm)
		hbm = (HBITMAP)LoadImage(NULL, szBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION);
	if (!hbm)
	{
		GAssert(false, "Error loading bitmap");
		return NULL;
	}

	// Get size of the bitmap
	GetObject(hbm, sizeof(bm), &bm);      // get size of bitmap

	// Create a DirectDrawSurface for this bitmap
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = bm.bmWidth;
	ddsd.dwHeight = bm.bmHeight;

	int nRes = pdd->CreateSurface(&ddsd, &pdds, NULL);
	if (nRes != DD_OK)
	{
		GAssert(false, "Error creating surface");
		ShowDDError(nRes);
		return NULL;
	}
	DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);

	DeleteObject(hbm);

	*dx = ddsd.dwWidth;
	*dy = ddsd.dwHeight;

	return pdds;
}

////////////////////////////////////////////////////////////////////////
//
//  DDReLoadBitmap
//
//  Load a bitmap from a file or resource into a directdraw surface.
//  Normaly used to re-load a surface after a restore.
//
////////////////////////////////////////////////////////////////////////
HRESULT DDReLoadBitmap(IDirectDrawSurface *pdds, LPCSTR szBitmap)
{
    HBITMAP             hbm;
    HRESULT             hr;

    //
    //  try to load the bitmap as a resource, if that fails, try it as a file
    //
    hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), szBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    if (hbm == NULL)
        hbm = (HBITMAP)LoadImage(NULL, szBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION);

    if (hbm == NULL)
    {
        OutputDebugString("handle is null\n");
        return E_FAIL;
    }

    hr = DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
    if (hr != DD_OK)
    {
        OutputDebugString("ddcopybitmap failed\n");
    }


    DeleteObject(hbm);
    return hr;
}

////////////////////////////////////////////////////////////////////////
//
//  DDCopyBitmap
//
//  Draw a bitmap into a DirectDrawSurface
//
////////////////////////////////////////////////////////////////////////
extern "C" HRESULT DDCopyBitmap(IDirectDrawSurface *pdds, HBITMAP hbm, int x, int y, int dx, int dy)
{
    HDC                 hdcImage;
    HDC                 hdc;
    BITMAP              bm;
    DDSURFACEDESC       ddsd;
    HRESULT             hr;

	GAssert(hbm && pdds, "These parameters can't be NULL");
    if (hbm == NULL || pdds == NULL)
        return E_FAIL;

    // Make sure this surface is restored.
    pdds->Restore();

    //  Select bitmap into a memoryDC so we can use it.
    hdcImage = CreateCompatibleDC(NULL);
    if (!hdcImage)
        OutputDebugString("createcompatible dc failed\n");
    SelectObject(hdcImage, hbm);

    // Get size of the bitmap
    GetObject(hbm, sizeof(bm), &bm);    // get size of bitmap
    dx = dx == 0 ? bm.bmWidth  : dx;    // use the passed size, unless zero
    dy = dy == 0 ? bm.bmHeight : dy;

    // Get size of surface.
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    pdds->GetSurfaceDesc(&ddsd);

    if ((hr = pdds->GetDC(&hdc)) == DD_OK)
    {
        StretchBlt(hdc, 0, 0, ddsd.dwWidth, ddsd.dwHeight, hdcImage, x, y, dx, dy, SRCCOPY);
        pdds->ReleaseDC(hdc);
    }

    DeleteDC(hdcImage);
    return hr;
}

////////////////////////////////////////////////////////////////////////
//
//  DDLoadPalette
//
//  Create a DirectDraw palette object from a bitmap resoure.
//
//  If the resource does not exist or NULL is passed create a
//  default 332 palette.
//
////////////////////////////////////////////////////////////////////////
extern "C" IDirectDrawPalette * DDLoadPalette(IDirectDraw2 *pdd, LPCSTR szBitmap)
{
    IDirectDrawPalette* ddpal;
    int                 i;
    int                 n;
    int                 fh;
    HRSRC               h;
    LPBITMAPINFOHEADER  lpbi;
    PALETTEENTRY        ape[256];
    RGBQUAD *           prgb;

    //
    // build a 332 palette as the default.
    //
    for (i=0; i<256; i++)
    {
        ape[i].peRed   = (BYTE)(((i >> 5) & 0x07) * 255 / 7);
        ape[i].peGreen = (BYTE)(((i >> 2) & 0x07) * 255 / 7);
        ape[i].peBlue  = (BYTE)(((i >> 0) & 0x03) * 255 / 3);
        ape[i].peFlags = (BYTE)0;
    }

    //
    // get a pointer to the bitmap resource.
    //
    if (szBitmap && (h = FindResource(NULL, szBitmap, RT_BITMAP)))
    {
        lpbi = (LPBITMAPINFOHEADER)LockResource(LoadResource(NULL, h));
        if (!lpbi)
            OutputDebugString("lock resource failed\n");
        prgb = (RGBQUAD*)((BYTE*)lpbi + lpbi->biSize);

        if (lpbi == NULL || lpbi->biSize < sizeof(BITMAPINFOHEADER))
            n = 0;
        else if (lpbi->biBitCount > 8)
            n = 0;
        else if (lpbi->biClrUsed == 0)
            n = 1 << lpbi->biBitCount;
        else
            n = lpbi->biClrUsed;

        //
        //  a DIB color table has its colors stored BGR not RGB
        //  so flip them around.
        //
        for(i=0; i<n; i++ )
        {
            ape[i].peRed   = prgb[i].rgbRed;
            ape[i].peGreen = prgb[i].rgbGreen;
            ape[i].peBlue  = prgb[i].rgbBlue;
            ape[i].peFlags = 0;
        }
    }
    else if (szBitmap && (fh = _lopen(szBitmap, OF_READ)) != -1)
    {
        BITMAPFILEHEADER bf;
        BITMAPINFOHEADER bi;

        _lread(fh, &bf, sizeof(bf));
        _lread(fh, &bi, sizeof(bi));
        _lread(fh, ape, sizeof(ape));
        _lclose(fh);

        if (bi.biSize != sizeof(BITMAPINFOHEADER))
            n = 0;
        else if (bi.biBitCount > 8)
            n = 0;
        else if (bi.biClrUsed == 0)
            n = 1 << bi.biBitCount;
        else
            n = bi.biClrUsed;

        //
        //  a DIB color table has its colors stored BGR not RGB
        //  so flip them around.
        //
        for(i=0; i<n; i++ )
        {
            BYTE r = ape[i].peRed;
            ape[i].peRed  = ape[i].peBlue;
            ape[i].peBlue = r;
        }
    }

    pdd->CreatePalette(DDPCAPS_8BIT, ape, &ddpal, NULL);

    return ddpal;
}

/*
 * DDColorMatch
 *
 * convert a RGB color to a pysical color.
 *
 * we do this by leting GDI SetPixel() do the color matching
 * then we lock the memory and see what it got mapped to.
 */
extern "C" DWORD DDColorMatch(IDirectDrawSurface *pdds, COLORREF rgb)
{
    COLORREF rgbT;
    HDC hdc;
    DWORD dw = CLR_INVALID;
    DDSURFACEDESC ddsd;
    HRESULT hres;

    //
    //  use GDI SetPixel to color match for us
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);             // save current pixel value
        SetPixel(hdc, 0, 0, rgb);               // set our value
        pdds->ReleaseDC(hdc);
    }

    //
    // now lock the surface so we can read back the converted color
    //
    ddsd.dwSize = sizeof(ddsd);
    while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;

    if (hres == DD_OK)
    {
        dw  = *(DWORD *)ddsd.lpSurface;                     // get DWORD
        dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount)-1;  // mask it to bpp
        pdds->Unlock(NULL);
    }

    //
    //  now put the color that was there back.
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }

    return dw;
}

/*
 * DDSetColorKey
 *
 * set a color key for a surface, given a RGB.
 * if you pass CLR_INVALID as the color key, the pixel
 * in the upper-left corner will be used.
 */
extern "C" HRESULT DDSetColorKey(IDirectDrawSurface *pdds, COLORREF rgb)
{
    DDCOLORKEY          ddck;

    ddck.dwColorSpaceLowValue  = DDColorMatch(pdds, rgb);
    ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
    return pdds->SetColorKey(DDCKEY_SRCBLT, &ddck);
}
