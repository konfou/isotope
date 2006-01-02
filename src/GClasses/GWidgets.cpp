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
#include "GDirList.h"
#ifdef WIN32
#include <direct.h>
#else // WIN32
#include <unistd.h>
#endif // !WIN32
#include "GFile.h"


GWidgetStyle::GWidgetStyle()
{
	m_nButtonFontSize = 14;
	m_nLabelFontSize = 14;
	m_fButtonFontWidth = (float).8;
	m_fLabelFontWidth = (float)1;
	m_cButtonTextColor = 0xff000000;
	m_cLabelTextColor = 0xff8888ff;
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

void GWidgetStyle::DrawLabelText(GImage* pImage, int x, int y, int w, int h, GString* pString, bool alignLeft, GColor c)
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
	pImage->DrawHardText(&r, szText, c, m_fLabelFontWidth);
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
	pImage->DrawBox(x, y, x + w, y + h, m_cTextBoxTextColor, false);
}

// ----------------------------------------------------------------------

GWidget::GWidget(GWidgetGroup* pParent, int x, int y, int w, int h)
{
	m_pParent = pParent;
	m_pStyle = pParent ? pParent->GetStyle() : NULL;
	m_rect.Set(x, y, w, h);
	if(pParent)
		pParent->AddWidget(this);
	else
		m_nID = -1;
	GAssert(m_nDebugCheck = 0x600df00d, "");
}

/*virtual*/ GWidget::~GWidget()
{
	GAssert(DebugCheck(), "corruption!");
	//GAssert(m_pParent || GetType() == Dialog, "Unexpected root widget");
	if(m_pParent)
		m_pParent->OnDestroyWidget(this);
}

#ifdef _DEBUG
bool GWidget::DebugCheck()
{
	GAssert(m_nDebugCheck == 0x600df00d, "corruption!");
	return true;
}
#endif // _DEBUG

void GWidget::SetPos(int x, int y)
{
	m_rect.x = x;
	m_rect.y = y;
	// todo: dirty the parent?
}

// ----------------------------------------------------------------------

GWidgetAtomic::GWidgetAtomic(GWidgetGroup* pParent, int x, int y, int w, int h)
 : GWidget(pParent, x, y, w, h)
{

}

/*virtual*/ GWidgetAtomic::~GWidgetAtomic()
{
}

void GWidgetAtomic::Draw(GWidgetGroupWithCanvas* pTarget)
{
	GRect r, rTmp;
	GImage* pSrcImage = GetImage(&r);
	GImage* pCanvas;
	int x = m_rect.x;
	int y = m_rect.y;
	GWidgetGroup* pGroup;
	for(pGroup = this->GetParent(); pGroup; pGroup = pGroup->GetParent())
	{
		pCanvas = pGroup->GetImage(&rTmp);
		if(pCanvas)
			pCanvas->AlphaBlit(x, y, pSrcImage, &r);
		if(pGroup == pTarget)
			break;
		x += pGroup->m_rect.x;
		y += pGroup->m_rect.y;
	}
}

/*virtual*/ void GWidgetAtomic::OnChar(char c)
{
	if(m_pParent)
		m_pParent->OnChar(c);
}

/*virtual*/ void GWidgetAtomic::OnMouseMove(int dx, int dy)
{
}

// ----------------------------------------------------------------------

GWidgetGroup::GWidgetGroup(GWidgetGroup* pParent, int x, int y, int w, int h)
 : GWidget(pParent, x, y, w, h)
{
	m_pWidgets = new GPointerArray(16);
	m_dirty = true;
}

/*virtual*/ GWidgetGroup::~GWidgetGroup()
{
	GWidget* pWidget;
	while(m_pWidgets->GetSize() > 0)
	{
		pWidget = (GWidget*)m_pWidgets->GetPointer(0);
		delete(pWidget);
	}
	delete(m_pWidgets);
}

void GWidgetGroup::AddWidget(GWidget* pWidget)
{
	m_pWidgets->AddPointer(pWidget);
	pWidget->m_nID = m_pWidgets->GetSize() - 1;
	m_dirty = true;
}

// todo: use a divide-and-conquer technique to improve performance
/*virtual*/ GWidgetAtomic* GWidgetGroup::FindAtomicWidget(int x, int y)
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
			{
				GRect* pRect = pWidget->GetRect();
				return ((GWidgetGroup*)pWidget)->FindAtomicWidget(x - pRect->x, y - pRect->y);
			}
		}
	}
	return NULL;
}

int GWidgetGroup::GetChildWidgetCount()
{
	return m_pWidgets->GetSize();
}

GWidget* GWidgetGroup::GetChildWidget(int n)
{
	return (GWidget*)m_pWidgets->GetPointer(n);
}

/*virtual*/ void GWidgetGroup::OnDestroyWidget(GWidget* pWidget)
{
	if(pWidget->m_nID >= 0)
	{
		// Remove the widget from my list
		GAssert(GetChildWidget(pWidget->m_nID) == pWidget, "bad id");
		int nLast = m_pWidgets->GetSize() - 1;
		GWidget* pLast = (GWidget*)m_pWidgets->GetPointer(nLast);
		pLast->m_nID = pWidget->m_nID;
		m_pWidgets->SetPointer(pWidget->m_nID, pLast);
		m_pWidgets->DeleteCell(nLast);
		pWidget->m_nID = -1;
		m_dirty = true;
	}
	if(m_pParent)
		m_pParent->OnDestroyWidget(pWidget);
}

// ----------------------------------------------------------------------

GWidgetGroupWithCanvas::GWidgetGroupWithCanvas(GWidgetGroup* pParent, int x, int y, int w, int h)
 : GWidgetGroup(pParent, x, y, w, h)
{
	m_image.SetSize(w, h);
}

/*virtual*/ GWidgetGroupWithCanvas::~GWidgetGroupWithCanvas()
{
}

