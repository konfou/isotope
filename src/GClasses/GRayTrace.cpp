/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GRayTrace.h"
#include "GMacros.h"
#include "GPointerQueue.h"
#include "GXML.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

Light RayHitData::MeasureLight(G3DObject* pUniverse, int nAllowedSubRays)
{
	return pPolygon->MeasureLight(pUniverse, this, nAllowedSubRays);
}

void Point3D::Transform(const struct Transform* pTransform)
{
	double c, s, t;

	// Size
	x *= pTransform->dScale;
	y *= pTransform->dScale;
	z *= pTransform->dScale;
/*
	// Roll
	c = pTransform->GetCosRoll();
	s = pTransform->GetSinRoll();
	t = x;
	x = x * c + y * s;
	y = y * c - t * s;
*/
	// Pitch;
	c = pTransform->GetCosPitch();
	s = pTransform->GetSinPitch();
	t = y;
	y = y * c + z * s;
	z = z * c - t * s;

	// Yaw
	c = pTransform->GetCosYaw();
	s = pTransform->GetSinYaw();
	t = z;
	z = z * c + x * s;
	x = x * c - t * s;

	// Offset
	Add(&pTransform->offset);
}

void Point3D::Untransform(const struct Transform* pTransform)
{
	double c, s, t;

	// Offset
	Subtract(&pTransform->offset);

	// Yaw
	c = pTransform->GetCosYaw();
	s = pTransform->GetSinYaw();
	t = z;
	z = z * c - x * s;
	x = x * c + t * s;

	// Pitch;
	c = pTransform->GetCosPitch();
	s = pTransform->GetSinPitch();
	t = y;
	y = y * c - z * s;
	z = z * c + t * s;
/*
	// Roll
	c = pTransform->GetCosRoll();
	s = pTransform->GetSinRoll();
	t = x;
	x = x * c - y * s;
	y = y * c + t * s;
*/
	// Size
	x /= pTransform->dScale;
	y /= pTransform->dScale;
	z /= pTransform->dScale;
}

void Point3D::FromXML(GXMLTag* pTag)
{
	GXMLAttribute* pAttr;
	pAttr = pTag->GetAttribute("x");
	if(pAttr)
		x = atof(pAttr->GetValue());
	pAttr = pTag->GetAttribute("y");
	if(pAttr)
		y = atof(pAttr->GetValue());
	pAttr = pTag->GetAttribute("z");
	if(pAttr)
		z = atof(pAttr->GetValue());
}

void Point3D::ToXML(GXMLTag* pTag)
{
	char szBuff[256];
	pTag->AddAttribute(new GXMLAttribute("x", dtoa(x, szBuff)));
	pTag->AddAttribute(new GXMLAttribute("y", dtoa(y, szBuff)));
	pTag->AddAttribute(new GXMLAttribute("z", dtoa(z, szBuff)));
}

void Triangle3D::FromXML(GXMLTag* pTag)
{
	GXMLAttribute* pAttr;
	pAttr = pTag->GetAttribute("v1");
	GAssert(pAttr, "missing vertex");
	if(pAttr)
		nPoints[0] = atoi(pAttr->GetValue());
	pAttr = pTag->GetAttribute("v2");
	GAssert(pAttr, "missing vertex");
	if(pAttr)
		nPoints[1] = atoi(pAttr->GetValue());
	pAttr = pTag->GetAttribute("v3");
	GAssert(pAttr, "missing vertex");
	if(pAttr)
		nPoints[2] = atoi(pAttr->GetValue());
}

void Triangle3D::ToXML(GXMLTag* pTag)
{
	char szBuff[32];
	pTag->AddAttribute(new GXMLAttribute("v1", itoa(nPoints[0], szBuff, 10)));
	pTag->AddAttribute(new GXMLAttribute("v2", itoa(nPoints[1], szBuff, 10)));
	pTag->AddAttribute(new GXMLAttribute("v3", itoa(nPoints[2], szBuff, 10)));
}

Vector ReflectVector(const Vector* pRay, const Vector* pPlane)
{
	// Z-correct
	double t1 = atan2(pPlane->dX, pPlane->dY);
	double dCos1 = cos(t1);
	double dSin1 = sin(t1);
	Vector p1;
	p1.dX = 0;
	p1.dY = pPlane->dY * dCos1 + pPlane->dX * dSin1;
	p1.dZ = pPlane->dZ;
	Vector r1;
	r1.dX = pRay->dX * dCos1 - pRay->dY * dSin1;
	r1.dY = pRay->dY * dCos1 + pRay->dX * dSin1;
	r1.dZ = pRay->dZ;

	// X-correct
	double t2 = atan2(p1.dZ, p1.dY);
	double dCos2 = cos(t2);
	double dSin2 = sin(t2);
	Vector r2;
	r2.dX = r1.dX;
	r2.dY = r1.dY * dCos2 + r1.dZ * dSin2;
	r2.dZ = r1.dZ * dCos2 - r1.dY * dSin2;

	// Reflect
	r2.dY = -r2.dY;

	// X-uncorrect
	r1.dX = r2.dX;
	r1.dY = r2.dY * dCos2 - r2.dZ * dSin2;
	r1.dZ = r2.dZ * dCos2 + r2.dY * dSin2;

	// Z-uncorrect
	r2.dX = r1.dX * dCos1 + r1.dY * dSin1;
	r2.dY = r1.dY * dCos1 - r1.dX * dSin1;
	r2.dZ = r1.dZ;

	return r2;
}

