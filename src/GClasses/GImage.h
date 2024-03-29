/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GIMAGE_H__
#define __GIMAGE_H__

#include <stdio.h>
#include "GMacros.h"

class GBezier;

// Note: This is a 4-channel color of the form 0xAARRGGBB.
// 0 is completely transparent.
// 0xff000000 is opaque black.
// 0xffffffff is opaque white.
typedef unsigned int GColor;

#define gBlue(c) ((c) & 0xff)
#define gGreen(c) (((c) >> 8) & 0xff)
#define gRed(c) (((c) >> 16) & 0xff)
#define gAlpha(c) (((c) >> 24) & 0xff)

inline GColor gRGB(int r, int g, int b)
{
	return ((b & 0xff) | ((g & 0xff) << 8) | ((r & 0xff) << 16) | 0xff000000);
}

inline GColor gARGB(int a, int r, int g, int b)
{
	return ((b & 0xff) | ((g & 0xff) << 8) | ((r & 0xff) << 16) | ((a & 0xff) << 24));
}

inline GColor MixColors(GColor a, GColor b, int nRatio)
{
	int n2 = 256 - nRatio;
	return gARGB(
		(gAlpha(a) * nRatio + gAlpha(b) * n2) >> 8,
		(gRed(a) * nRatio + gRed(b) * n2) >> 8,
		(gGreen(a) * nRatio + gGreen(b) * n2) >> 8,
		(gBlue(a) * nRatio + gBlue(b) * n2) >> 8
		);
}

inline GColor GetSpectrumColor(float f)
{
	int n = ((int)(f * 768)) % 768;
	int r = MIN(MAX(0, 512 - (n + n)) + MAX(0, n + n - 1024), 255);
	int g = MIN(MAX(0, 512 - (ABS(n - 256) << 1)), 255);
	int b = MIN(MAX(0, 512 - (ABS(n - 512) << 1)), 255);
	return gARGB(0xff, r, g, b);
}

inline GColor MultiplyBrightness(GColor c, float f)
{
	return gARGB(gAlpha(c),
		MIN(MAX(0, (int)(f * gRed(c))), 255),
		MIN(MAX(0, (int)(f * gGreen(c))), 255),
		MIN(MAX(0, (int)(f * gBlue(c))), 255));
}

struct GRect
{
	int x;
	int y;
	int w;
	int h;

	void Set(int _x, int _y, int _w, int _h)
	{
		x = _x;
		y = _y;
		w = _w;
		h = _h;
	}

	bool DoesInclude(int _x, int _y)
	{
		if(_x >= x && _y >= y && _x < x + w && _y < y + h)
			return true;
		else
			return false;
	}
};

// Represents an image
class GImage
{
protected:
	GColor* m_pPixels;
	int m_nWidth;
	int m_nHeight;

	bool LoadPixMap(FILE* pFile, bool bTextData, bool bGrayScale);
	bool SavePixMap(FILE* pFile, bool bTextData, bool bGrayScale);
	void FloodFillRecurser(int nX, int nY, unsigned char nSrcR, unsigned char nSrcG, unsigned char nSrcB, unsigned char nDstR, unsigned char nDstG, unsigned char nDstB, int nTolerance);

public:
	GImage();
	virtual ~GImage();

	// Load the image from a PNG as raw data
	bool LoadPNGFile(const unsigned char* pRawData, int nBytes);

	// Load the image from a PNG file
	bool LoadPNGFile(const char* szFilename);

	// Load the image from a BMP file
	bool LoadBMPFile(const char* szFilename);

	// Load the image from a BMP stream
	bool LoadBMPFile(FILE* pFile);

	// Load the image from a BMP raw data
	bool LoadBMPFile(const unsigned char* pRawData, int nLen);

	// Save the image to a BMP file
	bool SaveBMPFile(const char* szFilename);

	// Save the image to a PIG file
	bool SavePIGFile(const char* szFilename);

	// Save the image to a PIG file
	bool SavePIGFile(FILE* pFile);

	// Save the image from a PIG file
	bool LoadPIGFile(const unsigned char* pBytes, int nSize);

	// Save the image from a PPM file
	bool LoadPPMFile(const char* szFilename);

	// Save the image to a PPM file
	bool SavePPMFile(const char* szFilename);

	// Load the image from a PGM file
	bool LoadPGMFile(const char* szFilename);

	// Save the image to a PGM file
	bool SavePGMFile(const char* szFilename);

	// Save the image as a PNG to a stream
	bool GImage::SavePNGFile(FILE* pFile);

	// Save the image as a PNG to a file
	bool GImage::SavePNGFile(const char* szFilename);

	// Drawing Primitives
	inline void SetPixel(int nX, int nY, GColor color)
	{
		GAssert(nX >= 0 && nX < (int)m_nWidth && nY >= 0 && nY < (int)m_nHeight, "out of range (730)");
		m_pPixels[m_nWidth * nY + nX] = color;
	}

	// Get a pixel
	inline GColor GetPixel(int nX, int nY)
	{
		GAssert(nX >= 0 && nX < (int)m_nWidth && nY >= 0 && nY < (int)m_nHeight, "out of range (731)");
		return m_pPixels[m_nWidth * nY + nX];
	}

	// Set a pixel (may be out of range of the image)
	void SafeSetPixel(int nX, int nY, GColor color);
	
