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
	m_nButtonFontSize = 14;
	m_nLabelFontSize = 14;
	m_fButtonFontWidth = (float).8;
	m_fLabelFontWidth = (float)1;
	m_cButtonTextColor = 0;
	m_cLabelTextColor = 0x888888ff;
	m_cButtonPressedTextColor = gRGB(255, 255, 255);
	m_cTextBoxBorderColor = gRGB(255, 255, 255);
	m_cTextBoxTextColor = gRGB(255, 255, 255);
}

GWidgetStyle::~GWidgetStyle()
{
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

void GWidgetStyle::DrawLabelText(GImage* pImage, int x, int y, int w, int h, GString* pString, bool alignLeft)
{
	char* szText = (char*)alloca(pString->GetLength() + 1);
	pString->GetAnsi(szText);
	int nLabelFontSize = m_nLabelFontSize;
	if(h < nLabelFontSize)
		nLabelFontSize = h;
	int wid = pImage->MeasureHardTextWidth(nLabelFontSize, szText, m_fLabelFontWidth);
	GRect r;
	if(alignLeft)
		r.x = x;
	else
	{
		r.x = x + w - wid;
		if(r.x < x)
			r.x = x;
	}
	r.y = y + (h - nLabelFontSize) / 2;
	if(r.y < y)
		r.y = y;
	r.w = w - (r.x - x);
	r.h = nLabelFontSize;
	pImage->DrawHardText(&r, szText, m_cLabelTextColor, m_fLabelFontWidth);
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

GWidget::GWidget(GWidgetGroup* pParent, int x, int y, int w, int h)
{
	m_pParent = pParent;
	m_rect.Set(x, y, w, h);
	m_nAbsoluteX = -1;
	m_nAbsoluteY = -1;
	if(pParent)
		pParent->AddWidget(this);
}

/*virtual*/ GWidget::~GWidget()
{
}

void GWidget::SetPos(int x, int y)
{
	m_rect.x = x;
	m_rect.y = y;
	m_nAbsoluteX = -1;
	m_nAbsoluteY = -1;
}

void GWidget::CalcAbsolutePos()
{
	if(m_nAbsoluteX >= 0)
		return;
	if(m_pParent)
	{
		m_pParent->CalcAbsolutePos();
		m_nAbsoluteX = m_pParent->m_nAbsoluteX + m_rect.x;
		m_nAbsoluteY = m_pParent->m_nAbsoluteY + m_rect.y;
	}
	else
	{
		m_nAbsoluteX = m_rect.x;
		m_nAbsoluteY = m_rect.y;
	}
}

// ----------------------------------------------------------------------

GWidgetAtomic::GWidgetAtomic(GWidgetGroup* pParent, int x, int y, int w, int h)
 : GWidget(pParent, x, y, w, h)
{

}

/*virtual*/ GWidgetAtomic::~GWidgetAtomic()
{
}

// ----------------------------------------------------------------------

GWidgetGroup::GWidgetGroup(GWidgetGroup* pParent, int x, int y, int w, int h)
 : GWidget(pParent, x, y, w, h)
{
	m_pWidgets = new GPointerArray(16);
}

/*virtual*/ GWidgetGroup::~GWidgetGroup()
{
	int n;
	int nCount = m_pWidgets->GetSize();
	for(n = 0; n < nCount; n++)
		delete((GWidget*)m_pWidgets->GetPointer(n));
	delete(m_pWidgets);
}

void GWidgetGroup::AddWidget(GWidget* pWidget)
{
	m_pWidgets->AddPointer(pWidget);
}

// todo: use a divide-and-conquer technique to improve performance
GWidgetAtomic* GWidgetGroup::FindAtomicWidget(int x, int y)
{
	int n;
	int nCount = m_pWidgets->GetSize();
	GWidget* pWidget;
	for(n = 0; n < nCount; n++)
	{
		pWidget = (GWidget*)m_pWidgets->GetPointer(n);
		if(pWidget->GetRect()->DoesInclude(x, y))
		{
			if(pWidget->IsAtomicWidget())
				return (GWidgetAtomic*)pWidget;
			else
				return ((GWidgetGroup*)pWidget)->FindAtomicWidget(x - pWidget->m_rect.x, y - pWidget->m_rect.y);
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------

GWidgetContainer::GWidgetContainer(int w, int h)
 : GWidgetGroup(NULL, 0, 0, w, h)
{
	m_pStyle = new GWidgetStyle();
	m_pGrabbedWidget = NULL;
}

/*virtual*/ GWidgetContainer::~GWidgetContainer()
{
	ReleaseWidget();
	delete(m_pStyle);
}

void GWidgetContainer::Draw(GImage* pImage)
{
	int n;
	int nCount = m_pWidgets->GetSize();
	GWidget* pWidget;
	for(n = 0; n < nCount; n++)
	{
		pWidget = (GWidget*)m_pWidgets->GetPointer(n);
		pWidget->Draw(pImage);
	}
}

void GWidgetContainer::GrabWidget(GWidgetAtomic* pWidget)
{
	ReleaseWidget();
	m_pGrabbedWidget = pWidget;
	pWidget->Grab();
}

void GWidgetContainer::ReleaseWidget()
{
	if(!m_pGrabbedWidget)
		return;
	m_pGrabbedWidget->Release();
	m_pGrabbedWidget = NULL;
}


// ----------------------------------------------------------------------

GWidgetTextButton::GWidgetTextButton(GWidgetGroup* pParent, int x, int y, int w, int h, GString* pText)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_pStyle = pParent->GetStyle();
	m_image.SetSize(w * 2, h);
	m_text.Copy(pText);
	m_pressed = false;
}

/*virtual*/ GWidgetTextButton::~GWidgetTextButton()
{
}

/*virtual*/ void GWidgetTextButton::Grab()
{
	m_pressed = true;
}

/*virtual*/ void GWidgetTextButton::Release()
{
	m_pressed = false;
}

void GWidgetTextButton::Update()
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

GImage* GWidgetTextButton::GetImage(GRect* pOutRect)
{
	pOutRect->x = m_pressed ? m_image.GetWidth() / 2 : 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth() / 2;
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetTextButton::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w * 2, h);
}

void GWidgetTextButton::SetText(GString* pText)
{
	m_text.Copy(pText);
}

/*virtual*/ void GWidgetTextButton::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	CalcAbsolutePos();
	pImage->Blit(m_nAbsoluteX, m_nAbsoluteY, pSrcImage, &r);
}

// ----------------------------------------------------------------------

GWidgetTextLabel::GWidgetTextLabel(GWidgetGroup* pParent, int x, int y, int w, int h, GString* pText)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_pStyle = pParent->GetStyle();
	m_image.SetSize(w, h);
	m_text.Copy(pText);
	m_alignLeft = true;
}

/*virtual*/ GWidgetTextLabel::~GWidgetTextLabel()
{
}

/*virtual*/ void GWidgetTextLabel::Grab()
{
}

/*virtual*/ void GWidgetTextLabel::Release()
{
}

void GWidgetTextLabel::Update()
{
	m_image.Clear(0);
	m_pStyle->DrawLabelText(&m_image, 0, 0, m_image.GetWidth(), m_image.GetHeight(), &m_text, m_alignLeft);
}

GImage* GWidgetTextLabel::GetImage(GRect* pOutRect)
{
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth();
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetTextLabel::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
}

void GWidgetTextLabel::SetText(GString* pText)
{
	m_text.Copy(pText);
}

/*virtual*/ void GWidgetTextLabel::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	CalcAbsolutePos();
	pImage->Blit(m_nAbsoluteX, m_nAbsoluteY, pSrcImage, &r);
}

