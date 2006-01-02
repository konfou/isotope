/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GImage.h"
#include "GBitTable.h"
#include <math.h>
#include "GMacros.h"
#include <stdlib.h>
#include "GCompress.h"
#include "GBezier.h"
#include "GRayTrace.h"
#include "GHardFont.h"
#include "GPNG.h"
#include "GBits.h"
#include "GFile.h"

#ifdef WIN32
#include <windows.h>
#else // WIN32
typedef unsigned int DWORD;
typedef long LONG;
typedef short WORD;

#pragma pack(1)
typedef struct tagBITMAPINFOHEADER {
    DWORD  biSize;
    LONG   biWidth;
    LONG   biHeight;
    WORD   biPlanes;
    WORD   biBitCount;
    DWORD  biCompression;
    DWORD  biSizeImage;
    LONG   biXPelsPerMeter;
    LONG   biYPelsPerMeter;
    DWORD  biClrUsed;
    DWORD  biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER { 
  WORD    bfType; 
  DWORD   bfSize; 
  WORD    bfReserved1; 
  WORD    bfReserved2; 
  DWORD   bfOffBits; 
} BITMAPFILEHEADER, *PBITMAPFILEHEADER; 
#pragma pack()

#endif // not WIN32

GImage::GImage()
{
	m_pPixels = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
}

GImage::~GImage()
{
	delete(m_pPixels);
}

void GImage::SetSize(int nWidth, int nHeight)
{
	if(nWidth == (int)m_nWidth && nHeight == (int)m_nHeight)
		return;
	delete(m_pPixels);
	if(nWidth == 0 || nHeight == 0)
		m_pPixels = NULL;
	else
	{
		unsigned int nSize = nWidth * nHeight;
		m_pPixels = new GColor[nSize];
		memset(m_pPixels, '\0', nSize * sizeof(GColor));
	}
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

void GImage::CopyImage(GImage* pSourceImage)
{
	SetSize(pSourceImage->GetWidth(), pSourceImage->GetHeight());
	memcpy(m_pPixels, pSourceImage->GetRGBQuads(), m_nWidth * m_nHeight * sizeof(GColor));
}

void GImage::CopyImage(GImage* pSourceImage, int nLeft, int nTop, int nRight, int nBottom)
{
	int nWidth = nRight - nLeft + 1;
	int nHeight = nBottom - nTop + 1;
	SetSize(nWidth, nHeight);
	int x, y;
	GColor c;
	for(y = 0; y < nHeight; y++)
	{
		for(x = 0; x < nWidth; x++)
		{
			c = pSourceImage->GetPixel(x + nLeft, y + nTop);
			SetPixel(x, y, c);
		}
	}
}

void GImage::Clear(GColor color)
{
	unsigned int nSize = m_nWidth * m_nHeight;
	unsigned int nPos;
	for(nPos = 0; nPos < nSize; nPos++)
		m_pPixels[nPos] = color;
}

void GImage::SoftSetPixel(int nX, int nY, GColor color, double dOpacity)
{
	GColor cOld = GetPixel(nX, nY);
	GColor c = gRGB(
		(unsigned char)((1 - dOpacity) * gRed(cOld) + dOpacity * gRed(color)),
		(unsigned char)((1 - dOpacity) * gGreen(cOld) + dOpacity * gGreen(color)),
		(unsigned char)((1 - dOpacity) * gBlue(cOld) + dOpacity * gBlue(color)));
	SetPixel(nX, nY, c);
}

void GImage::SafeSetPixel(int nX, int nY, GColor color)
{
	if(nX < 0 || nX >= (int)m_nWidth)
		return;
	if(nY < 0 || nY >= (int)m_nHeight)
		return;
	SetPixel(nX, nY, color);
}

GColor GImage::SafeGetPixel(int nX, int nY)
{
	if(nX >= 0 && nX < (int)m_nWidth && nY >= 0 && nY < (int)m_nHeight)
		return GetPixel(nX, nY);
	else
		return 0;
}

GColor GImage::InterpolatePixel(float dX, float dY)
{
	int nX = (int)dX;
	int nY = (int)dY;
	float dXDif = dX - nX;
	float dYDif = dY - nY;
	GColor c1;
	GColor c2;
	c1 = SafeGetPixel(nX, nY);
	c2 = SafeGetPixel(nX + 1, nY);
	float dA1 = dXDif * (float)gAlpha(c2) + (1 - dXDif) * (float)gAlpha(c1);
	float dR1 = dXDif * (float)gRed(c2) + (1 - dXDif) * (float)gRed(c1);
	float dG1 = dXDif * (float)gGreen(c2) + (1 - dXDif) * (float)gGreen(c1);
	float dB1 = dXDif * (float)gBlue(c2) + (1 - dXDif) * (float)gBlue(c1);
	c1 = SafeGetPixel(nX, nY + 1);
	c2 = SafeGetPixel(nX + 1, nY + 1);
	float dA2 = dXDif * (float)gAlpha(c2) + (1 - dXDif) * (float)gAlpha(c1);
	float dR2 = dXDif * (float)gRed(c2) + (1 - dXDif) * (float)gRed(c1);
	float dG2 = dXDif * (float)gGreen(c2) + (1 - dXDif) * (float)gGreen(c1);
	float dB2 = dXDif * (float)gBlue(c2) + (1 - dXDif) * (float)gBlue(c1);
	return gARGB((int)(dYDif * dA2 + (1 - dYDif) * dA1),
				(int)(dYDif * dR2 + (1 - dYDif) * dR1),
				(int)(dYDif * dG2 + (1 - dYDif) * dG1),
				(int)(dYDif * dB2 + (1 - dYDif) * dB1));
}

bool GImage::LoadPPMFile(const char* szFilename)
{
	GAssert(szFilename, "no filename");
	FILE* pFile = fopen(szFilename, "rb");
	if(!pFile)
		return false;
	char pBuff[2];
	if(fread(pBuff, 2, 1, pFile) != 1)
	{
		fclose(pFile);
		return false;
	}
	if(pBuff[0] != 'P' && pBuff[0] != 'p')
	{
		fclose(pFile);
		return false;
	}
	bool bRet = false;
	if(pBuff[1] == '3')
		bRet = LoadPixMap(pFile, true, false);
	else if(pBuff[1] == '6')
		bRet = LoadPixMap(pFile, false, false);
	fclose(pFile);
	return bRet;
}

bool GImage::LoadPixMap(FILE* pFile, bool bTextData, bool bGrayScale)
{
	GAssert(pFile, "no file");
	int n;

	// Read past any white space and skip comments
	while(true)
	{
		n = fgetc(pFile);
		if(n == '#')
		{
			while(true)
			{
				n = fgetc(pFile);
				if(n == EOF)
					return false;
				if(n == '\n')
					break;
			}
		}
		if(n == EOF)
			return false;
		if(n > 32)
			break;
	}

	// Read until the next white space to get width
	char pBuff[16];
	pBuff[0] = n;
	int nPos = 1;
	while(true)
	{
		n = fgetc(pFile);
		if(n == EOF)
			return false;
		if(n <= 32)
			break;
		pBuff[nPos] = n;
		nPos++;
		if(nPos >= 16)
			return false;
	}
	pBuff[nPos] = '\0';
	int nWidth = atoi(pBuff);
	if(nWidth < 1)
		return false;

	// Read past any white space and skip comments
	while(true)
	{
		n = fgetc(pFile);
		if(n == '#')
		{
			while(true)
			{
				n = fgetc(pFile);
				if(n == EOF)
					return false;
				if(n == '\n')
					break;
			}
		}
		if(n == EOF)
			return false;
		if(n > 32)
			break;
	}


	// Read until the next white space to get height
	pBuff[0] = n;
	nPos = 1;
	while(true)
	{
		n = fgetc(pFile);
		if(n == EOF)
			return false;
		if(n <= 32)
			break;
		pBuff[nPos] = n;
		nPos++;
		if(nPos >= 16)
			return false;
	}
	pBuff[nPos] = '\0';
	int nHeight = atoi(pBuff);
	if(nHeight < 1)
		return false;

	// Read past any white space and skip comments
	while(true)
	{
		n = fgetc(pFile);
		if(n == '#')
		{
			while(true)
			{
				n = fgetc(pFile);
				if(n == EOF)
					return false;
				if(n == '\n')
					break;
			}
		}
		if(n == EOF)
			return false;
		if(n > 32)
			break;
	}

	// Read until the next white space to get range
	pBuff[0] = n;
	nPos = 1;
	while(true)
	{
		n = fgetc(pFile);
		if(n == EOF)
			return false;
		if(n <= 32)
			break;
		pBuff[nPos] = n;
		nPos++;
		if(nPos >= 16)
			return false;
	}
	pBuff[nPos] = '\0';
	int nRange = atoi(pBuff) + 1;
	if(nRange < 2)
		return false;

	// Read the data
	SetSize(nWidth, nHeight);


	if(bTextData)
	{
		return false;
	}
	else
	{
		int x;
		int y;
		unsigned int nRed;
		unsigned int nGreen;
		unsigned int nBlue;
		for(y = 0; y < nHeight; y++)
		{
			for(x = 0; x < nWidth; x++)
			{
				if(bGrayScale)
				{
					if(fread(pBuff, 1, 1, pFile) != 1)
						return false;
					nRed = (pBuff[0] << 8) / nRange;
					nGreen = (pBuff[0] << 8) / nRange;
					nBlue = (pBuff[0] << 8) / nRange;
				}
				else
				{
					if(fread(pBuff, 3, 1, pFile) != 1)
						return false;
					nRed = (pBuff[0] << 8) / nRange;
					nGreen = (pBuff[1] << 8) / nRange;
					nBlue = (pBuff[2] << 8) / nRange;
				}
				SetPixel(x, y, gRGB(nRed, nGreen, nBlue));
			}
		}
	}
	return true;
}

bool GImage::LoadPGMFile(const char* szFilename)
{
	GAssert(szFilename, "no filename");
	FILE* pFile = fopen(szFilename, "rb");
	if(!pFile)
		return false;
	char pBuff[2];
	if(fread(pBuff, 2, 1, pFile) != 1)
	{
		fclose(pFile);
		return false;
	}
	if(pBuff[0] != 'P' && pBuff[0] != 'p')
	{
		fclose(pFile);
		return false;
	}
	bool bRet = false;
	if(pBuff[1] == '2')
		bRet = LoadPixMap(pFile, true, true);
	else if(pBuff[1] == '5')
		bRet = LoadPixMap(pFile, false, true);
	fclose(pFile);
	return bRet;
}

bool GImage::SavePixMap(FILE* pFile, bool bTextData, bool bGrayScale)
{
	// Write header junk
	char szBuff[24];
	itoa(m_nWidth, szBuff, 10);
	fputs(szBuff, pFile);
	fputs(" ", pFile);
	itoa(m_nHeight, szBuff, 10);
	fputs(szBuff, pFile);
	fputs("\n255\n", pFile);

	// Write pixel data
	int x;
	int y;
	GColor col;
	for(y = 0; y < m_nHeight; y++)
	{
		for(x = 0; x < m_nWidth; x++)
		{
			if(bGrayScale)
			{
				col = GetPixel(x, y);
				unsigned char nGray = (77 * (int)gRed(col) + 150 * (int)gGreen(col) + 29 * (int)gBlue(col)) >> 8;
				if(fwrite(&nGray, 1, 1, pFile) != 1)
					return false;
			}
			else
			{
				col = GetPixel(x, y);
				int n;
				n = gRed(col);
				if(fwrite(&n, 1, 1, pFile) != 1)
					return false;
				n = gGreen(col);
				if(fwrite(&n, 1, 1, pFile) != 1)
					return false;
				n = gBlue(col);
				if(fwrite(&n, 1, 1, pFile) != 1)
					return false;
			}
		}
	}
	return true;
}

bool GImage::SavePPMFile(const char* szFilename)
{
	FILE* pFile = fopen(szFilename, "wb");
	if(!pFile)
		return false;
	if(fputs("P6\n", pFile) == EOF)
		return false;
	bool bRet = SavePixMap(pFile, false, false);
	fclose(pFile);
	return bRet;
}

bool GImage::SavePGMFile(const char* szFilename)
{
	FILE* pFile = fopen(szFilename, "wb");
	if(!pFile)
		return false;
	if(fputs("P5\n", pFile) == EOF)
		return false;
	bool bRet = SavePixMap(pFile, false, true);
	fclose(pFile);
	return bRet;
}

inline GColor ColorToGrayScale(GColor c)
{
	int nGray = (77 * gRed(c) + 150 * gGreen(c) + 29 * gBlue(c)) >> 8;
	return gRGB(nGray, nGray, nGray);
}

void GImage::ConvertToGrayScale()
{
	unsigned int nSize = m_nWidth * m_nHeight;
	unsigned int nPos;
	for(nPos = 0; nPos < nSize; nPos++)
		m_pPixels[nPos] = ColorToGrayScale(m_pPixels[nPos]);
}

void GImage::CreateBrightnessHistogram(GImage* pOutImage)
{
	// Create the histogram data
	unsigned int pnHistData[256];
	int n;
	for(n = 0; n < 256; n++)
		pnHistData[n] = 0;
	unsigned int nSize = m_nWidth * m_nHeight;
	unsigned int nPos;
	unsigned int nGray;
	unsigned int nMaxValue = 0;
	for(nPos = 0; nPos < nSize; nPos++)
	{
		nGray = ColorToGrayScale(m_pPixels[nPos]);
		pnHistData[nGray]++;
		if(pnHistData[nGray] > nMaxValue)
			nMaxValue = pnHistData[nGray];
	}

	// turn it into a picture
	pOutImage->SetSize(256, 256);
	pOutImage->Clear(gRGB(220, 220, 180));
	int x;
	int y;
	for(x = 0; x < 256; x++)
	{
		int nHeight = pnHistData[x] * 256 / nMaxValue;
		for(y = 0; y < nHeight; y++)
			pOutImage->SetPixel(x, 255 - y, gRGB(y, 255 - y, 80));
	}
}

void GImage::EqualizeColorSpread()
{
	// Create the histogram data
	unsigned int pnHistData[256];
	int n;
	for(n = 0; n < 256; n++)
		pnHistData[n] = 0;
	unsigned int nSize = m_nWidth * m_nHeight;
	unsigned int nPos;
	unsigned int nGray;
	unsigned int nMaxValue = 0;
	for(nPos = 0; nPos < nSize; nPos++)
	{
		nGray = ColorToGrayScale(m_pPixels[nPos]);
		pnHistData[nGray]++;
		if(pnHistData[nGray] > nMaxValue)
			nMaxValue = pnHistData[nGray];
	}

	// Turn it into cumulative histogram data
	for(n = 1; n < 256; n++)
		pnHistData[n] += pnHistData[n - 1];
	int nFactor = pnHistData[255] >> 8;

	// turn it into a picture
	int x;
	int y;
	GColor col;
	float fFactor;
	for(y = 0; y < m_nHeight; y++)
	{
		for(x = 0; x < m_nWidth; x++)
		{
			col = GetPixel(x, y);
			nGray = (77 * (int)gRed(col) + 150 * (int)gGreen(col) + 29 * (int)gBlue(col)) >> 8;
			fFactor = ((float)pnHistData[nGray] / (float)nFactor) / nGray;
			
			SetPixel(x, y, 
				gRGB(
				(char)MIN((int)((float)gRed(col) * fFactor), 255),
				(char)MIN((int)((float)gGreen(col) * fFactor), 255),
				(char)MIN((int)((float)gBlue(col) * fFactor), 255)
					));
		}
	}
}

void GImage::LocallyEqualizeColorSpread(int nLocalSize, float fExtent)
{
	if(nLocalSize % 2 != 0)
		nLocalSize++;
	GAssert(nLocalSize > 2, "Local size must be more than 2");
	
	// Create histograms for all the local regions
	int nHalfRegionSize = nLocalSize >> 1;
	int nHorizRegions = (m_nWidth + nHalfRegionSize - 1) / nHalfRegionSize + 1;
	int nVertRegions = (m_nHeight + nHalfRegionSize - 1) / nHalfRegionSize + 1;
	int* pArrHistograms = new int[256 * nHorizRegions * nVertRegions];
	memset(pArrHistograms, '\0', 256 * nHorizRegions * nVertRegions * sizeof(int));
	int* pHist;
	GAssert(pArrHistograms, "Failed to allocate memory");
	int nHoriz, nVert, nDX, nDY, nX, nY, nGrayscale, n;
	GColor col;
	for(nVert = 0; nVert < nVertRegions; nVert++)
	{
		for(nHoriz = 0; nHoriz < nHorizRegions; nHoriz++)
		{
			// Make a histogram for the local region
			pHist = pArrHistograms + 256 * (nHorizRegions * nVert + nHoriz);
			for(nDY = 0; nDY < nLocalSize; nDY++)
			{
				nY = (nVert - 1) * nHalfRegionSize + nDY;
				for(nDX = 0; nDX < nLocalSize; nDX++)
				{
					nX = (nHoriz - 1) * nHalfRegionSize + nDX;
					if(nX >= 0 && nX < (int)m_nWidth && nY >= 0 && nY < (int)m_nHeight)
					{
						col = GetPixel(nX, nY);
						nGrayscale = (77 * (int)gRed(col) + 150 * (int)gGreen(col) + 29 * (int)gBlue(col)) >> 8;
						pHist[nGrayscale]++;
					}
				}
			}
			
			// Turn the histogram into cumulative histogram data
			for(n = 1; n < 256; n++)
				pHist[n] += pHist[n - 1];
		}
	}
	
	// Equalize the colors
	float fFactor1, fFactor2, fFactor3, fFactor4, fFactorTop, fFactorBottom, fFactorInterpolated;
	float fInterp;
	for(nY = 0; nY < (int)m_nHeight; nY++)
	{
		nVert = nY / nHalfRegionSize;
		GAssert(nVert < nVertRegions, "Region out of range");
		nDY = nY % nHalfRegionSize;
		for(nX = 0; nX < (int)m_nWidth; nX++)
		{
			// Get the Pixel
			nHoriz = nX / nHalfRegionSize;
			GAssert(nHoriz < nHorizRegions, "Region out of range");
			nDX = nX % nHalfRegionSize;
			col = GetPixel(nX, nY);
			nGrayscale = (77 * (int)gRed(col) + 150 * (int)gGreen(col) + 29 * (int)gBlue(col)) >> 8;
			
			// Calculate equalization factor for quadrant 1
			pHist = pArrHistograms + 256 * (nHorizRegions * nVert + nHoriz);
			fFactor1 = ((float)pHist[nGrayscale] / (float)pHist[255]) * 255 / nGrayscale;
			
			// Calculate equalization factor for quadrant 2
			pHist = pArrHistograms + 256 * (nHorizRegions * nVert + (nHoriz + 1));
			fFactor2 = ((float)pHist[nGrayscale] / (float)pHist[255]) * 255 / nGrayscale;
			
			// Calculate equalization factor for quadrant 3
			pHist = pArrHistograms + 256 * (nHorizRegions * (nVert + 1) + nHoriz);
			fFactor3 = ((float)pHist[nGrayscale] / (float)pHist[255]) * 255 / nGrayscale;
			
			// Calculate equalization factor for quadrant 4
			pHist = pArrHistograms + 256 * (nHorizRegions * (nVert + 1) + (nHoriz + 1));
			fFactor4 = ((float)pHist[nGrayscale] / (float)pHist[255]) * 255 / nGrayscale;
			
			// Interpolate a factor from all 4 quadrants
			fInterp = (float)nDX / (float)(nHalfRegionSize - 1);
			fFactorTop = fInterp * fFactor2 + (1 - fInterp) * fFactor1;
			fFactorBottom = fInterp * fFactor4 + (1 - fInterp) * fFactor3;
			fInterp = (float)nDY / (float)(nHalfRegionSize - 1);
			fFactorInterpolated = (fInterp * fFactorBottom + (1 - fInterp) * fFactorTop) * fExtent + 1 - fExtent;

			// Set the Pixel
			SetPixel(nX, nY,
				gRGB(
					(char)MAX(0, MIN((int)((float)gRed(col) * fFactorInterpolated), 255)),
					(char)MAX(0, MIN((int)((float)gGreen(col) * fFactorInterpolated), 255)),
					(char)MAX(0, MIN((int)((float)gBlue(col) * fFactorInterpolated), 255))
					));
		}
	}
}

void GImage::SafeDrawLine(int nX1, int nY1, int nX2, int nY2, GColor color)
{
	/*
	// Note: This uses the DDA line drawing algorythm (which is slower than the
	// Bresenham algorythm, but I had to do it this way for a class I'm taking.)
	int nSteps = MAX(ABS(nX2 - nX1), ABS(nY2 - nY1));
	double dXStep = (double)(nX2 - nX1) / (double)nSteps;
	double dYStep = (double)(nY2 - nY1) / (double)nSteps;
	int n;
	double dX = nX1 + .5;
	double dY = nY1 + .5;
	SafeSetPixel((int)dX, (int)dY, color);
	for(n = 0; n < nSteps; n++)
	{
		dX += dXStep;
		dY += dYStep;
		SafeSetPixel((int)dX, (int)dY, color);
	}
	*/

	// Check nX1
	if(nX1 < 0)
	{
		if(nX2 < 0)
			return;
		nY1 = (nY1 - nY2) * nX2 / (nX2 - nX1) + nY2;
		nX1 = 0;
	}
	if(nX1 >= m_nWidth)
	{
		if(nX2 >= m_nWidth)
			return;
		nY1 = (nY1 - nY2) * (m_nWidth - 1 - nX2) / (nX2 - nX1) + nY2;
		nX1 = m_nWidth - 1;
	}

	// Check nY1
	if(nY1 < 0)
	{
		if(nY2 < 0)
			return;
		nX1 = (nX1 - nX2) * nY2 / (nY2 - nY1) + nX2;
		nY1 = 0;
	}
	if(nY1 >= m_nHeight)
	{
		if(nY2 >= m_nHeight)
			return;
		nX1 = (nX1 - nX2) * (m_nHeight - 1 - nY2) / (nY2 - nY1) + nX2;
		nY1 = m_nHeight - 1;
	}

	// Check nX2
	if(nX2 < 0)
	{
		if(nX1 < 0)
			return;
		nY2 = (nY2 - nY1) * nX1 / (nX1 - nX2) + nY1;
		nX2 = 0;
	}
	if(nX2 >= m_nWidth)
	{
		if(nX1 >= m_nWidth)
			return;
		nY2 = (nY2 - nY1) * (m_nWidth - 1 - nX1) / (nX2 - nX1) + nY1;
		nX2 = m_nWidth - 1;
	}

	// Check nY2
	if(nY2 < 0)
	{
		if(nY1 < 0)
			return;
		nX2 = (nX2 - nX1) * nY1 / (nY1 - nY2) + nX1;
		nY2 = 0;
	}
	if(nY2 >= m_nHeight)
	{
		if(nY1 >= m_nHeight)
			return;
		nX2 = (nX2 - nX1) * (m_nHeight - 1 - nY1) / (nY1 - nY2) + nX1;
		nY2 = m_nHeight - 1;
	}
	DrawLine(nX1, nY1, nX2, nY2, color);
}

// Note: This uses the Bresenham line drawing algorythm
void GImage::DrawLine(int nX1, int nY1, int nX2, int nY2, GColor color)
{
	int n;
	int nXDif = ABS(nX2 - nX1);
	int nYDif = ABS(nY2 - nY1);
	int nOverflow;
	int m;
	if(nXDif > nYDif)
	{
		if(nX2 < nX1)
		{
			n = nX2;
			nX2 = nX1;
			nX1 = n;
			n = nY2;
			nY2 = nY1;
			nY1 = n;
		}
		nOverflow = nXDif >> 1;
		m = nY1;
		if(nY1 < nY2)
		{
			for(n = nX1; n <= nX2; n++)
			{
				SetPixel(n, m, color);
				nOverflow += nYDif;
				if(nOverflow >= nXDif)
				{
					nOverflow -= nXDif;
					m++;
				}
			}
		}
		else
		{
			for(n = nX1; n <= nX2; n++)
			{
				SetPixel(n, m, color);
				nOverflow += nYDif;
				if(nOverflow >= nXDif)
				{
					nOverflow -= nXDif;
					m--;
				}
			}
		}
	}
	else
	{
		if(nY2 < nY1)
		{
			n = nX2;
			nX2 = nX1;
			nX1 = n;
			n = nY2;
			nY2 = nY1;
			nY1 = n;
		}
		nOverflow = nYDif >> 1;
		m = nX1;
		if(nX1 < nX2)
		{
			for(n = nY1; n <= nY2; n++)
			{
				SetPixel(m, n, color);
				nOverflow += nXDif;
				if(nOverflow >= nYDif)
				{
					nOverflow -= nYDif;
					m++;
				}
			}
		}
		else
		{
			for(n = nY1; n <= nY2; n++)
			{
				SetPixel(m, n, color);
				nOverflow += nXDif;
				if(nOverflow >= nYDif)
				{
					nOverflow -= nYDif;
					m--;
				}
			}
		}
	}
}

void GImage::DrawLineAntiAlias(int nX1, int nY1, int nX2, int nY2, GColor color)
{
	int n;
	int m;
	int nXDif = ABS(nX2 - nX1);
	int nYDif = ABS(nY2 - nY1);
	int nOverflow;
	GColor col;
	double d;
	if(nXDif > nYDif)
	{
		if(nX2 < nX1)
		{
			n = nX2;
			nX2 = nX1;
			nX1 = n;
			n = nY2;
			nY2 = nY1;
			nY1 = n;
		}
		nOverflow = 0;
		m = nY1;
		if(nY1 < nY2)
		{
			for(n = nX1; n <= nX2; n++)
			{
				d = (double)nOverflow / nXDif;
				col = GetPixel(n, m);
				SetPixel(n, m, 
					gRGB((unsigned char)(d * gRed(col) + (1 - d) * gRed(color)), (unsigned char)(d * gGreen(col) + (1 - d) * gGreen(color)), (unsigned char)(d * gBlue(col) + (1 - d) * gBlue(color))));
				col = GetPixel(n, m + 1);
				SetPixel(n, m + 1, 
					gRGB((unsigned char)((1 - d) * gRed(col) + d * gRed(color)), (unsigned char)((1 - d) * gGreen(col) + d * gGreen(color)), (unsigned char)((1 - d) * gBlue(col) + d * gBlue(color))));
				nOverflow += nYDif;
				if(nOverflow >= nXDif)
				{
					nOverflow -= nXDif;
					m++;
				}
			}
		}
		else
		{
			for(n = nX1; n <= nX2; n++)
			{
				d = (double)nOverflow / nXDif;
				col = GetPixel(n, m);
				SetPixel(n, m, 
					gRGB((unsigned char)(d * gRed(col) + (1 - d) * gRed(color)), (unsigned char)(d * gGreen(col) + (1 - d) * gGreen(color)), (unsigned char)(d * gBlue(col) + (1 - d) * gBlue(color))));
				col = GetPixel(n, m - 1);
				SetPixel(n, m - 1, 
					gRGB((unsigned char)((1 - d) * gRed(col) + d * gRed(color)), (unsigned char)((1 - d) * gGreen(col) + d * gGreen(color)), (unsigned char)((1 - d) * gBlue(col) + d * gBlue(color))));
				nOverflow += nYDif;
				if(nOverflow >= nXDif)
				{
					nOverflow -= nXDif;
					m--;
				}
			}
		}
	}
	else
	{
		if(nY2 < nY1)
		{
			n = nX2;
			nX2 = nX1;
			nX1 = n;
			n = nY2;
			nY2 = nY1;
			nY1 = n;
		}
		nOverflow = 0;
		m = nX1;
		if(nX1 < nX2)
		{
			for(n = nY1; n <= nY2; n++)
			{
				d = (double)nOverflow / nYDif;
				col = GetPixel(m, n);
				SetPixel(m, n, 
					gRGB((unsigned char)(d * gRed(col) + (1 - d) * gRed(color)), (unsigned char)(d * gGreen(col) + (1 - d) * gGreen(color)), (unsigned char)(d * gBlue(col) + (1 - d) * gBlue(color))));
				col = GetPixel(m + 1, n);
				SetPixel(m + 1, n, 
					gRGB((unsigned char)((1 - d) * gRed(col) + d * gRed(color)), (unsigned char)((1 - d) * gGreen(col) + d * gGreen(color)), (unsigned char)((1 - d) * gBlue(col) + d * gBlue(color))));
				nOverflow += nXDif;
				if(nOverflow >= nYDif)
				{
					nOverflow -= nYDif;
					m++;
				}
			}
		}
		else
		{
			for(n = nY1; n <= nY2; n++)
			{
				d = (double)nOverflow / nYDif;
				col = GetPixel(m, n);
				SetPixel(m, n, 
					gRGB((unsigned char)(d * gRed(col) + (1 - d) * gRed(color)), (unsigned char)(d * gGreen(col) + (1 - d) * gGreen(color)), (unsigned char)(d * gBlue(col) + (1 - d) * gBlue(color))));
				col = GetPixel(m - 1, n);
				SetPixel(m - 1, n, 
					gRGB((unsigned char)((1 - d) * gRed(col) + d * gRed(color)), (unsigned char)((1 - d) * gGreen(col) + d * gGreen(color)), (unsigned char)((1 - d) * gBlue(col) + d * gBlue(color))));
				nOverflow += nXDif;
				if(nOverflow >= nYDif)
				{
					nOverflow -= nYDif;
					m--;
				}
			}
		}
	}
}

void GImage::FloodFillRecurser(int nX, int nY, unsigned char nSrcR, unsigned char nSrcG, unsigned char nSrcB, unsigned char nDstR, unsigned char nDstG, unsigned char nDstB, int nTolerance)
{
	GColor col;
	int nDif;
	while(true)
	{
		col = GetPixel(nX - 1, nY);
		nDif = ABS((int)gRed(col) - nSrcR) + ABS((int)gGreen(col) - nSrcG) + ABS((int)gBlue(col) - nSrcB);
		if(nDif > nTolerance || nX < 1)
			break;
		nX--;
	}
	GBitTable btUp(m_nWidth);
	GBitTable btDn(m_nWidth);
	while(true)
	{
		SetPixel(nX, nY, gRGB(nDstR, nDstG, nDstB));

		if(nY > 0)
		{
			col = GetPixel(nX, nY - 1);
			if(gRed(col) != nDstR || gGreen(col) != nDstG || gBlue(col) != nDstB)
			{
				nDif = ABS((int)gRed(col) - nSrcR) + ABS((int)gGreen(col) - nSrcG) + ABS((int)gBlue(col) - nSrcB);
				if(nDif <= nTolerance)
					btUp.SetBit(nX, true);
			}
		}
		
		if(nY < (int)m_nHeight - 1)
		{
			col = GetPixel(nX, nY + 1);
			if(gRed(col) != nDstR || gGreen(col) != nDstG || gBlue(col) != nDstB)
			{
				nDif = ABS((int)gRed(col) - nSrcR) + ABS((int)gGreen(col) - nSrcG) + ABS((int)gBlue(col) - nSrcB);
				if(nDif <= nTolerance)
					btDn.SetBit(nX, true);
			}
		}

		nX++;
		if(nX >= (int)m_nWidth)
			break;
		col = GetPixel(nX, nY);
		nDif = ABS((int)gRed(col) - nSrcR) + ABS((int)gGreen(col) - nSrcG) + ABS((int)gBlue(col) - nSrcB);
		if(nDif > nTolerance)
			break;
	}
	bool bPrev = false;
	for(nX = 0; nX < (int)m_nWidth; nX++)
	{
		if(btUp.GetBit(nX))
		{
			if(!bPrev)
			{
				col = GetPixel(nX, nY - 1);
				if(gRed(col) != nDstR || gGreen(col) != nDstG || gBlue(col) != nDstB)
					FloodFillRecurser(nX, nY - 1, nSrcR, nSrcG, nSrcB, nDstR, nDstG, nDstB, nTolerance);
			}
			bPrev = true;
		}
		else
			bPrev = false;
	}
	bPrev = false;
	for(nX = 0; nX < (int)m_nWidth; nX++)
	{
		if(btDn.GetBit(nX))
		{
			if(!bPrev)
			{
				col = GetPixel(nX, nY + 1);
				if(gRed(col) != nDstR || gGreen(col) != nDstG || gBlue(col) != nDstB)
					FloodFillRecurser(nX, nY + 1, nSrcR, nSrcG, nSrcB, nDstR, nDstG, nDstB, nTolerance);
			}
			bPrev = true;
		}
		else
			bPrev = false;
	}
}

void GImage::FloodFill(int nX, int nY, GColor color, int nTolerance)
{
	GColor col = GetPixel(nX, nY);
	FloodFillRecurser(nX, nY, gRed(col), gGreen(col), gBlue(col), gRed(color), gGreen(color), gBlue(color), nTolerance);
}

void GImage::BoundaryFill(int nX, int nY, unsigned char nBoundaryR, unsigned char nBoundaryG, unsigned char nBoundaryB, unsigned char nFillR, unsigned char nFillG, unsigned char nFillB, int nTolerance)
{
	int nDif;
	GColor col = GetPixel(nX, nY);
	nDif = ABS((int)gRed(col) - nBoundaryR) + ABS((int)gGreen(col) - nBoundaryG) + ABS((int)gBlue(col) - nBoundaryB);
	if(nDif <= nTolerance)
		return;
	while(true)
	{
		col = GetPixel(nX - 1, nY);
		nDif = ABS((int)gRed(col) - nBoundaryR) + ABS((int)gGreen(col) - nBoundaryG) + ABS((int)gBlue(col) - nBoundaryB);
		if(nDif <= nTolerance || nX < 1)
			break;
		nX--;
	}
	GBitTable btUp(m_nWidth);
	GBitTable btDn(m_nWidth);
	while(true)
	{
		SetPixel(nX, nY, gRGB(nFillR, nFillG, nFillB));

		if(nY > 0)
		{
			col = GetPixel(nX, nY - 1);
			if(gRed(col) != nFillR || gGreen(col) != nFillG || gBlue(col) != nFillB)
			{
				nDif = ABS((int)gRed(col) - nBoundaryR) + ABS((int)gGreen(col) - nBoundaryG) + ABS((int)gBlue(col) - nBoundaryB);
				if(nDif > nTolerance)
					btUp.SetBit(nX, true);
			}
		}
		
		if(nY < (int)m_nHeight - 1)
		{
			col = GetPixel(nX, nY + 1);
			if(gRed(col) != nFillR || gGreen(col) != nFillG || gBlue(col) != nFillB)
			{
				nDif = ABS((int)gRed(col) - nBoundaryR) + ABS((int)gGreen(col) - nBoundaryG) + ABS((int)gBlue(col) - nBoundaryB);
				if(nDif > nTolerance)
					btDn.SetBit(nX, true);
			}
		}

		nX++;
		if(nX >= (int)m_nWidth)
			break;
		col = GetPixel(nX, nY);
		nDif = ABS((int)gRed(col) - nBoundaryR) + ABS((int)gGreen(col) - nBoundaryG) + ABS((int)gBlue(col) - nBoundaryB);
		if(nDif <= nTolerance)
			break;
	}
	bool bPrev = false;
	for(nX = 0; nX < (int)m_nWidth; nX++)
	{
		if(btUp.GetBit(nX))
		{
			if(!bPrev)
			{
				col = GetPixel(nX, nY - 1);
				if(gRed(col) != nFillR || gGreen(col) != nFillG || gBlue(col) != nFillB)
					BoundaryFill(nX, nY - 1, nBoundaryR, nBoundaryG, nBoundaryB, nFillR, nFillG, nFillB, nTolerance);
			}
			bPrev = true;
		}
		else
			bPrev = false;
	}
	bPrev = false;
	for(nX = 0; nX < (int)m_nWidth; nX++)
	{
		if(btDn.GetBit(nX))
		{
			if(!bPrev)
			{
				col = GetPixel(nX, nY + 1);
				if(gRed(col) != nFillR || gGreen(col) != nFillG || gBlue(col) != nFillB)
					BoundaryFill(nX, nY + 1, nBoundaryR, nBoundaryG, nBoundaryB, nFillR, nFillG, nFillB, nTolerance);
			}
			bPrev = true;
		}
		else
			bPrev = false;
	}
}

void GImage::DrawPolygon(int nPoints, int* pnPtArray, GColor color)
{
	if(nPoints < 3)
		return;
	float* pnPointArray = new float[nPoints << 1];
	int n;
	for(n = 0; n < nPoints << 1; n++)
		pnPointArray[n] = ((float)pnPtArray[n]) + .1f;

	for(n = 0; n < nPoints; n++)
	{
		int n2 = n + 1;
		if(n2 >= nPoints)
			n2 = 0;
		SafeDrawLine(pnPtArray[n << 1], pnPtArray[(n << 1) + 1], pnPtArray[n2 << 1], pnPtArray[(n2 << 1) + 1], color);
	}

	// Find Max and Min values
	int nXMin = (int)pnPointArray[0];
	int nXMax = (int)pnPointArray[0];
	int nYMin = (int)pnPointArray[1];
	int nYMax = (int)pnPointArray[1];
	int nPos = 2;
	for(n = 1; n < nPoints; n++)
	{
		if(pnPointArray[nPos] < nXMin)
			nXMin = (int)pnPointArray[nPos];
		if(pnPointArray[nPos] + 1 > nXMax)
			nXMax = (int)pnPointArray[nPos] + 1;
		nPos++;
		if(pnPointArray[nPos] < nYMin)
			nYMin = (int)pnPointArray[nPos];
		if(pnPointArray[nPos] + 1 > nYMax)
			nYMax = (int)pnPointArray[nPos] + 1;
		nPos++;
	}

	int nX, nY;
	int* pArrPts = new int[m_nWidth];
	for(nY = nYMin; nY <= nYMax; nY++)
	{
		memset(pArrPts, '\0', sizeof(int) * m_nWidth);
		
		// Calculate Intercepts
		for(n = 0; n < nPoints; n++)
		{
			int n2 = n + 1;
			if(n2 >= nPoints)
				n2 = 0;
			float nX1 = pnPointArray[n << 1];
			float nY1 = pnPointArray[(n << 1) + 1];
			float nX2 = pnPointArray[n2 << 1];
			float nY2 = pnPointArray[(n2 << 1) + 1];

			if(nY < nY1 && nY < nY2)
				continue;
			if(nY > nY1 && nY > nY2)
				continue;
			nX = (int)(nX1 + (nY - nY1) * (nX2 - nX1) / (nY2 - nY1));
			if(nX < 0)
				nX = 0;
			if(nX >= (int)m_nWidth)
				nX = (int)m_nWidth - 1;
			pArrPts[nX]++;
		}

		// Draw the Scan Line
		bool bOn = false;
		for(nX = nXMin; nX <= nXMax; nX++)
		{
			if(bOn || pArrPts[nX])
				SafeSetPixel(nX, nY, color);
			if(pArrPts[nX] & 1)
				bOn = !bOn;
		}
	}
	delete(pArrPts);
}

void GImage::DrawCircle(int nX, int nY, double dRadius, GColor color)
{
	double dAngle;
	double dStep = .9 / dRadius;
	if(dStep == 0)
		return;
	double dSin;
	double dCos;
	for(dAngle = 0; dAngle < 0.79; dAngle += dStep)
	{
		dSin = sin(dAngle);
		dCos = cos(dAngle);
		SafeSetPixel((int)(nX + dCos * dRadius), (int)(nY + dSin * dRadius), color);
		SafeSetPixel((int)(nX - dCos * dRadius), (int)(nY + dSin * dRadius), color);
		SafeSetPixel((int)(nX + dCos * dRadius), (int)(nY - dSin * dRadius), color);
		SafeSetPixel((int)(nX - dCos * dRadius), (int)(nY - dSin * dRadius), color);
		SafeSetPixel((int)(nX + dSin * dRadius), (int)(nY + dCos * dRadius), color);
		SafeSetPixel((int)(nX - dSin * dRadius), (int)(nY + dCos * dRadius), color);
		SafeSetPixel((int)(nX + dSin * dRadius), (int)(nY - dCos * dRadius), color);
		SafeSetPixel((int)(nX - dSin * dRadius), (int)(nY - dCos * dRadius), color);
	}
}

void GImage::DrawEllipse(int nX, int nY, double dRadius, double dHeightToWidthRatio, GColor color)
{
	double dAngle;
	double dStep = .9 / dRadius;
	if(dHeightToWidthRatio > 1)
		dStep /= dHeightToWidthRatio;
	if(dStep == 0)
		return;
	double dSin;
	double dCos;
	for(dAngle = 0; dAngle < 0.79; dAngle += dStep)
	{
		dSin = sin(dAngle);
		dCos = cos(dAngle);
		SafeSetPixel((int)(nX + dCos * dRadius), (int)(nY + dHeightToWidthRatio * dSin * dRadius), color);
		SafeSetPixel((int)(nX - dCos * dRadius), (int)(nY + dHeightToWidthRatio * dSin * dRadius), color);
		SafeSetPixel((int)(nX + dCos * dRadius), (int)(nY - dHeightToWidthRatio * dSin * dRadius), color);
		SafeSetPixel((int)(nX - dCos * dRadius), (int)(nY - dHeightToWidthRatio * dSin * dRadius), color);
		SafeSetPixel((int)(nX + dSin * dRadius), (int)(nY + dHeightToWidthRatio * dCos * dRadius), color);
		SafeSetPixel((int)(nX - dSin * dRadius), (int)(nY + dHeightToWidthRatio * dCos * dRadius), color);
		SafeSetPixel((int)(nX + dSin * dRadius), (int)(nY - dHeightToWidthRatio * dCos * dRadius), color);
		SafeSetPixel((int)(nX - dSin * dRadius), (int)(nY - dHeightToWidthRatio * dCos * dRadius), color);
	}
}

void GImage::Rotate(GImage* pSourceImage, int nX, int nY, double dAngle)
{
	int x;
	int y;
	float dCos = (float)cos(dAngle);
	float dSin = (float)sin(dAngle);
	for(y = 0; y < (int)m_nHeight; y++)
	{
		for(x = 0; x < (int)m_nWidth; x++)
		{
			float dX = (x - nX) * dCos - (y - nY) * dSin + nX;
			float dY = (x - nX) * dSin + (y - nY) * dCos + nY;
			GColor col = pSourceImage->InterpolatePixel(dX, dY);
			SetPixel(x, y, col);
		}
	}
}

void GImage::PasteImage(int nX, int nY, GImage* pSourceImage, GColor colTransparent)
{
	int x;
	int y;
	for(y = 0; y < pSourceImage->m_nHeight; y++)
	{
		if(nY + y >= m_nHeight)
			break;
		for(x = 0; x < pSourceImage->m_nWidth; x++)
		{
			if(nX + x >= m_nWidth)
				break;
			GColor col = pSourceImage->GetPixel(x, y);
			if(col != colTransparent)
				SetPixel(nX + x, nY + y, col);
		}
	}
}

void GImage::PasteImage(int nX, int nY, GImage* pSourceImage)
{
	int x;
	int y;
	for(y = 0; y < pSourceImage->m_nHeight; y++)
	{
		if(nY + y >= m_nHeight)
			break;
		for(x = 0; x < pSourceImage->m_nWidth; x++)
		{
			if(nX + x >= m_nWidth)
				break;
			GColor col = pSourceImage->GetPixel(x, y);
			SetPixel(nX + x, nY + y, col);
		}
	}
}

bool GImage::LoadPNGFile(const unsigned char* pRawData, int nBytes)
{
	return LoadPng(this, pRawData, nBytes);
}

bool GImage::LoadPNGFile(const char* szFilename)
{
	int nSize;
	char* pRawData = GFile::LoadFileToBuffer(szFilename, &nSize);
	if(!pRawData)
		return false;
	Holder<char*> hRawData(pRawData);
	return LoadPng(this, (const unsigned char*)pRawData, nSize);
}

bool GImage::SavePIGFile(FILE* pFile)
{
	// Copy to a buffer
	int nSize = m_nWidth * m_nHeight * 3 + 2 * sizeof(int);
	unsigned char* pBytes = new unsigned char[nSize];
	((int*)pBytes)[0] = m_nWidth;
	((int*)pBytes)[1] = m_nHeight;
	int nPos = 2 * sizeof(int);
	int x, y;
	GColor col;
	for(y = 0; y < m_nHeight; y++)
	{
		for(x = 0; x < m_nWidth; x++)
		{
			col = GetPixel(x, y);
			pBytes[nPos++] = gRed(col);
			pBytes[nPos++] = gGreen(col);
			pBytes[nPos++] = gBlue(col);
		}
	}
	GAssert(nPos == nSize, "internal error");

	// Compress it
	int nNewSize;
	unsigned char* pCompressed = GCompress::Compress(pBytes, nSize, &nNewSize);
	delete(pBytes);
	if(!pCompressed)
	{
		GAssert(false, "error compressing image");
		return false;
	}

	// Save it
	bool bOK = true;
	if(fwrite(&nNewSize, sizeof(int), 1, pFile) != 1)
		bOK = false;
	else if(fwrite(pCompressed, nNewSize, 1, pFile) != 1)
		bOK = false;
	delete(pCompressed);
	return bOK;
}

bool GImage::SavePIGFile(const char* szFilename)
{
	FILE* pFile = fopen(szFilename, "wb");
	if(!pFile)
		return false;
	bool bOK = SavePIGFile(pFile);
	fclose(pFile);
	return bOK;
}

bool GImage::SavePNGFile(FILE* pFile)
{
	return SavePng(this, pFile, true);
}

bool GImage::SavePNGFile(const char* szFilename)
{
	FILE* pFile = fopen(szFilename, "wb");
	if(!pFile)
		return false;
	bool bOK = SavePNGFile(pFile);
	fclose(pFile);
	return bOK;
}

bool GImage::SaveBMPFile(const char* szFilename)
{
	unsigned int nSize = m_nWidth * m_nHeight;

	BITMAPFILEHEADER h1;
	h1.bfType = 19778; // "BM"
	h1.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 3 * nSize;
	h1.bfReserved1 = 0;
	h1.bfReserved2 = 0;
	h1.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	BITMAPINFOHEADER h2;
	h2.biSize = sizeof(BITMAPINFOHEADER);
	h2.biWidth = m_nWidth;
	h2.biHeight = /*-(int)*/m_nHeight;
	h2.biPlanes = 1;
	h2.biBitCount = 24;
	h2.biCompression = 0;
	h2.biSizeImage = 3 * nSize;
	h2.biXPelsPerMeter = 3780;
	h2.biYPelsPerMeter = 3780;
	h2.biClrUsed = 0;
	h2.biClrImportant = 0;

	FILE* pFile = fopen(szFilename, "wb");
	if(!pFile)
		return false;

	if(fwrite(&h1, sizeof(BITMAPFILEHEADER), 1, pFile) != 1)
	{
		fclose(pFile);
		return false;
	}
	if(fwrite(&h2, sizeof(BITMAPINFOHEADER), 1, pFile) != 1)
	{
		fclose(pFile);
		return false;
	}
	int y;
	int x;
	GColor col;
	for(y = (int)m_nHeight - 1; y >= 0; y--)
	{
		for(x = 0; x < (int)m_nWidth; x++)
		{
			col = GetPixel(x, y);
			int nR = gRed(col);
			int nG = gGreen(col);
			int nB = gBlue(col);
			if(
				fwrite(&nB, 1, 1, pFile) != 1 ||
				fwrite(&nG, 1, 1, pFile) != 1 ||
				fwrite(&nR, 1, 1, pFile) != 1
				)
			{
				fclose(pFile);
				return false;
			}
		}
		int n = (x * 3) % 4;
		if(n > 0)
		{
			int nR = 0;
			while(n < 4) // Allign on word boundaries
			{
				fwrite(&nR, 1, 1, pFile);
				n++;
			}
		}
	}

	fclose(pFile);
	return true;
}

bool GImage::LoadPIGFile(const unsigned char* pBytes, int nSize)
{
	// Decompress the data
	GAssert(*(int*)pBytes == nSize - (int)sizeof(int), "bad data");
	int nNewSize;
	unsigned char* pData = GCompress::Decompress(pBytes + sizeof(int), nSize - sizeof(int), &nNewSize);
	if(!pData)
		return false;

	// Parse the data
	int nWidth = ((int*)pData)[0];
	int nHeight = ((int*)pData)[1];
	if(nSize != nWidth * nHeight * 3 + 2 * (int)sizeof(int))
	{
		GAssert(false, "bad PIG file");
		return false;
	}
	SetSize(nWidth, nHeight);
	int nPos = 2 * sizeof(int);
	int x, y, r, g, b;
	for(y = 0; y < nHeight; y++)
	{
		for(x = 0; x < nHeight; x++)
		{
			r = pData[nPos++];
			g = pData[nPos++];
			b = pData[nPos++];
			SetPixel(x, y, gRGB(r, g, b));
		}
	}
	return true;
}

bool GImage::LoadBMPFile(const char* szFilename)
{
	FILE* pFile = fopen(szFilename, "rb");
	if(!pFile)
		return false;
	bool bRet = LoadBMPFile(pFile);
	fclose(pFile);
	return bRet;
}

bool GImage::LoadBMPFile(FILE* pFile)
{
	BITMAPFILEHEADER h1;
	BITMAPINFOHEADER h2;
	if(fread(&h1, sizeof(BITMAPFILEHEADER), 1, pFile) != 1)
		return false;
	if(fread(&h2, sizeof(BITMAPINFOHEADER), 1, pFile) != 1)
		return false;
	if(h2.biBitCount != 24)
	{
		GAssert(false, "Sorry, this only supports 24-bit bitmaps");
		return false;
	}
	if(h2.biWidth < 1 || h2.biHeight < 1)
	{
		GAssert(false, "Sorry, this only supports bottom-up bitmaps");
		return false;
	}
	SetSize(h2.biWidth, h2.biHeight);

	int y;
	int x;
	unsigned char nR;
	unsigned char nG;
	unsigned char nB;
	for(y = (int)m_nHeight - 1; y >= 0; y--)
	{
		for(x = 0; x < (int)m_nWidth; x++)
		{
			if(
				fread(&nB, 1, 1, pFile) != 1 ||
				fread(&nG, 1, 1, pFile) != 1 ||
				fread(&nR, 1, 1, pFile) != 1
				)
				return false;
			SetPixel(x, y, gRGB(nR, nG, nB));
		}
		int n = (x * 3) % 4;
		if(n > 0)
		{
			nR = 0;
			while(n < 4) // Align on word boundaries
			{
				fread(&nR, 1, 1, pFile);
				n++;
			}
		}
	}

	return true;
}

bool GImage::LoadBMPFile(const unsigned char* pRawData, int nLen)
{
	if(nLen < (int)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)))
		return false;
	BITMAPFILEHEADER* h1 = (BITMAPFILEHEADER*)pRawData;
	BITMAPINFOHEADER* h2 = (BITMAPINFOHEADER*)((char*)h1 + sizeof(BITMAPFILEHEADER));
	const unsigned char* pData = (const unsigned char*)((char*)h2 + sizeof(BITMAPINFOHEADER));
	if(h2->biBitCount != 24)
	{
		GAssert(false, "Sorry, this only supports 24-bit bitmaps");
		return false;
	}
	if(h2->biWidth < 1 || h2->biHeight < 1)
	{
		GAssert(false, "Sorry, this only supports bottom-up bitmaps");
		return false;
	}
	SetSize(h2->biWidth, h2->biHeight);
	// todo: make sure nLen is big enough to hold all the data
	int y;
	int x;
	unsigned char nR;
	unsigned char nG;
	unsigned char nB;
	for(y = (int)m_nHeight - 1; y >= 0; y--)
	{
		for(x = 0; x < (int)m_nWidth; x++)
		{
			nB = *(pData++);
			nG = *(pData++);
			nR = *(pData++);
			SetPixel(x, y, gRGB(nR, nG, nB));
		}
		int n = (x * 3) % 4;
		if(n > 0)
		{
			while(n < 4) // Align on word boundaries
			{
				pData++;
				n++;
			}
		}
	}

	return true;
}

