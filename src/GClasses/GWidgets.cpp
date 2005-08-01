/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GWidgets.h"
#include "GImage.h"
#include "GArray.h"

GWidgetStyle::GWidgetStyle()
{
	m_pWidgets = new GPointerArray(32);
	m_nButtonFontSize = 14;
	m_fButtonFontWidth = (float).8;
	m_cButtonTextColor = 0;
	m_cButtonPressedTextColor = gRGB(255, 255, 255);
	m_cTextBoxBorderColor = gRGB(255, 255, 255);
	m_cTextBoxTextColor = gRGB(255, 255, 255);
}

GWidgetStyle::~GWidgetStyle()
{
	int n;
	int nCount = m_pWidgets->GetSize();
	for(n = 0; n < nCount; n++)
		delete((GWidget*)m_pWidgets->GetPointer(n));
	delete(m_pWidgets);
}

void GWidgetStyle::AddWidget(GWidget* pWidget)
{
	m_pWidgets->AddPointer(pWidget);
}

GWidget* GWidgetStyle::FindWidget(int x, int y)
{
	int n;
	int nCount = m_pWidgets->GetSize();
	GWidget* pWidget;
	for(n = 0; n < nCount; n++)
	{
		pWidget = (GWidget*)m_pWidgets->GetPointer(n);
		if(pWidget->GetRect()->DoesInclude(x, y))
			return pWidget;
	}
	return NULL;
}

void GWidgetStyle::DrawButtonText(GImage* pImage, int x, int y, int w, int h, GString* pString, bool pressed)
{
	char* szText = (char*)alloca(pString->GetLength() + 1);
	pString->GetAnsi(szText);
	int nButtonFontSize = m_nButtonFontSize;
	if(h < nButtonFontSize)
		nButtonFontSize = h;
	int wid = pImage->MeasureHardTextWidth(nButtonFontSize, szText, m_fButtonFontWidth);
	GRect r;
	r.x = x + (w - wid) / 2;
	if(r.x < x)
		r.x = x;
	r.y = y + (h - nButtonFontSize) / 2;
	if(r.y < y)
		r.y = y;
	r.w = w - (r.x - x);
	r.h = nButtonFontSize;
	pImage->DrawHardText(&r, szText, pressed ? m_cButtonPressedTextColor : m_cButtonTextColor, m_fButtonFontWidth);
}

void GWidgetStyle::DrawHorizCurvedOutSurface(GImage* pImage, int x, int y, int w, int h)
{
	if(w <= 0 || h <= 0)
		return;
	float fac = (float)1.1 / w;
	int n;
	for(n = 0; n < w; n++)
	{
		float t = (float)n * fac - (float).1;
		int shade = (int)(255 * (1 - (t * t)));
		pImage->DrawLine(x, y, x, y + h - 1, gRGB(shade, shade, 255));
		x++;
	}
}

void GWidgetStyle::DrawHorizCurvedInSurface(GImage* pImage, int x, int y, int w, int h, int colorMaskRed, int colorMaskGreen, int colorMaskBlue)
{
	if(w <= 0 || h <= 0)
		return;
	int n;
	for(n = 0; n < w; n++)
	{
		float t = (float)(n + n) / w - 1;
		int shade = (int)(64 + 191 * ((t * t)));
		pImage->DrawLine(x, y, x, y + h - 1, gRGB(colorMaskRed & shade, colorMaskGreen & shade, colorMaskBlue & shade));
		x++;
	}
}

void GWidgetStyle::DrawVertCurvedOutSurface(GImage* pImage, int x, int y, int w, int h)
{
	float fac = (float)1.1 / h;
	int yStart = 0;
	if(y < 0)
		yStart = -y;
	if(y + h > (int)pImage->GetHeight())
		h = pImage->GetHeight() - y;
	int n;
	for(n = yStart; n < h; n++)
	{
		float t = (float)n * fac - (float).1;
		int shade = (int)(255 * (1 - (t * t)));
		pImage->DrawLine(x, y, x + w - 1, y, gRGB(shade, shade, 255));
		y++;
	}
}

void GWidgetStyle::DrawVertCurvedInSurface(GImage* pImage, int x, int y, int w, int h)
{
	int n;
	for(n = 0; n < h; n++)
	{
		float t = (float)(n + n) / h - 1;
		int shade = (int)(64 + 191 * ((t * t)));
		pImage->DrawLine(x, y, x + w - 1, y, gRGB(0, 0, shade));
		y++;
	}
}

void GWidgetStyle::DrawCursor(GImage* pImage, int x, int y, int w, int h)
{
	pImage->DrawBox(x, y, w, h, m_cTextBoxTextColor, false);
}

