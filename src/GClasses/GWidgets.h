/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GWIDGETS_H__
#define __GWIDGETS_H__

#include "GImage.h"
#include "GString.h"

class GPointerArray;
class GWidget;


class GWidgetStyle
{
protected:
	GPointerArray* m_pWidgets;
	int m_nButtonFontSize;
	float m_fButtonFontWidth;
	GColor m_cButtonTextColor;
	GColor m_cButtonPressedTextColor;
	GColor m_cTextBoxBorderColor;
	GColor m_cTextBoxTextColor;

public:
	GWidgetStyle();
	~GWidgetStyle();

	void AddWidget(GWidget* pWidget);
	GWidget* FindWidget(int x, int y);
	GColor GetTextBoxBorderColor() { return m_cTextBoxBorderColor; }
	GColor GetTextBoxTextColor() { return m_cTextBoxBorderColor; }
	GColor GetTextBoxSelectedTextColor() { return m_cButtonTextColor; }
	int GetListBoxLineHeight() { return 16; }
	int GetDefaultScrollBarSize() { return 16; }
	void DrawButtonText(GImage* pImage, int x, int y, int w, int h, GString* pString, bool pressed);
	void DrawHorizCurvedOutSurface(GImage* pImage, int x, int y, int w, int h);
	void DrawHorizCurvedInSurface(GImage* pImage, int x, int y, int w, int h, int colorMaskRed = 0, int colorMaskGreen = 0, int colorMaskBlue = 255);
	void DrawVertCurvedOutSurface(GImage* pImage, int x, int y, int w, int h);
	void DrawVertCurvedInSurface(GImage* pImage, int x, int y, int w, int h);
	void DrawCursor(GImage* pImage, int x, int y, int w, int h);
};





class GWidget
{
public:
	enum WidgetType
	{
		Button,
		HScrollBar,
		VScrollBar,
		TextBox,
		ListBox,
		Custom,
	};

protected:
	GRect m_rect;

public:
	GWidget(int x, int y, int w, int h) { m_rect.Set(x, y, w, h); }
	virtual ~GWidget() {}

	virtual WidgetType GetType() = 0;
	GRect* GetRect() { return &m_rect; }
	void SetPos(int x, int y) { m_rect.x = x; m_rect.y = y; }
};





class GWidgetButton : public GWidget
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	GString m_text;
	bool m_pressed;

public:
	GWidgetButton(GWidgetStyle* pStyle, int x, int y, int w, int h, GString* pText);
	virtual ~GWidgetButton();

	virtual WidgetType GetType() { return Button; }
	GImage* GetImage(GRect* pOutRect);
	void SetPressed(bool b) { m_pressed = b; }
	bool IsPressed() { return m_pressed; }
	void SetSize(int w, int h);
	void SetText(GString* pText);
	void Draw(GImage* pImage);
	void Update();
};






class GWidgetHorizScrollBar : public GWidget
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	int m_nViewSize;
	int m_nModelSize;
	int m_nPos;

public:
	GWidgetHorizScrollBar(GWidgetStyle* pStyle, int x, int y, int w, int h, int nViewSize, int nModelSize);
	virtual ~GWidgetHorizScrollBar();

	virtual WidgetType GetType() { return HScrollBar; }
	GImage* GetImage(GRect* pOutRect);
	void SetViewSize(int n) { m_nViewSize = n; }
	void SetModelSize(int n) { m_nModelSize = n; }
	void SetPos(int n) { m_nPos = n; }
	void SetSize(int w, int h);
	void Draw(GImage* pImage);
	static void Draw(GImage* pImage, GRect* pR, GWidgetStyle* pStyle, int nPos, int nViewSize, int nModelSize);
	void Update();
};






class GWidgetVertScrollBar : public GWidget
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	int m_nViewSize;
	int m_nModelSize;
	int m_nPos;

public:
	GWidgetVertScrollBar(GWidgetStyle* pStyle, int x, int y, int w, int h, int nViewSize, int nModelSize);
	virtual ~GWidgetVertScrollBar();

	virtual WidgetType GetType() { return VScrollBar; }
	GImage* GetImage(GRect* pOutRect);
	void SetViewSize(int n) { m_nViewSize = n; }
	void SetModelSize(int n) { m_nModelSize = n; }
	void SetPos(int n) { m_nPos = n; }
	void SetSize(int w, int h);
	void Draw(GImage* pImage);
	static void Draw(GImage* pImage, GRect* pR, GWidgetStyle* pStyle, int nPos, int nViewSize, int nModelSize);
	void Update();
};







class GWidgetTextBox : public GWidget
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	GString m_text;

public:
	GWidgetTextBox(GWidgetStyle* pStyle, int x, int y, int w, int h);
	virtual ~GWidgetTextBox();

	virtual WidgetType GetType() { return TextBox; }
	GString* GetText() { return &m_text; }
	void SetText(const char* szText) { m_text.Copy(szText); Update(); }
	GImage* GetImage(GRect* pOutRect);
	void Draw(GImage* pImage);
	void Update();
};






class GWidgetListBox : public GWidget
{
public:
	enum BaseColor
	{
		red,
		yellow,
		green,
		cyan,
		blue,
		magenta,
	};

protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	GPointerArray* m_pItems;
	int m_nSelectedIndex;
	int m_nScrollPos;
	BaseColor m_eBaseColor;

public:
	GWidgetListBox(GWidgetStyle* pStyle, GPointerArray* pItems, int x, int y, int w, int h);
	virtual ~GWidgetListBox();

	virtual WidgetType GetType() { return ListBox; }
	void SetSize(int w, int h);
	GImage* GetImage(GRect* pOutRect);
	void SetBaseColor(BaseColor eBaseColor) { m_eBaseColor = eBaseColor; }
	void Draw(GImage* pImage);
	int GetSelection() { return m_nSelectedIndex; }
	void SetSelection(int n);
	int GetScrollPos() { return m_nScrollPos; }
	void SetScrollPos(int n);
	GPointerArray* GetItems() { return m_pItems; }
	void Update();
};


#endif // __GWIDGETS_H__