/*virtual*/ GImage* GWidgetGroupWithCanvas::GetImage(GRect* pOutRect)
{
	if(m_dirty)
		Update();
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth();
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetGroupWithCanvas::Draw(GWidgetGroupWithCanvas* pTarget)
{
	GRect r, rTmp;
	GImage* pSrcImage = GetImage(&r);
	GImage* pCanvas;
	int x = m_rect.x;
	int y = m_rect.y;
	GWidgetGroup* pGroup;
	for(pGroup = this->GetParent(); pGroup; pGroup = pGroup->GetParent())
	{
		pCanvas = pGroup->GetImage(&rTmp);
		if(pCanvas)
			pCanvas->Blit(x, y, pSrcImage, &r);
		if(pGroup == pTarget)
			break;
		x += pGroup->m_rect.x;
		y += pGroup->m_rect.y;
	}
}

// ----------------------------------------------------------------------

GWidgetDialog::GWidgetDialog(int w, int h, GColor cBackground)
 : GWidgetGroupWithCanvas(NULL, 0, 0, w, h)
{
	m_cBackground = cBackground;
	m_pStyle = new GWidgetStyle();
	m_pGrabbedWidget = NULL;
	m_pFocusWidget = NULL;
}

/*virtual*/ GWidgetDialog::~GWidgetDialog()
{
	ReleaseWidget();
	delete(m_pStyle);
}

/*virtual*/ void GWidgetDialog::Update()
{
	m_image.Clear(m_cBackground);
	m_dirty = false;
	int n;
	int nCount = m_pWidgets->GetSize();
	GWidget* pWidget;
	for(n = 0; n < nCount; n++)
	{
		pWidget = (GWidget*)m_pWidgets->GetPointer(n);
		pWidget->Draw(this);
	}
}

void GWidgetDialog::SetFocusWidget(GWidgetAtomic* pWidget)
{
	if(m_pFocusWidget != pWidget)
	{
		if(m_pFocusWidget)
			m_pFocusWidget->OnLoseFocus();
		m_pFocusWidget = pWidget;
		if(pWidget)
			pWidget->OnGetFocus();
	}
}

void GWidgetDialog::GrabWidget(GWidgetAtomic* pWidget, int mouseX, int mouseY)
{
	ReleaseWidget();
	m_pGrabbedWidget = pWidget;
	SetFocusWidget(pWidget);
	m_prevMouseX = mouseX;
	m_prevMouseY = mouseY;
	if(pWidget)
	{
		GWidget* pTmp;
		GRect* pRect;
		for(pTmp = pWidget; pTmp; pTmp = pTmp->GetParent())
		{
			pRect = pTmp->GetRect();
			mouseX -= pRect->x;
			mouseY -= pRect->y;
		}
		pWidget->Grab(mouseX, mouseY);
	}
}

void GWidgetDialog::ReleaseWidget()
{
	if(!m_pGrabbedWidget)
		return;

	// Use a local var in case the handler destroys this dialog
	GWidgetAtomic* pGrabbedWidget = m_pGrabbedWidget;
	m_pGrabbedWidget = NULL;
	pGrabbedWidget->Release();
}

/*virtual*/ void GWidgetDialog::OnDestroyWidget(GWidget* pWidget)
{
	if(pWidget == m_pGrabbedWidget)
		m_pGrabbedWidget = NULL;
	if(pWidget == m_pFocusWidget)
		m_pFocusWidget = NULL;
	GWidgetGroup::OnDestroyWidget(pWidget);
}

void GWidgetDialog::HandleChar(char c)
{
	if(!m_pFocusWidget)
		return;
	m_pFocusWidget->OnChar(c);
}

bool GWidgetDialog::HandleMousePos(int x, int y)
{
	if(!m_pGrabbedWidget)
		return false;
	if(x == m_prevMouseX && y == m_prevMouseY)
		return false;
	m_pGrabbedWidget->OnMouseMove(x - m_prevMouseX, y - m_prevMouseY);
	m_prevMouseX = x;
	m_prevMouseY = y;
	return true;
}

// ----------------------------------------------------------------------

GWidgetTextButton::GWidgetTextButton(GWidgetGroup* pParent, int x, int y, int w, int h, GString* pText)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_image.SetSize(w * 2, h);
	m_text.Copy(pText);
	m_pressed = false;
	m_dirty = true;
}

/*virtual*/ GWidgetTextButton::~GWidgetTextButton()
{
}

/*virtual*/ void GWidgetTextButton::Grab(int x, int y)
{
	m_pressed = true;
	Draw(NULL);
	m_pParent->OnPushTextButton(this);
}

/*virtual*/ void GWidgetTextButton::Release()
{
	m_pressed = false;
	Draw(NULL);
	m_pParent->OnReleaseTextButton(this);
}

void GWidgetTextButton::Update()
{
	m_dirty = false;

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
	if(m_dirty)
		Update();
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
	m_dirty = true;
}

void GWidgetTextButton::SetText(GString* pText)
{
	m_text.Copy(pText);
	m_dirty = true;
	Draw(NULL);
}

void GWidgetTextButton::SetText(const char* szText)
{
	m_text.Copy(szText);
	m_dirty = true;
	Draw(NULL);
}

// ----------------------------------------------------------------------

GWidgetTextLabel::GWidgetTextLabel(GWidgetGroup* pParent, int x, int y, int w, int h, GString* pText, GColor c)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_image.SetSize(w, h);
	m_text.Copy(pText);
	m_alignLeft = true;
	m_cForeground = c;
	m_cBackground = 0x00000000; // transparent
	m_dirty = true;
}

/*virtual*/ GWidgetTextLabel::~GWidgetTextLabel()
{
}

/*virtual*/ void GWidgetTextLabel::Grab(int x, int y)
{
	m_pParent->OnClickTextLabel(this);
}

/*virtual*/ void GWidgetTextLabel::Release()
{
}

void GWidgetTextLabel::Update()
{
	m_dirty = false;
	m_image.Clear(m_cBackground);
	m_pStyle->DrawLabelText(&m_image, 0, 0, m_image.GetWidth(), m_image.GetHeight(), &m_text, m_alignLeft, m_cForeground);
}

GImage* GWidgetTextLabel::GetImage(GRect* pOutRect)
{
	if(m_dirty)
		Update();
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
	m_dirty = true;
}

void GWidgetTextLabel::SetText(GString* pText)
{
	m_text.Copy(pText);
	m_dirty = true;
	Draw(NULL);
}

void GWidgetTextLabel::SetText(const char* szText)
{
	m_text.Copy(szText);
	m_dirty = true;
	Draw(NULL);
}

// ----------------------------------------------------------------------

GWidgetVCRButton::GWidgetVCRButton(GWidgetGroup* pParent, int x, int y, int w, int h, VCR_Type eType)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_image.SetSize(w * 2, h);
	m_eType = eType;
	m_pressed = false;
	m_dirty = true;
}

/*virtual*/ GWidgetVCRButton::~GWidgetVCRButton()
{
}

/*virtual*/ void GWidgetVCRButton::Grab(int x, int y)
{
	m_pressed = true;
	Draw(NULL);
	m_pParent->OnPushVCRButton(this);
}

/*virtual*/ void GWidgetVCRButton::Release()
{
	m_pressed = false;
	Draw(NULL);
}

