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
class GWidgetListBox;
class GWidgetTextButton;
class GWidgetVCRButton;
class GWidgetHorizScrollBar;
class GWidgetVertScrollBar;
class GWidgetGroupWithCanvas;
class GWidgetTextLabel;
class GWidgetFileSystemBrowser;
class GWidgetSliderTab;
class GWidgetTextBox;



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
	void DrawLabelText(GImage* pImage, int x, int y, int w, int h, GString* pString, bool alignLeft, bool bright);
	void DrawHorizCurvedOutSurface(GImage* pImage, int x, int y, int w, int h);
	void DrawHorizCurvedInSurface(GImage* pImage, int x, int y, int w, int h, int colorMaskRed = 0, int colorMaskGreen = 0, int colorMaskBlue = 255);
	void DrawVertCurvedOutSurface(GImage* pImage, int x, int y, int w, int h);
	void DrawVertCurvedInSurface(GImage* pImage, int x, int y, int w, int h);
	void DrawCursor(GImage* pImage, int x, int y, int w, int h);
};




// The base class of all GUI widgets
class GWidget
{
friend class GWidgetGroup;
friend class GWidgetGroupWithCanvas;
public:
	enum WidgetType
	{
		CheckBox,
		Custom,
		Dialog,
		FileSystemBrowser,
		Grid,
		HScrollBar,
		ListBox,
		ListBoxItem,
		ProgressBar,
		SliderTab,
		TextBox,
		TextButton,
		TextLabel,
		VCRButton,
		VScrollBar,
	};

protected:
	GRect m_rect;
	GWidgetGroup* m_pParent;
	GWidgetStyle* m_pStyle;
	int m_nID; // for use by the owning parent
#ifdef _DEBUG
	unsigned int m_nDebugCheck;
#endif // _DEBUG

public:
	GWidget(GWidgetGroup* m_pParent, int x, int y, int w, int h);
	virtual ~GWidget();

	virtual WidgetType GetType() = 0;
	virtual bool IsAtomicWidget() = 0;
	virtual void Draw(GWidgetGroupWithCanvas* pTarget) = 0;
	virtual GImage* GetImage(GRect* pOutRect) = 0;
	void SetPos(int x, int y);
	GRect* GetRect() { return &m_rect; }
	GWidgetGroup* GetParent() { return m_pParent; }
};





// The base class of all atomic widgets (widgets that are not composed of other widgets).
class GWidgetAtomic : public GWidget
{
friend class GWidgetDialog;
public:
	GWidgetAtomic(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetAtomic();

	virtual bool IsAtomicWidget() { return true; }
	virtual void Draw(GWidgetGroupWithCanvas* pTarget);
	virtual void OnChar(char c);
	virtual void OnMouseMove(int dx, int dy);
	virtual void OnGetFocus() {}
	virtual void OnLoseFocus() {}

protected:
	virtual void Grab(int x, int y) = 0;
	virtual void Release() = 0;
};






// The base class of all widgets that are composed of other widgets
class GWidgetGroup : public GWidget
{
friend class GWidget;
friend class GWidgetAtomic;
protected:
	GPointerArray* m_pWidgets;
	bool m_dirty;

public:
	GWidgetGroup(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetGroup();

	virtual bool IsAtomicWidget() { return false; }
	virtual GWidgetStyle* GetStyle() { return m_pParent->GetStyle(); }
	virtual GWidgetAtomic* FindAtomicWidget(int x, int y);
	virtual void OnDestroyWidget(GWidget* pWidget);
	int GetChildWidgetCount();
	GWidget* GetChildWidget(int n);

	virtual void OnPushTextButton(GWidgetTextButton* pButton)
	{
		if(m_pParent)
			m_pParent->OnReleaseTextButton(pButton);
	}

	virtual void OnReleaseTextButton(GWidgetTextButton* pButton)
	{
		if(m_pParent)
			m_pParent->OnReleaseTextButton(pButton);
	}

	virtual void OnPushVCRButton(GWidgetVCRButton* pButton)
	{
		if(m_pParent)
			m_pParent->OnPushVCRButton(pButton);
	}

	virtual void OnHorizScroll(GWidgetHorizScrollBar* pScrollBar)
	{
		if(m_pParent)
			m_pParent->OnHorizScroll(pScrollBar);
	}

	virtual void OnVertScroll(GWidgetVertScrollBar* pScrollBar)
	{
		if(m_pParent)
			m_pParent->OnVertScroll(pScrollBar);
	}

	virtual void OnClickTextLabel(GWidgetTextLabel* pLabel)
	{
		if(m_pParent)
			m_pParent->OnClickTextLabel(pLabel);
	}

	virtual void OnSelectFilename(GWidgetFileSystemBrowser* pBrowser, const char* szFilename)
	{
		if(m_pParent)
			m_pParent->OnSelectFilename(pBrowser, szFilename);
	}

	virtual void OnChangeListSelection(GWidgetListBox* pListBox)
	{
		if(m_pParent)
			m_pParent->OnChangeListSelection(pListBox);
	}

	virtual void OnTextBoxTextChanged(GWidgetTextBox* pTextBox)
	{
		if(m_pParent)
			m_pParent->OnTextBoxTextChanged(pTextBox);
	}

	virtual void OnTextBoxPressEnter(GWidgetTextBox* pTextBox)
	{
		if(m_pParent)
			m_pParent->OnTextBoxPressEnter(pTextBox);
	}

	virtual void OnChar(char c)
	{
		if(m_pParent)
			m_pParent->OnChar(c);
	}

	virtual void OnClickTab(GWidgetSliderTab* pTab)
	{
		if(m_pParent)
			m_pParent->OnClickTab(pTab);
	}

	virtual void OnSlideTab(GWidgetSliderTab* pTab, int dx, int dy)
	{
		if(m_pParent)
			m_pParent->OnSlideTab(pTab, dx, dy);
	}

protected:
	void AddWidget(GWidget* pWidget);
};







class GWidgetGroupWithCanvas : public GWidgetGroup
{
protected:
	GImage m_image;

public:
	GWidgetGroupWithCanvas(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetGroupWithCanvas();

	virtual GImage* GetImage(GRect* pOutRect);
	virtual void Draw(GWidgetGroupWithCanvas* pTarget);

protected:
	virtual void Update() = 0;
};






// A form or dialog
class GWidgetDialog : public GWidgetGroupWithCanvas
{
protected:
	GWidgetAtomic* m_pGrabbedWidget;
	GWidgetAtomic* m_pFocusWidget;
	GColor m_cBackground;
	int m_prevMouseX;
	int m_prevMouseY;

public:
	GWidgetDialog(int w, int h, GColor cBackground);
	virtual ~GWidgetDialog();

