/*
	Copyright (C) 1999, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GFile.h"
#include "GDDraw.h"

GLevel::GLevel(GWindow *pScreen, char* szFilename)
{
	m_pMap = NULL;
   Blocks = NULL;
   m_pScreen = pScreen;

	m_nScreenWid = pScreen->GetWidth();
	m_nScreenHgt = pScreen->GetHeight();
   Load(szFilename);
}

GLevel::GLevel(GWindow *pScreen, GBlock* pBlocks, int Width, int Height)
{
	m_pMap = NULL;
   Blocks = NULL;
   m_pScreen = pScreen;

	m_nScreenWid = pScreen->GetWidth();
	m_nScreenHgt = pScreen->GetHeight();
   Init(pBlocks, Width, Height);
}

GLevel::~GLevel()
{
	delete [] m_pMap;
}

void GLevel::SetBlocks(GBlock* pBlocks)
{
	Blocks = pBlocks;
	m_PosX = 0;
	m_PosY = 0;
	m_BlockWidth = Blocks->m_BlockWidth;
	m_BlockHeight = Blocks->m_BlockHeight;
	m_nScreenBlockWid = m_nScreenWid / m_BlockWidth;
	m_nScreenBlockHgt = m_nScreenHgt / m_BlockHeight;
	m_PixelWidth = m_Width * m_BlockWidth;
	m_PixelHeight = m_Height * m_BlockHeight;
	m_nMaxXPos = m_PixelWidth - m_nScreenWid;
	m_nMaxYPos = m_PixelHeight - m_nScreenHgt;
}

void GLevel::Init(GBlock* pBlocks, int Width, int Height)
{
	m_Width = Width;
	m_Height = Height;

	m_nSize = m_Width * m_Height;
	m_pMap = new short[m_nSize];
	if(!m_pMap)
		MessageBox(m_pScreen->GetHWnd(), "Failed to allocate memory", "Error", MB_OK);

	SetBlocks(pBlocks);

	int i;
	for(i = 0; i < m_nSize; i++)
		m_pMap[i] = 0;
}

BOOL GLevel::Load(const char *szFilename)
{
	if(szFilename == NULL) return FALSE;
	FILE *fp;

	fp = fopen(szFilename, "rb");
	if(!fp)
	{
		GAssert(false, "Couldn't find level file");
		return FALSE;
	}

	fread(&m_Width, sizeof(int), 1, fp);
	fread(&m_Height, sizeof(int), 1, fp);
	int nBGCol;
	int nBlockWid;
	int nBlockHgt;
	fread(&nBlockWid, sizeof(int), 1, fp);
	fread(&nBlockHgt, sizeof(int), 1, fp);
	fread(&nBGCol, sizeof(int), 1, fp);
	fread(&m_szBlocksFilename, 128, 1, fp);

	m_nSize = m_Width * m_Height;
	m_pMap = new short[m_nSize];

	fread(m_pMap, sizeof(short), m_nSize, fp);
	fclose(fp);

	if(!GFile::DoesFileExist(m_szBlocksFilename))
	{
		if(GFile::DoesFileExist("blocks.bmp"))
			strcpy(m_szBlocksFilename, "blocks.bmp");
		else
			GAssert(false, "Couldn't find blocks file");
	}
	GBlock* pBlocks = new GBlock(m_pScreen, m_szBlocksFilename, nBGCol, nBlockWid, nBlockHgt);
	SetBlocks(pBlocks);
	return TRUE;
}

BOOL GLevel::Save(const char *szFilename)
{
	FILE *fp;

	fp = fopen(szFilename, "wb");
	if(fp == NULL) return FALSE;

	fwrite(&m_Width, sizeof(int), 1, fp);
	fwrite(&m_Height, sizeof(int), 1, fp);
	int nBlockWid = Blocks->m_BlockWidth;
	fwrite(&nBlockWid, sizeof(int), 1, fp);
	int nBlockHgt = Blocks->m_BlockHeight;
	fwrite(&nBlockHgt, sizeof(int), 1, fp);
	int nBGCol = Blocks->m_ColorKey;
	fwrite(&nBGCol, sizeof(int), 1, fp);
	fwrite(&m_szBlocksFilename, 128, 1, fp);
	m_nSize = m_Width * m_Height;
	fwrite(m_pMap, sizeof(short), m_nSize, fp);
	fclose(fp);
	return TRUE;
}

void GLevel::SetBlocksFilename(char* szFilename)
{
	strcpy(m_szBlocksFilename, szFilename);
}

char* GLevel::GetBlocksFilename()
{
	return(m_szBlocksFilename);
}

void GLevel::Clear()
{
	for(int i=0; i<m_nSize; i++)
		m_pMap[i] = 0;
}

void GLevel::Fill(int BlockNum)
{
	for(int i=0; i<m_nSize; i++)
		m_pMap[i] = BlockNum;
}

void GLevel::SetPos(int x, int y)
{
	HorizScroll(x - m_PosX, true);
	VertScroll(y - m_PosY, true);
}

// Positive values scroll down, negative scroll up
void GLevel::VertScroll(int nOffset, bool bWrap)
{
   if(bWrap)
   {
      if(nOffset > 0)
      {
	      if (m_PosY + nOffset < (m_Height * m_BlockHeight)) m_PosY += nOffset;
	      else m_PosY = nOffset - (m_Height * m_BlockHeight) + m_PosY;
      }
      else
      {
	      if(m_PosY + nOffset >= 0) m_PosY += nOffset;
	      else m_PosY = (m_Height * m_BlockHeight) + nOffset + m_PosY;
      }
   }
   else
   {
      m_PosY += nOffset;
	   if(nOffset > 0)
      {
         if(m_PosY > (m_Height - m_nScreenBlockHgt) * m_BlockHeight)
		      m_PosY = (m_Height - m_nScreenBlockHgt) * m_BlockHeight;
      }
      else
      {
	      if(m_PosY < 0)
            m_PosY = 0;
      }
   }
}

// Positive values scroll right, negative scroll left
void GLevel::HorizScroll(int nOffset, bool bWrap)
{
   if(bWrap)
   {
      if(nOffset > 0)
      {
	      if (m_PosX + nOffset < (m_Width * m_BlockWidth)) m_PosX += nOffset;
	      else m_PosX = nOffset - (m_Width * m_BlockWidth) + m_PosX;
      }
      else
      {
	      if (m_PosX + nOffset >= 0) m_PosX += nOffset;
	      else m_PosX = (m_Width * m_BlockWidth) + nOffset + m_PosX;
      }
   }
   else
   {
      m_PosX += nOffset;
      if(nOffset > 0)
      {
         if(m_PosX > (m_Width - m_nScreenBlockWid) * m_BlockWidth)
		      m_PosX = (m_Width - m_nScreenBlockWid) * m_BlockWidth;
      }
      else
      {
   	   if(m_PosX < 0)
            m_PosX = 0;
      }
   }
}

void GLevel::ScreenBlockSize(int Width, int Height)
{
	m_nScreenBlockWid = Width;
	m_nScreenBlockHgt = Height;
}

short GLevel::GetBlock(int MapX, int MapY)
{
   if(MapX < 0 || MapX >= m_Width || MapY < 0 || MapY > m_Height)
      return(0);
   return m_pMap[MapX + (MapY * m_Width)];
}

void GLevel::SetBlock(int MapX, int MapY, short Block)
{
	m_pMap[MapX + (MapY * m_Width)] = Block;
}

void GLevel::LoadBlocks(GBlock *pBlocks)
{
	Blocks = pBlocks;
}

void GLevel::BltBlock(GDispRect* lpDDS, int xdest, int ydest, int w, int h, int xoff, int yoff)
{
	HRESULT rval;
	RECT src;
	short block_num;
	int x1, y1;
	int mapx, mapy;

	mapx = ((m_PosX + xdest) % m_PixelWidth) / m_BlockWidth;
	mapy = ((m_PosY + ydest) % m_PixelHeight) / m_BlockHeight;

	block_num = m_pMap[(mapy * m_Width) + mapx];

	if(block_num == 0) return;

	int TILE_SW = Blocks->m_PixelWidth / m_BlockWidth;

	x1 = block_num % TILE_SW;
	x1 = (x1 * m_BlockWidth) + xoff;

	y1 = block_num / TILE_SW;
	y1 = (y1 * m_BlockHeight) + yoff;

	src.top = y1;
	src.left = x1;
	src.bottom = y1+h;
	src.right = x1+w;

	rval = lpDDS->m_lpDDS->BltFast(xdest, ydest, Blocks->m_lpDDS, &src, DDBLTFAST_WAIT);
	if(rval == DDERR_SURFACELOST) Blocks->Restore();
}

void GLevel::BltBlockTrans(GDispRect* lpDDS, int xdest, int ydest, int w, int h, int xoff, int yoff)
{
	HRESULT rval;
	RECT src;
	short block_num;
	int x1, y1;
	int mapx, mapy;

	mapx = ((m_PosX + xdest) % m_PixelWidth) / m_BlockWidth;
	mapy = ((m_PosY + ydest) % m_PixelHeight) / m_BlockHeight;

	block_num = m_pMap[(mapy * m_Width) + mapx];

	if(block_num == 0) return;

	int TILE_SW = Blocks->m_PixelWidth / m_BlockWidth;

	x1 = block_num % TILE_SW;
	x1 = (x1 * m_BlockWidth) + xoff;

	y1 = block_num / TILE_SW;
	y1 = (y1 * m_BlockHeight) + yoff;

	src.top = y1;
	src.left = x1;
	src.bottom = y1+h;
	src.right = x1+w;

	rval = lpDDS->m_lpDDS->BltFast(xdest, ydest, Blocks->m_lpDDS, &src, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	if(rval == DDERR_SURFACELOST) Blocks->Restore();
}

void GLevel::Draw(GDispRect* lpDDS)
{
	int i,j;
	int xoffset, yoffset;
	int xcoord = 0, ycoord = 0;

	xoffset = m_PosX % m_BlockWidth;
	yoffset = m_PosY % m_BlockHeight;

	// FIRST ROW
	BltBlock(lpDDS, 0, 0, m_BlockWidth-xoffset, m_BlockWidth-yoffset, xoffset, yoffset);

	for(i = 0; i < m_nScreenBlockWid - 1; i++)
   {
		xcoord += m_BlockWidth;
		BltBlock(lpDDS, xcoord-xoffset, 0, m_BlockWidth, m_BlockHeight-yoffset, 0, yoffset);
   }

	xcoord += m_BlockWidth;
	BltBlock(lpDDS, xcoord-xoffset, 0, xoffset, m_BlockHeight-yoffset, 0, yoffset);

	// NEXT X NUMBER OF ROWS
	for(j=0; j<m_nScreenBlockHgt-1; j++)
	{
		xcoord = m_BlockWidth;
		ycoord += m_BlockHeight;
		BltBlock(lpDDS, 0, ycoord-yoffset, m_BlockWidth-xoffset, m_BlockHeight, xoffset, 0);

		for(i=0; i<m_nScreenBlockWid-1; i++)
		{
			BltBlock(lpDDS, xcoord-xoffset, ycoord-yoffset, m_BlockWidth, m_BlockHeight, 0, 0);
			xcoord += m_BlockWidth;
		}

		BltBlock(lpDDS, xcoord-xoffset, ycoord-yoffset, xoffset, m_BlockHeight, 0, 0);
	}

	// LAST ROW
	xcoord = 0;
	ycoord += m_BlockHeight;
	BltBlock(lpDDS, 0, ycoord-yoffset, m_BlockWidth-xoffset, yoffset, xoffset, 0);

	for(i=0; i<m_nScreenBlockWid-1; i++)
	{
		xcoord += m_BlockWidth;
		BltBlock(lpDDS, xcoord-xoffset, ycoord-yoffset, m_BlockWidth, yoffset, 0, 0);
	}

	xcoord += m_BlockWidth;
	BltBlock(lpDDS, xcoord-xoffset, ycoord-yoffset, xoffset, yoffset, 0, 0);
}

void GLevel::DrawTrans(GDispRect* lpDDS)
{
	int i,j;
	int xoffset, yoffset;
	int xcoord = 0, ycoord = 0;

	xoffset = m_PosX % m_BlockWidth;
	yoffset = m_PosY % m_BlockHeight;

	// FIRST ROW
	BltBlockTrans(lpDDS, 0, 0, m_BlockWidth-xoffset, m_BlockWidth-yoffset, xoffset, yoffset);

	for(i=0; i<m_nScreenBlockWid-1; i++)
  {
		xcoord += m_BlockWidth;
		BltBlockTrans(lpDDS, xcoord-xoffset, 0, m_BlockWidth, m_BlockHeight-yoffset, 0, yoffset);
  }

	xcoord += m_BlockWidth;
	BltBlockTrans(lpDDS, xcoord-xoffset, 0, xoffset, m_BlockHeight-yoffset, 0, yoffset);

	// NEXT X NUMBER OF ROWS
	for(j=0; j<m_nScreenBlockHgt-1; j++)
	{
		xcoord = m_BlockWidth;
		ycoord += m_BlockHeight;
		BltBlockTrans(lpDDS, 0, ycoord-yoffset, m_BlockWidth-xoffset, m_BlockHeight, xoffset, 0);

		for(i=0; i<m_nScreenBlockWid-1; i++)
		{
			BltBlockTrans(lpDDS, xcoord-xoffset, ycoord-yoffset, m_BlockWidth, m_BlockHeight, 0, 0);
			xcoord += m_BlockWidth;
		}

		BltBlockTrans(lpDDS, xcoord-xoffset, ycoord-yoffset, xoffset, m_BlockHeight, 0, 0);
	}

	// LAST ROW
	xcoord = 0;
	ycoord += m_BlockHeight;
	BltBlockTrans(lpDDS, 0, ycoord-yoffset, m_BlockWidth-xoffset, yoffset, xoffset, 0);

	for(i=0; i<m_nScreenBlockWid-1; i++)
	{
		xcoord += m_BlockWidth;
		BltBlockTrans(lpDDS, xcoord-xoffset, ycoord-yoffset, m_BlockWidth, yoffset, 0, 0);
	}

	xcoord += m_BlockWidth;
	BltBlockTrans(lpDDS, xcoord-xoffset, ycoord-yoffset, xoffset, yoffset, 0, 0);
}

void GLevel::DrawClipped(GDispRect* lpDDS, LPRECT ClipRect)
{
	int xoffset = (m_PosX + ClipRect->left) % m_BlockWidth;
	int yoffset = (m_PosY + ClipRect->top) % m_BlockWidth;
	int xcoord = 0;
	int ycoord = 0;
	int minwidth = (m_BlockWidth - xoffset) < (ClipRect->right - ClipRect->left)
									? (m_BlockWidth - xoffset) : (ClipRect->right - ClipRect->left);
	int minheight = (m_BlockHeight - yoffset) < (ClipRect->bottom - ClipRect->top)
									 ? (m_BlockHeight - yoffset) : (ClipRect->bottom - ClipRect->top);

	// FIRST ROW
	BltBlock(lpDDS, ClipRect->left, ClipRect->top, minwidth, minheight, xoffset, yoffset);
	xcoord = ClipRect->left + minwidth;

	while ((xcoord + m_BlockWidth) < ClipRect->right)
	{
		BltBlock(lpDDS, xcoord, ClipRect->top, m_BlockWidth, minheight, 0, yoffset);
		xcoord += m_BlockWidth;
	}

	BltBlock(lpDDS, xcoord, ClipRect->top, ClipRect->right - xcoord,	minheight, 0, yoffset);

	// NEXT X NUMBER OF ROWS
	ycoord = ClipRect->top + minheight;

	while((ycoord + m_BlockHeight) < ClipRect->bottom)
	{
		BltBlock(lpDDS, ClipRect->left, ycoord, minwidth, m_BlockHeight,	xoffset, 0);
		xcoord = ClipRect->left + minwidth;

		while((xcoord + m_BlockWidth) < ClipRect->right)
		{
			BltBlock(lpDDS, xcoord, ycoord, m_BlockWidth, m_BlockHeight, 0, 0);
			xcoord += m_BlockWidth;
		}

		BltBlock(lpDDS, xcoord, ycoord, ClipRect->right - xcoord,	m_BlockHeight, 0, 0);
		ycoord += m_BlockHeight;
	}

	// LAST ROW
	BltBlock(lpDDS, ClipRect->left, ycoord, minwidth, ClipRect->bottom - ycoord, xoffset, 0);
	xcoord = ClipRect->left + minwidth;

	while((xcoord + m_BlockWidth) < ClipRect->right)
	{
		BltBlock(lpDDS, xcoord, ycoord, m_BlockWidth, ClipRect->bottom - ycoord,	0, 0);
		xcoord += m_BlockWidth;
	}

	BltBlock(lpDDS, xcoord, ycoord, ClipRect->right - xcoord,	ClipRect->bottom - ycoord, 0, 0);
}
/*
GPoint GLevel::GetBlockCoords(int x, int y)
{
   GPoint pTmp;
   pTmp.x = x / m_BlockWidth;
   pTmp.y = y / m_BlockHeight;
   return(pTmp);
}
   
GPoint GLevel::GetScreenCoords(int x, int y)
{
   GPoint pTmp;
   pTmp.x = x * m_BlockWidth;
   pTmp.y = y * m_BlockHeight;
   return(pTmp);
}
*/
short GLevel::GetBiggestBlock(int x1, int y1, int x2, int y2)
{
/*   if(x2 < x1)
   {
      int nTmp = x2;
      x2 = x1;
      x1 = nTmp;
   }
   if(y2 < y1)
   {
      int nTmp = y2;
      y2 = y1;
      y1 = nTmp;
   }   

   short nRetVal = 0;
   if(y1 == y2)
   {
      int n;
      GPoint gTmp;
      gTmp = GetBlockCoords(x1, y1);
      while(GetScreenCoords(gTmp.x, gTmp.y).x < x2)
      {
         n = GetBlock(gTmp.x, gTmp.y);
         if(n > nRetVal)
            nRetVal = n;
         gTmp.x++;
      }
   }
   else
   {
      int n;
      GPoint gTmp;
      gTmp = GetBlockCoords(x1, y1);
      while(GetScreenCoords(gTmp.x, gTmp.y).y < y2)
      {
         n = GetBlock(gTmp.x, gTmp.y);
         if(n > nRetVal)
            nRetVal = n;
         gTmp.y++;
      }
   }
   return(nRetVal);*/ return 0;
}