bool IsOnCorrectSideOfLine(double fX, double fY, double fZ, struct Point3D* pPoint1, struct Point3D* pPoint2, struct Point3D* pPoint3)
{
	double fDenom;
	double fNom;

	fDenom = (fX - pPoint1->x) * (pPoint2->y - pPoint3->y) + (fY - pPoint1->y) * (pPoint3->x - pPoint2->x);
	if(fDenom != 0) //> .00001f)
	{
		fNom = fX * (pPoint3->y - pPoint2->y) + pPoint2->x * (fY - pPoint3->y) + pPoint3->x * (pPoint2->y - fY);
		if(fDenom >= 0)
		{
			if(fNom >= 0)
				return true;
			else
				return false;
		}
		else
		{
			if(fNom >= 0)
				return false;
			else
				return true;
		}
	}

	fDenom = (fY - pPoint1->y) * (pPoint2->z - pPoint3->z) + (fZ - pPoint1->z) * (pPoint3->y - pPoint2->y);
	if(fDenom != 0) // > .00001f)
	{
		fNom = fY * (pPoint3->z - pPoint2->z) + pPoint2->y * (fZ - pPoint3->z) + pPoint3->y * (pPoint2->z - fZ);
		if(fDenom >= 0)
		{
			if(fNom >= 0)
				return true;
			else
				return false;
		}
		else
		{
			if(fNom >= 0)
				return false;
			else
				return true;
		}
	}

	fDenom = (fZ - pPoint1->z) * (pPoint2->x - pPoint3->x) + (fX - pPoint1->x) * (pPoint3->z - pPoint2->z);
	if(fDenom != 0) // > .00001f)
	{
		fNom = fZ * (pPoint3->x - pPoint2->x) + pPoint2->z * (fX - pPoint3->x) + pPoint3->z * (pPoint2->x - fX);
		if(fDenom >= 0)
		{
			if(fNom >= 0)
				return true;
			else
				return false;
		}
		else
		{
			if(fNom >= 0)
				return false;
			else
				return true;
		}
	}

	return true;
}

bool IsInsideTriangle(double fX, double fY, double fZ, struct Point3D* pPoint1, struct Point3D* pPoint2, struct Point3D* pPoint3)
{
	if(!IsOnCorrectSideOfLine(fX, fY, fZ, pPoint1, pPoint2, pPoint3))
		return false;
	if(!IsOnCorrectSideOfLine(fX, fY, fZ, pPoint2, pPoint3, pPoint1))
		return false;
	if(!IsOnCorrectSideOfLine(fX, fY, fZ, pPoint3, pPoint1, pPoint2))
		return false;
	return true;
}

double GetDistToEdge(double fX, double fY, double fZ, struct Point3D* pPoint1, struct Point3D* pPoint2, struct Point3D* pPoint3)
{
	double fDenom;
	double fNom;

	fDenom = (fX - pPoint1->x) * (pPoint2->y - pPoint3->y) + (fY - pPoint1->y) * (pPoint3->x - pPoint2->x);
	if(fDenom != 0)
	{
		fNom = fX * (pPoint3->y - pPoint2->y) + pPoint2->x * (fY - pPoint3->y) + pPoint3->x * (pPoint2->y - fY);
		return fNom / fDenom;
	}

	fDenom = (fY - pPoint1->y) * (pPoint2->z - pPoint3->z) + (fZ - pPoint1->z) * (pPoint3->y - pPoint2->y);
	if(fDenom != 0)
	{
		fNom = fY * (pPoint3->z - pPoint2->z) + pPoint2->y * (fZ - pPoint3->z) + pPoint3->y * (pPoint2->z - fZ);
		return fNom / fDenom;
	}

	fDenom = (fZ - pPoint1->z) * (pPoint2->x - pPoint3->x) + (fX - pPoint1->x) * (pPoint3->z - pPoint2->z);
	if(fDenom != 0)
	{
		fNom = fZ * (pPoint3->x - pPoint2->x) + pPoint2->z * (fX - pPoint3->x) + pPoint3->z * (pPoint2->x - fX);
		return fNom / fDenom;
	}

	return 0;
}

double GetDistanceUntilRayHitsSphere(struct Ray* pRay, const struct Point3D* pCenter, double fRadius)
{
	double fMag = (pRay->vDirection.dX * (pCenter->x - pRay->point.x) + 
					pRay->vDirection.dY * (pCenter->y - pRay->point.y) + 
					pRay->vDirection.dZ * (pCenter->z - pRay->point.z));
	double fDeltaX = pCenter->x - pRay->point.x - (fMag * pRay->vDirection.dX);
	double fDeltaY = pCenter->y - pRay->point.y - (fMag * pRay->vDirection.dY);
	double fDeltaZ = pCenter->z - pRay->point.z - (fMag * pRay->vDirection.dZ);
	double fDiscr = (fRadius * fRadius) - (fDeltaX * fDeltaX + fDeltaY * fDeltaY + fDeltaZ * fDeltaZ);
	if(fDiscr < 0)
		return 0;
	double fPM = sqrt(fDiscr);
	if(fMag - fPM > 0)
		return(fMag - fPM);
	else if(fMag + fPM > 0)
		return(fMag + fPM);
	else
		return 0;
}