// ----------------------------------------------------------------------

GWidgetVCRButton::GWidgetVCRButton(GWidgetGroup* pParent, int x, int y, int w, int h, VCR_Type eType)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_pStyle = pParent->GetStyle();
	m_image.SetSize(w * 2, h);
	m_eType = eType;
	m_pressed = false;
}

/*virtual*/ GWidgetVCRButton::~GWidgetVCRButton()
{
}

/*virtual*/ void GWidgetVCRButton::Grab()
{
	m_pressed = true;
}

/*virtual*/ void GWidgetVCRButton::Release()
{
	m_pressed = false;
}

void GWidgetVCRButton::DrawIcon(int nHorizOfs, int nVertOfs)
{
	int nMinSize = m_image.GetWidth() / 2;
	if(nMinSize > (int)m_image.GetHeight())
		nMinSize = m_image.GetHeight();
	int nArrowSize = nMinSize / 3;
	int n;
	if(m_eType == ArrowRight)
	{
		for(n = 0; n < nArrowSize; n++)
			m_image.DrawLine(nHorizOfs - nArrowSize / 2 + n,
							nVertOfs + m_image.GetHeight() / 2 - nArrowSize + n + 1,
							nHorizOfs - nArrowSize / 2 + n,
							nVertOfs + m_image.GetHeight() / 2 + nArrowSize - n - 1,
							0);
	}
	else if(m_eType == ArrowLeft)
	{
		for(n = 0; n < nArrowSize; n++)
			m_image.DrawLine(nHorizOfs + nArrowSize / 2 - n,
							nVertOfs + m_image.GetHeight() / 2 - nArrowSize + n + 1,
							nHorizOfs + nArrowSize / 2 - n,
							nVertOfs + m_image.GetHeight() / 2 + nArrowSize - n - 1,
							0);
	}
	if(m_eType == ArrowDown)
	{
		for(n = 0; n < nArrowSize; n++)
			m_image.DrawLine(nHorizOfs + m_image.GetWidth() / 2 - nArrowSize + n + 1,
							nVertOfs - nArrowSize / 2 + n,
							nHorizOfs + m_image.GetHeight() / 2 + nArrowSize - n - 1,						
							nVertOfs - nArrowSize / 2 + n,
							0);
	}
	else if(m_eType == ArrowUp)
	{
		for(n = 0; n < nArrowSize; n++)
			m_image.DrawLine(nHorizOfs + m_image.GetWidth() / 2 - nArrowSize + n + 1,
							nVertOfs + nArrowSize / 2 - n,
							nHorizOfs + m_image.GetHeight() / 2 + nArrowSize - n - 1,						
							nVertOfs + nArrowSize / 2 - n,
							0);
	}
	else if(m_eType == Square)
	{
		m_image.DrawBox(nHorizOfs + m_image.GetWidth() / 2 - nArrowSize,
						nVertOfs + m_image.GetHeight() / 2 - nArrowSize,
						nHorizOfs + m_image.GetWidth() / 2 + nArrowSize,
						nVertOfs + m_image.GetHeight() / 2 + nArrowSize,
						true, 0);
	}
}

