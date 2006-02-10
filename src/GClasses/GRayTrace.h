/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GRAYTRACE_H__
#define __GRAYTRACE_H__

#include <stdio.h>
#include <math.h>
#include "GArray.h"
#include "GMacros.h"
#include "GImage.h"

class GRayTraceLight;
class GRayTraceMaterial;
class GRayTraceObject;
class GRayTraceScene;

typedef float GRayTraceReal;

// This class represents a color. It's more precise than GColor, but takes up more
// memory. Note that the ray tracer ignores the alpha channel because the material
// class has its own translucency value.
class GRayTraceColor
{
public:
	GRayTraceReal a, r, g, b;

	GRayTraceColor()
	{
		a = (GRayTraceReal)1;
		r = (GRayTraceReal)0;
		g = (GRayTraceReal)0;
		b = (GRayTraceReal)0;
	}

	GRayTraceColor(GRayTraceColor* pThat)
	{
		a = pThat->a;
		r = pThat->r;
		g = pThat->g;
		b = pThat->b;
	}

	GRayTraceColor(GColor c)
	{
		a = ((GRayTraceReal)gAlpha(c)) / 255;
		r = ((GRayTraceReal)gRed(c)) / 255;
		g = ((GRayTraceReal)gGreen(c)) / 255;
		b = ((GRayTraceReal)gBlue(c)) / 255;
	}

	GRayTraceColor(GRayTraceReal alpha, GRayTraceReal red, GRayTraceReal green, GRayTraceReal blue)
	{
		Set(alpha, red, green, blue);
	}

	bool IsBlack()
	{
		if(r == 0 && g == 0 && b == 0)
			return true;
		else
			return false;
	}

	void Set(GRayTraceReal alpha, GRayTraceReal red, GRayTraceReal green, GRayTraceReal blue)
	{
		a = alpha;
		r = red;
		g = green;
		b = blue;
	}

	void Copy(GRayTraceColor* pThat)
	{
		a = pThat->a;
		r = pThat->r;
		g = pThat->g;
		b = pThat->b;
	}

	void Add(GRayTraceColor* pThat)
	{
		a = MAX(a, pThat->a);
		r = MIN((GRayTraceReal)1, r + pThat->r);
		g = MIN((GRayTraceReal)1, g + pThat->g);
		b = MIN((GRayTraceReal)1, b + pThat->b);
	}

	void Multiply(GRayTraceReal mag)
	{
		r *= mag;
		g *= mag;
		b *= mag;
	}

	void Multiply(GRayTraceColor* pThat)
	{
		a *= pThat->a;
		r *= pThat->r;
		g *= pThat->g;
		b *= pThat->b;
	}

	GColor GetGColor()
	{
		return gARGB((int)(a * 255), (int)(r * 255), (int)(g * 255), (int)(b * 255));
	}
};





class GRayTraceVector
{
public:
	GRayTraceReal m_vals[3];

	GRayTraceVector()
	{
		m_vals[0] = 0;
		m_vals[1] = 0;
		m_vals[2] = 0;
	}

	GRayTraceVector(const GRayTraceVector* pThat)
	{
		m_vals[0] = pThat->m_vals[0];
		m_vals[1] = pThat->m_vals[1];
		m_vals[2] = pThat->m_vals[2];
	}

	GRayTraceVector(GRayTraceReal x, GRayTraceReal y, GRayTraceReal z)
	{
		m_vals[0] = x;
		m_vals[1] = y;
		m_vals[2] = z;
	}

	void Copy(const GRayTraceVector* pThat)
	{
		m_vals[0] = pThat->m_vals[0];
		m_vals[1] = pThat->m_vals[1];
		m_vals[2] = pThat->m_vals[2];
	}

	void Copy(GRayTraceReal x, GRayTraceReal y, GRayTraceReal z)
	{
		m_vals[0] = x;
		m_vals[1] = y;
		m_vals[2] = z;
	}