void Transform::FromXML(GXMLTag* pTag)
{
	offset.FromXML(pTag);
	GXMLAttribute* pAttr;
	pAttr = pTag->GetAttribute("Scale");
	if(pAttr)
		dScale = atof(pAttr->GetValue());
/*	pAttr = pTag->GetAttribute("Roll");
	if(pAttr)
		dRoll = atof(pAttr->GetValue());*/
	pAttr = pTag->GetAttribute("Pitch");
	if(pAttr)
		dPitch = atof(pAttr->GetValue());
	pAttr = pTag->GetAttribute("Yaw");
	if(pAttr)
		dYaw = atof(pAttr->GetValue());
}

void Transform::ToXML(GXMLTag* pTag)
{
	char szBuff[256];
	offset.ToXML(pTag);
	if(dScale != 0)
		pTag->AddAttribute(new GXMLAttribute("Scale", dtoa(dScale, szBuff)));
/*	if(dRoll != 0)
		pTag->AddAttribute(new GXMLAttribute("Roll", dtoa(dScale, szBuff)));*/
	if(dPitch != 0)
		pTag->AddAttribute(new GXMLAttribute("Pitch", dtoa(dScale, szBuff)));
	if(dYaw != 0)
		pTag->AddAttribute(new GXMLAttribute("Yaw", dtoa(dScale, szBuff)));
}

G3DObject::G3DObject()
{
}

G3DObject::~G3DObject()
{
}

/*static*/ G3DObject* G3DObject::ObjectsFromXML(GXMLTag* pObjectsTag)
{
	GXMLAttribute* pMainAttr = pObjectsTag->GetAttribute("Main");
	if(!pMainAttr)
	{
		GAssert(false, "expected a 'Main' attribute");
		return NULL;
	}
	return G3DObject::FromXML(pObjectsTag, pMainAttr->GetValue(), NULL);
}

/*static*/ G3DObject* G3DObject::FromXML(GXMLTag* pObjectsTag, const char* szName, G3DObject* pParent)
{
	GAssert(stricmp(pObjectsTag->GetName(), "Objects") == 0, "unexpected tag");
	GXMLTag* pChildTag;
	GXMLAttribute* pNameAttr;
	for(pChildTag = pObjectsTag->GetFirstChildTag(); pChildTag; pChildTag = pObjectsTag->GetNextChildTag(pChildTag))
	{
		pNameAttr = pChildTag->GetAttribute("Name");
		if(!pNameAttr)
		{
			GAssert(false, "object has no name");
			continue;
		}
		if(stricmp(szName, pNameAttr->GetValue()) == 0)
		{
			if(stricmp(pChildTag->GetName(), "Agg") == 0)
				return G3DObjAgg::FromXML(pObjectsTag, pChildTag, pParent);
			else if(stricmp(pChildTag->GetName(), "Poly") == 0)
				return G3DObjPolygon::FromXML(pChildTag, pParent);
			else
			{
				GAssert(false, "unexpected tag");
				return NULL;
			}
		}
	}
	GAssert(false, "Object with that name not found");
	return NULL;
}

char* dtoa(double d, char* szBuff)
{
	sprintf(szBuff, "%f", d);
	
	// Remove trailing zeros
	int n;
	for(n = strlen(szBuff) - 1; n > 0 && szBuff[n] != '.'; n--)
	{
		if(szBuff[n] == '0')
			szBuff[n] = '\0';
		else
			break;
	}
	if(szBuff[n] == '.')
		szBuff[n] = '\0';

	return szBuff;
}

G3DObjAgg::G3DObjAgg(G3DObject* pParent) : G3DObject()
{
	m_pFrames = new GPointerArray(4);
}

G3DObjAgg::~G3DObjAgg()
{
	int nFrames = GetFrameCount();
	int n;
	for(n = 0; n < nFrames; n++)
	{
		GTransformObjectPairArray* pFrame = GetFrame(n);
		int nCount = pFrame->GetSize();
		int i;
		for(i = 0; i < nCount; i++)
		{
			struct TransformObjectPair* pPair = pFrame->GetPair(i);
			delete(pPair->pObject);
		}
	}
	delete(m_pFrames);
}