void GWidgetVCRButton::Update()
{
	// Draw the non-pressed image
	int w = m_image.GetWidth() / 2;
	int h = m_image.GetHeight();
	m_pStyle->DrawVertCurvedOutSurface(&m_image, 0, 0, w, h);
	DrawIcon(0, 0);
	m_image.DrawBox(0, 0, w - 1, h - 1, gRGB(64, 64, 64), false);

	// Draw the pressed image
	int nHorizOfs = (int)(w * (float).05);
	int nVertOfs = (int)(h * (float).15);
	m_pStyle->DrawVertCurvedInSurface(&m_image, w, 0, w, h);
	DrawIcon(nHorizOfs + w, nVertOfs);
	m_image.DrawBox(w, 0, w + w - 1, h - 1, gRGB(255, 255, 255), false);
}

GImage* GWidgetVCRButton::GetImage(GRect* pOutRect)
{
	pOutRect->x = m_pressed ? m_image.GetWidth() / 2 : 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth() / 2;
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetVCRButton::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w * 2, h);
}

void GWidgetVCRButton::SetType(VCR_Type eType)
{
	m_eType = eType;
}

/*virtual*/ void GWidgetVCRButton::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	CalcAbsolutePos();
	pImage->Blit(m_nAbsoluteX, m_nAbsoluteY, pSrcImage, &r);
}