	void Set(GRayTraceReal x, GRayTraceReal y, GRayTraceReal z)
	{
		m_vals[0] = x;
		m_vals[1] = y;
		m_vals[2] = z;
	}

	inline void Normalize()
	{
		GRayTraceReal mag = (GRayTraceReal)sqrt(GetMagnitudeSquared());
		m_vals[0] /= mag;
		m_vals[1] /= mag;
		m_vals[2] /= mag;
	}

	inline GRayTraceReal GetDistanceSquared(const GRayTraceVector* pThat) const
	{
		return (pThat->m_vals[0] - m_vals[0]) * (pThat->m_vals[0] - m_vals[0]) +
			(pThat->m_vals[1] - m_vals[1]) * (pThat->m_vals[1] - m_vals[1]) +
			(pThat->m_vals[2] - m_vals[2]) * (pThat->m_vals[2] - m_vals[2]);
	}

	inline double GetMagnitudeSquared() const
	{
		return (m_vals[0] * m_vals[0] + m_vals[1] * m_vals[1] + m_vals[2] * m_vals[2]);
	}

	inline void Add(const GRayTraceVector* pThat)
	{
		m_vals[0] += pThat->m_vals[0];
		m_vals[1] += pThat->m_vals[1];
		m_vals[2] += pThat->m_vals[2];
	}

	inline void Add(GRayTraceReal mag, const GRayTraceVector* pThat)
	{
		m_vals[0] += mag * pThat->m_vals[0];
		m_vals[1] += mag * pThat->m_vals[1];
		m_vals[2] += mag * pThat->m_vals[2];
	}

	inline void Subtract(const GRayTraceVector* pThat)
	{
		m_vals[0] -= pThat->m_vals[0];
		m_vals[1] -= pThat->m_vals[1];
		m_vals[2] -= pThat->m_vals[2];
	}

	inline void Multiply(GRayTraceReal mag)
	{
		m_vals[0] *= mag;
		m_vals[1] *= mag;
		m_vals[2] *= mag;
	}

	inline GRayTraceReal DotProduct(const GRayTraceVector* pThat)
	{
		return m_vals[0] * pThat->m_vals[0] +
			m_vals[1] * pThat->m_vals[1] +
			m_vals[2] * pThat->m_vals[2];
	}

	inline void CrossProduct(GRayTraceVector* pA, GRayTraceVector* pB)
	{
		m_vals[0] = pA->m_vals[1] * pB->m_vals[2] - pA->m_vals[2] * pB->m_vals[1];
		m_vals[1] = pA->m_vals[2] * pB->m_vals[0] - pA->m_vals[0] * pB->m_vals[2];
		m_vals[2] = pA->m_vals[0] * pB->m_vals[1] - pA->m_vals[1] * pB->m_vals[0];
	}

	inline void ComputeTriangleNormal(const GRayTraceVector* pPoint1, const GRayTraceVector* pPoint2, const GRayTraceVector* pPoint3)
	{
		GRayTraceVector a(pPoint2);
		a.Subtract(pPoint1);
		GRayTraceVector b(pPoint3);
		b.Subtract(pPoint1);
		CrossProduct(&a, &b);
		Normalize();
	}

	void ComputePlaneEquasion(const GRayTraceVector* pPoint1, const GRayTraceVector* pPoint2, const GRayTraceVector* pPoint3, GRayTraceReal* pOutD)
	{
		ComputeTriangleNormal(pPoint1, pPoint2, pPoint3);
		*pOutD = -(this->DotProduct(pPoint1));
	}

	void ComputeReflectionVector(GRayTraceVector* pRay, GRayTraceVector* pNormal);

	// dLon and dLat are in radians
	inline void GetLatLon(double* pdLat, double* pdLon) const
	{
		*pdLat = atan2(m_vals[1], sqrt(m_vals[0] * m_vals[0] + m_vals[2] * m_vals[2]));
		*pdLon = atan2(m_vals[0], m_vals[2]);
		if((*pdLon) < (-PI / (double)2))
			(*pdLon) += (2 * PI);
	}

