/*
	Copyright (C) 1999, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GDDRAW_H__
#define __GDDRAW_H__

#include <ddraw.h>
#include <vfw.h>
#include "GLList.h"
#include "GMacros.h"

class GSmallArray;
class GImage;
class GWindow;
class GMidi;

extern "C"
{
	// Direct Draw Standard Functions
	extern IDirectDrawPalette* DDLoadPalette(IDirectDraw2 *pdd, LPCSTR szBitmap);
	extern IDirectDrawSurface* DDLoadBitmap(IDirectDraw2 *pdd, LPCSTR szBitmap, int dx, int dy);
	extern IDirectDrawSurface* DDLoadSizeBitmap(IDirectDraw2 *pdd, LPCSTR szBitmap, int *dx, int *dy);
	extern HRESULT             DDReLoadBitmap(IDirectDrawSurface *pdds, LPCSTR szBitmap);
	extern HRESULT             DDCopyBitmap(IDirectDrawSurface *pdds, HBITMAP hbm, int x, int y, int dx, int dy);
	extern DWORD               DDColorMatch(IDirectDrawSurface *pdds, COLORREF rgb);
	extern HRESULT             DDSetColorKey(IDirectDrawSurface *pdds, COLORREF rgb);
}

void ShowDDError(int nError);

// This is a rect to which graphics will be drawn
class GDispRect
{
public:
	int m_PixelWidth;
	int m_PixelHeight;
	char* m_pFilename;
	RECT SrcRect;
	RECT DestRect;
	DDSURFACEDESC m_DDSD;
	LPDIRECTDRAWSURFACE m_lpDDS;
	int m_ColorKey;

protected:
	HDC m_DC;
	HFONT m_Font;

public:
	GDispRect();
	GDispRect(GWindow *pScreen, int Width, int Height);
	virtual ~GDispRect();

	bool Init(GWindow *pScreen, HBITMAP hBM);
	bool Init(GWindow *pScreen, int Width, int Height);
	void SetDest(int t, int l, int b, int r);
 	void SetSrc(int t, int l, int b, int r);
	void SetBackGroundCol(int col);
	void Restore(void);
	void Fill(DWORD FillColor);
	void SetFilename(const char* szFilename);

	void ChangeFont(const char* FontName, int Width, int Height, int Attributes = FW_NORMAL);
	void SetFont(void);
	void TextXY(int X, int Y, COLORREF Col, LPCTSTR pString);
	void PutPixel(int X, int Y, int Col);
	int  GetPixel(int X, int Y);
	void Box(int X1,int Y1,int X2,int Y2,int Col, bool bFill);
	void Line(int X1, int Y1, int X2, int Y2, int Col);
	void Circle(int X, int Y, int Radius, int Col);

	virtual HRESULT Draw(GDispRect* lpDDS);
	virtual HRESULT DrawFast(int X, int Y, GDispRect* lpDDS);
	virtual HRESULT DrawTrans(int X, int Y, GDispRect* lpDDS);
	virtual HRESULT DrawClipped(int X, int Y, GDispRect* lpDDS, LPRECT ClipRect);
	virtual HRESULT DrawWindowed(GWindow* pScreen, GDispRect* lpDDS);
	virtual HRESULT DrawScaled(int X, int Y, float Factor, GDispRect* lpDDS);
	virtual HRESULT DrawHFlip(int X, int Y, GDispRect* lpDDS);
	virtual HRESULT DrawVFlip(int X, int Y, GDispRect* lpDDS);
	void GetDC();
	void ReleaseDC();
	HRESULT Lock();
	HRESULT UnLock();

protected:
	void SetInitialValues(int nWidth, int nHeight);
	void Clip(int *DestX, int *DestY, RECT *SrcRect, RECT *DestRect);
	void PutPix(int X, int Y, int Col);
	void SafePutPix(int X, int Y, int Col);
	void VLine(int Y1, int Y2, int X, int Col);
	void HLine(int X1, int X2, int Y, int Col);
};

// This makes a window or full-screen for DirectDraw
class GWindow
{
private:
	int m_dwPixelWidth;
	int m_dwPixelHeight;
	LPDIRECTDRAW2 m_lpDD;
	GDispRect* m_lpDDSFront;
	GDispRect* m_lpDDSBack;
	LPDIRECTDRAWPALETTE m_lpDDPalette;
	LPDIRECTDRAWCLIPPER m_lpClipper;
	DWORD m_BPP;
	HWND m_hWnd;
	bool m_bFullScreen;
	bool m_bFlipped;

public:
	enum
	{
		COLORS_256 = 8,         // (not enough colors)
		COLORS_65536 = 16,      // (just right)
		COLORS_4294967296 = 32, // (too slow)
	};
   
	GWindow();
	GWindow(HWND hWnd, int nWidth, int nHeight, bool bFullScreen, DWORD bpp = COLORS_65536);
	virtual ~GWindow();

	HWND GetHWnd() { return m_hWnd; }
	HWND GetSafeHWnd();
	DWORD GetWidth() { return m_dwPixelWidth; }
	DWORD GetHeight() { return m_dwPixelHeight; }
	BOOL LoadBitmap(const char* szFilename);
	HRESULT Update();
//	void GetColor(int col, GColor* pCol);
//	void SetColor(int col, GColor* pCol);
	BOOL LoadPalette(const char* szFilename);
	void GetPalette(int Start, int Count, LPPALETTEENTRY lpPE);
	void SetPalette(int Start, int Count, LPPALETTEENTRY lpPE);
//	void Fade(GColor* pCol, int delay);
	LPDIRECTDRAW2 GetDD(void) { return m_lpDD; }
	GDispRect* GetDispRect(void) { return m_lpDDSBack; }
	int GetBPP(void) { return m_BPP; }

private:
//	void Fill(DWORD FillColor);
//	void FillPalette(GColor* pCol);
//	void GreyScale(void);
//  void FadeIn(int delay, LPPALETTEENTRY lpPE);
//	void FadeOut(int delay);
	LPDIRECTDRAWPALETTE GetPalette(void) { return m_lpDDPalette; }
	GDispRect* GetFront(void) { return m_lpDDSFront; }
	BOOL CreateFullScreen(HWND hWnd, DWORD nWidth, DWORD nHeight, DWORD BitsPerPix);
	BOOL CreateWindowed(HWND hWnd, int nWidth, int nHeight);
	void Restore();
};

// This is for displaying a Bitmap as background
class GBitmap : public GDispRect
{
public:
	int m_XOffset;
	int m_YOffset;

public:
	GBitmap();
	GBitmap(GWindow *pScreen, char *szFilename);
	virtual ~GBitmap();

	void Load(GWindow *pScreen, char *szFilename);
	void VScroll(int nOffset);
	void HScroll(int nOffset);
	void MoveTo(int XOffset, int YOffset);
	virtual HRESULT Draw(GDispRect* lpDDS);
};

// This holds the blocks that levels are made out of
class GBlock : public GDispRect
{
public:
	int m_BlockWidth;
	int m_BlockHeight;
	int m_nBlocksWide;
	int m_nBlocksHigh;

protected:
   int m_nBlockCount;

public:
	GBlock();
	GBlock(GWindow *pScreen, const char* szFilename, int nBGCol, int w, int h);
   
	// Returns the number of blocks in this set of blocks
	int GetBlockCount() { return m_nBlockCount; }

	// Sets a pixel in a specified block to a certain color
	void SetBlockPixel(int nXPos, int nYPos, int nBlock, int nCol);

	// Gets a pixel in the specified block
	int GetBlockPixel(int nXPos, int nYPos, int nBlock);

	// Draw the frame you want where you want
	void DrawBlock(int nXPos, int nYPos, int nBlock);

	// This is only slightly slower than DrawBlock(), but much better
	// because it won't try to draw off the screen
	void SafeDrawBlock(int nXPos, int nYPos, int nBlock);

	BOOL LoadBitmap(GWindow *pScreen, const char* szFilename, int nBGCol, int w, int h);
};

class GFont : public GBlock
{
protected:

public:
   GFont(GWindow* pWindow, const char* szFilename, int nBGCol, int nLetterWid, int LetterHgt);
   virtual ~GFont();
   
   void PrintString(int nXPos, int nYPos, const char* szText);
};

// A level
class GLevel
{
public:
	int m_PosX;
	int m_PosY;
	int m_BlockWidth;
	int m_BlockHeight;
	int m_Width;
	int m_Height;
	GBlock *Blocks;

private:
	int m_PixelWidth;
	int m_PixelHeight;
	int m_nScreenBlockWid;
	int m_nScreenBlockHgt;
	int m_nSize;
	int m_nScreenWid;
	int m_nScreenHgt;
	int m_nMaxXPos;
	int m_nMaxYPos;
	short *m_pMap;
	char m_szBlocksFilename[128];
	GWindow* m_pScreen;

public:
	GLevel(GWindow *pScreen, char* szFilename);
	GLevel(GWindow *pScreen, GBlock* pBlocks, int Width, int Height);
   ~GLevel();

	void SetBlocksFilename(char* szFilename);
	char* GetBlocksFilename();
	BOOL Load(const char *szFilename);
	BOOL Save(const char *szFilename);
	void Clear(void);
	void Fill(int BlockNum);
	void BltBlock(GDispRect* lpDDS, int xdest, int ydest, int w, int h, int xoff, int yoff);
	void BltBlockTrans(GDispRect* lpDDS, int xdest, int ydest, int w, int h, int xoff, int yoff);
	void Draw(GDispRect* lpDDS);
	void DrawTrans(GDispRect* lpDDS);
	void DrawClipped(GDispRect* lpDDS, LPRECT ClipRect);
	void SetPos(int x, int y);
	void ScreenBlockSize(int Width, int Height);
	short GetBlock(int MapX, int MapY);
	void SetBlock(int MapX, int MapY, short Block);
	void SetBlocks(GBlock* pBlocks);
	void LoadBlocks(GBlock *pBlocks);
	int GetScreenBlockWid() { return(m_nScreenBlockWid); }
	int GetScreenBlockHgt() { return(m_nScreenBlockHgt); }

   // Get the extreme-most positions for call to SetPos w/o wrapping
   int GetMinXPos() { return 0; }
   int GetMinYPos() { return 0; }
   int GetMaxXPos() { return m_nMaxXPos; }
   int GetMaxYPos() { return m_nMaxYPos; }
   
   // Get the size of the level in pixels 
   int GetPixWid()  { return m_PixelWidth; }
   int GetPixHgt()  { return m_PixelHeight; }
   
   // Pass in 2 points on screen (must make a horiz or vertical line)
   // and this will return the highest block intersected
   short GetBiggestBlock(int x1, int y1, int x2, int y2);
/*
   // Pass in block-position and get screen position of top-left corner
   GPoint GetBlockCoords(int x, int y);
   
   // Pass in block-position and get screen position of top-left corner   
   GPoint GetScreenCoords(int x, int y);
*/
private:
   void Init(GBlock* pBlocks, int Width, int Height);
   void HorizScroll(int nOffset, bool bWrap);
   void VertScroll(int nOffset, bool bWrap);
};