void GWidgetVCRButton::DrawIcon(int nHorizOfs, int nVertOfs)
{
	int nMinSize = m_image.GetWidth() / 2;
	if(nMinSize > (int)m_image.GetHeight())
		nMinSize = m_image.GetHeight();
	int nArrowSize = nMinSize / 3;
	int hh = m_image.GetHeight() / 2;
	int n;
	if(m_eType == ArrowRight)
	{
		for(n = 0; n < nArrowSize; n++)
			m_image.DrawLine(nHorizOfs + hh - nArrowSize / 2 + n,
							nVertOfs + hh - nArrowSize + n + 1,
							nHorizOfs + hh - nArrowSize / 2 + n,
							nVertOfs + hh + nArrowSize - n - 1,
							0);
	}
	else if(m_eType == ArrowLeft)
	{
		for(n = 0; n < nArrowSize; n++)
			m_image.DrawLine(nHorizOfs + hh + nArrowSize / 2 - n,
							nVertOfs + hh - nArrowSize + n + 1,
							nHorizOfs + hh + nArrowSize / 2 - n,
							nVertOfs + hh + nArrowSize - n - 1,
							0);
	}
	if(m_eType == ArrowDown)
	{
		for(n = 0; n < nArrowSize; n++)
			m_image.DrawLine(nHorizOfs + hh - nArrowSize + n + 1,
							nVertOfs + hh - nArrowSize / 2 + n,
							nHorizOfs + hh + nArrowSize - n - 1,						
							nVertOfs + hh - nArrowSize / 2 + n,
							0);
	}
	else if(m_eType == ArrowUp)
	{
		for(n = 0; n < nArrowSize; n++)
			m_image.DrawLine(nHorizOfs + hh - nArrowSize + n + 1,
							nVertOfs + hh + nArrowSize / 2 - n,
							nHorizOfs + hh + nArrowSize - n - 1,						
							nVertOfs + hh + nArrowSize / 2 - n,
							0);
	}
	else if(m_eType == Square)
	{
		m_image.DrawBox(nHorizOfs + hh - nArrowSize,
						nVertOfs + hh - nArrowSize,
						nHorizOfs + hh + nArrowSize,
						nVertOfs + hh + nArrowSize,
						true, 0);
	}
}

void GWidgetVCRButton::Update()
{
	m_dirty = false;

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
	if(m_dirty)
		Update();
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
	m_dirty = true;
}

void GWidgetVCRButton::SetType(VCR_Type eType)
{
	m_eType = eType;
	m_dirty = true;
}

// ----------------------------------------------------------------------

GWidgetProgressBar::GWidgetProgressBar(GWidgetGroup* pParent, int x, int y, int w, int h)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_image.SetSize(w, h);
	m_fProgress = 0;
	m_dirty = true;
}

/*virtual*/ GWidgetProgressBar::~GWidgetProgressBar()
{
}

/*virtual*/ void GWidgetProgressBar::Grab(int x, int y)
{
}

/*virtual*/ void GWidgetProgressBar::Release()
{
}

void GWidgetProgressBar::Update()
{
	m_dirty = false;
	m_image.Clear(0xff000000);
	int w = m_image.GetWidth();
	int h = m_image.GetHeight();
	if(m_fProgress > 0)
		m_pStyle->DrawVertCurvedOutSurface(&m_image, 1, 1, (int)(m_fProgress * (w - 2)), h - 2);
	m_image.DrawBox(0, 0, w - 1, h - 1, gRGB(64, 64, 64), false);
}

GImage* GWidgetProgressBar::GetImage(GRect* pOutRect)
{
	if(m_dirty)
		Update();
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth();
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetProgressBar::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
	m_dirty = true;
}

void GWidgetProgressBar::SetProgress(float fProgress)
{
	m_fProgress = fProgress;
	m_dirty = true;
	Draw(NULL);
}

// ----------------------------------------------------------------------

GWidgetCheckBox::GWidgetCheckBox(GWidgetGroup* pParent, int x, int y, int w, int h)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_image.SetSize(w * 2, h);
	m_checked = false;
	m_dirty = true;
}

/*virtual*/ GWidgetCheckBox::~GWidgetCheckBox()
{
}

/*virtual*/ void GWidgetCheckBox::Grab(int x, int y)
{
	// todo: gray the box?
}

/*virtual*/ void GWidgetCheckBox::Release()
{
	m_checked = !m_checked;
	Draw(NULL);
}

void GWidgetCheckBox::Update()
{
	m_dirty = false;

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
	if(m_dirty)
		Update();
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
	m_dirty = true;
}

void GWidgetCheckBox::SetChecked(bool checked)
{
	m_checked = checked;
	m_dirty = true;
}

// ----------------------------------------------------------------------

GWidgetSliderTab::GWidgetSliderTab(GWidgetGroup* pParent, int x, int y, int w, int h, bool vertical, bool impressed)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_image.SetSize(w, h);
	m_vertical = vertical;
	m_impressed = impressed;
	m_dirty = true;
}

/*virtual*/ GWidgetSliderTab::~GWidgetSliderTab()
{
}

/*virtual*/ void GWidgetSliderTab::Grab(int x, int y)
{
	m_pParent->OnClickTab(this);
}

/*virtual*/ void GWidgetSliderTab::Release()
{
}

/*virtual*/ void GWidgetSliderTab::OnMouseMove(int dx, int dy)
{
	m_pParent->OnSlideTab(this, dx, dy);
}

void GWidgetSliderTab::Update()
{
	m_dirty = false;
	if(m_rect.w <= 0 || m_rect.h <= 0)
		return;
	if(m_vertical)
	{
		if(m_impressed)
			m_pStyle->DrawHorizCurvedInSurface(&m_image, 0, 0, m_rect.w, m_rect.h);
		else
			m_pStyle->DrawHorizCurvedOutSurface(&m_image, 0, 0, m_rect.w, m_rect.h);
	}
	else
	{
		if(m_impressed)
			m_pStyle->DrawVertCurvedInSurface(&m_image, 0, 0, m_rect.w, m_rect.h);
		else
			m_pStyle->DrawVertCurvedOutSurface(&m_image, 0, 0, m_rect.w, m_rect.h);
	}
}

GImage* GWidgetSliderTab::GetImage(GRect* pOutRect)
{
	if(m_dirty)
		Update();
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_rect.w;
	pOutRect->h = m_rect.h;
	return &m_image;
}

void GWidgetSliderTab::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
	m_dirty = true;
}

// ----------------------------------------------------------------------