// ----------------------------------------------------------------------

GWidgetCheckBox::GWidgetCheckBox(GWidgetGroup* pParent, int x, int y, int w, int h)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_pStyle = pParent->GetStyle();
	m_image.SetSize(w * 2, h);
	m_checked = false;
}

/*virtual*/ GWidgetCheckBox::~GWidgetCheckBox()
{
}

/*virtual*/ void GWidgetCheckBox::Grab()
{
	// todo: gray the box?
}

/*virtual*/ void GWidgetCheckBox::Release()
{
	m_checked = !m_checked;
}

void GWidgetCheckBox::Update()
{
	// Draw the non-checked image
	int w = m_image.GetWidth() / 2;
	int h = m_image.GetHeight();
	m_image.Clear(0);
	m_image.DrawBox(1, 1, w - 2, h - 2, gRGB(64, 128, 128), false);
	m_image.DrawBox(2, 2, w - 3, h - 3, 0xffffffff, true);

	// Draw the checked image
	m_image.DrawBox(w + 1, 1, w + w - 2, h - 2, gRGB(64, 128, 128), false);
	m_image.DrawBox(w + 2, 2, w + w - 3, h - 3, 0xffffffff, true);

	m_image.DrawLine(w + 4, 4, w + w - 5, h - 5, 0);
	m_image.DrawLine(w + 5, 4, w + w - 5, h - 6, 0);
	m_image.DrawLine(w + 4, 5, w + w - 6, h - 5, 0);

	m_image.DrawLine(w + w - 5, 4, w + 4, h - 5, 0);
	m_image.DrawLine(w + w - 6, 4, w + 4, h - 6, 0);
	m_image.DrawLine(w + w - 5, 5, w + 5, h - 5, 0);
}

GImage* GWidgetCheckBox::GetImage(GRect* pOutRect)
{
	pOutRect->x = m_checked ? m_image.GetWidth() / 2 : 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth() / 2;
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetCheckBox::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w * 2, h);
}

void GWidgetCheckBox::SetChecked(bool checked)
{
	m_checked = checked;
}

/*virtual*/ void GWidgetCheckBox::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	CalcAbsolutePos();
	pImage->Blit(m_nAbsoluteX, m_nAbsoluteY, pSrcImage, &r);
}

// ----------------------------------------------------------------------

GWidgetHorizScrollBar::GWidgetHorizScrollBar(GWidgetGroup* pParent, int x, int y, int w, int h, int nViewSize, int nModelSize)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_pStyle = pParent->GetStyle();
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

/*virtual*/ void GWidgetHorizScrollBar::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	CalcAbsolutePos();
	pImage->Blit(m_nAbsoluteX, m_nAbsoluteY, pSrcImage, &r);
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

GWidgetVertScrollBar::GWidgetVertScrollBar(GWidgetGroup* pParent, int x, int y, int w, int h, int nViewSize, int nModelSize)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_pStyle = pParent->GetStyle();
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

/*virtual*/ void GWidgetVertScrollBar::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	CalcAbsolutePos();
	pImage->Blit(m_nAbsoluteX, m_nAbsoluteY, pSrcImage, &r);
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

GWidgetTextBox::GWidgetTextBox(GWidgetGroup* pParent, int x, int y, int w, int h)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_pStyle = pParent->GetStyle();
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

