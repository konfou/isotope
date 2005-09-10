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
class GWidgetAtomic;
class GWidgetGroup;
class GWidget;


class GWidgetStyle
{
protected:
	int m_nButtonFontSize;
	int m_nLabelFontSize;
	float m_fButtonFontWidth;
	float m_fLabelFontWidth;
	GColor m_cButtonTextColor;
	GColor m_cLabelTextColor;
	GColor m_cButtonPressedTextColor;
	GColor m_cTextBoxBorderColor;
	GColor m_cTextBoxTextColor;

public:
	GWidgetStyle();
	~GWidgetStyle();

	GColor GetTextBoxBorderColor() { return m_cTextBoxBorderColor; }
	GColor GetTextBoxTextColor() { return m_cTextBoxBorderColor; }
	GColor GetTextBoxSelectedTextColor() { return m_cButtonTextColor; }
	int GetListBoxLineHeight() { return 16; }
	int GetDefaultScrollBarSize() { return 16; }
	void DrawButtonText(GImage* pImage, int x, int y, int w, int h, GString* pString, bool pressed);
	void DrawLabelText(GImage* pImage, int x, int y, int w, int h, GString* pString, bool alignLeft);
	void DrawHorizCurvedOutSurface(GImage* pImage, int x, int y, int w, int h);
	void DrawHorizCurvedInSurface(GImage* pImage, int x, int y, int w, int h, int colorMaskRed = 0, int colorMaskGreen = 0, int colorMaskBlue = 255);
	void DrawVertCurvedOutSurface(GImage* pImage, int x, int y, int w, int h);
	void DrawVertCurvedInSurface(GImage* pImage, int x, int y, int w, int h);
	void DrawCursor(GImage* pImage, int x, int y, int w, int h);
};





class GWidget
{
friend class GWidgetGroup;
public:
	enum WidgetType
	{
		CheckBox,
		Container,
		Custom,
		Grid,
		HScrollBar,
		ListBox,
		TextBox,
		TextButton,
		TextLabel,
		VCRButton,
		VScrollBar,
	};

protected:
	GRect m_rect;
	GWidgetGroup* m_pParent;
	int m_nAbsoluteX;
	int m_nAbsoluteY;

public:
	GWidget(GWidgetGroup* m_pParent, int x, int y, int w, int h);
	virtual ~GWidget();

	virtual WidgetType GetType() = 0;
	virtual GImage* GetImage(GRect* pOutRect) = 0;
	virtual bool IsAtomicWidget() = 0;
	virtual void Draw(GImage* pImage) = 0;
	void SetPos(int x, int y);
	GRect* GetRect() { return &m_rect; }
	GWidgetGroup* GetParent() { return m_pParent; }

protected:
	void CalcAbsolutePos();
};






class GWidgetAtomic : public GWidget
{
friend class GWidgetContainer;
public:
	GWidgetAtomic(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetAtomic();

	virtual bool IsAtomicWidget() { return true; }

protected:
	virtual void Grab() = 0;
	virtual void Release() = 0;
};






class GWidgetGroup : public GWidget
{
friend class GWidget;
protected:
	GPointerArray* m_pWidgets;

public:
	GWidgetGroup(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetGroup();

	virtual bool IsAtomicWidget() { return false; }
	virtual GWidgetStyle* GetStyle() { return m_pParent->GetStyle(); }
	GWidgetAtomic* FindAtomicWidget(int x, int y);

protected:
	void AddWidget(GWidget* pWidget);
};






class GWidgetContainer : public GWidgetGroup
{
protected:
	GWidgetStyle* m_pStyle;
	GWidgetAtomic* m_pGrabbedWidget;

public:
	GWidgetContainer(int w, int h);
	virtual ~GWidgetContainer();

	virtual WidgetType GetType() { return Container; }
	virtual GImage* GetImage(GRect* pOutRect) { return NULL; }
	virtual void Draw(GImage* pImage);
	virtual GWidgetStyle* GetStyle() { return m_pStyle; }
	GWidgetAtomic* GetGrabbedWidget() { return m_pGrabbedWidget; }
	void GrabWidget(GWidgetAtomic* pWidget);
	void ReleaseWidget();
};





class GWidgetTextButton : public GWidgetAtomic
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	GString m_text;
	bool m_pressed;

public:
	GWidgetTextButton(GWidgetGroup* pParent, int x, int y, int w, int h, GString* pText);
	virtual ~GWidgetTextButton();

	virtual WidgetType GetType() { return TextButton; }
	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GImage* pImage);
	void SetSize(int w, int h);
	void SetText(GString* pText);
	void Update();
	bool IsPressed() { return m_pressed; }

protected:
	virtual void Grab();
	virtual void Release();
};





class GWidgetTextLabel : public GWidgetAtomic
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	GString m_text;
	bool m_alignLeft;

public:
	GWidgetTextLabel(GWidgetGroup* pParent, int x, int y, int w, int h, GString* pText);
	virtual ~GWidgetTextLabel();

	virtual WidgetType GetType() { return TextLabel; }
	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GImage* pImage);
	void SetSize(int w, int h);
	void SetText(GString* pText);
	void Update();
	void SetAlignLeft(bool bAlignLeft) { m_alignLeft = bAlignLeft; }

protected:
	virtual void Grab();
	virtual void Release();
};





class GWidgetVCRButton : public GWidgetAtomic
{
public:
	enum VCR_Type
	{
		ArrowLeft,
		ArrowRight, // Play
		ArrowUp,
		ArrowDown,
		Square,  // Stop
	};

protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	VCR_Type m_eType;
	bool m_pressed;

public:
	GWidgetVCRButton(GWidgetGroup* pParent, int x, int y, int w, int h, VCR_Type eType);
	virtual ~GWidgetVCRButton();

