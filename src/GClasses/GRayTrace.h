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

/*
struct Vector;
class G3DObjPolygon;
class G3DObject;
class GXMLTag;
struct Transform;




struct PolygonProperties
{
	bool bSmooth;
	GColor color;

	// dReflectivity + dTranslucency must be <= 1.  The amount the polygon gets it's own color is 1 - (dReflectivity + dTranslucency)
	double dReflectivity;
	double dTranslucency;

	PolygonProperties()
	{
		bSmooth = false;
		color = 0xffffff;
		dReflectivity = 0;
		dTranslucency = 0;
	}
};





struct Point3D
{
	double x, y, z;

	Point3D()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	Point3D(double dx, double dy, double dz)
	{
		x = dx;
		y = dy;
		z = dz;
	}

	inline double GetDistance(const Point3D* pThat) const
	{
		return sqrt((pThat->x - x) * (pThat->x - x) +
					(pThat->y - y) * (pThat->y - y) +
					(pThat->z - z) * (pThat->z - z));
	}

	inline double GetDistanceFromOrigin() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	inline double GetDistanceFromOriginSquared() const
	{
		return (x * x + y * y + z * z);
	}

	inline void Add(double dMagnitude, const struct Vector* pVector);

	inline void Add(const Point3D* pPoint)
	{
		x += pPoint->x;
		y += pPoint->y;
		z += pPoint->z;
	}

	inline void Subtract(const Point3D* pPoint)
	{
		x -= pPoint->x;
		y -= pPoint->y;
		z -= pPoint->z;
	}

	inline void GetLatLon(double* pdLat, double* pdLon) const
	{
		*pdLat = atan2(y, sqrt(x * x + z * z));
		*pdLon = atan2(x, z);
		if((*pdLon) < (-PI / (double)2))
			(*pdLon) += (2 * PI);
	}

	inline void Multiply(double dMag)
	{
		x *= dMag;
		y *= dMag;
		z *= dMag;
	}

	inline void DotProduct(const Point3D* pPoint)
	{
		x *= pPoint->x;
		y *= pPoint->y;
		z *= pPoint->z;
	}
	
	void Transform(const struct Transform* pTransform);
	void Untransform(const struct Transform* pTransform);
	void FromXML(GXMLTag* pTag);
	void ToXML(GXMLTag* pTag);
};



class GPoint3DArray : public GSmallArray
{
public:
	GPoint3DArray(int nGrowBy) : GSmallArray(sizeof(struct Point3D), nGrowBy) { }
	virtual ~GPoint3DArray() { }

	inline struct Point3D* GetPoint(int nIndex) { return (struct Point3D*)_GetCellRef(nIndex); }
	inline void AddPoint(struct Point3D* pPoint) { _AddCellByRef(pPoint); }
	inline void SetPoint(int nCell, struct Point3D* pPoint) { _SetCellByRef(nCell, pPoint); }
};






struct Vector
{
	double dX, dY, dZ;

	Vector()
	{
		dX = 0;
		dY = 0;
		dZ = 0;
	}

	inline void Normalize()
	{
		double dMag = sqrt(dX * dX + dY * dY + dZ * dZ);
		dX /= dMag;
		dY /= dMag;
		dZ /= dMag;
	}

	inline void Add(Vector v)
	{
		dX += v.dX;
		dY += v.dY;
		dZ += v.dZ;
	}

	inline void Invert()
	{
		dX = -dX;
		dY = -dY;
		dZ = -dZ;
	}

	inline bool HasZeroMagnitude() const
	{
		return(dX == 0 && dY == 0 && dZ == 0);
	}

	inline double GetSimilarity(const Vector* pThat) const
	{
		return(
				(dX * pThat->dX + dY * pThat->dY + dZ * pThat->dZ) / 
				(
					sqrt(dX * dX + dY * dY + dZ * dZ) * 
					sqrt(pThat->dX * pThat->dX + pThat->dY * pThat->dY + pThat->dZ * pThat->dZ)
				)
			);
	}

	inline void FromAToB(const Point3D* pA, const Point3D* pB)
	{
		dX = pB->x - pA->x;
		dY = pB->y - pA->y;
		dZ = pB->z - pA->z;
	}

	// dLon and dLat are in radians
	inline void FromLonLat(double dLon, double dLat)
	{
		dY = sin(dLat);
		double dTmp = cos(dLat);
		dX = dTmp * sin(dLon);
		dZ = dTmp * cos(dLon);
	}
};










struct Light
{
	double dRed;
	double dGreen;
	double dBlue;

	Light()
	{
		dRed = 0;
		dGreen = 0;
		dBlue = 0;
	}

	inline GColor ToPixel(double dThreshold)
	{
		return gRGB((int)((MIN(dRed, dThreshold) / dThreshold) * 255.999),
					(int)((MIN(dGreen, dThreshold) / dThreshold) * 255.999),
					(int)((MIN(dBlue, dThreshold) / dThreshold) * 255.999));
	}
};







struct PointOfLight
{
	Point3D point;
	Light light;
};

class GPointOfLightArray : public GSmallArray
{
public:
	GPointOfLightArray(int nGrowBy) : GSmallArray(sizeof(struct PointOfLight), nGrowBy) { }
	virtual ~GPointOfLightArray() { }

	inline struct PointOfLight* GetPoint(int nIndex) { return (struct PointOfLight*)_GetCellRef(nIndex); }
	inline void AddPoint(struct PointOfLight* pPoint) { _AddCellByRef(pPoint); }
	inline void SetPoint(int nCell, struct PointOfLight* pPoint) { _SetCellByRef(nCell, pPoint); }
};








struct Ray
{
	Point3D point;
	Vector vDirection;
};





struct Triangle3D
{
	int nPoints[3]; // The ID's of the three vertices of the triangle
	Vector vPhong[3];

	void FromXML(GXMLTag* pTag);
	void ToXML(GXMLTag* pTag);
};

class GTriangle3DArray : public GSmallArray
{
public:
	GTriangle3DArray(int nGrowBy) : GSmallArray(sizeof(struct Triangle3D), nGrowBy) { }
	virtual ~GTriangle3DArray() { }

	inline struct Triangle3D* GetTri(int nIndex) { return (struct Triangle3D*)_GetCellRef(nIndex); }
	inline void AddTri(struct Triangle3D* pTri) { _AddCellByRef(pTri); }
	inline void SetTri(int nCell, struct Triangle3D* pTri) { _SetCellByRef(nCell, pTri); }
};







struct RayHitData
{
	// Info about the ray
	double distance; // The distance the ray travelled until the collision
	Vector vDirection; // The vector of the ray that hit the surface
	
	// Info about the surface it hit
	G3DObjPolygon* pPolygon; // The polygon it hit
	int nFrame;
	int nTotalFrames;
	struct Triangle3D* pTriangle; // The triangle it hit
	Vector vSurface; // Normal to the surface
	Point3D point; // The position where the ray hit the surface
	bool bFront; // Did it hit the front or back of the triangle?

	RayHitData()
	{
		distance = 0;
		pPolygon = NULL;
		pTriangle = NULL;
		nFrame = 0;
		nTotalFrames = 0;
	}

	Light MeasureLight(G3DObject* pUniverse, int nAllowedSubRays);
};






inline Vector GetNormalVector(const Point3D* pPoint1, const Point3D* pPoint2, const Point3D* pPoint3)
{
	Vector v;
	v.dX = pPoint1->y * (pPoint2->z - pPoint3->z) + pPoint2->y * (pPoint3->z - pPoint1->z) + pPoint3->y * (pPoint1->z - pPoint2->z);
	v.dY = pPoint1->z * (pPoint2->x - pPoint3->x) + pPoint2->z * (pPoint3->x - pPoint1->x) + pPoint3->z * (pPoint1->x - pPoint2->x);
	v.dZ = pPoint1->x * (pPoint2->y - pPoint3->y) + pPoint2->x * (pPoint3->y - pPoint1->y) + pPoint3->x * (pPoint1->y - pPoint2->y);
	return v;
}

inline Vector CalculateSurfaceFormula(Point3D* pPoint1, Point3D* pPoint2, Point3D* pPoint3, double* pfD)
{
	Vector v = GetNormalVector(pPoint1, pPoint2, pPoint3);
	*pfD = -(v.dX * pPoint1->x + v.dY * pPoint1->y + v.dZ * pPoint1->z);
	return v;
}

inline void Point3D::Add(double dMagnitude, const Vector* pVector)
{
	x += dMagnitude * pVector->dX;
	y += dMagnitude * pVector->dY;
	z += dMagnitude * pVector->dZ;
}







#define MAX_RAY_DISTANCE 1000000000.0

class G3DObject
{
public:
	enum OBJTYPE
	{
		OT_AGG,
		OT_POLYGON,
	};

	G3DObject();
	virtual ~G3DObject();

	static G3DObject* ObjectsFromXML(GXMLTag* pObjectsTag);
	static G3DObject* FromXML(GXMLTag* pObjectsTag, const char* szName, G3DObject* pParent);
	GXMLTag* ToXML();

	virtual OBJTYPE GetObjectType() = 0;
	virtual const Point3D* GetCenter() = 0;
	virtual double GetRadius() = 0;
	virtual double GetFarthestPoint(const Point3D* pPoint) = 0;
	virtual void FireRay(struct Ray* pRay, struct RayHitData* pOutResults, int nFrame, int nTotalFrames) = 0;
	virtual void Transform(const struct Transform* pTransform) = 0;
	virtual void Untransform(const struct Transform* pTransform) = 0;
};







struct TransformObjectPair
{
	struct Transform trans;
	G3DObject* pObject;
};

class GTransformObjectPairArray : public GSmallArray
{
public:
	GTransformObjectPairArray(int nGrowBy) : GSmallArray(sizeof(struct TransformObjectPair), nGrowBy) { }
	virtual ~GTransformObjectPairArray() { }

	inline struct TransformObjectPair* GetPair(int nIndex) { return (struct TransformObjectPair*)_GetCellRef(nIndex); }
	inline void AddPair(struct TransformObjectPair* pPair) { _AddCellByRef(pPair); }
	inline void SetPair(int nCell, struct TransformObjectPair* pPair) { _SetCellByRef(nCell, pPair); }
};








class G3DObjAgg : public G3DObject
{
protected:
	GPointerArray* m_pFrames;

	// Bounding Sphere
	bool m_bKnowCenter;
	Point3D m_center;
	bool m_bKnowRadius;
	double m_dRadius;

public:
	G3DObjAgg(G3DObject* pParent);
	virtual ~G3DObjAgg();

	virtual const Point3D* GetCenter();
	virtual double GetRadius();
	virtual double GetFarthestPoint(const Point3D* pPoint);
	GXMLTag* ToXML();
	static G3DObjAgg* FromXML(GXMLTag* pObjectsTag, GXMLTag* pObjTag, G3DObject* pParent);
	virtual OBJTYPE GetObjectType() { return OT_AGG; }
	int GetFrameCount() { return m_pFrames->GetSize(); }
	void AddFrame() { m_pFrames->AddPointer(new GTransformObjectPairArray(8)); }
	GTransformObjectPairArray* GetFrame(int n) { return (GTransformObjectPairArray*)m_pFrames->GetPointer(n); }
	virtual void FireRay(struct Ray* pRay, struct RayHitData* pOutResults, int nFrame, int nTotalFrames);
	virtual void Transform(const struct Transform* pTransform);
	virtual void Untransform(const struct Transform* pTransform);
};









class G3DObjPolygon : public G3DObject
{
protected:
	struct PolygonProperties m_properties;
	GPointerArray* m_pFrames;
	GTriangle3DArray* m_pTris;
	struct RayHitData m_RayHit;

	// Bounding Sphere
	bool m_bKnowCenter;
	Point3D m_center;
	bool m_bKnowRadius;
	double m_dRadius;

	Vector GetPhongNormal(struct RayHitData* pRayHit);
	Vector GetPhongNormalAtVertex(int nFrame, struct Triangle3D* pTriangle, int nCorner);

public:
	G3DObjPolygon(G3DObject* pParent);
	virtual ~G3DObjPolygon();

	virtual const Point3D* GetCenter();
	virtual double GetRadius();
	virtual double GetFarthestPoint(const Point3D* pPoint);
	GXMLTag* ToXML();
	static G3DObjPolygon* FromXML(GXMLTag* pObjTag, G3DObject* pParent);
	int GetFrameCount() { return m_pFrames->GetSize(); }
	void AddFrame() { m_pFrames->AddPointer(new GPoint3DArray(8)); }
	GPoint3DArray* GetFrame(int n) { return (GPoint3DArray*)m_pFrames->GetPointer(n); }
	virtual OBJTYPE GetObjectType() { return OT_POLYGON; }
	virtual void Transform(const struct Transform* pTransform);
	virtual void Untransform(const struct Transform* pTransform);
	struct PolygonProperties* GetProperties() { return &m_properties; }
	virtual void FireRay(struct Ray* pRay, struct RayHitData* pOutResults, int nFrame, int nTotalFrames);
	Light MeasureLight(G3DObject* pUniverse, struct RayHitData* pRayHit, int nAllowedSubRays);

};




bool IsInsideTriangle(double fX, double fY, double fZ, struct Point3D* pPoint1, struct Point3D* pPoint2, struct Point3D* pPoint3);
double GetDistanceUntilRayHitsSphere(struct Ray* pRay, const struct Point3D* pCenter, double fRadius);
char* dtoa(double d, char* szBuff);
*/