/*virtual*/ void GWidgetTextBox::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	CalcAbsolutePos();
	pImage->Blit(m_nAbsoluteX, m_nAbsoluteY, pSrcImage, &r);
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

GWidgetListBox::GWidgetListBox(GWidgetGroup* pParent, GPointerArray* pItems, int x, int y, int w, int h)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_pItems = pItems;
	m_pStyle = pParent->GetStyle();
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

/*virtual*/ void GWidgetListBox::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	CalcAbsolutePos();
	pImage->Blit(m_nAbsoluteX, m_nAbsoluteY, pSrcImage, &r);
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

// ----------------------------------------------------------------------

GWidgetGrid::GWidgetGrid(GWidgetGroup* pParent, GPointerArray* pRows, int nColumns, int x, int y, int w, int h)
: GWidgetGroup(pParent, x, y, w, h)
{
	m_nColumns = nColumns;
	m_pRows = pRows;
	m_pStyle = pParent->GetStyle();
	m_image.SetSize(w, h);
	m_nHScrollPos = 0;
	m_nVScrollPos = 0;
	m_nRowHeight = 20;
	m_pColumnHeaders = new GWidget*[m_nColumns];
	m_nColumnWidths = new int[m_nColumns];
	int n;
	for(n = 0; n < nColumns; n++)
	{
		m_pColumnHeaders[n] = NULL;
		m_nColumnWidths[n] = 80;
	}
}

/*virtual*/ GWidgetGrid::~GWidgetGrid()
{
	delete(m_pColumnHeaders);
	delete(m_nColumnWidths);
}

GImage* GWidgetGrid::GetImage(GRect* pOutRect)
{
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth();
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

/*virtual*/ void GWidgetGrid::Draw(GImage* pImage)
{
	GRect r;
	GImage* pSrcImage = GetImage(&r);
	CalcAbsolutePos();
	pImage->Blit(m_nAbsoluteX, m_nAbsoluteY, pSrcImage, &r);
}

void GWidgetGrid::SetHScrollPos(int n)
{
	m_nHScrollPos = n;
}

void GWidgetGrid::SetVScrollPos(int n)
{
	m_nVScrollPos = n;
}

void GWidgetGrid::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
}

void GWidgetGrid::AddBlankRow()
{
	GWidget** pNewRow = new GWidget*[m_nColumns];
	int n;
	for(n = 0; n < m_nColumns; n++)
		pNewRow[n] = NULL;
	m_pRows->AddPointer(pNewRow);
}

GWidget* GWidgetGrid::GetWidget(int col, int row)
{
	GAssert(col >= 0 && col < m_nColumns, "out of range");
	GWidget** pRow = (GWidget**)m_pRows->GetPointer(row);
	return pRow[col];
}

void GWidgetGrid::SetWidget(int col, int row, GWidget* pWidget)
{
	GAssert(col >= 0 && col < m_nColumns, "out of range");
	GWidget** pRow = (GWidget**)m_pRows->GetPointer(row);
	pRow[col] = pWidget;
	int nColPos = 0;
	int n;
	for(n = 0; n < col; n++)
		nColPos += m_nColumnWidths[n];
	pWidget->SetPos(nColPos, (row + 1) * m_nRowHeight);
}

GWidget* GWidgetGrid::GetColumnHeader(int col)
{
	GAssert(col >= 0 && col < m_nColumns, "out of range");
	return m_pColumnHeaders[col];
}

void GWidgetGrid::SetColumnHeader(int col, GWidget* pWidget)
{
	GAssert(col >= 0 && col < m_nColumns, "out of range");
	m_pColumnHeaders[col] = pWidget;
	int nColPos = 0;
	int n;
	for(n = 0; n < col; n++)
		nColPos += m_nColumnWidths[n];
	pWidget->SetPos(nColPos, 0);
}