bool GImage::Crop(int nLeft, int nTop, int nRight, int nBottom)
{
	nRight++;
	nBottom++;
	int nWidth = nRight - nLeft;
	int nHeight = nBottom - nTop;
	if(nWidth < 1 || nHeight < 1)
		return false;
	unsigned int nNewSize = nWidth * nHeight;
	GColor* pNewQuads = new GColor[nNewSize];
	
	// Copy overlapping parts of the pics
	GColor col;
	unsigned int nPos;
	int x, y;
	for(y = 0; y < (int)m_nHeight; y++)
	{
		if(y < nTop)
			continue;
		if(y >= nBottom)
			continue;
		for(x = 0; x < (int)m_nWidth; x++)
		{
			if(x < nLeft)
				continue;
			if(x >= nRight)
				continue;
			col = GetPixel(x, y);
			nPos = (nWidth * (y - nTop) + x - nLeft) << 2;
			pNewQuads[nPos] = col;
		}
	}

	// Replace the old buffer with the new one
	delete(m_pPixels);
	m_pPixels = pNewQuads;
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	return true;
}

void GImage::DrawBox(int nX1, int nY1, int nX2, int nY2, GColor color, bool bFill)
{
	if(bFill)
	{
		int x, y;
		for(x = nX1; x <= nX2; x++)
		{
			for(y = nY1; y <= nY2; y++)
				SetPixel(x, y, color);
		}
	}
	else
	{
		int n;
		for(n = nX1; n <= nX2; n++)
		{
			SetPixel(n, nY1, color);
			SetPixel(n, nY2, color);
		}
		for(n = nY1 + 1; n < nY2; n++)
		{
			SetPixel(nX1, n, color);
			SetPixel(nX2, n, color);
		}
	}
}