	// dLon and dLat are in radians
	//inline void FromLonLat(double dLon, double dLat)
	//{
	//	dY = sin(dLat);
	//	double dTmp = cos(dLat);
	//	dX = dTmp * sin(dLon);
	//	dZ = dTmp * cos(dLon);
	//}

	//inline double GetSimilarity(const Vector* pThat) const
	//{
	//	return(
	//			(dX * pThat->dX + dY * pThat->dY + dZ * pThat->dZ) / 
	//			(
	//				sqrt(dX * dX + dY * dY + dZ * dZ) * 
	//				sqrt(pThat->dX * pThat->dX + pThat->dY * pThat->dY + pThat->dZ * pThat->dZ)
	//			)
	//		);
	//}
};





class GRayTraceCamera
{
protected:
	GRayTraceVector m_lookFromPoint;
	GRayTraceVector m_lookAtPoint;
	GRayTraceVector m_viewUpVector;
	GRayTraceReal m_viewAngle; // in radians
	int m_maxDepth;
	int m_nRaysPerPixel;
	int m_nWidth, m_nHeight;

public:
	GRayTraceCamera()
	{
		m_lookFromPoint.Set(2, 0, 0);
		m_lookAtPoint.Set(0, 0, 0);
		m_viewUpVector.Set(0, 1, 0);
		m_viewAngle = (GRayTraceReal)(PI * 3 / 4);
		m_nRaysPerPixel = 1;
		m_maxDepth = 4;
		m_nWidth = 320;
		m_nHeight = 320;
	}

	~GRayTraceCamera()
	{
	}

	void SetRaysPerPixel(int n) { m_nRaysPerPixel = n; }
	GRayTraceVector* GetLookFromPoint() { return &m_lookFromPoint; }
	GRayTraceVector* GetLookAtPoint() { return &m_lookAtPoint; }
	GRayTraceVector* GetViewUpVector() { return &m_viewUpVector; }
	void SetViewAngle(GRayTraceReal val) { m_viewAngle = val; }
	GRayTraceReal GetViewAngle() { return m_viewAngle; }
	void SetMaxDepth(int val) { m_maxDepth = val; }
	void SetImageSize(int width, int height) { m_nWidth = width; m_nHeight = height; }
	int GetImageWidth() { return m_nWidth; }
	int GetImageHeight() { return m_nHeight; }
	int GetMaxDepth() { return m_maxDepth; }
};




class GRayTraceRay
{
public:
	GRayTraceVector m_collisionPoint;
	GRayTraceVector m_normalVector;
	GRayTraceVector m_reflectionVector;
	GRayTraceColor m_color;
	GRayTraceReal m_indexOfRefraction;

	GRayTraceRay();
	GRayTraceRay(GRayTraceRay* pThat);
	~GRayTraceRay();

	void Cast(GRayTraceScene* pScene, GRayTraceVector* pRayOrigin, GRayTraceVector* pDirectionVector, int nMaxDepth);
	bool ComputeTransmissionVector(GRayTraceVector* pDirectionVector, GRayTraceVector* pTransmissionVector, GRayTraceReal oldIndexOfRefraction, GRayTraceReal newIndexOfRefraction);
	GRayTraceObject* FindClosestIntersection(GRayTraceScene* pScene, GRayTraceVector* pRayOrigin, GRayTraceVector* pDirectionVector, GRayTraceReal* pOutDistance);
};




class GRayTraceScene
{
protected:
	GRayTraceColor m_backgroundColor;
	GRayTraceColor m_ambientLight;
	GPointerArray* m_pMaterials;
	GPointerArray* m_pObjects;
	GPointerArray* m_pLights;
	GRayTraceCamera* m_pCamera;

	// Rendering values
	GImage* m_pImage;
	int m_nY;
	GRayTraceVector m_pixSide;;
	GRayTraceVector m_pixDX;
	GRayTraceVector m_pixDY;

public:
	GRayTraceScene();
	~GRayTraceScene();

