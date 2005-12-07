/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GSPHEREIMAGE_H__
#define __GSPHEREIMAGE_H__

#include "GMacros.h"
#include <math.h>
#include "GRayTrace.h"

class GSphereImagePixelEnumerator;
class QImage;
class GImage;
class G3DObject;

// Represents a spherical image with pixels evenly distributed on the surface of the sphere
class GSphereImage
{
friend class GSphereImagePixelEnumerator;
protected:
	int m_nQuarterGirth;
	double m_dHalfGirth;
	double m_dRadius;
	double m_dTheta;
	int m_nWidth;
	int m_nHeight;

public:
	GSphereImage(int nQuarterGirth); // nQuarterGirth is one fourth the circumference of the sphere
	virtual ~GSphereImage();

	// returns one fourth the circumference of the sphere	
	int GetQuarterGirth()
	{
		return m_nQuarterGirth;
	}

	// Returns the width of the flattened image that textures the sphere
	int GetWidth()
	{
		return m_nWidth;
	}

	// Returns the height of the flattened image that textures the sphere
	int GetHeight()
	{
		return m_nHeight;
	}

	// Returns the radius of the sphere
	double GetRadius()
	{
		return m_dRadius;
	}

	// Returns the angle between ajacent pixels
	double GetTheta()
	{
		return m_dTheta;
	}

	// This expects dLat to be between -PI/2 and PI/2 (inclusively), and dLon to be between -PI/2 to 3*PI/2 (inclusively)
	void GetFlattenedCoords(double dLon, double dLat, double* pdX, double* pdY);

	// This expects nX and nY to be somewhere on the flat image
	void GetSphereCoords(int nX, int nY, double* pdLon, double* pdLat);

protected:
	inline void ToFlat(double dLon, double dLat, double* pdX, double* pdY)
	{
		*pdY = (dLat * m_dHalfGirth) / PI;
		*pdX = (cos(dLat) * dLon * m_dHalfGirth) / PI;
	}

	inline void ToSphere(int nX, int nY, double* pdLon, double* pdLat)
	{
		*pdLat = (PI * (double)nY) / m_dHalfGirth;
		*pdLon = (PI * (double)nX) / (cos(*pdLat) * m_dHalfGirth);
	}
};


// This class iterates through the pixels in a GSphereImage
class GSphereImagePixelEnumerator
{
protected:
	GSphereImage* m_pSphere;
	int m_nX;
	int m_nY;
	int m_nFrontLeft;
	int m_nFrontRight;
	int m_nBackLeft;
	int m_nBackRight;

public:
	GSphereImagePixelEnumerator(GSphereImage* pSphere)
	{
		m_pSphere = pSphere;
		Reset();
	}

	void Reset()
	{
		m_nX = 0;
		m_nY = 0;
		RecalculateBounds();
	}

	bool GetNext(int* pnX, int* pnY);

protected:
	void RecalculateBounds();
};



class GPreRendered3DScreen : public GSphereImage
{
protected:
	GImage* m_pImage;
	double* m_pDepthMap;
	struct Transform m_camera;
	double m_dZoom;
	bool m_bFlatScreen;

public:
	GPreRendered3DScreen(int nSize);
	virtual ~GPreRendered3DScreen();

	GImage* GetFlatImage()
	{
		return m_pImage;
	}

	double* GetDepthMap()
	{
		return m_pDepthMap;
	}

	// These values are expressed in radians and can be in any range
	void SetCameraAngle(const struct Transform* pCamera, bool bFlatScreen);

	// x, and y are relative to the center of the screen
	void ScreenPixelToSphereCoords(int x, int y, double* pdLon, double* pdLat);

	// x, and y are relative to the center of the screen
	unsigned int GetPixel(int x, int y, double* pDepth);

	void Render(G3DObject* pUniverse, const Point3D* pCameraPos, int nAllowedSubRays, int nFrame, int nTotalFrames, double dLightThreshold, GImage* pBackgroundImage);
	bool save(FILE* pFile);
};

#define RENDER_CAMERA_DIST_FACTOR 20

class GPreRendered3DSprite : public GSphereImage
{
protected:
	GImage* m_pImages;
	double* m_pDepthMap;
	int m_nArraySize;
	int* m_pArray;
	int m_nImageSize;
	int m_nTotalImageWidth;

public:
	// Note: This will allocate a GImage, but it will not resize it.  It is your
	//       job to call GetImages and resize the object it returns as appropriate
	//       before you put pixels in it.  I realize that makes this a wierd class,
	//       but it suits my purposes for now.
	GPreRendered3DSprite(int nSize, int nImageSize);
	virtual ~GPreRendered3DSprite();

	GImage* GetImages()
	{
		return m_pImages;
	}

	double* GetDepthMap()
	{
		return m_pDepthMap;
	}

	int GetArraySize()
	{
		return m_nArraySize;
	}

	// Returns the total width of the big image of all the frames side-by-side
	int GetTotalImageWidth()
	{
		return m_nTotalImageWidth;
	}

	// Returns the width or height of a single image (they're always square)
	int GetImageSize()
	{
		return m_nImageSize;
	}

	// Pass in coordinates on the flattened sphere image and this will return
	// the horizontal offset where the corresponding frame begins in m_pImages.
	int GetHorizOffset(int nX, int nY)
	{
		int nIndex = nY * m_nWidth + nX;
		GAssert(nIndex >= 0 && nIndex < m_nArraySize, "out of range");
		return m_pArray[nIndex];
	}

	int GetHorizOffset(double dLon, double dLat)
	{
		double nX, nY;
		GetFlattenedCoords(dLon, dLat, &nX, &nY);
		return GetHorizOffset((int)nX, (int)nY);
	}

	void RenderFrames(G3DObject*pObj3D, int nFrame, int nFrames, double dSourceSize, GColor transparentColor, int nAllowedSubRays, double dLightThreshold);

protected:
	void RenderFrame(G3DObject*pObj3D, int nFrame, int nFrames, double dLon, double dLat, double dSourceSize, int nHorizOffset, GColor transparentColor, int nAllowedSubRays, double dLightThreshold);
};

#endif // __GSPHEREIMAGE_H__