// ----------------------------------------------------------------------

GWidgetButton::GWidgetButton(GWidgetStyle* pStyle, int x, int y, int w, int h, GString* pText)
: GWidget(x, y, w, h)
{
	m_pStyle = pStyle;
	m_image.SetSize(w * 2, h);
	m_text.Copy(pText);
	m_pressed = false;
}

/*virtual*/ GWidgetButton::~GWidgetButton()
{
}

void GWidgetButton::Update()
{
	// Draw the non-pressed image
	int w = m_image.GetWidth() / 2;
	int h = m_image.GetHeight();
	m_pStyle->DrawVertCurvedOutSurface(&m_image, 0, 0, w, h);
	m_pStyle->DrawButtonText(&m_image, 0, 0, w, h, &m_text, false);
	m_image.DrawBox(0, 0, w - 1, h - 1, gRGB(64, 64, 64), false);

	// Draw the pressed image
	int nHorizOfs = (int)(w * (float).05);
	int nVertOfs = (int)(h * (float).15);
	m_pStyle->DrawVertCurvedInSurface(&m_image, w, 0, w, h);
	m_pStyle->DrawButtonText(&m_image, w + nHorizOfs, nVertOfs, w - nHorizOfs, h - nVertOfs, &m_text, true);
	m_image.DrawBox(w, 0, w + w - 1, h - 1, gRGB(255, 255, 255), false);
}

GImage* GWidgetButton::GetImage(GRect* pOutRect)
{
	pOutRect->x = m_pressed ? m_image.GetWidth() / 2 : 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth() / 2;
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetButton::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w * 2, h);
}

void GWidgetButton::SetText(GString* pText)
{
	m_text.Copy(pText);
}

void GWidgetButton::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	pImage->Blit(m_rect.x, m_rect.y, pSrcImage, &r);
}

// ----------------------------------------------------------------------

GWidgetHorizScrollBar::GWidgetHorizScrollBar(GWidgetStyle* pStyle, int x, int y, int w, int h, int nViewSize, int nModelSize)
: GWidget(x, y, w, h)
{
	m_pStyle = pStyle;
	m_image.SetSize(w, h);
	m_nViewSize = nViewSize;
	m_nModelSize = nModelSize;
	m_nPos = 0;
}

/*virtual*/ GWidgetHorizScrollBar::~GWidgetHorizScrollBar()
{
}

GImage* GWidgetHorizScrollBar::GetImage(GRect* pOutRect)
{
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth();
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetHorizScrollBar::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
}

void GWidgetHorizScrollBar::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	pImage->Blit(m_rect.x, m_rect.y, pSrcImage, &r);
}

/*static*/ void GWidgetHorizScrollBar::Draw(GImage* pImage, GRect* pR, GWidgetStyle* pStyle, int nPos, int nViewSize, int nModelSize)
{
	// Calculations
	int nButtonSize = pR->h;
	if(pR->w / 4 < nButtonSize)
		nButtonSize = pR->w / 4;
	int nSlideAreaSize = pR->w - nButtonSize - nButtonSize;
	int nTabSize = nSlideAreaSize * nViewSize / nModelSize;
	if(nTabSize < pR->h)
		nTabSize = pR->h;
	if(nTabSize > nSlideAreaSize)
		nTabSize = nSlideAreaSize;
	int nTabPos = nPos * nSlideAreaSize / nModelSize;
	if(nTabPos > nSlideAreaSize - nTabSize)
		nTabPos = nSlideAreaSize - nTabSize;
	nTabPos += nButtonSize;

	// Draw the sliding area
	pStyle->DrawVertCurvedInSurface(pImage, pR->x + nButtonSize, pR->y, nSlideAreaSize, pR->h);

	// Draw the sliding tab
	pStyle->DrawVertCurvedOutSurface(pImage, pR->x + nTabPos, pR->y, nTabSize, pR->h);

	// Draw the buttons
	pStyle->DrawHorizCurvedOutSurface(pImage, pR->x, pR->y, nButtonSize, pR->h);
	pStyle->DrawHorizCurvedOutSurface(pImage, pR->x + pR->w - nButtonSize, pR->y, nButtonSize, pR->h);
	int nArrowSize = nButtonSize / 3;
	int h = pR->h / 2;
	int n;
	for(n = 0; n < nArrowSize; n++)
	{
		pImage->DrawLine(pR->x + nArrowSize + n, pR->y + h - n, pR->x + nArrowSize + n, pR->y + h + n, 0);
		pImage->DrawLine(pR->x + pR->w - nArrowSize - n, pR->y + h - n, pR->x + pR->w - nArrowSize - n, pR->y + h + n, 0);
	}
}