	// Call this to render the whole image in one pass
	void Render();

	// Call this before calling RenderLine(). It resets the image and
	// computes values necessary for rendering.
	void RenderBegin();

	// Call this to render a singe horizontal line of the image. Returns true if there's
	// still more rendering to do. Returns false if it's done. You must call RenderBegin()
	// once before you start calling this method.
	bool RenderLine();

	// This calls RenderBegine and then renders a single pixel. It's not efficient
	// to call this method for every pixel. The only purpose for this method is to
	// make debugging the ray tracer easier. Pick a pixel that isn't rendered the way
	// you want and step through the ray tracing process to see why.
	GColor RenderSinglePixel(int x, int y);

	// Returns the rendered image (or partially rendered image). Returns NULL if Render()
	// or RenderBegin() has not been called yet.
	GImage* GetImage() { return m_pImage; }

	GImage* DropImage()
	{
		GImage* pImage = m_pImage;
		m_pImage = NULL;
		return pImage;
	}

	void SetBackgroundColor(GRayTraceReal a, GRayTraceReal r, GRayTraceReal g, GRayTraceReal b)
	{
		m_backgroundColor.Set(a, r, g, b);
	}

	void SetAmbientLight(GRayTraceReal r, GRayTraceReal g, GRayTraceReal b)
	{
		m_ambientLight.Set(1, r, g, b);
	}

	GRayTraceColor* GetAmbientLight() { return &m_ambientLight; }

	GRayTraceCamera* GetCamera()
	{
		return m_pCamera;
	}

	void AddMaterial(GRayTraceMaterial* pMaterial);
	void AddObject(GRayTraceObject* pObject);
	void AddLight(GRayTraceLight* pLight);
	int GetObjectCount();
	GRayTraceObject* GetObject(int n);
	int GetLightCount();
	GRayTraceLight* GetLight(int n);
	GRayTraceColor* GetBackgroundColor() { return &m_backgroundColor; }
};




class GRayTraceLight
{
protected:
	GRayTraceColor m_color;
	GRayTraceReal m_jitter;

public:
	GRayTraceLight(GRayTraceReal r, GRayTraceReal g, GRayTraceReal b, GRayTraceReal jitter);
	virtual ~GRayTraceLight();

	virtual void ComputeColorContribution(GRayTraceScene* pScene, GRayTraceRay* pRay, GRayTraceMaterial* pMaterial) = 0;
};





class GRayTraceDirectionalLight : public GRayTraceLight
{
protected:
	GRayTraceVector m_direction;

public:
	GRayTraceDirectionalLight(GRayTraceReal dx, GRayTraceReal dy, GRayTraceReal dz, GRayTraceReal r, GRayTraceReal g, GRayTraceReal b, GRayTraceReal jitter);
	virtual ~GRayTraceDirectionalLight();

	virtual void ComputeColorContribution(GRayTraceScene* pScene, GRayTraceRay* pRay, GRayTraceMaterial* pMaterial);
};




class GRayTracePointLight : public GRayTraceLight
{
protected:
	GRayTraceVector m_position;

public:
	GRayTracePointLight(GRayTraceReal x, GRayTraceReal y, GRayTraceReal z, GRayTraceReal r, GRayTraceReal g, GRayTraceReal b, GRayTraceReal jitter);
	virtual ~GRayTracePointLight();

	virtual void ComputeColorContribution(GRayTraceScene* pScene, GRayTraceRay* pRay, GRayTraceMaterial* pMaterial);
};




class GRayTraceMaterial
{
public:
	enum ColorType
	{
		Diffuse = 0,
		Specular,
		Reflective,
		Transmissive,
		Ambient,
		Emissive, // for geometry lights
	};

protected:
	GRayTraceColor m_colors[6];
	GRayTraceReal m_indexOfRefraction;
	GRayTraceReal m_specularExponent;
	GRayTraceReal m_glossiness;
	GRayTraceReal m_translucency;

public:
	GRayTraceMaterial();
	~GRayTraceMaterial();