void GImage::SoftDrawBox(int nX1, int nY1, int nX2, int nY2, GColor color, double dOpacity)
{
	int x, y;
	for(x = nX1; x <= nX2; x++)
	{
		for(y = nY1; y <= nY2; y++)
			SoftSetPixel(x, y, color, dOpacity);
	}
}

void GImage::Scale(unsigned int nNewWidth, unsigned int nNewHeight)
{
	int nOldWidth = m_nWidth;
	int nOldHeight = m_nHeight;
	GImage tmpImage;
	tmpImage.SwapData(this);
	SetSize(nNewWidth, nNewHeight);
	GColor col;
	int x, y;
	for(y = 0; y < (int)nNewHeight; y++)
	{
		for(x = 0; x < (int)nNewWidth; x++)
		{
			col = tmpImage.InterpolatePixel((float)(x * nOldWidth) / nNewWidth, (float)(y * nOldHeight) / nNewHeight);
			SetPixel(x, y, col);
		}
	}
}

void GImage::FlipHorizontally()
{
	GColor c1;
	GColor c2;
	int x, y;
	int nHalfWidth = (int)m_nWidth >> 1;
	for(y = 0; y < (int)m_nHeight; y++)
	{
		for(x = 0; x < nHalfWidth; x++)
		{
			c1 = GetPixel(x, y);
			c2 = GetPixel(m_nWidth - 1 - x, y);
			SetPixel(x, y, c2);
			SetPixel(m_nWidth - 1 - x, y, c1);
		}
	}
}