GWidgetHorizScrollBar::GWidgetHorizScrollBar(GWidgetGroup* pParent, int x, int y, int w, int h, int nViewSize, int nModelSize)
: GWidgetGroupWithCanvas(pParent, x, y, w, h)
{
	m_nViewSize = nViewSize;
	m_nModelSize = nModelSize;
	m_nPos = 0;
	int wid = GetButtonWidth();
	m_pLeftButton = new GWidgetVCRButton(this, 0, 0, wid, h, GWidgetVCRButton::ArrowLeft);
	m_pRightButton = new GWidgetVCRButton(this, m_rect.w - wid, 0, wid, h, GWidgetVCRButton::ArrowRight);
	m_pLeftTab = new GWidgetSliderTab(this, 0, 0, w, 0, false, true);
	m_pTab = new GWidgetSliderTab(this, 0, 0, w, 0, false, false);
	m_pRightTab = new GWidgetSliderTab(this, 0, 0, w, 0, false, true);
}

/*virtual*/ GWidgetHorizScrollBar::~GWidgetHorizScrollBar()
{
}

int GWidgetHorizScrollBar::GetButtonWidth()
{
	if((m_rect.w >> 2) < m_rect.h)
		return (m_rect.w >> 2);
	else
		return m_rect.h;
}

void GWidgetHorizScrollBar::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
	m_dirty = true;
}

/*virtual*/ void GWidgetHorizScrollBar::Update()
{
	m_dirty = false;

	// Calculations
	int wid = m_image.GetWidth();
	int hgt = m_image.GetHeight();
	int nButtonSize = GetButtonWidth();
	int nSlideAreaSize = wid - nButtonSize - nButtonSize;
	int nTabSize = nSlideAreaSize * m_nViewSize / m_nModelSize;
	if(nTabSize < hgt)
		nTabSize = hgt;
	if(nTabSize > nSlideAreaSize)
		nTabSize = nSlideAreaSize;
	int nTabPos = m_nPos * nSlideAreaSize / m_nModelSize;
	if(nTabPos > nSlideAreaSize - nTabSize)
		nTabPos = nSlideAreaSize - nTabSize;
	nTabPos += nButtonSize;

	// Draw the three tab areas
	m_pLeftTab->SetPos(nButtonSize, 0);
	m_pLeftTab->SetSize(nTabPos - nButtonSize, m_rect.h);
	m_pTab->SetPos(nTabPos, 0);
	m_pTab->SetSize(nTabSize, m_rect.h);
	m_pRightTab->SetPos(nTabPos + nTabSize, 0);
	m_pRightTab->SetSize(nSlideAreaSize + nButtonSize - (nTabPos + nTabSize), m_rect.h);
	m_pLeftTab->Draw(this);
	m_pTab->Draw(this);
	m_pRightTab->Draw(this);

	// Draw the buttons
	m_pLeftButton->Draw(this);
	m_pRightButton->Draw(this);
}

/*virtual*/ void GWidgetHorizScrollBar::OnPushVCRButton(GWidgetVCRButton* pButton)
{
	if(pButton == m_pLeftButton)
	{
		m_nPos -= m_nViewSize / 5;
		if(m_nPos < 0)
			m_nPos = 0;
	}
	else
	{
		GAssert(pButton == m_pRightButton, "unexpected button");
		m_nPos += m_nViewSize / 5;
		if(m_nPos > m_nModelSize - m_nViewSize)
			m_nPos = m_nModelSize - m_nViewSize;
	}
	m_dirty = true;
	if(m_pParent)
		m_pParent->OnHorizScroll(this);
}

/*virtual*/ void GWidgetHorizScrollBar::OnClickTab(GWidgetSliderTab* pTab)
{
	if(pTab == m_pLeftTab)
	{
		m_nPos -= m_nViewSize;
		if(m_nPos < 0)
			m_nPos = 0;
		m_dirty = true;
		if(m_pParent)
			m_pParent->OnHorizScroll(this);
	}
	else if(pTab == m_pRightTab)
	{
		m_nPos += m_nViewSize;
		if(m_nPos > m_nModelSize - m_nViewSize)
			m_nPos = m_nModelSize - m_nViewSize;
		m_dirty = true;
		if(m_pParent)
			m_pParent->OnHorizScroll(this);
	}
}

/*virtual*/ void GWidgetHorizScrollBar::OnSlideTab(GWidgetSliderTab* pTab, int dx, int dy)
{
	if(pTab != m_pTab)
		return;
	m_nPos += dx * m_nModelSize / m_nViewSize;
	if(m_nPos < 0)
		m_nPos = 0;
	else if(m_nPos > m_nModelSize - m_nViewSize)
		m_nPos = m_nModelSize - m_nViewSize;
	m_dirty = true;
	Draw(NULL);
	if(m_pParent)
		m_pParent->OnHorizScroll(this);
}

// ----------------------------------------------------------------------

GWidgetVertScrollBar::GWidgetVertScrollBar(GWidgetGroup* pParent, int x, int y, int w, int h, int nViewSize, int nModelSize)
: GWidgetGroupWithCanvas(pParent, x, y, w, h)
{
	m_nViewSize = nViewSize;
	m_nModelSize = nModelSize;
	m_nPos = 0;
	int hgt = GetButtonHeight();
	m_pUpButton = new GWidgetVCRButton(this, 0, 0, w, hgt, GWidgetVCRButton::ArrowUp);
	m_pDownButton = new GWidgetVCRButton(this, 0, m_rect.h - hgt, w, hgt, GWidgetVCRButton::ArrowDown);
	m_pAboveTab = new GWidgetSliderTab(this, 0, 0, w, 0, true, true);
	m_pTab = new GWidgetSliderTab(this, 0, 0, w, 0, true, false);
	m_pBelowTab = new GWidgetSliderTab(this, 0, 0, w, 0, true, true);
}

/*virtual*/ GWidgetVertScrollBar::~GWidgetVertScrollBar()
{
}

int GWidgetVertScrollBar::GetButtonHeight()
{
	if((m_rect.h >> 2) < m_rect.w)
		return (m_rect.h >> 2);
	else
		return m_rect.w;
}

void GWidgetVertScrollBar::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
	m_dirty = true;
}