/*static*/ G3DObjAgg* G3DObjAgg::FromXML(GXMLTag* pObjectsTag, GXMLTag* pObjTag, G3DObject* pParent)
{
	if(stricmp(pObjectsTag->GetName(), "Objects") != 0)
	{
		GAssert(false, "unexpected tag");
		return NULL;
	}
	if(stricmp(pObjTag->GetName(), "Agg") != 0)
	{
		GAssert(false, "unexpected tag");
		return NULL;
	}
	G3DObjAgg* pObjAgg = new G3DObjAgg(pParent);
	GXMLTag* pFrameTag;
	int nFrame = 0;
	for(pFrameTag = pObjTag->GetFirstChildTag(); pFrameTag; pFrameTag = pObjTag->GetNextChildTag(pFrameTag))
	{
		pObjAgg->AddFrame();
		GTransformObjectPairArray* pFrame = pObjAgg->GetFrame(nFrame);
		GXMLTag* pPairTag;
		for(pPairTag = pFrameTag->GetFirstChildTag(); pPairTag; pPairTag = pFrameTag->GetNextChildTag(pPairTag))
		{
			GXMLAttribute* pNameAttr = pPairTag->GetAttribute("Name");
			if(pNameAttr)
			{
				// Deserialize the child object
				TransformObjectPair pair;
				pair.pObject = G3DObject::FromXML(pObjectsTag, pNameAttr->GetValue(), pObjAgg);
				if(!pair.pObject)
				{
					GAssert(false, "Failed to load a polygon");
					delete(pObjAgg);
					return NULL;
				}

				// Deserialize the transform
				pair.trans.FromXML(pPairTag);

				// Apply the transform
				pair.pObject->Transform(&pair.trans);

				// Store the polygon
				pFrame->AddPair(&pair);
			}
		}
		nFrame++;
	}
	return pObjAgg;
}

void G3DObjAgg::Transform(const struct Transform* pTransform)
{
	int nFrames = GetFrameCount();
	int n;
	for(n = 0; n < nFrames; n++)
	{
		GTransformObjectPairArray* pFrame = GetFrame(n);
		int nPairCount = pFrame->GetSize();
		int i;
		for(i = 0; i < nPairCount; i++)
		{
			TransformObjectPair* pPair = pFrame->GetPair(i);
			pPair->pObject->Transform(pTransform);
		}
	}
}

void G3DObjAgg::Untransform(const struct Transform* pTransform)
{
	int nFrames = GetFrameCount();
	int n;
	for(n = 0; n < nFrames; n++)
	{
		GTransformObjectPairArray* pFrame = GetFrame(n);
		int nPairCount = pFrame->GetSize();
		int i;
		for(i = 0; i < nPairCount; i++)
		{
			TransformObjectPair* pPair = pFrame->GetPair(i);
			pPair->pObject->Untransform(pTransform);
		}
	}
}

void G3DObjAgg::FireRay(struct Ray* pRay, struct RayHitData* pOutResults, int nFrame, int nTotalFrames)
{
	// Make sure the ray will hit a bounding sphere
	pOutResults->distance = 0;
	if(GetDistanceUntilRayHitsSphere(pRay, GetCenter(), GetRadius()) <= 0)
		return;

	// Try each of the child objects
	int nActualFrame = nFrame * GetFrameCount() / nTotalFrames;
	GTransformObjectPairArray* pFrame = GetFrame(nActualFrame);	
	double fClosestHit = MAX_RAY_DISTANCE;
	TransformObjectPair* pPair;
	struct RayHitData tmp;
	int nPairCount = pFrame->GetSize();
	int n;
	for(n = 0; n < nPairCount; n++)
	{
		pPair = pFrame->GetPair(n);
		pPair->pObject->FireRay(pRay, &tmp, nFrame, nTotalFrames);
		if(tmp.distance > 0 && tmp.distance < fClosestHit)
		{
			fClosestHit = tmp.distance;
			*pOutResults = tmp;
		}
	}
}

const Point3D* G3DObjAgg::GetCenter()
{
	if(m_bKnowCenter)
		return &m_center;

	// Average the centers of all the child objects
	m_center.x = 0;
	m_center.y = 0;
	m_center.z = 0;
	int nFrameCount = GetFrameCount();
	int nFrame;
	int nPointCount = 0;
	for(nFrame = 0; nFrame < nFrameCount; nFrame++)
	{
		GTransformObjectPairArray* pFrame = GetFrame(nFrame);
		int nPairs = pFrame->GetSize();
		int n;
		for(n = 0; n < nPairs; n++)
		{
			TransformObjectPair* pPair = pFrame->GetPair(n);
			const Point3D* pPoint = pPair->pObject->GetCenter();
			m_center.x += pPoint->x;
			m_center.y += pPoint->y;
			m_center.z += pPoint->z;
			nPointCount++;
		}
	}
	m_center.x /= nPointCount;
	m_center.y /= nPointCount;
	m_center.z /= nPointCount;
	m_bKnowCenter = true;
	return &m_center;
}

double G3DObjAgg::GetRadius()
{
	if(m_bKnowRadius)
		return m_dRadius;

	m_dRadius = GetFarthestPoint(GetCenter());
	m_bKnowRadius = true;
	return m_dRadius;
}

double G3DObjAgg::GetFarthestPoint(const Point3D* pPoint)
{
	double dFarthest = 0;
	double dTmp;
	int nFrameCount = GetFrameCount();
	int nFrame;
	for(nFrame = 0; nFrame < nFrameCount; nFrame++)
	{
		GTransformObjectPairArray* pFrame = GetFrame(nFrame);
		int nPairs = pFrame->GetSize();
		int n;
		for(n = 0; n < nPairs; n++)
		{
			TransformObjectPair* pPair = pFrame->GetPair(n);
			dTmp = pPair->pObject->GetFarthestPoint(pPoint);
			if(dTmp > dFarthest)
				dFarthest = dTmp;
		}
	}
	return dFarthest;
}


// ---------------------------------------------------------------------------