void GImage::FlipVertically()
{
	GColor c1;
	GColor c2;
	int x, y;
	int nHalfHeight = (int)m_nHeight >> 1;
	for(x = 0; x < (int)m_nWidth; x++)
	{
		for(y = 0; y < nHalfHeight; y++)
		{
			c1 = GetPixel(x, y);
			c2 = GetPixel(x, m_nHeight - 1 - y);
			SetPixel(x, y, c2);
			SetPixel(x, m_nHeight - 1 - y, c1);
		}
	}
}

void GImage::SwapData(GImage* pSwapImage)
{
	GColor* pTmpRGBQuads = m_pPixels;
	unsigned int nTmpWidth = m_nWidth;
	unsigned int nTmpHeight = m_nHeight;
	m_pPixels = pSwapImage->m_pPixels;
	m_nWidth = pSwapImage->m_nWidth;
	m_nHeight = pSwapImage->m_nHeight;
	pSwapImage->m_pPixels = pTmpRGBQuads;
	pSwapImage->m_nWidth = nTmpWidth;
	pSwapImage->m_nHeight = nTmpHeight;
}

void GImage::Convolve(GImage* pKernel)
{
	GImage NewImage;
	NewImage.SetSize(m_nWidth, m_nHeight);
	GColor c1;
	GColor c2;
	int nHalfKWidth = (int)pKernel->m_nWidth >> 1;
	int nHalfKHeight = (int)pKernel->m_nHeight >> 1;
	int nRSum, nGSum, nBSum;
	int nRTot, nGTot, nBTot;
	nRTot = 0;
	nGTot = 0;
	nBTot = 0;
	int kx, ky;
	for(ky = 0; ky < (int)pKernel->m_nHeight; ky++)
	{
		for(kx = 0; kx < (int)pKernel->m_nWidth; kx++)
		{
			c1 = pKernel->GetPixel(kx, ky);
			nRTot += gRed(c1);
			nGTot += gGreen(c1);
			nBTot += gBlue(c1);
		}
	}
	int x, y;
	for(y = 0; y < (int)m_nHeight; y++)
	{
		for(x = 0; x < (int)m_nWidth; x++)
		{
			nRSum = 0;
			nGSum = 0;
			nBSum = 0;
			for(ky = 0; ky < (int)pKernel->m_nHeight; ky++)
			{
				for(kx = 0; kx < (int)pKernel->m_nWidth; kx++)
				{
					c1 = pKernel->GetPixel(pKernel->m_nWidth - kx - 1, pKernel->m_nHeight - ky - 1);
					c2 = SafeGetPixel(x + kx - nHalfKWidth, y + ky - nHalfKHeight);
					nRSum += gRed(c1) * gRed(c2);
					nGSum += gGreen(c1) * gGreen(c2);
					nBSum += gBlue(c1) * gBlue(c2);
				}
			}
			NewImage.SetPixel(x, y, gRGB(nRSum / nRTot, nGSum / nGTot, nBSum / nBTot));
		}
	}
	SwapData(&NewImage);
}