// ---------------------------------- CS 655 -----------------------------------------

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

	GRayTraceVector(GRayTraceVector* pThat)
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

	void Copy(GRayTraceVector* pThat)
	{
		m_vals[0] = pThat->m_vals[0];
		m_vals[1] = pThat->m_vals[1];
		m_vals[2] = pThat->m_vals[2];
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

	void ComputeReflectionVector(GRayTraceVector* pRay, GRayTraceVector* pNormal);

	inline void GetLatLon(double* pdLat, double* pdLon) const
	{
		*pdLat = atan2(m_vals[1], sqrt(m_vals[0] * m_vals[0] + m_vals[2] * m_vals[2]));
		*pdLon = atan2(m_vals[0], m_vals[2]);
		if((*pdLon) < (-PI / (double)2))
			(*pdLon) += (2 * PI);
	}
};



class GRayTraceCamera
{
protected:
	GRayTraceVector m_lookFromPoint;
	GRayTraceVector m_lookAtPoint;
	GRayTraceVector m_viewUpVector;
	GRayTraceReal m_viewAngle; // in radians
	GRayTraceReal m_maxDepth;
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
		m_maxDepth = 1e10;
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
	void SetMaxDepth(GRayTraceReal val) { m_maxDepth = val; }
	void SetImageSize(int width, int height) { m_nWidth = width; m_nHeight = height; }
	int GetImageWidth() { return m_nWidth; }
	int GetImageHeight() { return m_nHeight; }
};