G3DObjPolygon::G3DObjPolygon(G3DObject* pParent) : G3DObject()
{
	m_pFrames = new GPointerArray(8);
	m_pTris = new GTriangle3DArray(8);
	m_bKnowCenter = false;
	m_bKnowRadius = false;
}

G3DObjPolygon::~G3DObjPolygon()
{
	int nFrames = GetFrameCount();
	int n;
	for(n = 0; n < nFrames; n++)
		delete(GetFrame(n));
	delete(m_pFrames);
	delete(m_pTris);
}

const Point3D* G3DObjPolygon::GetCenter()
{
	if(m_bKnowCenter)
		return &m_center;

	// Average all the points to find an approximate center
	m_center.x = 0;
	m_center.y = 0;
	m_center.z = 0;
	int nFrames = GetFrameCount();
	int nFrame;
	int nPointCount = 0;
	for(nFrame = 0; nFrame < nFrames; nFrame++)
	{
		GPoint3DArray* pFrame = GetFrame(nFrame);
		int nPoints = pFrame->GetSize();
		int n;
		for(n = 0; n < nPoints; n++)
		{
			Point3D* pPoint = pFrame->GetPoint(n);
			m_center.x += pPoint->x;
			m_center.y += pPoint->y;
			m_center.z += pPoint->z;
			nPointCount++;
		}
	}
	m_center.x /= nPointCount;
	m_center.y /= nPointCount;
	m_center.z /= nPointCount;
	m_bKnowCenter = true;
	return &m_center;
}

double G3DObjPolygon::GetFarthestPoint(const Point3D* pPoint)
{
	double dFarthest = 0;
	double dTmp;
	int nFrames = GetFrameCount();
	int nFrame;
	for(nFrame = 0; nFrame < nFrames; nFrame++)
	{
		GPoint3DArray* pFrame = GetFrame(nFrame);
		int nPoints = pFrame->GetSize();
		int n;
		for(n = 0; n < nPoints; n++)
		{
			Point3D* pTmp = pFrame->GetPoint(n);
			dTmp = pPoint->GetDistance(pTmp);
			if(dTmp > dFarthest)
				dFarthest = dTmp;
		}
	}
	return dFarthest;
}

double G3DObjPolygon::GetRadius()
{
	if(m_bKnowRadius)
		return m_dRadius;

	m_dRadius = GetFarthestPoint(GetCenter());
	m_bKnowRadius = true;
	return m_dRadius;
}

/*static*/ G3DObjPolygon* G3DObjPolygon::FromXML(GXMLTag* pObjTag, G3DObject* pParent)
{
	G3DObjPolygon* pPoly = new G3DObjPolygon(pParent);
	GXMLTag* pChildTag;
	int nFrame = 0;
	int nPointCount = -1;
	for(pChildTag = pObjTag->GetFirstChildTag(); pChildTag; pChildTag = pObjTag->GetNextChildTag(pChildTag))
	{
		if(stricmp(pChildTag->GetName(), "Frame") == 0)
		{
			// Check the point count
			if(nPointCount == -1)
				nPointCount = pChildTag->GetChildTagCount();
			else if(nPointCount != pChildTag->GetChildTagCount())
			{
				GAssert(false, "inconsistent number of points");
				delete(pPoly);
				return NULL;
			}

			// Add a frame
			pPoly->AddFrame();
			GPoint3DArray* pFrame = pPoly->GetFrame(nFrame);
			
			// Deserialize all the points;
			GXMLTag* pPointTag;
			for(pPointTag = pChildTag->GetFirstChildTag(); pPointTag; pPointTag = pChildTag->GetNextChildTag(pPointTag))
			{
				if(stricmp(pPointTag->GetName(), "p") != 0)
				{
					GAssert(false, "expected a 'p' tag");
					delete(pPoly);
					return NULL;
				}
				Point3D point;
				point.FromXML(pPointTag);
				pFrame->AddPoint(&point);
			}

			nFrame++;
		}
		else if(stricmp(pChildTag->GetName(), "Tri") == 0)
		{
			Triangle3D tri;
			tri.FromXML(pChildTag);
			GAssert(nPointCount == -1 || tri.nPoints[0] <= nPointCount, "out of range (703)");
			GAssert(nPointCount == -1 || tri.nPoints[1] <= nPointCount, "out of range (704)");
			GAssert(nPointCount == -1 || tri.nPoints[2] <= nPointCount, "out of range (705)");
			pPoly->m_pTris->AddTri(&tri);
		}
		else
		{
			GAssert(false, "Unexpected tag");
			delete(pPoly);
			return NULL;
		}
	}
	return pPoly;
}

void G3DObjPolygon::Transform(const struct Transform* pTransform)
{
	int nFrameCount = GetFrameCount();
	int n;
	for(n = 0; n < nFrameCount; n++)
	{
		GPoint3DArray* pFrame = GetFrame(n);
		int nCount = pFrame->GetSize();
		int n;
		for(n = 0; n < nCount; n++)
		{
			Point3D* pPoint = pFrame->GetPoint(n);
			pPoint->Transform(pTransform);
		}
	}
}