void GImage::ConvolveKernel(GImage* pKernel)
{
	GImage NewImage;
	NewImage.SetSize(m_nWidth, m_nHeight);
	GColor c1;
	GColor c2;
	int nHalfKWidth = (int)pKernel->m_nWidth >> 1;
	int nHalfKHeight = (int)pKernel->m_nHeight >> 1;
	int nRSum, nGSum, nBSum;
	int x, y;
	int kx, ky;
	for(y = 0; y < (int)m_nHeight; y++)
	{
		for(x = 0; x < (int)m_nWidth; x++)
		{
			nRSum = 0;
			nGSum = 0;
			nBSum = 0;
			for(ky = 0; ky < (int)pKernel->m_nHeight; ky++)
			{
				for(kx = 0; kx < (int)pKernel->m_nWidth; kx++)
				{
					c1 = pKernel->GetPixel(pKernel->m_nWidth - kx - 1, pKernel->m_nHeight - ky - 1);
					c2 = SafeGetPixel(x + kx - nHalfKWidth, y + ky - nHalfKHeight);
					nRSum += gRed(c1) * gRed(c2);
					nGSum += gGreen(c1) * gGreen(c2);
					nBSum += gBlue(c1) * gBlue(c2);
				}
			}
			nRSum = MIN(MAX(nRSum, 0), 255);
			nGSum = MIN(MAX(nGSum, 0), 255);
			nBSum = MIN(MAX(nBSum, 0), 255);
			NewImage.SetPixel(x, y, gRGB(nRSum, nGSum, nBSum));
		}
	}
	SwapData(&NewImage);
}