/*virtual*/ void GWidgetVertScrollBar::Update()
{
	m_dirty = false;

	// Calculations
	if(m_nModelSize < m_nViewSize)
		m_nModelSize = m_nViewSize;
	int wid = m_image.GetWidth();
	int hgt = m_image.GetHeight();
	int nButtonSize = GetButtonHeight();
	int nSlideAreaSize = hgt - nButtonSize - nButtonSize;
	int nTabSize = nSlideAreaSize * m_nViewSize / m_nModelSize;
	if(nTabSize < wid)
		nTabSize = wid;
	if(nTabSize > nSlideAreaSize)
		nTabSize = nSlideAreaSize;
	if(m_nPos < 0)
		m_nPos = 0;
	int nTabPos = m_nPos * nSlideAreaSize / m_nModelSize;
	if(nTabPos > nSlideAreaSize - nTabSize)
		nTabPos = nSlideAreaSize - nTabSize;
	nTabPos += nButtonSize;

	// Draw the three tab areas
	m_pAboveTab->SetPos(0, nButtonSize);
	m_pAboveTab->SetSize(m_rect.w, nTabPos - nButtonSize);
	m_pTab->SetPos(0, nTabPos);
	m_pTab->SetSize(m_rect.w, nTabSize);
	m_pBelowTab->SetPos(0, nTabPos + nTabSize);
	m_pBelowTab->SetSize(m_rect.w, nSlideAreaSize + nButtonSize - (nTabPos + nTabSize));
	m_pAboveTab->Draw(this);
	m_pTab->Draw(this);
	m_pBelowTab->Draw(this);

	// Draw the buttons
	m_pUpButton->Draw(this);
	m_pDownButton->Draw(this);
}

/*virtual*/ void GWidgetVertScrollBar::OnPushVCRButton(GWidgetVCRButton* pButton)
{
	if(pButton == m_pUpButton)
	{
		m_nPos -= m_nViewSize / 5;
		if(m_nPos < 0)
			m_nPos = 0;
	}
	else
	{
		GAssert(pButton == m_pDownButton, "unexpected button");
		m_nPos += m_nViewSize / 5;
		if(m_nPos > m_nModelSize - m_nViewSize)
			m_nPos = m_nModelSize - m_nViewSize;
	}
	m_dirty = true;
	if(m_pParent)
		m_pParent->OnVertScroll(this);
}

/*virtual*/ void GWidgetVertScrollBar::OnClickTab(GWidgetSliderTab* pTab)
{
	if(pTab == m_pAboveTab)
	{
		m_nPos -= m_nViewSize;
		if(m_nPos < 0)
			m_nPos = 0;
		m_dirty = true;
		if(m_pParent)
			m_pParent->OnVertScroll(this);
	}
	else if(pTab == m_pBelowTab)
	{
		m_nPos += m_nViewSize;
		if(m_nPos > m_nModelSize - m_nViewSize)
			m_nPos = m_nModelSize - m_nViewSize;
		m_dirty = true;
		if(m_pParent)
			m_pParent->OnVertScroll(this);
	}
}

/*virtual*/ void GWidgetVertScrollBar::OnSlideTab(GWidgetSliderTab* pTab, int dx, int dy)
{
	if(pTab != m_pTab)
		return;
	m_nPos += dy * m_nModelSize / m_nViewSize;
	if(m_nPos < 0)
		m_nPos = 0;
	else if(m_nPos > m_nModelSize - m_nViewSize)
		m_nPos = m_nModelSize - m_nViewSize;
	m_dirty = true;
	Draw(NULL);
	if(m_pParent)
		m_pParent->OnVertScroll(this);
}

// ----------------------------------------------------------------------

GWidgetTextBox::GWidgetTextBox(GWidgetGroup* pParent, int x, int y, int w, int h)
: GWidgetAtomic(pParent, x, y, w, h)
{
	m_image.SetSize(w, h);
	m_dirty = true;
	m_bGotFocus = false;
	m_bPassword = false;
	m_nAnchorPos = 0;
	m_nCursorPos = 0;
	m_nMouseDelta = 0;
}

/*virtual*/ GWidgetTextBox::~GWidgetTextBox()
{
}

void GWidgetTextBox::SetText(const char* szText)
{
	m_text.Copy(szText);
	m_nAnchorPos = m_text.GetLength();
	m_nCursorPos = m_nAnchorPos;
	m_dirty = true;
	Draw(NULL);
}

GImage* GWidgetTextBox::GetImage(GRect* pOutRect)
{
	if(m_dirty)
		Update();
	pOutRect->x = 0;
	pOutRect->y = 0;
	pOutRect->w = m_image.GetWidth();
	pOutRect->h = m_image.GetHeight();
	return &m_image;
}

void GWidgetTextBox::Update()
{
	m_dirty = false;

	// Draw the background area
	int w = m_image.GetWidth();
	int h = m_image.GetHeight();
	m_pStyle->DrawVertCurvedInSurface(&m_image, 0, 0, w, h);
	m_image.DrawBox(0, 0, w - 1, h - 1, m_bGotFocus ? m_pStyle->GetTextBoxBorderColor() : m_pStyle->GetTextBoxSelectedTextColor(), false);

	// Draw the text
	char* szText = (char*)alloca(m_text.GetLength() + 1);
	m_text.GetAnsi(szText);
	GRect r;
	r.x = 1;
	r.y = 3;
	r.w = w - 2;
	r.h = h - 4;
	if(m_bPassword)
	{
		int i;
		for(i = 0; szText[i] != '\0'; i++)
			szText[i] = '#';
	}
	m_image.DrawHardText(&r, szText, m_pStyle->GetTextBoxTextColor(), 1);

	// Draw the cursor or selection
	if(!m_bGotFocus)
		return; // don't waste time drawing the cursor for inactive text boxes
	int nSelStart = m_nAnchorPos;
	int nSelEnd = m_nCursorPos;
	if(nSelEnd < nSelStart)
	{
		int nTmp = nSelEnd;
		nSelEnd = nSelStart;
		nSelStart = nTmp;
	}
	szText[nSelEnd] = '\0';
	int nSelEndPos = m_image.MeasureHardTextWidth(r.h, szText, 1) + r.x;
	if(nSelEndPos > w - 3)
		nSelEndPos = w - 3;
	if(nSelStart == nSelEnd)
		m_pStyle->DrawCursor(&m_image, nSelEndPos, 2, 2, h - 5);
	else
	{
		szText[nSelStart] = '\0';
		int nSelStartPos = m_image.MeasureHardTextWidth(r.h, szText, 1) + r.x;
		r.x = nSelStartPos;
		r.w = nSelEndPos - nSelStartPos;
		m_image.InvertRect(&r);
	}
}