	// Draw a translucent pixel
	void SoftSetPixel(int nX, int nY, GColor color, double dOpacity);

	// Get a pixel (may be out of range of the image)
	GColor SafeGetPixel(int nX, int nY);

	// Returns an interpolated pixel
	GColor InterpolatePixel(float dX, float dY);

	// Draw a line
	void DrawLine(int nX1, int nY1, int nX2, int nY2, GColor color);

	// Draw a line (may include parts outside the bounds of the image)
	void SafeDrawLine(int nX1, int nY1, int nX2, int nY2, GColor color);

	// Draw an anti-aliassed line
	void DrawLineAntiAlias(int nX1, int nY1, int nX2, int nY2, GColor color);

	// Draw a box
	void DrawBox(int nX1, int nY1, int nX2, int nY2, GColor color, bool bFill);

	// Draw a translucent box
	void SoftDrawBox(int nX1, int nY1, int nX2, int nY2, GColor color, double dOpacity);

	// Draw a circle
	void DrawCircle(int nX, int nY, double dRadius, GColor color);
	
	// Draw an ellipse
	void DrawEllipse(int nX, int nY, double dRadius, double dHeightToWidthRatio, GColor color);

	// Tolerant flood fill
	void FloodFill(int nX, int nY, GColor color, int nTolerance);

	// Tolerant boundary fill
	void BoundaryFill(int nX, int nY, unsigned char nBoundaryR, unsigned char nBoundaryG, unsigned char nBoundaryB, unsigned char nFillR, unsigned char nFillG, unsigned char nFillB, int nTolerance);

	// Draws a filled-in triangle
	void FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, GColor c);

	// Draws a polygon
	void DrawPolygon(int nPoints, int* pnPointArray, GColor color);

	// Determines how many pixels of width are required to print a line of text
	int MeasureHardTextWidth(int height, const char* szText, float width);

	// Counts the number of characters that can be printed in the given horizArea
	int CountHardTextChars(int horizArea, int height, const char* szText, float width);

	// Draws some text (using a built-in hard-coded font)
	void DrawHardText(GRect* pRect, const char* szText, GColor col, float width);

	// Draw a line based on 3D coordinates and the specified camera
	void Draw3DLine(const struct Point3D* pA, const struct Point3D* pB, struct Transform* pCamera, GColor color);

	// Draw a bezier curve based on 3D coordinates and the specified camera
	void DrawBezier(GBezier* pCurve, GColor color, double dStart, double dEnd, double dStep, struct Transform* pCamera);

	// Fill the entire image with a single color
	void Clear(GColor color);

	// Flip the image horizontally
	void FlipHorizontally();

	// Flip the image vertically
	void FlipVertically();

	// Rotate the image around the specified point
	void Rotate(GImage* pSourceImage, int nX, int nY, double dAngle);

	// Erase the image and resize it
	void SetSize(int nWidth, int nHeight);

	// Scale the image
	void Scale(unsigned int nNewWidth, unsigned int nNewHeight);
	
	// Crop.  You can crop bigger by using values outside the picture
	bool Crop(int nLeft, int nTop, int nRight, int nBottom);

	// Converts the image to gray scale
	void ConvertToGrayScale();

	// Equalizes the color histogram
	void EqualizeColorSpread();

	// Locally equalize the color histogram
	void LocallyEqualizeColorSpread(int nLocalSize, float fExtent = 1);

	// Blur the image
	void Blur(double dFactor);

	// Sharpen the image
	void Sharpen(double dFactor);

	// Inverts the pixels in the image
	void Invert();

	// Inverts the pixels in a particular rect
	void InvertRect(GRect* pRect);

	// Finds edges and makes them glow
	void MakeEdgesGlow(float fThresh, int nThickness, int nOpacity, GColor color);

	void Convolve(GImage* pKernel);

	void ConvolveKernel(GImage* pKernel);

	void HorizDifferenceize();
	void HorizSummize();

	void SwapData(GImage* pSwapImage);
	GColor* GetRGBQuads() { return m_pPixels; }
	void CopyImage(GImage* pSourceImage);
	void CopyImage(GImage* pSourceImage, int nLeft, int nTop, int nRight, int nBottom);
	void PasteImage(int nX, int nY, GImage* pSourceImage, GColor colTransparent);
	void PasteImage(int nX, int nY, GImage* pSourceImage);

	// Returns the width of the image in pixels
	unsigned int GetWidth() { return m_nWidth; }

	// Returns the height of the image in pixels
	unsigned int GetHeight() { return m_nHeight; }

	// Blit an image into this image. The source rect must be within the source image.
	// The dest area can be out of the dest image. The alpha channel is ignored.
	void Blit(int x, int y, GImage* pSource, GRect* pSourceRect);

	// Blit an image into this image. The source rect must be within the source image.
	// The dest area can be out of the dest image. Also performs alpha blending.
	void AlphaBlit(int x, int y, GImage* pSource, GRect* pSourceRect);

	// Analysis Tools
	void CreateBrightnessHistogram(GImage* pOutImage);

	// Munges the image. nStyle should be 0, 1, 2, or 3. Each value munges a different way.
	// fExtent should be between 0 and 1, where 0 doesn't change much and 1 totally munges it.
	// You're responsible to delete the munged image this returns
	GImage* Munge(int nStyle, float fExtent);

};

#endif // __GIMAGE_H__