	GRayTraceColor* GetColor(ColorType eType) { return &m_colors[eType]; }
	void SetColor(ColorType eType, GRayTraceReal r, GRayTraceReal g, GRayTraceReal b);
	GRayTraceReal GetIndexOfRefraction() { return m_indexOfRefraction; }
	GRayTraceReal GetSpecularExponent() { return m_specularExponent; }
	GRayTraceReal GetGlossiness() { return m_glossiness; }
	GRayTraceReal GetTranslucency() { return m_translucency; }
	void SetIndexOfRefraction(GRayTraceReal val) { m_indexOfRefraction = val; }
	void SetSpecularExponent(GRayTraceReal val) { m_specularExponent = val; }
	void SetGlossiness(GRayTraceReal val) { m_glossiness = val; }
	void SetTranslucency(GRayTraceReal val) { m_translucency = val; }

	void ComputeColor(GRayTraceScene* pScene, GRayTraceRay* pRay);
};




class GRayTraceObject
{
protected:
	GRayTraceMaterial* m_pMaterial;

public:
	GRayTraceObject(GRayTraceMaterial* pMaterial)
	{
		m_pMaterial = pMaterial;
	}

	virtual ~GRayTraceObject()
	{
	}

	virtual GRayTraceReal ComputeRayDistance(GRayTraceVector* pRayOrigin, GRayTraceVector* pRayDirection) = 0;
	virtual void ComputeNormalVector(GRayTraceVector* pOutNormalVector, GRayTraceVector* pPoint) = 0;
	virtual bool IsCulled() = 0;
	GRayTraceMaterial* GetMaterial() { return m_pMaterial; }
};




class GRayTraceSphere : public GRayTraceObject
{
protected:
	GRayTraceVector m_center;
	GRayTraceReal m_radius;

public:
	GRayTraceSphere(GRayTraceMaterial* pMaterial, GRayTraceReal x, GRayTraceReal y, GRayTraceReal z, GRayTraceReal radius);
	virtual ~GRayTraceSphere();

	virtual GRayTraceReal ComputeRayDistance(GRayTraceVector* pRayOrigin, GRayTraceVector* pRayDirection);
	virtual void ComputeNormalVector(GRayTraceVector* pOutNormalVector, GRayTraceVector* pPoint);
	virtual bool IsCulled() { return false; }
};






class GRayTraceTriMesh : public GRayTraceObject
{
protected:
	int m_nPoints;
	GRayTraceVector* m_pPoints;
	int m_nTriangles;
	int* m_pTriangles;
	int m_nNormals;
	GRayTraceVector* m_pNormals;
	int m_nTextureCoords;
	GRayTraceReal* m_pTextureCoords;
	bool m_bCulling;
	int m_nHitTriangle;

public:
	GRayTraceTriMesh(GRayTraceMaterial* pMaterial, int nPoints, int nTriangles, int nNormals, int nTextureCoords);
	virtual ~GRayTraceTriMesh();

	virtual GRayTraceReal ComputeRayDistance(GRayTraceVector* pRayOrigin, GRayTraceVector* pRayDirection);
	virtual void ComputeNormalVector(GRayTraceVector* pOutNormalVector, GRayTraceVector* pPoint);
	virtual bool IsCulled() { return m_bCulling; }
	void ActivateCulling() { m_bCulling = true; }
	void SetPoint(int nIndex, GRayTraceVector* pPoint);
	void SetTriangle(int nIndex, int v1, int v2, int v3);
	void SetNormal(int nIndex, GRayTraceVector* pNormal);
	void SetTextureCoord(int nIndex, GRayTraceReal x, GRayTraceReal y);

protected:
	GRayTraceReal ComputeRayDistanceToTriangle(int nTriangle, GRayTraceVector* pRayOrigin, GRayTraceVector* pRayDirection);
	bool IsPointWithinPlanarPolygon(GRayTraceVector* pPoint, GRayTraceVector** ppVertices, int nVertices);

};

#endif // __GRAYTRACE_H__