	virtual WidgetType GetType() { return Dialog; }
	virtual GWidgetStyle* GetStyle() { return m_pStyle; }
	GWidgetAtomic* GetGrabbedWidget() { return m_pGrabbedWidget; }
	void GrabWidget(GWidgetAtomic* pWidget, int mouseX, int mouseY);
	void ReleaseWidget();
	virtual void OnDestroyWidget(GWidget* pWidget);
	void HandleChar(char c);
	bool HandleMousePos(int x, int y);
	virtual void Update();
};




// A button with text on it
class GWidgetTextButton : public GWidgetAtomic
{
protected:
	GImage m_image;
	GString m_text;
	bool m_pressed;
	bool m_dirty;

public:
	GWidgetTextButton(GWidgetGroup* pParent, int x, int y, int w, int h, GString* pText);
	virtual ~GWidgetTextButton();

	virtual WidgetType GetType() { return TextButton; }
	virtual GImage* GetImage(GRect* pOutRect);
	void SetSize(int w, int h);
	void SetText(GString* pText);
	void SetText(const char* szText);
	bool IsPressed() { return m_pressed; }

protected:
	void Update();
	virtual void Grab(int x, int y);
	virtual void Release();
};





// A text label
class GWidgetTextLabel : public GWidgetAtomic
{
protected:
	GImage m_image;
	GString m_text;
	bool m_alignLeft;
	bool m_bBright;
	bool m_dirty;
	bool m_bOpaqueBackground;

public:
	GWidgetTextLabel(GWidgetGroup* pParent, int x, int y, int w, int h, GString* pText, bool bBright);
	virtual ~GWidgetTextLabel();

	virtual WidgetType GetType() { return TextLabel; }
	virtual GImage* GetImage(GRect* pOutRect);
	void SetSize(int w, int h);
	GString* GetText() { return &m_text; }
	void SetText(GString* pText);
	void SetText(const char* szText);
	void SetOpaqueBackground() { m_bOpaqueBackground = true; }
	void SetAlignLeft(bool bAlignLeft) { m_alignLeft = bAlignLeft; m_dirty = true; }

protected:
	void Update();
	virtual void Grab(int x, int y);
	virtual void Release();
};




// A button with a common icon on it
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
	GImage m_image;
	VCR_Type m_eType;
	bool m_pressed;
	bool m_dirty;

public:
	GWidgetVCRButton(GWidgetGroup* pParent, int x, int y, int w, int h, VCR_Type eType);
	virtual ~GWidgetVCRButton();