class GRayTraceRay
{
public:
	GRayTraceVector m_directionVector;
	GRayTraceVector m_collisionPoint;
	GRayTraceVector m_normalVector;
	GRayTraceVector m_reflectionVector;
	GRayTraceColor m_color;

	GRayTraceRay();
	~GRayTraceRay();

	void Cast(GRayTraceScene* pScene, GRayTraceVector* pRayOrigin, GRayTraceVector* pScreenPoint);
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

	virtual void ComputeColorContribution(GRayTraceRay* pRay, GRayTraceMaterial* pMaterial) = 0;
};





class GRayTraceDirectionalLight : public GRayTraceLight
{
protected:
	GRayTraceVector m_direction;

public:
	GRayTraceDirectionalLight(GRayTraceReal dx, GRayTraceReal dy, GRayTraceReal dz, GRayTraceReal r, GRayTraceReal g, GRayTraceReal b, GRayTraceReal jitter);
	virtual ~GRayTraceDirectionalLight();

	virtual void ComputeColorContribution(GRayTraceRay* pRay, GRayTraceMaterial* pMaterial);
};




class GRayTracePointLight : public GRayTraceLight
{
protected:
	GRayTraceVector m_position;

public:
	GRayTracePointLight(GRayTraceReal x, GRayTraceReal y, GRayTraceReal z, GRayTraceReal r, GRayTraceReal g, GRayTraceReal b, GRayTraceReal jitter);
	virtual ~GRayTracePointLight();

	virtual void ComputeColorContribution(GRayTraceRay* pRay, GRayTraceMaterial* pMaterial);
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
};



#endif // __GRAYTRACE_H__
