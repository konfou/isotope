/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GSphereImage.h"
#include "GImage.h"
#include "GCompress.h"

#define MAX_DIST 1000000000

GSphereImage::GSphereImage(int nQuarterGirth)
{
	m_nQuarterGirth = nQuarterGirth;
	m_nWidth = m_nQuarterGirth * 4 + 2;
	m_nHeight = m_nQuarterGirth * 2 + 1;
	m_dHalfGirth = (double)(2 * nQuarterGirth);
	m_dRadius = m_dHalfGirth / PI;
	m_dTheta = atan(1 / m_dRadius);
}

GSphereImage::~GSphereImage()
{
}

void GSphereImage::GetFlattenedCoords(double dLon, double dLat, double* pdX, double* pdY)
{
	GAssert(dLon >= -PI / 2 && dLon <= (PI * 3) / 2, "out of range (712)");
	GAssert(dLat >= -PI / 2 && dLat <= PI / 2, "out of range (713)");
	bool bBack = false;
	if(dLon >= (PI / 2))
	{
		dLon -= PI;
		bBack = true;
	}
	ToFlat(dLon, dLat, pdX, pdY);
	(*pdX) += m_nQuarterGirth;
	(*pdY) += m_nQuarterGirth;
	if(bBack)
		(*pdX) += (m_nQuarterGirth * 2 + 1);
	GAssert((*pdX) >= 0 && (*pdX) < m_nWidth, "out of range (714)");
	GAssert((*pdY) >= 0 && (*pdY) < m_nHeight, "out of range (715)");
}

void GSphereImage::GetSphereCoords(int nX, int nY, double* pdLon, double* pdLat)
{
	GAssert(nX >= 0 && nX < m_nWidth, "out of range (716)");
	GAssert(nY >= 0 && nY < m_nHeight, "out of range (717)");
	bool bBack = false;
	if(nX > m_nQuarterGirth + m_nQuarterGirth)
	{
		nX -= (m_nQuarterGirth + m_nQuarterGirth + 1);
		bBack = true;
	}
	nX -= m_nQuarterGirth;
	nY -= m_nQuarterGirth;
	ToSphere(nX, nY, pdLon, pdLat);
	if(bBack)
		(*pdLon) += PI;
	//GAssert(*pdLon >= -PI / 2 && *pdLon <= (PI * 3) / 2, "out of range (718)");
	//GAssert(*pdLat >= -PI / 2 && *pdLat <= PI / 2, "out of range (719)");
}

// ---------------------------------------------------------------------------

inline int RoundDoubleDown(double d)
{
	return (int)d;
}

inline int RoundDoubleUp(double d)
{
	int n = (int)d;
	if(d - (double)n > 0)
		n++;
	return n;
}

void GSphereImagePixelEnumerator::RecalculateBounds()
{
	double dLon, dLat;
	double dX, dY;
	m_pSphere->GetSphereCoords(0, m_nY, &dLon, &dLat);
	m_pSphere->GetFlattenedCoords(-PI / 2, dLat, &dX, &dY);
	m_nFrontLeft = RoundDoubleDown(dX);
	m_pSphere->GetFlattenedCoords((PI / 2) - .000001, dLat, &dX, &dY);
	m_nFrontRight = RoundDoubleUp(dX);
	GAssert(m_nFrontRight >= m_nFrontLeft, "bad math");
	m_pSphere->GetFlattenedCoords(PI / 2, dLat, &dX, &dY);
	m_nBackLeft = RoundDoubleDown(dX);
	GAssert(m_nBackLeft >= m_nFrontRight, "bad math");
	m_pSphere->GetFlattenedCoords((PI * 3) / 2, dLat, &dX, &dY);
	m_nBackRight = RoundDoubleUp(dX);
	GAssert(m_nBackRight >= m_nBackLeft, "bad math");
}

bool GSphereImagePixelEnumerator::GetNext(int* pnX, int* pnY)
{
	if(m_nY >= m_pSphere->m_nQuarterGirth * 2 + 1)
		return false;
	*pnX = m_nX;
	*pnY = m_nY;
	m_nX++;
	if(m_nX > m_nBackRight)
	{
		m_nY++;
		m_nX = 0;
		if(m_nY >= m_pSphere->m_nQuarterGirth * 2 + 1)
			return false;
		RecalculateBounds();
	}
	if(m_nX > m_nFrontRight && m_nX < m_nBackLeft)
		m_nX = m_nBackLeft;
	if(m_nX < m_nFrontLeft)
		m_nX = m_nFrontLeft;
	return true;
}

// ------------------------------------------------------------------------