void GImage::Blur(double dFactor)
{
	// Calculate how big of a kernel we need
	int nFactor = (int)dFactor;
	int nWidth = nFactor * 2 + 1;
	GImage imgKernel;
	imgKernel.SetSize(nWidth, nWidth);

	// Produce the blurring kernel
	double dTmp = dFactor / 8;
	double d;
	int n;
	int x, y;
	for(y = 0; y < nWidth; y++)
	{
		for(x = 0; x < nWidth; x++)
		{
			d = pow((double)2, -(sqrt((double)((nFactor - x) * (nFactor - x) + (nFactor - y) * (nFactor - y))) / dTmp));
			n = MAX(0, MIN(255, (int)(d * 256)));
			imgKernel.SetPixel(x, y, gRGB(n, n, n));
		}
	}

	// Convolve the kernel with the image
	Convolve(&imgKernel);
}

void GImage::Sharpen(double dFactor)
{
	GImage imgBlurred;
	imgBlurred.CopyImage(this);
	imgBlurred.Blur(dFactor);
	GImage imgTmp;
	imgTmp.SetSize(m_nWidth, m_nHeight);
	GColor col;
	int nRed, nGreen, nBlue;
	int x, y;
	for(y = 0; y < (int)m_nHeight; y++)
	{
		for(x = 0; x < (int)m_nWidth; x++)
		{
			col = GetPixel(x, y);
			nRed = 2 * gRed(col);
			nGreen = 2 * gGreen(col);
			nBlue = 2 * gBlue(col);
			col = imgBlurred.GetPixel(x, y);
			nRed = MAX(0, MIN(255, (int)(nRed - gRed(col))));
			nGreen = MAX(0, MIN(255, (int)(nGreen - gGreen(col))));
			nBlue = MAX(0, MIN(255, (int)(nBlue - gBlue(col))));
			imgTmp.SetPixel(x, y, gRGB(nRed, nGreen, nBlue));
		}
	}
	SwapData(&imgTmp);
}