	virtual WidgetType GetType() { return VCRButton; }
	virtual GImage* GetImage(GRect* pOutRect);
	void SetSize(int w, int h);
	void SetType(VCR_Type eType);
	bool IsPressed() { return m_pressed; }

protected:
	void Update();
	virtual void Grab(int x, int y);
	virtual void Release();
	void DrawIcon(int nHorizOfs, int nVertOfs);
};






class GWidgetProgressBar : public GWidgetAtomic
{
protected:
	GImage m_image;
	float m_fProgress;
	bool m_dirty;

public:
	GWidgetProgressBar(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetProgressBar();

	virtual WidgetType GetType() { return ProgressBar; }
	virtual GImage* GetImage(GRect* pOutRect);
	void SetSize(int w, int h);
	void SetProgress(float fProgress);
	float GetProgress() { return m_fProgress; }

protected:
	void Update();
	virtual void Grab(int x, int y);
	virtual void Release();
};






class GWidgetCheckBox : public GWidgetAtomic
{
protected:
	GImage m_image;
	bool m_checked;
	bool m_dirty;

public:
	GWidgetCheckBox(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetCheckBox();

	virtual WidgetType GetType() { return CheckBox; }
	virtual GImage* GetImage(GRect* pOutRect);
	void SetSize(int w, int h);
	void SetChecked(bool checked);
	bool IsChecked() { return m_checked; }

protected:
	void Update();
	virtual void Grab(int x, int y);
	virtual void Release();
};







class GWidgetSliderTab : public GWidgetAtomic
{
protected:
	GImage m_image;
	bool m_vertical;
	bool m_impressed;
	bool m_dirty;

public:
	GWidgetSliderTab(GWidgetGroup* pParent, int x, int y, int w, int h, bool vertical, bool impressed);
	virtual ~GWidgetSliderTab();

	virtual WidgetType GetType() { return SliderTab; }
	virtual GImage* GetImage(GRect* pOutRect);
	void SetSize(int w, int h);

	// for internal implementation
	virtual void OnMouseMove(int dx, int dy);

protected:
	void Update();
	virtual void Grab(int x, int y);
	virtual void Release();
};









class GWidgetHorizScrollBar : public GWidgetGroupWithCanvas
{
protected:
	int m_nViewSize;
	int m_nModelSize;
	int m_nPos;
	GWidgetVCRButton* m_pLeftButton;
	GWidgetVCRButton* m_pRightButton;
	GWidgetSliderTab* m_pLeftTab;
	GWidgetSliderTab* m_pTab;
	GWidgetSliderTab* m_pRightTab;

public:
	GWidgetHorizScrollBar(GWidgetGroup* pParent, int x, int y, int w, int h, int nViewSize, int nModelSize);
	virtual ~GWidgetHorizScrollBar();

	virtual WidgetType GetType() { return HScrollBar; }
	void SetViewSize(int n) { m_nViewSize = n; m_dirty = true; }
	void SetModelSize(int n) { m_nModelSize = n; m_dirty = true; }
	int GetPos() { return m_nPos; }
	void SetPos(int n) { m_nPos = n; m_dirty = true; }
	void SetSize(int w, int h);

	// for internal implementation
	virtual void OnPushVCRButton(GWidgetVCRButton* pButton);
	virtual void OnSlideTab(GWidgetSliderTab* pTab, int dx, int dy);
	virtual void OnClickTab(GWidgetSliderTab* pTab);

protected:
	virtual void Update();
	int GetButtonWidth();
};






class GWidgetVertScrollBar : public GWidgetGroupWithCanvas
{
protected:
	int m_nViewSize;
	int m_nModelSize;
	int m_nPos;
	GWidgetVCRButton* m_pUpButton;
	GWidgetVCRButton* m_pDownButton;
	GWidgetSliderTab* m_pAboveTab;
	GWidgetSliderTab* m_pTab;
	GWidgetSliderTab* m_pBelowTab;

public:
	GWidgetVertScrollBar(GWidgetGroup* pParent, int x, int y, int w, int h, int nViewSize, int nModelSize);
	virtual ~GWidgetVertScrollBar();

	virtual WidgetType GetType() { return VScrollBar; }
	void SetViewSize(int n) { m_nViewSize = n; m_dirty = true; }
	void SetModelSize(int n) { m_nModelSize = n; m_dirty = true; }
	int GetPos() { return m_nPos; }
	void SetPos(int n) { m_nPos = n; m_dirty = true; }
	void SetSize(int w, int h);