/*virtual*/ void GWidgetTextBox::OnChar(char c)
{
	if(c == '\b')
	{
		if(m_nAnchorPos == m_nCursorPos)
		{
			if(m_nAnchorPos <= 0)
				return;
			m_nAnchorPos--;
		}
		if(m_nCursorPos < m_nAnchorPos)
		{
			int nTmp = m_nCursorPos;
			m_nCursorPos = m_nAnchorPos;
			m_nAnchorPos = nTmp;
		}
		m_text.Remove(m_nAnchorPos, m_nCursorPos - m_nAnchorPos);
		m_nCursorPos = m_nAnchorPos;
	}
	else if(c == '\r')
	{
		m_pParent->OnTextBoxPressEnter(this);
		return;
	}
	else
	{
		if(m_nAnchorPos != m_nCursorPos)
		{
			if(m_nCursorPos < m_nAnchorPos)
			{
				int nTmp = m_nCursorPos;
				m_nCursorPos = m_nAnchorPos;
				m_nAnchorPos = nTmp;
			}
			m_text.Remove(m_nAnchorPos, m_nCursorPos - m_nAnchorPos);
			m_nCursorPos = m_nAnchorPos;
		}
		if(m_nCursorPos >= m_text.GetLength())
            m_text.Add(c);
		else
			m_text.InsertChar(m_nCursorPos, (wchar_t)c);
		m_nCursorPos++;
		m_nAnchorPos++;
	}
	m_pParent->OnTextBoxTextChanged(this);
	m_dirty = true;
	Draw(NULL);
}

/*virtual*/ void GWidgetTextBox::Grab(int x, int y)
{
	char* szText = (char*)alloca(m_text.GetLength() + 1);
	m_text.GetAnsi(szText);
	m_nMouseDelta = 0;
	m_nAnchorPos = m_image.CountHardTextChars(x - 1 + 3, m_image.GetHeight() - 4, szText, 1);
	m_nCursorPos = m_nAnchorPos;
	m_dirty = true;
	Draw(NULL);
}

/*virtual*/ void GWidgetTextBox::Release()
{
}

/*virtual*/ void GWidgetTextBox::OnMouseMove(int dx, int dy)
{
	m_nMouseDelta += dx;
	int nNewCursorPos = m_nAnchorPos + m_nMouseDelta / 6;
	if(nNewCursorPos < 0)
		nNewCursorPos = 0;
	if(nNewCursorPos > m_text.GetLength())
		nNewCursorPos = m_text.GetLength();
	if(nNewCursorPos != m_nCursorPos)
	{
		m_nCursorPos = nNewCursorPos;
		m_dirty = true;
		Draw(NULL);
	}
}

/*virtual*/ void GWidgetTextBox::OnGetFocus()
{
	m_bGotFocus = true;
	m_dirty = true;
	Draw(NULL);
}

/*virtual*/ void GWidgetTextBox::OnLoseFocus()
{
	m_bGotFocus = false;
	m_dirty = true;
	Draw(NULL);
}

// ----------------------------------------------------------------------

GWidgetListBoxItem::GWidgetListBoxItem(GWidgetListBox* pParent, const wchar_t* wszText)
	: GWidgetAtomic((GWidgetGroup*)pParent, 0, 0, 0, 0)
{
	m_sText = new GString();
	m_sText->Copy(wszText);
	m_nIndex = pParent->GetSize() - 1;
	pParent->SetItemRect(&m_rect, m_nIndex);
}

/*virtual*/ GWidgetListBoxItem::~GWidgetListBoxItem()
{
	delete(m_sText);
}

/*virtual*/ void GWidgetListBoxItem::Grab(int x, int y)
{
	((GWidgetListBox*)m_pParent)->OnGrabItem(m_nIndex);
}

/*virtual*/ void GWidgetListBoxItem::Draw(GWidgetGroupWithCanvas* pTarget)
{
	((GWidgetListBox*)m_pParent)->Draw(pTarget); // todo: this isn't a very efficient way to do this
}

// ----------------------------------------------------------------------

GWidgetListBox::GWidgetListBox(GWidgetGroup* pParent, int x, int y, int w, int h)
: GWidgetGroupWithCanvas(pParent, x, y, w, h)
{
	m_nSelectedIndex = -1;
	m_nScrollPos = 0;
	m_eBaseColor = blue;
}

/*virtual*/ GWidgetListBox::~GWidgetListBox()
{
}

void GWidgetListBox::SetSelection(int n)
{
	m_nSelectedIndex = n;
	m_dirty = true;
}

void GWidgetListBox::SetScrollPos(int n)
{
	m_nScrollPos = n;
	m_dirty = true;
}

void GWidgetListBox::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
	m_dirty = true;
}

/*virtual*/ void GWidgetListBox::Update()
{
	m_dirty = false;

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
	//int nFirstItem = m_nScrollPos / nItemHeight;
	r.x = 1;
	r.y = 1 - (m_nScrollPos % nItemHeight);
	r.w = w - 2;
	r.h = nItemHeight;
	bool bScrollBar = false;
	if(m_nScrollPos > 0 || m_pWidgets->GetSize() * nItemHeight > h)
		bScrollBar = true;
	int nScrollBarWidth = m_pStyle->GetDefaultScrollBarSize();
	if(bScrollBar)
	{
		if(w / nScrollBarWidth < 3)
			nScrollBarWidth = w / 3;
		r.w -= nScrollBarWidth;
	}

	// Draw the items
	char szAnsi[256];
	GWidgetListBoxItem* pItem;
	int nCount = m_pWidgets->GetSize();
	int n;
	GString* pText;
	for(n = 0; n < nCount; n++)
	{
		if(r.y >= h)
			break;
		pItem = GetItem(n);
		pText = pItem->GetText();
		pText->GetAnsi(szAnsi, 256);
		if(n == m_nSelectedIndex)
		{
			m_pStyle->DrawVertCurvedOutSurface(&m_image, r.x, r.y, r.w, r.h);
			r.y += 2;
			r.h -= 2;
			m_image.DrawHardText(&r, szAnsi, m_pStyle->GetTextBoxSelectedTextColor(), 1);
			r.y -= 2;
			r.h += 2;
		}
		else
		{
			r.y += 2;
			r.h -= 2;
			m_image.DrawHardText(&r, szAnsi, m_pStyle->GetTextBoxTextColor(), 1);
			r.y -= 2;
			r.h += 2;
		}
		r.y += r.h;
	}
/*
	// Draw the scroll bar
	if(bScrollBar)
	{
		r.x = w - 1 - nScrollBarWidth;
		r.y = 1;
		r.w = nScrollBarWidth;
		r.h = h - 2;
		GWidgetVertScrollBar::Draw(&m_image, &r, m_pStyle, m_nScrollPos, h, m_pWidgets->GetSize() * nItemHeight);
	}
*/
	// Draw the border
	m_image.DrawBox(0, 0, w - 1, h - 1, m_pStyle->GetTextBoxBorderColor(), false);
}