void GWidgetHorizScrollBar::Update()
{
	GRect r;
	r.x = 0;
	r.y = 0;
	r.w = m_image.GetWidth();
	r.h = m_image.GetHeight();
	Draw(&m_image, &r, m_pStyle, m_nPos, m_nViewSize, m_nModelSize);
}

// ----------------------------------------------------------------------

GWidgetVertScrollBar::GWidgetVertScrollBar(GWidgetStyle* pStyle, int x, int y, int w, int h, int nViewSize, int nModelSize)
: GWidget(x, y, w, h)
{
	m_pStyle = pStyle;
	m_image.SetSize(w, h);
	m_nViewSize = nViewSize;
	m_nModelSize = nModelSize;
	m_nPos = 0;
}

/*virtual*/ GWidgetVertScrollBar::~GWidgetVertScrollBar()
{
}

GImage* GWidgetVertScrollBar::GetImage(GRect* pOutRect)
{
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth();
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetVertScrollBar::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
}

void GWidgetVertScrollBar::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	pImage->Blit(m_rect.x, m_rect.y, pSrcImage, &r);
}

/*static*/ void GWidgetVertScrollBar::Draw(GImage* pImage, GRect* pR, GWidgetStyle* pStyle, int nPos, int nViewSize, int nModelSize)
{
	// Calculations
	int nButtonSize = pR->w;
	if(pR->h / 4 < nButtonSize)
		nButtonSize = pR->h / 4;
	int nSlideAreaSize = pR->h - nButtonSize - nButtonSize;
	int nTabSize = nSlideAreaSize * nViewSize / nModelSize;
	if(nTabSize < pR->w)
		nTabSize = pR->w;
	if(nTabSize > nSlideAreaSize)
		nTabSize = nSlideAreaSize;
	int nTabPos = nPos * nSlideAreaSize / nModelSize;
	if(nTabPos > nSlideAreaSize - nTabSize)
		nTabPos = nSlideAreaSize - nTabSize;
	nTabPos += nButtonSize;

	// Draw the sliding area
	pStyle->DrawHorizCurvedInSurface(pImage, pR->x, pR->y + nButtonSize, pR->w, nSlideAreaSize);

	// Draw the sliding tab
	pStyle->DrawHorizCurvedOutSurface(pImage, pR->x, pR->y + nTabPos, pR->w, nTabSize);

	// Draw the buttons
	pStyle->DrawVertCurvedOutSurface(pImage, pR->x, pR->y, pR->w, nButtonSize);
	pStyle->DrawVertCurvedOutSurface(pImage, pR->x, pR->y + pR->h - nButtonSize, pR->w, nButtonSize);
	int nArrowSize = nButtonSize / 3;
	int w = pR->w / 2;
	int n;
	for(n = 0; n < nArrowSize; n++)
	{
		pImage->DrawLine(pR->x + w - n, pR->y + nArrowSize + n, pR->x + w + n, pR->y + nArrowSize + n, 0);
		pImage->DrawLine(pR->x + w - n, pR->y + pR->h - nArrowSize - n, pR->x + w + n, pR->y + pR->h - nArrowSize - n, 0);
	}
}

void GWidgetVertScrollBar::Update()
{
	GRect r;
	r.x = 0;
	r.y = 0;
	r.w = m_image.GetWidth();
	r.h = m_image.GetHeight();
	Draw(&m_image, &r, m_pStyle, m_nPos, m_nViewSize, m_nModelSize);
}

// ----------------------------------------------------------------------

GWidgetTextBox::GWidgetTextBox(GWidgetStyle* pStyle, int x, int y, int w, int h)
: GWidget(x, y, w, h)
{
	m_pStyle = pStyle;
	m_image.SetSize(w, h);
}

/*virtual*/ GWidgetTextBox::~GWidgetTextBox()
{
}

GImage* GWidgetTextBox::GetImage(GRect* pOutRect)
{
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth();
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetTextBox::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	pImage->Blit(m_rect.x, m_rect.y, pSrcImage, &r);
}