int GWidgetGrid::GetColumnWidth(int col)
{
	GAssert(col >= 0 && col < m_nColumns, "out of range");
	return m_nColumnWidths[col];
}

void GWidgetGrid::SetColumnWidth(int col, int nWidth)
{
	GAssert(col >= 0 && col < m_nColumns, "out of range");
	m_nColumnWidths[col] = nWidth;
}

void GWidgetGrid::UpdateAll()
{
	// Calculations
	int w = m_image.GetWidth() - m_pStyle->GetListBoxLineHeight();
	int h = m_image.GetHeight() - m_pStyle->GetListBoxLineHeight();

	// Draw the background area
	m_image.Clear(0x00000000);

	// Draw columns
	int nColumnStart = 0;
	int nColumnWidth;
	int nLeftPos;
	int nLeftClip, nRightClip;
	int nVertPos, nVertHeight, nModelVertPos;
	GWidget* pWidget;
	GRect r;
	GImage* pImage;
	int n, i;
	for(n = 0; n < m_nColumns; n++)
	{
		nColumnWidth = m_nColumnWidths[n];
		if(nColumnStart + nColumnWidth > m_nHScrollPos)
		{
			// Calculate left clip amount
			nLeftPos = nColumnStart - m_nHScrollPos;
			if(nLeftPos >= w)
				break;
			if(nLeftPos + nColumnWidth <= w)
				nRightClip = 0;
			else
				nRightClip = w - (nLeftPos + nColumnWidth);
			if(nLeftPos >= 0)
				nLeftClip = 0;
			else
			{
				nLeftClip = -nLeftPos;
				nLeftPos = 0;
			}

			// Draw the header widget
			pWidget = m_pColumnHeaders[n];
			if(pWidget)
			{
				pImage = pWidget->GetImage(&r);
				if(r.w > nColumnWidth)
					r.w = nColumnWidth;
				if(r.h > m_nRowHeight)
					r.h = m_nRowHeight;
				r.x += nLeftClip;
				r.w -= nLeftClip;
				r.w -= nRightClip;
				m_image.Blit(nLeftPos, 0, pImage, &r);
			}

			// Draw all the widgets in the column
			nVertPos = m_nRowHeight;
			nVertHeight = m_nVScrollPos % m_nRowHeight;
			i = (m_nVScrollPos + m_nRowHeight - 1) / m_nRowHeight - 1; // The funky math in this line is to work around how C++ rounds negative fractions up
			nModelVertPos = i * m_nRowHeight;
			while(nVertPos < h)
			{
				if(i >= m_pRows->GetSize())
					break;
				if(nModelVertPos + m_nRowHeight > m_nVScrollPos)
				{
					pWidget = ((GWidget**)m_pRows->GetPointer(i))[n];
					pImage = pWidget->GetImage(&r);
					if(r.w > nColumnWidth)
						r.w = nColumnWidth;
					if(r.h > m_nRowHeight)
						r.h = m_nRowHeight;
					r.x += nLeftClip;
					r.w -= nLeftClip;
					r.w -= nRightClip;
					r.y += (m_nRowHeight - nVertHeight);
					r.h -= (m_nRowHeight - nVertHeight);
					if(nVertPos + r.h > h)
						r.h -= (nVertPos + r.h - h);
					m_image.Blit(nLeftPos, nVertPos, pImage, &r);
				}
				nVertPos += nVertHeight;
				nVertHeight = m_nRowHeight;
				nModelVertPos += m_nRowHeight;
				i++;
			}
		}
		nColumnStart += nColumnWidth;
	}

	// Draw the scroll bars
	/*
		r.x = w - 1 - nScrollBarWidth;
		r.y = 1;
		r.w = nScrollBarWidth;
		r.h = h - 2;
		GWidgetVertScrollBar::Draw(&m_image, &r, m_pStyle, m_nScrollPos, h, m_pItems->GetSize() * nItemHeight);
	*/
}

