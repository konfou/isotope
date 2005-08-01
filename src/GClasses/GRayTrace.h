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






struct Transform
{
public:
	double dScale;
protected:
//	double dRoll; // Z-axis
	double dPitch; // Y-axis (longitude)
	double dYaw; // X-axis (latitude)

//	double dCosRoll;
//	double dSinRoll;
	double dCosPitch;
	double dSinPitch;
	double dCosYaw;
	double dSinYaw;

public:
	Point3D offset;


	Transform()
	{
		dScale = 1;
//		dRoll = 0;
		SetPitch(0);
		SetYaw(0);
	}

	void FromXML(GXMLTag* pTag);
	void ToXML(GXMLTag* pTag);

	// Note: the vector expresses pitch, yaw, and scale only
	Vector ToVector()
	{
		Vector v;
		v.dY = dScale * dSinPitch;
		v.dZ = dScale * dCosPitch;
		v.dX = v.dZ * dSinYaw;
		v.dZ *= dCosYaw;
		return v;
	}

	void FromVector(const Vector* pV)
	{
		double dX2 = pV->dX * pV->dX;
		double dZ2 = pV->dZ * pV->dZ;
		dScale = sqrt(dX2 + pV->dY * pV->dY + dZ2);
		SetPitch(atan2(pV->dY, sqrt(dX2 + dZ2)));

		// Ajust range of yaw
		double dY = atan2(pV->dY, pV->dZ);
		if(dY < (-PI / (double)2))
			dY += (2 * PI);
		SetYaw(dY);
	}

	//void SetRoll(double d) { dRoll = d; dCosRoll = cos(d); dSinRoll = sin(d); }
	void SetPitch(double d) { dPitch = d; dCosPitch = cos(d); dSinPitch = sin(d); }
	void SetYaw(double d) { dYaw = d; dCosYaw = cos(d); dSinYaw = sin(d); }
	//double GetRoll() const { return dRoll; }
	double GetPitch() const { return dPitch; }
	double GetYaw() const { return dYaw; }
	double GetCosPitch() const { return dCosPitch; }
	double GetSinPitch() const { return dSinPitch; }
	double GetCosYaw() const { return dCosYaw; }
	double GetSinYaw() const { return dSinYaw; }
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

#endif // __GRAYTRACE_H__