	virtual WidgetType GetType() { return VCRButton; }
	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GImage* pImage);
	void SetSize(int w, int h);
	void SetType(VCR_Type eType);
	void Update();
	bool IsPressed() { return m_pressed; }

protected:
	virtual void Grab();
	virtual void Release();
	void DrawIcon(int nHorizOfs, int nVertOfs);
};






class GWidgetCheckBox : public GWidgetAtomic
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	bool m_checked;

public:
	GWidgetCheckBox(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetCheckBox();

	virtual WidgetType GetType() { return CheckBox; }
	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GImage* pImage);
	void SetSize(int w, int h);
	void SetChecked(bool checked);
	void Update();
	bool IsChecked() { return m_checked; }

protected:
	virtual void Grab();
	virtual void Release();
};






class GWidgetHorizScrollBar : public GWidgetAtomic
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	int m_nViewSize;
	int m_nModelSize;
	int m_nPos;

public:
	GWidgetHorizScrollBar(GWidgetGroup* pParent, int x, int y, int w, int h, int nViewSize, int nModelSize);
	virtual ~GWidgetHorizScrollBar();

	virtual WidgetType GetType() { return HScrollBar; }
	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GImage* pImage);
	void SetViewSize(int n) { m_nViewSize = n; }
	void SetModelSize(int n) { m_nModelSize = n; }
	void SetPos(int n) { m_nPos = n; }
	void SetSize(int w, int h);
	static void Draw(GImage* pImage, GRect* pR, GWidgetStyle* pStyle, int nPos, int nViewSize, int nModelSize);
	void Update();

protected:
	virtual void Grab() {};
	virtual void Release() {};
};






class GWidgetVertScrollBar : public GWidgetAtomic
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	int m_nViewSize;
	int m_nModelSize;
	int m_nPos;

public:
	GWidgetVertScrollBar(GWidgetGroup* pParent, int x, int y, int w, int h, int nViewSize, int nModelSize);
	virtual ~GWidgetVertScrollBar();

	virtual WidgetType GetType() { return VScrollBar; }
	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GImage* pImage);
	void SetViewSize(int n) { m_nViewSize = n; }
	void SetModelSize(int n) { m_nModelSize = n; }
	void SetPos(int n) { m_nPos = n; }
	void SetSize(int w, int h);
	static void Draw(GImage* pImage, GRect* pR, GWidgetStyle* pStyle, int nPos, int nViewSize, int nModelSize);
	void Update();

protected:
	virtual void Grab() {};
	virtual void Release() {};
};







class GWidgetTextBox : public GWidgetAtomic
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	GString m_text;

public:
	GWidgetTextBox(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetTextBox();

	virtual WidgetType GetType() { return TextBox; }
	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GImage* pImage);
	GString* GetText() { return &m_text; }
	void SetText(const char* szText) { m_text.Copy(szText); Update(); }
	void Update();

protected:
	virtual void Grab() {};
	virtual void Release() {};
};






class GWidgetListBox : public GWidgetAtomic
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
	// Takes ownership of pItems
	GWidgetListBox(GWidgetGroup* pParent, GPointerArray* pItems, int x, int y, int w, int h);
	virtual ~GWidgetListBox();

	virtual WidgetType GetType() { return ListBox; }
	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GImage* pImage);
	void SetSize(int w, int h);
	void SetBaseColor(BaseColor eBaseColor) { m_eBaseColor = eBaseColor; }
	int GetSelection() { return m_nSelectedIndex; }
	void SetSelection(int n);
	int GetScrollPos() { return m_nScrollPos; }
	void SetScrollPos(int n);
	GPointerArray* GetItems() { return m_pItems; }
	void Update();

protected:
	virtual void Grab() {};
	virtual void Release() {};
};





class GWidgetGrid : public GWidgetGroup
{
protected:
	GWidgetStyle* m_pStyle;
	GImage m_image;
	GPointerArray* m_pRows;
	int m_nHScrollPos;
	int m_nVScrollPos;
	int m_nColumns;
	int m_nRowHeight;
	GWidget** m_pColumnHeaders;
	int* m_nColumnWidths;

public:
	// Takes ownership of pItems
	GWidgetGrid(GWidgetGroup* pParent, GPointerArray* pItems, int nColumns, int x, int y, int w, int h);
	virtual ~GWidgetGrid();

	virtual WidgetType GetType() { return Grid; }
	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GImage* pImage);
	void SetSize(int w, int h);
	int GetRowHeight() { return m_nRowHeight; }
	void SetRowHeight(int n) { m_nRowHeight = n; }
	int GetHScrollPos() { return m_nHScrollPos; }
	int GetVScrollPos() { return m_nVScrollPos; }
	void SetHScrollPos(int n);
	void SetVScrollPos(int n);

	// Adds an empty row to the grid
	void AddBlankRow();

	// Returns the complete collection of all cell widgets
	GPointerArray* GetRows() { return m_pRows; }

	// Returns the number of columns
	int GetColumnCount() { return m_nColumns; }

	// Gets the widget in the specified cell
	GWidget* GetWidget(int col, int row);

	// Sets the widget in the specified cell
	void SetWidget(int col, int row, GWidget* pWidget);

	// Sets the widget in a column header
	GWidget* GetColumnHeader(int col);

	// Gets the widget in a column header
	void SetColumnHeader(int col, GWidget* pWidget);

	// Gets the width of a column
	int GetColumnWidth(int col);

	// Sets the width of a column
	void SetColumnWidth(int col, int nWidth);

	// Redraws the cell.  It doesn't tell the widget in that cell to redraw itself, so
	// you should do that before calling UpdateCell.
	//void UpdateCell(int col, int row);

	// Updates all the visible cells
	void UpdateAll();
};


#endif // __GWIDGETS_H__