GPreRendered3DScreen::GPreRendered3DScreen(int nSize) : GSphereImage(nSize)
{
	m_pImage = new GImage();
	m_pImage->SetSize(GetWidth(), GetHeight());
	m_pDepthMap = new double[GetWidth() * GetHeight()];
	GAssert(m_pDepthMap, "out of memory");
	Transform t;
	SetCameraAngle(&t, true);
}

GPreRendered3DScreen::~GPreRendered3DScreen()
{
	delete [] m_pDepthMap;
	delete(m_pImage);
}

void GPreRendered3DScreen::SetCameraAngle(const struct Transform* pCamera, bool bFlatScreen)
{
	m_camera = *pCamera;
	m_dZoom = pCamera->dScale * m_dRadius;
	m_camera.dScale = 1;
	m_bFlatScreen = bFlatScreen;
}

void GPreRendered3DScreen::ScreenPixelToSphereCoords(int x, int y, double* pdLon, double* pdLat)
{
	// Zoom
	Point3D point;
	if(m_bFlatScreen)
	{
		point.x = (double)x / m_dZoom;
		point.y = (double)y / m_dZoom;
		point.z = m_dRadius;
	}
	else
	{
		point.x = m_dRadius * tan((double)x / m_dZoom);
		point.y = m_dRadius * tan((double)y / m_dZoom);
		point.z = m_dRadius;
	}

	point.Transform(&m_camera);

	// Convert to sphere coords
	point.GetLatLon(pdLat, pdLon);

	GAssert(*pdLat <= PI / (double)2 && *pdLat >= -PI / (double)2, "out of range (710)");
	GAssert(*pdLon <= 3 * PI / (double)2 && *pdLon >= -PI / (double)2, "out of range (711)");
}

unsigned int GPreRendered3DScreen::GetPixel(int x, int y, double* pDepth)
{
	double dLon, dLat;
	ScreenPixelToSphereCoords(x, y, &dLon, &dLat);
	double dX, dY;
	GetFlattenedCoords(dLon, dLat, &dX, &dY);
	// todo: interpolate the pixel
	int nImageX = (int)(dX + .5);
	int nImageY = (int)(dY + .5);
	*pDepth = m_pDepthMap[nImageY * m_nWidth + nImageX];
	return m_pImage->GetPixel(nImageX, nImageY);
}

void GPreRendered3DScreen::Render(G3DObject* pUniverse, const Point3D* pCameraPos, int nAllowedSubRays, int nFrame, int nTotalFrames, double dLightThreshold, GImage* pBackgroundImage)
{
	m_pImage->SetSize(GetWidth(), GetHeight());
	GSphereImagePixelEnumerator enumerator(this);
	Ray ray;
	ray.point = *pCameraPos;
	GColor c;
	RayHitData results;
	double dLon, dLat;
	int nFlattenedX, nFlattenedY;
	while(enumerator.GetNext(&nFlattenedX, &nFlattenedY))
	{
		GetSphereCoords(nFlattenedX, nFlattenedY, &dLon, &dLat);
		ray.vDirection.FromLonLat(dLon, dLat);
		pUniverse->FireRay(&ray, &results, nFrame, nTotalFrames);
		if(results.distance > 0)
		{
			Light l = results.MeasureLight(pUniverse, nAllowedSubRays);
			c = l.ToPixel(dLightThreshold);
		}
		else
		{
			if(pBackgroundImage)
				c = pBackgroundImage->InterpolatePixel(((float)dLon + (float)PI / 2) / ((float)PI + (float)PI) * (pBackgroundImage->GetWidth() - 1), ((float)dLat + (float)PI / 2) / (float)PI * (pBackgroundImage->GetHeight() - 1));
			else
				c = 0;
			results.distance = MAX_DIST;
		}
		m_pImage->SetPixel(nFlattenedX, nFlattenedY, c);
		m_pDepthMap[m_nWidth * nFlattenedY + nFlattenedX] = results.distance;
	}
}

bool GPreRendered3DScreen::save(FILE* pFile)
{
	if(fwrite(&m_nQuarterGirth, sizeof(int), 1, pFile) != 1)
		return false;
	if(!m_pImage->SavePIGFile(pFile))
		return false;
	int nNewSize;
	unsigned char* pDM = GCompress::Compress((unsigned char*)m_pDepthMap, GetWidth() * GetHeight() * sizeof(double), &nNewSize);
	if(!pDM)
		return false;
	if(fwrite(&nNewSize, sizeof(int), 1, pFile) != 1)
	{
		delete(pDM);
		return false;
	}
	if(fwrite(pDM, nNewSize, 1, pFile) != 1)
	{
		delete(pDM);
		return false;
	}
	delete(pDM);
	return true;
}