void GWidgetListBox::OnGrabItem(int nIndex)
{
	SetSelection(nIndex);
	m_dirty = true; // todo: only redraw the list item
	Draw(NULL);
	if(m_pParent)
		m_pParent->OnChangeListSelection(this);
}

GWidgetListBoxItem* GWidgetListBox::GetItem(int n)
{
	return (GWidgetListBoxItem*)m_pWidgets->GetPointer(n);
}

int GWidgetListBox::GetSize()
{
	return m_pWidgets->GetSize();
}

void GWidgetListBox::SetItemRect(GRect* pRect, int nIndex)
{
	int nItemHeight = m_pStyle->GetListBoxLineHeight(); 
	pRect->Set(0, nIndex * nItemHeight, m_rect.w, nItemHeight);
	m_dirty = true;
}

void GWidgetListBox::Clear()
{
	int n;
	for(n = 0; n < m_pWidgets->GetSize(); n++)
	{
		GWidget* pWidget = GetChildWidget(n);
//		if(pWidget == m_pVertScrollBar)
//			continue;
//		if(pWidget == m_pHorizScrollBar)
//			continue;
		delete(pWidget);
		n--;
	}
	m_dirty = true;
}

// ----------------------------------------------------------------------

GWidgetGrid::GWidgetGrid(GWidgetGroup* pParent, GPointerArray* pRows, int nColumns, int x, int y, int w, int h)
: GWidgetGroupWithCanvas(pParent, x, y, w, h)
{
	m_nColumns = nColumns;
	m_pRows = pRows;
	m_nRowHeight = 20;
	m_pColumnHeaders = new GWidget*[m_nColumns];
	m_nColumnWidths = new int[m_nColumns];
	int n;
	for(n = 0; n < nColumns; n++)
	{
		m_pColumnHeaders[n] = NULL;
		m_nColumnWidths[n] = 80;
	}
	int nScrollBarSize = m_pStyle->GetDefaultScrollBarSize();
	m_pVertScrollBar = new GWidgetVertScrollBar(this, w - nScrollBarSize, 0, nScrollBarSize, h, h - m_nRowHeight - nScrollBarSize, h);
	m_pHorizScrollBar = new GWidgetHorizScrollBar(this, 0, h - nScrollBarSize, w - nScrollBarSize, nScrollBarSize, w - nScrollBarSize, 80 * nColumns);
}

/*virtual*/ GWidgetGrid::~GWidgetGrid()
{
	delete(m_pColumnHeaders);
	delete(m_nColumnWidths);
}

void GWidgetGrid::SetHScrollPos(int n)
{
	m_pHorizScrollBar->SetPos(n);
	m_dirty = true;
}

void GWidgetGrid::SetVScrollPos(int n)
{
	m_pVertScrollBar->SetPos(n);
	m_dirty = true;
}

void GWidgetGrid::SetSize(int w, int h)
{
	m_rect.w = w;
	m_rect.h = h;
	m_image.SetSize(w, h);
	m_dirty = true;
}

void GWidgetGrid::AddBlankRow()
{
	GWidget** pNewRow = new GWidget*[m_nColumns];
	int n;
	for(n = 0; n < m_nColumns; n++)
		pNewRow[n] = NULL;
	m_pRows->AddPointer(pNewRow);
	m_dirty = true;
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
	// todo: figure out how to handle the dirty flag here
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
	// todo: figure out how to handle the dirty flag here
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
	m_dirty = true;
}

/*virtual*/ void GWidgetGrid::Update()
{
	m_dirty = false;

	// Calculations
	int w = m_image.GetWidth() - m_pStyle->GetDefaultScrollBarSize();
	int h = m_image.GetHeight() - m_pStyle->GetDefaultScrollBarSize();

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
	int nHScrollPos = m_pHorizScrollBar->GetPos();
	int nVScrollPos = m_pVertScrollBar->GetPos();
	GImage* pImage;
	int n, i;
	for(n = 0; n < m_nColumns; n++)
	{
		nColumnWidth = m_nColumnWidths[n];
		if(nColumnStart + nColumnWidth > nHScrollPos)
		{
			// Calculate left clip amount
			nLeftPos = nColumnStart - nHScrollPos;
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
				if(pImage)
				{
					if(r.w > nColumnWidth)
						r.w = nColumnWidth;
					if(r.h > m_nRowHeight)
						r.h = m_nRowHeight;
					r.x += nLeftClip;
					r.w -= nLeftClip;
					r.w -= nRightClip;
					m_image.Blit(nLeftPos, 0, pImage, &r);
				}
			}

			// Draw all the widgets in the column
			nVertPos = m_nRowHeight;
			nVertHeight = m_nRowHeight - (nVScrollPos % m_nRowHeight);
			i = nVScrollPos / m_nRowHeight;
			nModelVertPos = i * m_nRowHeight;
			while(nVertPos < h)
			{
				if(i >= m_pRows->GetSize())
					break;
				if(nModelVertPos + m_nRowHeight > nVScrollPos)
				{
					pWidget = ((GWidget**)m_pRows->GetPointer(i))[n];
					if(pWidget)
					{
						pImage = pWidget->GetImage(&r);
						if(pImage)
						{
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
					}
				}
				nVertPos += nVertHeight;
				nVertHeight = m_nRowHeight;
				nModelVertPos += m_nRowHeight;
				i++;
			}
		}
		nColumnStart += nColumnWidth;
	}
	while(n < m_nColumns)
	{
		nColumnStart += m_nColumnWidths[n];
		n++;
	}

	// Draw the scroll bars
	m_pVertScrollBar->SetModelSize(MAX(m_pRows->GetSize() * m_nRowHeight, m_rect.h));
	m_pVertScrollBar->Draw(this);
	m_pHorizScrollBar->SetModelSize(MAX(nColumnStart, m_rect.w));
	m_pHorizScrollBar->Draw(this);
}

void GWidgetGrid::FlushItems()
{
	bool bCol;
	int n, i;
	for(n = 0; n < m_pWidgets->GetSize(); n++)
	{
		GWidget* pWidget = (GWidget*)m_pWidgets->GetPointer(n);
		if(pWidget == m_pVertScrollBar)
			continue;
		if(pWidget == m_pHorizScrollBar)
			continue;
		bCol = false;
		for(i = 0; i < m_nColumns; i++)
		{
			if(m_pColumnHeaders[i] == pWidget)
			{
				bCol = true;
				break;
			}
		}
		if(bCol)
			continue;
		delete(pWidget);
		n--;
	}
	int nCount = m_pRows->GetSize();
	for(n = 0; n < nCount; n++)
		delete((GWidget**)m_pRows->GetPointer(n));
	m_pRows->Clear();

	m_dirty = true;
}