void G3DObjPolygon::Untransform(const struct Transform* pTransform)
{
	int nFrameCount = GetFrameCount();
	int n;
	for(n = 0; n < nFrameCount; n++)
	{
		GPoint3DArray* pFrame = GetFrame(n);
		int nCount = pFrame->GetSize();
		int n;
		for(n = 0; n < nCount; n++)
		{
			Point3D* pPoint = pFrame->GetPoint(n);
			pPoint->Untransform(pTransform);
		}
	}
}

Light G3DObjPolygon::MeasureLight(G3DObject* pUniverse, struct RayHitData* pRayHit, int nAllowedSubRays)
{
	// Get the normal-vector of the surface we hit
	Vector vSurface;
	if(m_properties.bSmooth)
		vSurface = GetPhongNormal(pRayHit);
	else
		vSurface = pRayHit->vSurface;

	// Fire sub-rays
	Light lReflection;
	Light lTranslucence;
	if(nAllowedSubRays > 0)
	{
		// Fire Reflection ray
		RayHitData rhdReflection;
		if(m_properties.dReflectivity > 0)
		{
			Ray r;
			r.point = pRayHit->point;
			r.vDirection = ReflectVector(&pRayHit->vDirection, &vSurface);
			r.point.Add(.000001, &r.vDirection);
			pUniverse->FireRay(&r, &rhdReflection, rhdReflection.nFrame, rhdReflection.nTotalFrames);
		}

		// Fire Translucence ray
		RayHitData rhdTranslucence;
		if(m_properties.dTranslucency > 0)
		{
			Ray r;
			r.point = pRayHit->point;
			r.vDirection = pRayHit->vDirection;
			r.point.Add(.000001, &r.vDirection);
			pUniverse->FireRay(&r, &rhdTranslucence, rhdTranslucence.nFrame, rhdTranslucence.nTotalFrames);
		}

		// Limit the number of recursive sub-rays.  (This algorithm isn't very accurate about firing the number of sub-rays specified, but it gets in the general ballpark which is good enough.)
		if(rhdReflection.distance > 0 && rhdTranslucence.distance > 0)
			nAllowedSubRays = (int)((double)nAllowedSubRays / 1.41421356); // this number has no significant meaning.  It's utter coincidence that it is very close to sqrt(2).
		else
			nAllowedSubRays--;

		// Measure lighting
		GAssert(m_properties.dReflectivity + m_properties.dTranslucency <= 1, "out of range (706)");
		if(rhdReflection.distance > 0)
			lReflection = rhdReflection.MeasureLight(pUniverse, nAllowedSubRays);
		if(rhdTranslucence.distance > 0)
			lTranslucence = rhdTranslucence.MeasureLight(pUniverse, nAllowedSubRays);
	}

	// Measure the light that falls on this surface
	double dBrightness = 0;
	if(m_properties.dReflectivity + m_properties.dTranslucency < 1)
	{
		dBrightness = (
						ABS(
							pRayHit->vSurface.dZ / 
								sqrt(pRayHit->vSurface.dX * pRayHit->vSurface.dX + 
									pRayHit->vSurface.dY * pRayHit->vSurface.dY + 
									pRayHit->vSurface.dZ * pRayHit->vSurface.dZ)
							) + 1
						 ) / 2;
	}

	// Mix the three lightings in appropriate proportions
	Light l;
	dBrightness *= ((double)1 - (m_properties.dReflectivity + m_properties.dTranslucency));
	l.dRed = m_properties.dReflectivity * lReflection.dRed +
			m_properties.dTranslucency * lTranslucence.dRed +
			dBrightness * gRed(m_properties.color);
	l.dGreen = m_properties.dReflectivity * lReflection.dGreen +
			m_properties.dTranslucency * lTranslucence.dGreen +
			dBrightness * gGreen(m_properties.color);
	l.dBlue = m_properties.dReflectivity * lReflection.dBlue +
			m_properties.dTranslucency * lTranslucence.dBlue +
			dBrightness * gBlue(m_properties.color);
	return l;
}

// A, and B are two vertices of the triangle and C is a point inside the triangle
// pvA and pvB are the phong vectors as vertex A and B respectively
Vector GetTriangleInterpolationFactor(Point3D A, Point3D B, Point3D C, const Vector* pvA, const Vector* pvB)
{
	// Shift until A is at the origin
	B.x -= A.x;
	B.y -= A.y;
	B.z -= A.z;
	C.x -= A.x;
	C.y -= A.y;
	C.z -= A.z;

	// Rotate around Y axis until B.x is 0
	double T2 = atan2(B.x, B.z);
	double dCosT2 = cos(T2);
	double dSinT2 = sin(T2);
	//B.x = 0;
	B.z = B.z * dCosT2 + B.x * dSinT2;
	double tmp = C.x;
	C.x = C.x * dCosT2 - C.z * dSinT2;
	C.z = C.z * dCosT2 + tmp * dSinT2;

	// Rotate around X axis until B.y is 0
	double T3 = atan2(B.y, B.z);
	double dCosT3 = cos(T3);
	double dSinT3 = sin(T3);
	//B.y = 0;
	B.z = B.z * dCosT3 + B.y * dSinT3;
	tmp = C.y;
	C.y = C.y * dCosT3 - C.z * dSinT3;
	C.z = C.z * dCosT3 + tmp * dSinT3;

	// Rotate around Z axis until C.x is 0
	double T4 = atan2(C.x, C.y);
	//C.x = 0;
	C.y = C.y * cos(T4) + C.x * sin(T4);

	// Interpolate between the vectors
	double dBFac = C.z / B.z;
	double dAFac = 1 - dBFac;
	Vector v;
	v.dX = dAFac * pvA->dX + dBFac * pvB->dX;
	v.dY = dAFac * pvA->dY + dBFac * pvB->dY;
	v.dZ = dAFac * pvA->dZ + dBFac * pvB->dZ;

	// Normalize to inverse of distance from edge
	double dMag = ABS(C.y);
	if(dMag < .000001)
		dMag = .000001;
	v.dX /= dMag;
	v.dY /= dMag;
	v.dZ /= dMag;

	return v;
}