void GWidgetTextBox::Update()
{
	// Calculations
	int w = m_image.GetWidth();
	int h = m_image.GetHeight();

	// Draw the background area
	m_pStyle->DrawVertCurvedInSurface(&m_image, 0, 0, w, h);
	m_image.DrawBox(0, 0, w - 1, h - 1, m_pStyle->GetTextBoxBorderColor(), false);

	// Draw the text
	char* szText = (char*)alloca(m_text.GetLength() + 1);
	m_text.GetAnsi(szText);
	int nCursorPos = m_image.MeasureHardTextWidth(h - 2, szText, 1) + 2;
	if(nCursorPos > w - 3)
		nCursorPos = w - 3;
	GRect r;
	r.x = 1;
	r.y = 1;
	r.w = w - 2;
	r.h = h - 2;
	m_image.DrawHardText(&r, szText, m_pStyle->GetTextBoxTextColor(), 1);

	// Draw the cursor
	m_pStyle->DrawCursor(&m_image, nCursorPos, 2, 4, h - 3);
}

// ----------------------------------------------------------------------

GWidgetListBox::GWidgetListBox(GWidgetStyle* pStyle, GPointerArray* pItems, int x, int y, int w, int h)
: GWidget(x, y, w, h)
{
	m_pItems = pItems;
	m_pStyle = pStyle;
	m_image.SetSize(w, h);
	m_nSelectedIndex = -1;
	m_nScrollPos = 0;
	m_eBaseColor = blue;
}

/*virtual*/ GWidgetListBox::~GWidgetListBox()
{
}

GImage* GWidgetListBox::GetImage(GRect* pOutRect)
{
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth();
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetListBox::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	pImage->Blit(m_rect.x, m_rect.y, pSrcImage, &r);
}

void GWidgetListBox::SetSelection(int n)
{
	m_nSelectedIndex = n;
}

void GWidgetListBox::SetScrollPos(int n)
{
	m_nScrollPos = n;
}

void GWidgetListBox::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
}

void GWidgetListBox::Update()
{
	// Calculations
	int w = m_image.GetWidth();
	int h = m_image.GetHeight();

	// Draw the background area
	switch(m_eBaseColor)
	{
		case red: m_pStyle->DrawHorizCurvedInSurface(&m_image, 0, 0, w, h, 255, 0, 0); break;
		case yellow: m_pStyle->DrawHorizCurvedInSurface(&m_image, 0, 0, w, h, 255, 255, 0); break;
		case green: m_pStyle->DrawHorizCurvedInSurface(&m_image, 0, 0, w, h, 0, 255, 0); break;
		case cyan: m_pStyle->DrawHorizCurvedInSurface(&m_image, 0, 0, w, h, 0, 255, 255); break;
		case blue: m_pStyle->DrawHorizCurvedInSurface(&m_image, 0, 0, w, h, 0, 0, 255); break;
		case magenta: m_pStyle->DrawHorizCurvedInSurface(&m_image, 0, 0, w, h, 255, 0, 255); break;
	}

	// Calculate rects
	GRect r;
	int nItemHeight = m_pStyle->GetListBoxLineHeight(); 
	int nFirstItem = m_nScrollPos / nItemHeight;
	r.x = 1;
	r.y = 1 - (m_nScrollPos % nItemHeight);
	r.w = w - 2;
	r.h = nItemHeight;
	bool bScrollBar = false;
	if(m_nScrollPos > 0 || m_pItems->GetSize() * nItemHeight > h)
		bScrollBar = true;
	int nScrollBarWidth = m_pStyle->GetDefaultScrollBarSize();
	if(bScrollBar)
	{
		if(w / nScrollBarWidth < 3)
			nScrollBarWidth = w / 3;
		r.w -= nScrollBarWidth;
	}

	// Draw the items
	char* szLine;
	int nCount = m_pItems->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
	{
		if(r.y >= h)
			break;
		szLine = (char*)m_pItems->GetPointer(n);
		if(n == m_nSelectedIndex)
		{
			m_pStyle->DrawVertCurvedOutSurface(&m_image, r.x, r.y, r.w, r.h);
			r.y += 2;
			r.h -= 2;
			m_image.DrawHardText(&r, szLine, m_pStyle->GetTextBoxSelectedTextColor(), 1);
			r.y -= 2;
			r.h += 2;
		}
		else
		{
			r.y += 2;
			r.h -= 2;
			m_image.DrawHardText(&r, szLine, m_pStyle->GetTextBoxTextColor(), 1);
			r.y -= 2;
			r.h += 2;
		}
		r.y += r.h;
	}

	// Draw the scroll bar
	if(bScrollBar)
	{
		r.x = w - 1 - nScrollBarWidth;
		r.y = 1;
		r.w = nScrollBarWidth;
		r.h = h - 2;
		GWidgetVertScrollBar::Draw(&m_image, &r, m_pStyle, m_nScrollPos, h, m_pItems->GetSize() * nItemHeight);
	}

	// Draw the border
	m_image.DrawBox(0, 0, w - 1, h - 1, m_pStyle->GetTextBoxBorderColor(), false);
}