/*virtual*/ void GWidgetGrid::OnVertScroll(GWidgetVertScrollBar* pScrollBar)
{
	m_dirty = true; // todo: only update the rows
	Draw(NULL);
}

/*virtual*/ void GWidgetGrid::OnHorizScroll(GWidgetHorizScrollBar* pScrollBar)
{
	m_dirty = true; // todo: only update the rows
	Draw(NULL);
}

/*virtual*/ GWidgetAtomic* GWidgetGrid::FindAtomicWidget(int x, int y)
{
	GRect* pRect = m_pVertScrollBar->GetRect();
	if(x >= pRect->x)
		return m_pVertScrollBar->FindAtomicWidget(x - pRect->x, y - pRect->y);
	pRect = m_pHorizScrollBar->GetRect();
	if(y >= pRect->y)
		return m_pHorizScrollBar->FindAtomicWidget(x - pRect->x, y - pRect->y);
	int xOrig = x;
	int yOrig = y;
	x += m_pHorizScrollBar->GetPos();
	GWidget** pRow;
	if(y < m_nRowHeight)
		pRow = m_pColumnHeaders;
	else
	{
		y += m_pVertScrollBar->GetPos();
		y -= m_nRowHeight;
		y /= m_nRowHeight;
		if(y >= 0 && y < m_pRows->GetSize())
			pRow = (GWidget**)m_pRows->GetPointer(y);
		else
			return NULL;
	}
	GWidget* pWidget = NULL;
	int nColLeft = 0;
	int n;
	for(n = 0; n < m_nColumns; n++)
	{
		nColLeft += m_nColumnWidths[n];
		if(nColLeft > x)
		{
			pWidget = pRow[n];
			break;
		}
	}
	if(pWidget)
	{
		if(pWidget->IsAtomicWidget())
			return (GWidgetAtomic*)pWidget;
		else
		{
			GRect* pRect = pWidget->GetRect();
			return ((GWidgetGroup*)pWidget)->FindAtomicWidget(xOrig - pRect->x, yOrig - pRect->y);
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------

GWidgetFileSystemBrowser::GWidgetFileSystemBrowser(GWidgetGroup* pParent, int x, int y, int w, int h, const char* szExtension)
 : GWidgetGroup(pParent, x, y, w, h)
{
	int nPathHeight = m_pStyle->GetListBoxLineHeight();
	GString s;
	m_pPath = new GWidgetTextLabel(this, 0, 0, w, nPathHeight, &s, 0xff8888ff);
	m_pPath->SetBackgroundColor(0xff000000);
	m_pListItems = new GPointerArray(64);
	m_pFiles = new GWidgetGrid(this, m_pListItems, 3, 0, nPathHeight, w, h - nPathHeight);

	// Column Headers
	m_pFiles->SetColumnWidth(0, 300);
	m_pFiles->SetColumnWidth(1, 50);
	m_pFiles->SetColumnWidth(2, 50);
	GWidgetTextButton* pButton;
	s.Copy(L"Filename");
	pButton = new GWidgetTextButton(m_pFiles, 0, 0, 300, 20, &s);
	m_pFiles->SetColumnHeader(0, pButton);
	s.Copy(L"Size");
	pButton = new GWidgetTextButton(m_pFiles, 0, 0, 50, 20, &s);
	m_pFiles->SetColumnHeader(1, pButton);
	s.Copy(L"Date");
	pButton = new GWidgetTextButton(m_pFiles, 0, 0, 50, 20, &s);
	m_pFiles->SetColumnHeader(2, pButton);

	// Extension
	int nExtLen = 0;
	if(szExtension)
		nExtLen = strlen(szExtension);
	if(nExtLen > 0)
	{
		m_szExtension = new char[nExtLen + 1];
		strcpy(m_szExtension, szExtension);
	}
	else
		m_szExtension = NULL;

	ReloadFileList();
}

/*virtual*/ GWidgetFileSystemBrowser::~GWidgetFileSystemBrowser()
{
	delete(m_pListItems);
	delete(m_szExtension);
}

/*virtual*/ void GWidgetFileSystemBrowser::Draw(GWidgetGroupWithCanvas* pTarget)
{
	m_pPath->Draw(pTarget);
	m_pFiles->Draw(pTarget);
}

void GWidgetFileSystemBrowser::AddFilename(bool bDir, const char* szFilename)
{
	int nRows = m_pFiles->GetRows()->GetSize();
	m_pFiles->AddBlankRow();
	GString s;
	s.Copy(szFilename);
	GWidgetTextLabel* pLabel = new GWidgetTextLabel(m_pFiles, 0, 0, 100, 20, &s, bDir ? 0xffffffff : 0xff44ffaa);
	m_pFiles->SetWidget(0, nRows, pLabel);
}

void GWidgetFileSystemBrowser::ReloadFileList()
{
	m_pFiles->FlushItems();
	{
		getcwd(m_szPath, 256);
		m_pPath->SetText(m_szPath);
		if(strlen(m_szPath) >
#ifdef WIN32
							3)
#else // WIN32
							1)
#endif // !WIN32
		AddFilename(true, "..");
	}

	// Dirs
	{
		GDirList dl(false, false, true, false);
		while(true)
		{
			const char* szDir = dl.GetNext();
			if(!szDir)
				break;
			AddFilename(true, szDir);
		}
	}

	// Files
	{
		char szExt[256];
		GDirList dl(false, true, false, false);
		while(true)
		{
			const char* szFilename = dl.GetNext();
			if(!szFilename)
				break;
			if(m_szExtension)
			{
				_splitpath(szFilename, NULL, NULL, NULL, szExt);
				if(stricmp(szExt, m_szExtension) != 0)
					continue;
			}
			AddFilename(false, szFilename);
		}
	}
}

/*virtual*/ void GWidgetFileSystemBrowser::OnClickTextLabel(GWidgetTextLabel* pLabel)
{
	GString* pText = pLabel->GetText();
	char* szFilename = (char*)alloca(pText->GetLength() + 1);
	pText->GetAnsi(szFilename);
	strcat(m_szPath, "/");
	strcat(m_szPath, szFilename);
	if(chdir(m_szPath) == 0)
	{
		m_pFiles->SetVScrollPos(0);
		ReloadFileList();
		Draw(NULL);
	}
	else
	{
		if(m_pParent)
			m_pParent->OnSelectFilename(this, m_szPath);
	}
}