void GetCorners(bool bFlip, int nSide, int* pnCorners)
{
	if(!bFlip)
	{
		switch(nSide)
		{
		case 0:
			pnCorners[0] = 0;
			pnCorners[1] = 1;
			pnCorners[2] = 2;
			break;
		case 1:
			pnCorners[0] = 1;
			pnCorners[1] = 2;
			pnCorners[2] = 0;
			break;
		case 2:
			pnCorners[0] = 2;
			pnCorners[1] = 0;
			pnCorners[2] = 1;
			break;
		default:
			GAssert(false, "unexpected case");
		}
	}
	else
	{
		switch(nSide)
		{
		case 0:
			pnCorners[0] = 0;
			pnCorners[1] = 2;
			pnCorners[2] = 1;
			break;
		case 1:
			pnCorners[0] = 1;
			pnCorners[1] = 0;
			pnCorners[2] = 2;
			break;
		case 2:
			pnCorners[0] = 2;
			pnCorners[1] = 1;
			pnCorners[2] = 0;
			break;
		default:
			GAssert(false, "unexpected case");
		}
	}
}

void GetVertices(struct Triangle3D* pTri, bool bFlip, int nSide, int* pnPoints)
{
	int nCorners[3];
	GetCorners(bFlip, nSide, nCorners);
	pnPoints[0] = pTri->nPoints[nCorners[0]];
	pnPoints[1] = pTri->nPoints[nCorners[1]];
	pnPoints[2] = pTri->nPoints[nCorners[2]];
}

Vector G3DObjPolygon::GetPhongNormalAtVertex(int nFrame, struct Triangle3D* pTriangle, int nCorner)
{
	if(!pTriangle->vPhong[nCorner].HasZeroMagnitude())
		return pTriangle->vPhong[nCorner];

	// Get the three vertexes
	int nVertexes[3];
	GetVertices(pTriangle, false, nCorner, nVertexes);
	int nVertex = nVertexes[0];
	int nV1 = nVertexes[1];
	int nV2 = nVertexes[2];

	// Add normal of this triangle to the vector
	GPoint3DArray* pFrame = GetFrame(nFrame);
	Point3D* pPoint1 = pFrame->GetPoint(nVertex);
	Point3D* pPoint2 = pFrame->GetPoint(nV1);
	Point3D* pPoint3 = pFrame->GetPoint(nV2);
	Vector v = GetNormalVector(pPoint1, pPoint2, pPoint3);

	// Find all the triangles that touch the center vertex
	GPointerQueue pQ;
	int nTriCount = m_pTris->GetSize();
	int nTri;
	struct Triangle3D* pTri;
	for(nTri = 0; nTri < nTriCount; nTri++)
	{
		pTri = m_pTris->GetTri(nTri);
		if(pTri == pTriangle)
			continue;
		int n;
		for(n = 0; n < 3; n++)
		{
			if(pTri->nPoints[n] == nVertex)
			{
				pQ.Push(pTri);
				break;
			}
		}
	}

	// Copy the buffer to a queue
	int nTris = pQ.GetSize();
	if(nTris < 1)
		return v;
	struct Triangle3D** pTris = (struct Triangle3D**)alloca(nTris * sizeof(struct Triangle3D*));
	for(nTri = 0; nTri < nTris; nTri++)
		pTris[nTri] = (struct Triangle3D*)pQ.Pop();

	// Run around the loop in both directions simultaneously and add up normals.
	while(nV1 != nV2)
	{
		// Find a triangle that connects with nVertex and either nV1 or nV2
		bool bGotOne = false;
		for(nTri = 0; nTri < nTris; nTri++)
		{
			pTri = pTris[nTri];

			// Try all three rotations
			int nSide;
			for(nSide = 0; nSide < 3; nSide++)
			{
				// Try both front and back
				int i;
				for(i = 0; i < 2; i++)
				{
					// Check if vertex 0 is nVertex
					bool bFlip = (i == 0 ? false : true);
					int nVerts[3];
					GetVertices(pTri, bFlip, nSide, nVerts);
					if(nVerts[0] != nVertex)
						continue;

					// See if it matches either side
					if(nVerts[2] == nV1)
					{
						bGotOne = true;
						nV1 = nVerts[1];
					}
					else if(nVerts[1] == nV2)
					{
						bGotOne = true;
						nV2 = nVerts[2];
					}

					// Add the normal to the vector
					if(bGotOne)
					{
						// Add the normal
						pPoint1 = pFrame->GetPoint(nVerts[0]);
						pPoint2 = pFrame->GetPoint(nVerts[1]);
						pPoint3 = pFrame->GetPoint(nVerts[2]);
						Vector tmp = GetNormalVector(pPoint1, pPoint2, pPoint3);
						v.Add(tmp);
						break;
					}
				}
				if(bGotOne)
					break;
			}

			// Throw out the matching triangle so we don't use it again
			if(bGotOne)
			{
				pTris[nTri] = pTris[nTris - 1];
				nTris--;
				break;
			}
		}

		// If there was no match then we're done.  (This will happen if there isn't a closed circle around this vertex.)
		if(!bGotOne)
			break;
	}

	// Cache the result
	v.Normalize();
	pTriangle->vPhong[nCorner] = v;
	return v;
}