void GImage::Invert()
{
	int x, y;
	GColor col;
	for(y = 0; y < (int)m_nHeight; y++)
	{
		for(x = 0; x < (int)m_nWidth; x++)
		{
			col = GetPixel(x, y);
			SetPixel(x, y, gRGB(255 - gRed(col), 255 - gGreen(col), 255 - gBlue(col)));
		}
	}
}

void GImage::InvertRect(GRect* pRect)
{
	int x, y;
	GColor col;
	for(y = pRect->y; y < pRect->y + pRect->h; y++)
	{
		for(x = pRect->x; x < pRect->x + pRect->w; x++)
		{
			col = GetPixel(x, y);
			SetPixel(x, y, gRGB(255 - gRed(col), 255 - gGreen(col), 255 - gBlue(col)));
		}
	}
}

void GImage::MakeEdgesGlow(float fThresh, int nThickness, int nOpacity, GColor color)
{
	// Make initial mask
	GImage tmp;
	tmp.SetSize(m_nWidth, m_nHeight);
	GColor col, colLeft, colRight, colTop, colBottom;
	int x, y, dif;
	for(y = m_nHeight - 2; y > 0; y--)
	{
		for(x = m_nWidth - 2; x > 0; x--)
		{
			col = GetPixel(x, y);
			colLeft = GetPixel(x - 1, y);
			colRight = GetPixel(x + 1, y);
			colTop = GetPixel(x, y - 1);
			colBottom = GetPixel(x, y + 1);
			dif = ABS((int)gRed(col) - (int)gRed(colLeft)) + ABS((int)gGreen(col) - (int)gGreen(colLeft)) + ABS((int)gBlue(col) - (int)gBlue(colLeft)) +
				ABS((int)gRed(col) - (int)gRed(colRight)) + ABS((int)gGreen(col) - (int)gGreen(colRight)) + ABS((int)gBlue(col) - (int)gBlue(colRight)) +
				ABS((int)gRed(col) - (int)gRed(colTop)) + ABS((int)gGreen(col) - (int)gGreen(colTop)) + ABS((int)gBlue(col) - (int)gBlue(colTop)) +
				ABS((int)gRed(col) - (int)gRed(colBottom)) + ABS((int)gGreen(col) - (int)gGreen(colBottom)) + ABS((int)gBlue(col) - (int)gBlue(colBottom));
			if((float)dif / 3072 > fThresh)
				tmp.SetPixel(x, y, nThickness + 1);
		}
	}

	// Make the glowing
	int n;
	for(n = nThickness; n >= 0; n--)
	{
		for(y = m_nHeight - 2; y > 0; y--)
		{
			for(x = m_nWidth - 2; x > 0; x--)
			{
				col = tmp.GetPixel(x, y);
				if(col > (unsigned int)n)
				{
					tmp.SetPixel(x - 1, y, tmp.GetPixel(x - 1, y) | n);
					tmp.SetPixel(x + 1, y, tmp.GetPixel(x + 1, y) | n);
					tmp.SetPixel(x, y - 1, tmp.GetPixel(x, y - 1) | n);
					tmp.SetPixel(x, y + 1, tmp.GetPixel(x, y + 1) | n);
					col = MixColors(color, GetPixel(x, y), nOpacity);
					SetPixel(x, y, col);
				}
			}
		}
	}
}

void GImage::HorizDifferenceize()
{
	if(m_nWidth <= 1)
		return;
	GColor c1, c2;
	int x, y;
	for(y = 0; y < m_nHeight; y++)
	{
		c1 = GetPixel(0, y);
		for(x = 1; x < m_nWidth; x++)
		{
			c2 = GetPixel(x, y);
			SetPixel(x, y, gRGB(
				(256 + gRed(c2) - gRed(c1)) % 256,
				(256 + gGreen(c2) - gGreen(c1)) % 256,
				(256 + gBlue(c2) - gBlue(c1)) % 256));
			c1 = c2;
		}
	}
}

void GImage::HorizSummize()
{
	if(m_nWidth <= 1)
		return;
	GColor c1, c2;
	int x, y;
	for(y = 0; y < m_nHeight; y++)
	{
		c1 = GetPixel(0, y);
		for(x = 1; x < m_nWidth; x++)
		{
			c2 = GetPixel(x, y);
			SetPixel(x, y, gRGB(
				(256 + gRed(c2) + gRed(c1)) % 256,
				(256 + gGreen(c2) + gGreen(c1)) % 256,
				(256 + gBlue(c2) + gBlue(c1)) % 256));
			c1 = c2;
		}
	}
}

