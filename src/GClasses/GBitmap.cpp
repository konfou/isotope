/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GDDraw.h"

GBitmap::GBitmap()
{
	m_XOffset = 0;
	m_YOffset = 0;
}

GBitmap::GBitmap(GWindow *pScreen, char *szFilename) : GDispRect()
{
	Load(pScreen, szFilename);

	m_XOffset = 0;
	m_YOffset = 0;
}

GBitmap::~GBitmap()
{
	m_lpDDS = NULL;
}

void GBitmap::Load(GWindow *pScreen, char *szFilename)
{
	m_lpDDS = DDLoadSizeBitmap(pScreen->GetDD(), szFilename, &m_PixelWidth, &m_PixelHeight);

	SetFilename(szFilename);

	DestRect.top = 0;
	DestRect.left = 0;
	DestRect.bottom = m_PixelHeight;
	DestRect.right = m_PixelWidth;
}

void GBitmap::VScroll(int nOffset)
{
	m_YOffset -= nOffset;
	if(m_YOffset > m_PixelHeight) m_YOffset -= m_PixelHeight;
	if(m_YOffset < 0) m_YOffset = m_PixelHeight + m_YOffset;
}

void GBitmap::HScroll(int nOffset)
{
	m_XOffset -= nOffset;
	if(m_XOffset < 0) m_XOffset = m_PixelWidth + m_XOffset;
	if(m_XOffset > m_PixelWidth) m_XOffset -= m_PixelWidth;
}

void GBitmap::MoveTo(int XOffset, int YOffset)
{
	m_XOffset = XOffset;
	m_YOffset = YOffset;
}

HRESULT GBitmap::Draw(GDispRect* lpDDS)
{
	int rVal;
	SetSrc(m_PixelHeight - m_YOffset, m_PixelWidth - m_XOffset, m_PixelHeight, m_PixelWidth);
	lpDDS->m_lpDDS->BltFast(0, 0, m_lpDDS, &SrcRect, DDBLTFAST_WAIT);

	SetSrc(m_PixelHeight - m_YOffset, 0, m_PixelHeight, m_PixelWidth - m_XOffset);
	lpDDS->m_lpDDS->BltFast(m_XOffset, 0, m_lpDDS, &SrcRect, DDBLTFAST_WAIT);

	SetSrc(0, m_PixelWidth - m_XOffset, m_PixelHeight - m_YOffset, m_PixelWidth);
	lpDDS->m_lpDDS->BltFast(0, m_YOffset, m_lpDDS, &SrcRect, DDBLTFAST_WAIT);

	SetSrc(0, 0, m_PixelHeight - m_YOffset, m_PixelWidth - m_XOffset);
	rVal = lpDDS->m_lpDDS->BltFast(m_XOffset, m_YOffset, m_lpDDS, &SrcRect, DDBLTFAST_WAIT);
//	if(rVal != DD_OK)
//		ShowDDError(rVal);
	return rVal;
}