Vector G3DObjPolygon::GetPhongNormal(struct RayHitData* pRayHit)
{
	// Calculate the angle at each of the three vertices of the triange the ray hit
	Vector v[3];
	int nVertex;
	int nFrame = pRayHit->nFrame * GetFrameCount() / pRayHit->nTotalFrames;
	for(nVertex = 0; nVertex < 3; nVertex++)
	{
		v[nVertex] = GetPhongNormalAtVertex(nFrame, pRayHit->pTriangle, nVertex);
		if(!pRayHit->bFront)
			v[nVertex].Invert();
	}

	// Interpolate the brightness of the pixel
	GPoint3DArray* pFrame = GetFrame(nFrame);
	struct Point3D* pPoint1 = pFrame->GetPoint(pRayHit->pTriangle->nPoints[0]);
	struct Point3D* pPoint2 = pFrame->GetPoint(pRayHit->pTriangle->nPoints[1]);
	struct Point3D* pPoint3 = pFrame->GetPoint(pRayHit->pTriangle->nPoints[2]);
	Vector v1 = GetTriangleInterpolationFactor(*pPoint1, *pPoint2, pRayHit->point, &v[0], &v[1]);
	Vector v2 = GetTriangleInterpolationFactor(*pPoint2, *pPoint3, pRayHit->point, &v[1], &v[2]);
	Vector v3 = GetTriangleInterpolationFactor(*pPoint3, *pPoint1, pRayHit->point, &v[2], &v[0]);
	Vector vResults;
	vResults.dX = v1.dX + v2.dX + v3.dX;
	vResults.dY = v1.dY + v2.dY + v3.dY;
	vResults.dZ = v1.dZ + v2.dZ + v3.dZ;
	vResults.Normalize();
	return vResults;
}

void G3DObjPolygon::FireRay(struct Ray* pRay, struct RayHitData* pOutResults, int nFrame, int nTotalFrames)
{
	pOutResults->distance = 0;

	// Make sure the ray will hit a bounding sphere
	if(GetDistanceUntilRayHitsSphere(pRay, GetCenter(), GetRadius()) <= 0)
		return;

	// Try each triangle in the polygon
	int nActualFrame = nFrame * GetFrameCount() / nTotalFrames;
	GPoint3DArray* pFrame = GetFrame(nActualFrame);
	struct Point3D* pPoint1;
	struct Point3D* pPoint2;
	struct Point3D* pPoint3;
	struct Triangle3D* pTriangle;
	double fDenom;
	double fDist;
	double fClosestIntersection = MAX_RAY_DISTANCE;
	Point3D hit;
	int nCount = m_pTris->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
	{
		// Get the 3 vertices
		pTriangle = m_pTris->GetTri(n);
		pPoint1 = pFrame->GetPoint(pTriangle->nPoints[0]);
		pPoint2 = pFrame->GetPoint(pTriangle->nPoints[1]);
		pPoint3 = pFrame->GetPoint(pTriangle->nPoints[2]);

		// Calculate formula for surface of plane that triangle is on. (Ax + By + Cz + D = 0)
		double fD;
		Vector vSurface = CalculateSurfaceFormula(pPoint1, pPoint2, pPoint3, &fD);
		bool bFront = (vSurface.GetSimilarity(&pRay->vDirection) < 0 ? false : true);

		// Calculate distance ray must travel to intersect the plane
		fDenom = vSurface.dX * pRay->vDirection.dX + 
				vSurface.dY * pRay->vDirection.dY + 
				vSurface.dZ * pRay->vDirection.dZ;
		if(fDenom == 0)
			continue;
		fDist = -(vSurface.dX * pRay->point.x + 
				vSurface.dY * pRay->point.y + 
				vSurface.dZ * pRay->point.z + fD) / fDenom;
		if(fDist < 0 || fDist >= fClosestIntersection)
			continue;

		// See if it falls within the triangle
		hit = pRay->point;
		hit.Add(fDist, &pRay->vDirection);
		if(!IsInsideTriangle(hit.x, hit.y, hit.z, pPoint1, pPoint2, pPoint3))
			continue;

		// We have a collision!
		fClosestIntersection = fDist;

		// Record info about the ray and the surface it hit
		pOutResults->distance = fDist;
		pOutResults->vDirection = pRay->vDirection;
		pOutResults->pPolygon = this;
		pOutResults->pTriangle = pTriangle;
		pOutResults->point = hit;
		pOutResults->vSurface = vSurface;
		pOutResults->bFront = bFront;
	}
}

