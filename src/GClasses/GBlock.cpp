/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GFile.h"
#include "GDDraw.h"

typedef struct
{
	char maker;
	char version;
	char encoding;
	char bpp;
	short x, y;
	short width, height;
	short hres, vres;
	char ega[48];
	char reserved;
	char planes;
	short bpl;
	short type;
	char filler[58];
} PcxHeader;

GBlock::GBlock(GWindow *pScreen, const char* szFilename, int nBGCol, int w, int h)
{
	LoadBitmap(pScreen, szFilename, nBGCol, w, h);
}

GBlock::GBlock()
{
	DestRect.top = 0;
	DestRect.left = 0;
	DestRect.bottom = 0;
	DestRect.right = 0;
}

/*
int PcxLoad (char *fileName, Bitmap *bitmap,
	unsigned char *pal)
{
	FILE *input;
	PcxHeader header;
	int pitch, i, j;
	unsigned char *bits, run = 0, c;

	//Open PCX file
	if ((input = fopen(fileName, "rb")) == NULL)
		return FALSE;

	//Read the PCX Header
	fread(&header, sizeof(PcxHeader), 1, input);

	//Create bitmap
	if (!BitmapCreate(bitmap, header.width + 1,
		header.height + 1))
		return FALSE;

	//Lock the bitmap
	if (!BitmapLock(bitmap, &bits, &pitch))
		return FALSE;

	//Decode PCX
	for (j = 0; j < bitmap->height; j++)
	{
		for (i = 0; i < bitmap->width; i++)
		{
			if (run == 0)
			{
				c = fgetc(input);

				if ((c & 0xC0) == 0xC0)
				{
					run = c & 0x3F;
					c = fgetc(input);
				}
				else
					run = 1;
			}

			if (run > 0)
			{
				bits[(j * pitch) + i] = c;
				run--;
			}
		}
	}

	//Unlock bitmap
	BitmapUnlock(bitmap, bits);
	
	//Read palette data
	fseek(input, -768, SEEK_END);
	fread(pal, sizeof(char), 768, input);
	
	//Close the file
	fclose(input);

	//Return with success
	return TRUE;
}

int PcxSave (char *fileName, Bitmap *bitmap,
	unsigned char *pal)
{
	FILE *output;
	PcxHeader header;
	int pitch, i, j;
	unsigned char *bits, run = 0, c;
	
	//Open output file
	if ((output = fopen(fileName, "wb")) == NULL)
		return FALSE;

	//Fill in the header
	header.maker = 10;
	header.version = 5;
	header.encoding = 1;
	header.bpp = 8;
	header.x = 0;
	header.y = 0;
	header.width = bitmap->width - 1;
	header.height = bitmap->height - 1;
	header.hres = 0;
	header.vres = 0;
	header.reserved = 0;
	header.planes = 1;
	header.bpl = bitmap->width;
	header.type = 0;

	//Write header to file
	fwrite(&header, sizeof(PcxHeader), 1, output);

	//Lock the bitmap
	if (!BitmapLock(bitmap, &bits, &pitch))
		return FALSE;

	//Encode PCX
	for (j = 0; j < bitmap->height; j++)
	{
		run = 0;
		
		for (i = 0; i < bitmap->width; i++)
		{
			if (run == 0)
			{
				c = bits[(j * pitch) + i];
				run = 1;
			}
			else
			{
				if (bits[(j * pitch) + i] != c ||
					run == 63 || i == bitmap->width - 1)
				{
					if (run == 1 && c < 192)
						fputc(c, output);
					else
					{
						fputc(run + 192, output);
						fputc(c, output);
					}

					c = bits[(j * pitch) + i];
					run = 1;
				}
				else
					run++;
			}
		}
		
		if (run == 1 && c < 192)
			fputc(c, output);
		else
		{
			fputc(run + 192, output);
			fputc(c, output);
		}
	}

	//Unlock bitmap
	BitmapUnlock(bitmap, bits);

	//Write the palette
	fputc(12, output);
	fwrite(pal, sizeof(char), 768, output);
	
	//Close the file
	fclose(output);
	
	//Return with success
	return TRUE;
}
*/

/*
BOOL GBlock::SaveBitmap(const char* szFilename)
{
   DDSaveBitmap(m_lpDDs);
}
*/

BOOL GBlock::LoadBitmap(GWindow *pScreen, const char* szFilename, int nBGCol, int w, int h)
{
	if(szFilename == NULL)
		return FALSE;

	if(!GFile::DoesFileExist(szFilename))
		return FALSE;

	m_lpDDS = DDLoadSizeBitmap(pScreen->GetDD(), szFilename, &m_PixelWidth, &m_PixelHeight);
	if(m_lpDDS == NULL)
		return FALSE;

	SetFilename(szFilename);

	m_BlockWidth = w;
	m_BlockHeight = h;
	
	DestRect.top = 0;
	DestRect.left = 0;
	DestRect.bottom = m_PixelHeight;
	DestRect.right = m_PixelWidth;

	SrcRect.top = 0;
	SrcRect.left = 0;
	SrcRect.bottom = m_PixelHeight;
	SrcRect.right = m_PixelWidth;

	m_nBlocksWide = m_PixelWidth / w;
	m_nBlocksHigh = m_PixelHeight / h;
	m_nBlockCount = m_nBlocksWide * m_nBlocksHigh;

	SetBackGroundCol(nBGCol);

	return TRUE;
}

void GBlock::SafeDrawBlock(int nXPos, int nYPos, int nBlock)
{
GAssert(false, "Error, not implemented yet");
/*
	if(nXPos < 0 || nXPos >= (int)Screen->GetWidth() - m_BlockWidth)
		return;
	if(nYPos < 0 || nYPos >= (int)Screen->GetHeight() - m_BlockHeight)
		return;
	DrawBlock(nXPos, nYPos, nBlock);*/
}

void GBlock::DrawBlock(int nXPos, int nYPos, int nBlock)
{
GAssert(false, "Error, not implemented yet");
/*
	RECT rSrc;
	rSrc.top = (nBlock / m_nBlocksWide) * m_BlockHeight;
	rSrc.left = (nBlock % m_nBlocksWide) * m_BlockWidth;
	rSrc.bottom = rSrc.top + m_BlockHeight - 1;
	rSrc.right = rSrc.left + m_BlockWidth - 1;

	int rval = Screen->GetDispRect()->m_lpDDS->BltFast(nXPos, nYPos, m_lpDDS, &rSrc, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	if(rval == DDERR_SURFACELOST)
		Restore();*/
}

void GBlock::SetBlockPixel(int nXPos, int nYPos, int nBlock, int nCol)
{
	int y = (nBlock / m_nBlocksWide) * m_BlockHeight + nYPos;
	int x = (nBlock % m_nBlocksWide) * m_BlockWidth + nXPos;

	unsigned char* pBitmap = (unsigned char*)m_DDSD.lpSurface;
	unsigned short* pPix = (unsigned short*)(pBitmap + y * m_DDSD.lPitch + x + x);
	*pPix = (unsigned short)nCol;
}

int GBlock::GetBlockPixel(int nXPos, int nYPos, int nBlock)
{
	int y = (nBlock / m_nBlocksWide) * m_BlockHeight + nYPos;
	int x = (nBlock % m_nBlocksWide) * m_BlockWidth + nXPos;

	unsigned char* pBitmap = (unsigned char*)m_DDSD.lpSurface;
	unsigned short* pPix = (unsigned short*)(pBitmap + y * m_DDSD.lPitch + x + x);
	return((int)*pPix);
}