	// for internal implementation
	virtual void OnPushVCRButton(GWidgetVCRButton* pButton);
	virtual void OnSlideTab(GWidgetSliderTab* pTab, int dx, int dy);
	virtual void OnClickTab(GWidgetSliderTab* pTab);

protected:
	virtual void Update();
	int GetButtonHeight();
};







class GWidgetTextBox : public GWidgetAtomic
{
protected:
	GImage m_image;
	GString m_text;
	bool m_dirty;
	bool m_bGotFocus;
	bool m_bPassword;

public:
	GWidgetTextBox(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetTextBox();

	virtual WidgetType GetType() { return TextBox; }
	virtual GImage* GetImage(GRect* pOutRect);
	GString* GetText() { return &m_text; }
	void SetText(const char* szText);
	virtual void OnChar(char c);
	void SetPassword() { m_bPassword = true; }

protected:
	void Update();
	virtual void Grab(int x, int y);
	virtual void Release();
	virtual void OnGetFocus();
	virtual void OnLoseFocus();
};








class GWidgetListBoxItem : public GWidgetAtomic
{
protected:
	GString* m_sText;
	int m_nIndex;

public:
	GWidgetListBoxItem(GWidgetListBox* pParent, const wchar_t* wszText);
	virtual ~GWidgetListBoxItem();

	virtual WidgetType GetType() { return ListBoxItem; }
	virtual GImage* GetImage(GRect* pOutRect) { return NULL; }
	virtual void Draw(GWidgetGroupWithCanvas* pTarget);
	GString* GetText() { return m_sText; }

protected:
	virtual void Grab(int x, int y);

	virtual void Release()
	{
	}
};








class GWidgetListBox : public GWidgetGroupWithCanvas
{
friend class GWidgetListBoxItem;
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
	int m_nSelectedIndex;
	int m_nScrollPos;
	BaseColor m_eBaseColor;

public:
	// Takes ownership of pItems
	GWidgetListBox(GWidgetGroup* pParent, int x, int y, int w, int h);
	virtual ~GWidgetListBox();

	virtual WidgetType GetType() { return ListBox; }
	void SetSize(int w, int h);
	void SetBaseColor(BaseColor eBaseColor) { m_eBaseColor = eBaseColor; m_dirty = true; }
	int GetSelection() { return m_nSelectedIndex; }
	void SetSelection(int n);
	int GetScrollPos() { return m_nScrollPos; }
	void SetScrollPos(int n);
	int GetSize();
	GWidgetListBoxItem* GetItem(int n);
	void Clear();

protected:
	virtual void Update();
	void OnGrabItem(int nIndex);
	void SetItemRect(GRect* pRect, int nIndex);
};





class GWidgetGrid : public GWidgetGroupWithCanvas
{
protected:
	GPointerArray* m_pRows;
	int m_nColumns;
	int m_nRowHeight;
	GWidget** m_pColumnHeaders;
	int* m_nColumnWidths;
	GWidgetVertScrollBar* m_pVertScrollBar;
	GWidgetHorizScrollBar* m_pHorizScrollBar;

public:
	// Takes ownership of pItems
	GWidgetGrid(GWidgetGroup* pParent, GPointerArray* pItems, int nColumns, int x, int y, int w, int h);
	virtual ~GWidgetGrid();

	virtual WidgetType GetType() { return Grid; }
	virtual GWidgetAtomic* FindAtomicWidget(int x, int y);
	void SetSize(int w, int h);
	int GetRowHeight() { return m_nRowHeight; }
	void SetRowHeight(int n) { m_nRowHeight = n; m_dirty = true; }
	int GetHScrollPos() { return m_pHorizScrollBar->GetPos(); }
	int GetVScrollPos() { return m_pVertScrollBar->GetPos(); }
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

	// Deletes all the rows and all the widgets in them
	void FlushItems();

	// for internal implementation
	virtual void OnVertScroll(GWidgetVertScrollBar* pScrollBar);

	// for internal implementation
	virtual void OnHorizScroll(GWidgetHorizScrollBar* pScrollBar);

protected:
	virtual void Update();
};





class GWidgetFileSystemBrowser : public GWidgetGroup
{
protected:
	GWidgetTextLabel* m_pPath;
	GWidgetGrid* m_pFiles;
	GPointerArray* m_pListItems;
	char* m_szExtension;
	char m_szPath[256];

public:
	// szExtension should be NULL if you want to allow all extensions
	GWidgetFileSystemBrowser(GWidgetGroup* pParent, int x, int y, int w, int h, const char* szExtension);
	virtual ~GWidgetFileSystemBrowser();

	virtual WidgetType GetType() { return FileSystemBrowser; }
	virtual GImage* GetImage(GRect* pOutRect) { return NULL; }
	virtual void Draw(GWidgetGroupWithCanvas* pTarget);

	// for internal implementation
	virtual void OnClickTextLabel(GWidgetTextLabel* pLabel);

protected:
	void ReloadFileList();
	void AddFilename(bool bDir, const char* szFilename);
};




#endif // __GWIDGETS_H__