// Note: If you use this class, there must be a 256x256 24-bit bitmap called blank.bmp
// in your program folder.  (Why?  Beause it's the only way I could figure out how to
// make it work--if you can fix it and tell me how, I'd appreciate it.)
class GTile
{
friend class GSprite;
public:
	GTile();
	virtual ~GTile();

	bool LoadTiles(GWindow* pScreen, const char* szFilename, int nTileWidth, int nTileHeight);
	bool LoadTiles(GWindow* pScreen, GImage* pImage, int nTileWidth, int nTileHeight);
	int GetTileCount();
	void SetTile(int nTile);
	void Draw(int nX, int nY);
	void DrawScaled(int nX, int nY, float fScale);

protected:
	GDispRect** m_pArrDispRects;
	int m_nSelectedDispRect;
	int m_nDispRectCount;
	int m_nTileCount;
	int m_nTilesPerDispRect;
	int m_nHorizTilesPerDispRect;
	int m_nTileWidth;
	int m_nTileHeight;
	GWindow* m_pScreen;
};

class GSprite : public GBucket
{
public:
	int m_nXPos;
	int m_nYPos;
	int m_Frame;
	GTile* m_pTiles;

	GSprite();
	virtual ~GSprite();

	virtual void Init(GTile* pTiles, GWindow* pWindow);
	void SetPos(int nX, int nY) { m_nXPos = nX; m_nYPos = nY; }
	void SetFrame(int nFrame);
	int GetFrame() { return m_Frame; }
	void SetBackGroundCol(int nCol);

	void Draw();
	void DrawScaled(float fScale);
	virtual int Compare(GBucket* pBucket) { return -1; }
	virtual void Update() { GAssert(false, "You're supposed to override this method"); }
	bool DoesOverlap(GSprite* pSprite);

private:
	void SelectFrame();

};

class GGame
{
public:
	HINSTANCE m_hInst;
	int m_nCmdShow;
	HWND m_hWnd;
	GWindow* m_pWindow;
	GLevel* m_pLevel;
	GBitmap* m_pBackground;
	GMidi* m_pBGMusic;
	GSprite* m_pMainGuy;
	GSprite* m_pSprites;
	GSmallArray* m_pImages;

	GGame(HINSTANCE hInst, int nCmdShow, int nWidth, int nHeight, bool bFullScreen);
	virtual ~GGame();
};

#endif // __GDDRAW_H__