void GImage::Draw3DLine(const struct Point3D* pA, const struct Point3D* pB, struct Transform* pCamera, GColor color)
{
	struct Point3D a = *pA;
	struct Point3D b = *pB;
	a.Transform(pCamera);
	b.Transform(pCamera);
	SafeDrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
}

void GImage::DrawBezier(GBezier* pCurve, GColor color, double dStart, double dEnd, double dStep, struct Transform* pCamera)
{
	Transform t;
	Point3D prev;
	Point3D point;
	pCurve->GetPoint(dStart, &prev);
	point.Transform(pCamera);
	while(true)
	{
		dStart += dStep;
		if(dStart > dEnd)
			break;
		pCurve->GetPoint(dStart, &point);
		Draw3DLine(&point, &prev, pCamera, color);
		prev = point;
	}
}

int GImage::MeasureHardTextWidth(int height, const char* szText, float width)
{
	bool bBig = (height > 24);
	int sx, sy, sw, sh;
	int x = 0;
	int nCharWidth;
	while(true)
	{
		char c = *szText;
		if(c == '\0')
			break;
		if(bBig)
		{
			GHardFont_GetCharCoords(c, &sx, &sy, &sw, &sh);
			nCharWidth = (int)(sw * height * width) / sh;
		}
		else
		{
			GSmallHardFont_GetCharCoords(c, &sx, &sy, &sw, &sh);
			nCharWidth = sw;
		}
		x += nCharWidth;
		szText++;
	}
	return x;
}

int GImage::CountHardTextChars(int horizArea, int height, const char* szText, float width)
{
	bool bBig = (height > 24);
	int sx, sy, sw, sh;
	int x = 0;
	int nCharWidth;
	int nChars = 0;
	while(true)
	{
		char c = *szText;
		if(c == '\0')
			break;
		if(bBig)
		{
			GHardFont_GetCharCoords(c, &sx, &sy, &sw, &sh);
			nCharWidth = (int)(sw * height * width) / sh;
		}
		else
		{
			GSmallHardFont_GetCharCoords(c, &sx, &sy, &sw, &sh);
			nCharWidth = sw;
		}
		x += nCharWidth;
		szText++;
		if(x > horizArea)
			break;
		nChars++;
	}
	return nChars;
}

void GImage::DrawHardText(GRect* pRect, const char* szText, GColor col, float width)
{
	bool bBig = (pRect->h > 24);
	int sx, sy, sw, sh;
	int x = 0;
	while(true)
	{
		char c = *szText;
		if(c == '\0')
			break;
		if(bBig)
			GHardFont_GetCharCoords(c, &sx, &sy, &sw, &sh);
		else
		{
			GSmallHardFont_GetCharCoords(c, &sx, &sy, &sw, &sh);
			GAssert(sh <= 24, "problem with big threshold");
		}
		GAssert(pRect->x >= 0, "todo: this case not handled yet");
		int yStart = 0;
		if(pRect->y < 0)
			yStart = -pRect->y;
		if(bBig)
		{
			int h = pRect->h;
			if(pRect->y + h > m_nHeight)
				h = m_nHeight - pRect->y;
			int nCharWidth = (int)(sw * pRect->h * width) / sh;
			int n;
			for(n = 0; n < nCharWidth; n++)
			{
				if(x >= pRect->w)
					break;
				int y;
				for(y = yStart; y < h; y++)
					SetPixel(pRect->x + x, pRect->y + y, GHardFont_GetAlphaBlended(sx + n * sw / nCharWidth, sy + y * sh / pRect->h, col, GetPixel(pRect->x + x, pRect->y + y)));
				x++;
			}
		}
		else
		{
			if(pRect->y + sh > m_nHeight)
				sh = m_nHeight - pRect->y;
			int n;
			for(n = 0; n < sw; n++)
			{
				if(x >= pRect->w)
					break;
				int y;
				for(y = yStart; y < sh; y++)
					SetPixel(pRect->x + x, pRect->y + y, GSmallHardFont_GetAlphaBlended(sx + n, sy + y, col, GetPixel(pRect->x + x, pRect->y + y)));
				x++;
			}
		}
		szText++;
	}
}

void GImage::Blit(int x, int y, GImage* pSource, GRect* pSourceRect)
{
	int sx = pSourceRect->x;
	int sy = pSourceRect->y;
	int sw = pSourceRect->w;
	int sh = pSourceRect->h;
	if(x < 0)
	{
		sx -= x;
		sw += x;
		x = 0;
	}
	if(x + sw > m_nWidth)
		sw = m_nWidth - x;
	if(y < 0)
	{
		sy -= y;
		sh += y;
		y = 0;
	}
	if(y + sh > m_nHeight)
		sh = m_nHeight - y;
	int dst = y * m_nWidth + x;
	int src = sy * pSource->m_nWidth + sx;
	sw *= sizeof(GColor);
	for( ; sh > 0; sh--)
	{
		memcpy(&m_pPixels[dst], &pSource->m_pPixels[src], sw);
		dst += m_nWidth;
		src += pSource->m_nWidth;
	}
}

void GImage::AlphaBlit(int x, int y, GImage* pSource, GRect* pSourceRect)
{
	int sx = pSourceRect->x;
	int sy = pSourceRect->y;
	int sw = pSourceRect->w;
	int sh = pSourceRect->h;
	if(x < 0)
	{
		sx -= x;
		sw += x;
		x = 0;
	}
	if(x + sw > m_nWidth)
		sw = m_nWidth - x;
	if(y < 0)
	{
		sy -= y;
		sh += y;
		y = 0;
	}
	if(y + sh > m_nHeight)
		sh = m_nHeight - y;
	int dst = y * m_nWidth + x;
	int src = sy * pSource->m_nWidth + sx;
	int xx, a;
	GColor pix, pixOld;
	for( ; sh > 0; sh--)
	{
		for(xx = 0; xx < sw; xx++)
		{
			pix = pSource->m_pPixels[src + xx];
			a = gAlpha(pix);
			pixOld = m_pPixels[dst + xx];
			m_pPixels[dst + xx] = gRGB((a * gRed(pix) + (256 - a) * gRed(pixOld)) >> 8,
										(a * gGreen(pix) + (256 - a) * gGreen(pixOld)) >> 8,
										(a * gBlue(pix) + (256 - a) * gBlue(pixOld)) >> 8);
		}
		dst += m_nWidth;
		src += pSource->m_nWidth;
	}
}

GImage* GImage::Munge(int nStyle, float fExtent)
{
	GImage* pMunged = new GImage();
	pMunged->SetSize(m_nWidth, m_nHeight);
	int x, y;
	GColor col;
	double d;
	switch(nStyle)
	{
		case 0: // particle-blur (pick a random pixel in the proximity)
			for(y = 0; y < m_nHeight; y++)
			{
				for(x = 0; x < m_nWidth; x++)
				{
					col = SafeGetPixel(
										(int)(x + fExtent * m_nWidth * (GBits::GetRandomDouble() - .5)),
										(int)(y + fExtent * m_nHeight * (GBits::GetRandomDouble() - .5))
									);
					pMunged->SetPixel(x, y, col);
				}
			}
			break;

		case 1: // shadow threshold (throw out all pixels below a certain percent of the total brighness)
			{
				fExtent = 1 - fExtent;
				fExtent *= fExtent;
				fExtent *= fExtent;
				fExtent = 1 - fExtent;

				// Create the histogram data
				unsigned int pnHistData[256];
				int n;
				for(n = 0; n < 256; n++)
					pnHistData[n] = 0;
				unsigned int nSize = m_nWidth * m_nHeight;
				unsigned int nPos;
				unsigned int nGray;
				for(nPos = 0; nPos < nSize; nPos++)
				{
					nGray = ColorToGrayScale(m_pPixels[nPos]);
					pnHistData[gGreen(nGray)]++;
				}

				// Turn it into cumulative histogram data
				for(n = 1; n < 256; n++)
					pnHistData[n] += pnHistData[n - 1];

				// Find the cut-off
				unsigned int nCutOff = (unsigned int)(fExtent * pnHistData[255]);
				for(n = 0; n < 256 && pnHistData[n] < nCutOff; n++)
				{
				}

				// Copy all the data above the threshold
				for(y = 0; y < m_nHeight; y++)
				{
					for(x = 0; x < m_nWidth; x++)
					{
						col = GetPixel(x, y);
						nGray = gGreen(ColorToGrayScale(col));
						if(nGray > (unsigned int)n)
							pMunged->SetPixel(x, y, col);
					}
				}
			}
			break;

		case 2: // waves
			for(y = 0; y < m_nHeight; y++)
			{
				for(x = 0; x < m_nWidth; x++)
				{
					col = SafeGetPixel(
										(int)(x + fExtent * (m_nWidth / 2) * cos((double)x * 16 / m_nWidth)),
										(int)(y + fExtent * (m_nHeight / 2) * sin((double)y * 16 / m_nHeight))
									);
					pMunged->SetPixel(x, y, col);
				}
			}
			break;

		case 3: // waved in or out of the middle
				fExtent = 1 - fExtent;
				fExtent *= fExtent;
				fExtent = 1 - fExtent;
			for(y = 0; y < m_nHeight; y++)
			{
				for(x = 0; x < m_nWidth; x++)
				{
					d = atan2((double)(y - m_nHeight / 2), (double)(x - m_nWidth / 2));
					d = fExtent * cos(d * 5);
					col = SafeGetPixel(
										(int)((1 - d) * x + d * (m_nWidth / 2)),
										(int)((1 - d) * y + d * (m_nHeight / 2))
									);
					pMunged->SetPixel(x, y, col);
				}
			}
			break;
	}
	return pMunged;
}