// ---------------------------------------------------------------------

GPreRendered3DSprite::GPreRendered3DSprite(int nSize, int nImageSize) : GSphereImage(nSize)
{
	// Initialize the array that indexes into the images
	m_nImageSize = nImageSize;
	m_nArraySize = m_nWidth * m_nHeight;
	m_pArray = new int[m_nArraySize];
	memset(m_pArray, '\0', m_nArraySize * sizeof(void*));
	GSphereImagePixelEnumerator enumerator(this);
	int nFlattenedX, nFlattenedY;
	int nOffset = 0;
	while(enumerator.GetNext(&nFlattenedX, &nFlattenedY))
	{
		int nElement = nFlattenedY * m_nWidth + nFlattenedX;
		m_pArray[nElement] = nOffset;
		nOffset += nImageSize;
	}
	m_nTotalImageWidth = nOffset;
	m_pImages = new GImage();
	m_pImages->SetSize(m_nTotalImageWidth, m_nImageSize);
	m_pDepthMap = new double[m_nTotalImageWidth * m_nImageSize];
	GAssert(m_pDepthMap, "out of memory");
}

GPreRendered3DSprite::~GPreRendered3DSprite()
{
	delete [] m_pDepthMap;
	delete(m_pImages);
	delete(m_pArray);
}

void GPreRendered3DSprite::RenderFrame(G3DObject*pObj3D, int nFrame, int nFrames, double dLon, double dLat, double dSourceSize, int nHorizOffset, GColor transparentColor, int nAllowedSubRays, double dLightThreshold)
{
	// Calculate camera pos
	Point3D cameraPos;
	double dCosLat = cos(dLat);
	double dSinLat = sin(dLat);
	double dCosLon = cos(dLon);
	double dSinLon = sin(dLon);
	cameraPos.x = RENDER_CAMERA_DIST_FACTOR * dSourceSize * dSinLon * dCosLat;
	cameraPos.y = RENDER_CAMERA_DIST_FACTOR * dSourceSize * dSinLat;
	cameraPos.z = RENDER_CAMERA_DIST_FACTOR * dSourceSize * dCosLon * dCosLat;

	// Render each pixel
	Ray ray;
	RayHitData rhd;
	int x, y;
	for(y = 0; y < m_nImageSize; y++)
	{
		for(x = 0; x < m_nImageSize; x++)
		{
			// Make target point
			Point3D target;
			target.x = dSourceSize * (double)(x - (m_nImageSize / 2)) / m_nImageSize;
			target.y = dSourceSize * (double)(y - (m_nImageSize / 2)) / m_nImageSize;

			// Transform target by latitude
			target.z = -target.y * dSinLat;
			target.y = target.y * dCosLat;

			// Transoform target by longitude
			double dTmp = target.x;
			target.x = target.x * dCosLon + target.z * dSinLon;
			target.z = target.z * dCosLon - dTmp * dSinLon;

			// Make the ray
			ray.point = cameraPos;
			ray.vDirection.FromAToB(&cameraPos, &target);
			ray.vDirection.Normalize();

			// Fire the ray
			pObj3D->FireRay(&ray, &rhd, nFrame, nFrames);
			if(rhd.distance > 0)
			{
				Light l = rhd.MeasureLight(pObj3D, nAllowedSubRays);
				m_pImages->SetPixel(x + nHorizOffset, y, l.ToPixel(dLightThreshold));
			}
			else
			{
				m_pImages->SetPixel(x + nHorizOffset, y, transparentColor);
				rhd.distance = MAX_DIST;
			}
			m_pDepthMap[y * m_nTotalImageWidth + x + nHorizOffset] = rhd.distance;
		}
	}
}

void GPreRendered3DSprite::RenderFrames(G3DObject*pObj3D, int nFrame, int nFrames, double dSourceSize, GColor transparentColor, int nAllowedSubRays, double dLightThreshold)
{
	int nHorizOffset = 0;
	double dLon, dLat;
	int nFlattenedX, nFlattenedY;
	GSphereImagePixelEnumerator enumerator(this);
	while(enumerator.GetNext(&nFlattenedX, &nFlattenedY))
	{
		GetSphereCoords(nFlattenedX, nFlattenedY, &dLon, &dLat);
		RenderFrame(pObj3D, nFrame, nFrames, dLon, dLat, dSourceSize, nHorizOffset, transparentColor, nAllowedSubRays, dLightThreshold);
		nHorizOffset += m_nImageSize;
	}
}
