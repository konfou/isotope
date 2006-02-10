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
#ifndef DARWIN
#include <malloc.h>
#endif // !DARWIN

void GRayTraceVector::ComputeReflectionVector(GRayTraceVector* pRay, GRayTraceVector* pNormal)
{
	Copy(pNormal);
	Multiply(pNormal->DotProduct(pRay) * -2);
	Add(pRay);
}

// -----------------------------------------------------------------------------

#define MIN_RAY_DISTANCE ((GRayTraceReal).001)

GRayTraceRay::GRayTraceRay()
{
	m_indexOfRefraction = 1;
}

GRayTraceRay::GRayTraceRay(GRayTraceRay* pThat)
{
	m_indexOfRefraction = pThat->m_indexOfRefraction;
}

GRayTraceRay::~GRayTraceRay()
{
}

bool GRayTraceRay::ComputeTransmissionVector(GRayTraceVector* pDirectionVector, GRayTraceVector* pTransmissionVector, GRayTraceReal oldIndexOfRefraction, GRayTraceReal newIndexOfRefraction)
{
	double ratio = (double)oldIndexOfRefraction / newIndexOfRefraction;
	double comp = (double)pDirectionVector->DotProduct(&m_normalVector);
	double tmp = (double)1 - (ratio * ratio * ((double)1 - (comp * comp)));
	if(tmp < 0)
		return false;
	pTransmissionVector->Copy(&m_normalVector);
	pTransmissionVector->Multiply((GRayTraceReal)(-ratio * comp - sqrt(tmp)));
	GRayTraceVector x(pDirectionVector);
	x.Multiply((GRayTraceReal)ratio);
	pTransmissionVector->Add(&x);
	pTransmissionVector->Normalize();
	return true;
}

GRayTraceObject* GRayTraceRay::FindClosestIntersection(GRayTraceScene* pScene, GRayTraceVector* pRayOrigin, GRayTraceVector* pDirectionVector, GRayTraceReal* pOutDistance)
{
	// Find the closest intersection
	GRayTraceReal distance;
	GRayTraceReal closestDistance = (GRayTraceReal)1e30; // todo: unmagic this value
	GRayTraceObject* pClosestObject = NULL;
	int nCount = pScene->GetObjectCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		GRayTraceObject* pObject = pScene->GetObject(n);
		distance = pObject->ComputeRayDistance(pRayOrigin, pDirectionVector);
		if(distance < closestDistance && distance > MIN_RAY_DISTANCE)
		{
			closestDistance = distance;
			pClosestObject = pObject;
		}
	}
	*pOutDistance = closestDistance;
	return pClosestObject;
}

void GRayTraceRay::Cast(GRayTraceScene* pScene, GRayTraceVector* pRayOrigin, GRayTraceVector* pDirectionVector, int nMaxDepth)
{
	// Find the object
	GRayTraceReal distance;
	GRayTraceObject* pClosestObject = FindClosestIntersection(pScene, pRayOrigin, pDirectionVector, &distance);
	if(!pClosestObject)
	{
		m_color.Copy(pScene->GetBackgroundColor());
		return;
	}

	// Compute the collision point
	m_collisionPoint.Copy(pDirectionVector);
	m_collisionPoint.Multiply(distance);
	m_collisionPoint.Add(pRayOrigin);

	// Compute the normal
	pClosestObject->ComputeNormalVector(&m_normalVector, &m_collisionPoint);
	if(!pClosestObject->IsCulled())
	{
		if(pDirectionVector->DotProduct(&m_normalVector) > 0)
			m_normalVector.Multiply(-1);
	}

	// Compute the reflection vector
	m_reflectionVector.ComputeReflectionVector(pDirectionVector, &m_normalVector);

	// Compute the color
	GRayTraceMaterial* pMaterial = pClosestObject->GetMaterial();
	pMaterial->ComputeColor(pScene, this);

	// Case child rays
	if(nMaxDepth > 0)
	{
		// Reflection
		GRayTraceColor* pReflectedColor = pMaterial->GetColor(GRayTraceMaterial::Reflective);
		if(!pReflectedColor->IsBlack())
		{
			GRayTraceRay reflectionRay(this);
			reflectionRay.Cast(pScene, &m_collisionPoint, &m_reflectionVector, nMaxDepth - 1);
			reflectionRay.m_color.Multiply(pReflectedColor);
			m_color.Add(&reflectionRay.m_color);
		}

		// Transmission
		GRayTraceColor* pTransmissionColor = pMaterial->GetColor(GRayTraceMaterial::Transmissive);
		if(!pTransmissionColor->IsBlack())
		{
			// Compute transmission ray
			GRayTraceReal newIndexOfRefraction;
			if(m_indexOfRefraction > .9999 && m_indexOfRefraction < 1.0001)
				newIndexOfRefraction = pMaterial->GetIndexOfRefraction();
			else
				newIndexOfRefraction = 1;
			GRayTraceVector transmissionVector;
			if(!ComputeTransmissionVector(pDirectionVector, &transmissionVector, m_indexOfRefraction, newIndexOfRefraction))
			{
				// total internal reflection occurs
				transmissionVector.Copy(&m_reflectionVector);
				newIndexOfRefraction = m_indexOfRefraction;
			}

			// Cast transmission ray
			GRayTraceRay transmissionRay(this);
			transmissionRay.m_indexOfRefraction = newIndexOfRefraction;
			transmissionRay.Cast(pScene, &m_collisionPoint, &transmissionVector, nMaxDepth - 1);
			transmissionRay.m_color.Multiply(pTransmissionColor);
			m_color.Add(&transmissionRay.m_color);
		}
	}
}

// -----------------------------------------------------------------------------

GRayTraceScene::GRayTraceScene()
: m_backgroundColor(1, (GRayTraceReal).6, (GRayTraceReal).7, (GRayTraceReal).5),
  m_ambientLight(1, (GRayTraceReal).3, (GRayTraceReal).3, (GRayTraceReal).3)
{
	m_pMaterials = new GPointerArray(256);
	m_pObjects = new GPointerArray(256);
	m_pLights = new GPointerArray(32);
	m_pCamera = new GRayTraceCamera();
	m_pImage = NULL;
	m_nY = -1;
}

GRayTraceScene::~GRayTraceScene()
{
	int n, nCount;
	nCount = m_pMaterials->GetSize();
	for(n = 0; n < nCount; n++)
		delete((GRayTraceMaterial*)m_pMaterials->GetPointer(n));
	delete(m_pMaterials);
	nCount = m_pObjects->GetSize();
	for(n = 0; n < nCount; n++)
		delete((GRayTraceObject*)m_pObjects->GetPointer(n));
	delete(m_pObjects);
	nCount = m_pLights->GetSize();
	for(n = 0; n < nCount; n++)
		delete((GRayTraceLight*)m_pLights->GetPointer(n));
	delete(m_pCamera);
	delete(m_pImage);
}

int GRayTraceScene::GetObjectCount()
{
	return m_pObjects->GetSize();
}

GRayTraceObject* GRayTraceScene::GetObject(int n)
{
	return (GRayTraceObject*)m_pObjects->GetPointer(n);
}

int GRayTraceScene::GetLightCount()
{
	return m_pLights->GetSize();
}

GRayTraceLight* GRayTraceScene::GetLight(int n)
{
	return (GRayTraceLight*)m_pLights->GetPointer(n);
}

void GRayTraceScene::AddMaterial(GRayTraceMaterial* pMaterial)
{
	m_pMaterials->AddPointer(pMaterial);
}

void GRayTraceScene::AddObject(GRayTraceObject* pObject)
{
	m_pObjects->AddPointer(pObject);
}

void GRayTraceScene::AddLight(GRayTraceLight* pLight)
{
	m_pLights->AddPointer(pLight);
}

void GRayTraceScene::RenderBegin()
{
	// Allocate an image
	if(!m_pImage)
		m_pImage = new GImage();
	int nWidth = m_pCamera->GetImageWidth();
	int nHeight = m_pCamera->GetImageHeight();
	m_pImage->SetSize(nWidth, nHeight);
	m_pImage->Clear(0xff808080);

	// Precompute vectors
	GRayTraceVector n(m_pCamera->GetLookAtPoint());
	n.Subtract(m_pCamera->GetLookFromPoint());
	GRayTraceVector v(m_pCamera->GetViewUpVector());
	GRayTraceVector u;
	u.CrossProduct(&v, &n);
	u.Normalize();
	v.Normalize();
	GRayTraceReal halfViewHeight = (GRayTraceReal)(tan(m_pCamera->GetViewAngle() / 2) * sqrt(n.GetMagnitudeSquared()));
	GRayTraceReal halfViewWidth = halfViewHeight * nWidth / nHeight;
	u.Multiply(halfViewWidth);
	v.Multiply(halfViewHeight);
	m_pixSide.Copy(m_pCamera->GetLookAtPoint());
	m_pixSide.Subtract(&u);
	m_pixSide.Subtract(&v);
	m_pixDX.Copy(&u);
	m_pixDX.Multiply((GRayTraceReal)2 / nWidth);
	m_pixDY.Copy(&v);
	m_pixDY.Multiply((GRayTraceReal)2 / nHeight);
	m_nY = nHeight - 1;
}

bool GRayTraceScene::RenderLine()
{
	if(m_nY < 0)
		return false;
	GRayTraceRay ray;
	int x;
	GRayTraceVector screenPoint(&m_pixSide);
	GRayTraceVector* pCameraPoint = m_pCamera->GetLookFromPoint();
	GRayTraceVector directionVector;
	screenPoint.Copy(&m_pixSide);
	m_pixSide.Add(&m_pixDY);
	int nWidth = m_pImage->GetWidth();
	for(x = nWidth - 1; x >= 0; x--)
	{
		directionVector.Copy(&screenPoint);
		directionVector.Subtract(pCameraPoint);
		directionVector.Normalize();
		ray.Cast(this, pCameraPoint, &directionVector, m_pCamera->GetMaxDepth());
		m_pImage->SetPixel(x, m_nY, ray.m_color.GetGColor());
		screenPoint.Add(&m_pixDX);
	}
	if(--m_nY >= 0)
		return true;
	else
		return false;
}

void GRayTraceScene::Render()
{
	RenderBegin();
	while(RenderLine())
	{
	}
}

GColor GRayTraceScene::RenderSinglePixel(int x, int y)
{
	// Init the rendering
	RenderBegin();

	// Compute the ray direction
	GRayTraceColor col;
	GRayTraceVector* pCameraPoint = m_pCamera->GetLookFromPoint();
	GRayTraceVector screenPoint(&m_pixSide);
	m_pixDY.Multiply((GRayTraceReal)(m_pImage->GetHeight() - 1 - y));
	screenPoint.Add(&m_pixDY);
	m_pixDX.Multiply((GRayTraceReal)(m_pImage->GetWidth() - 1 - x));
	screenPoint.Add(&m_pixDX);

	// Cast the ray
	GRayTraceRay ray;
	GRayTraceVector directionVector(&screenPoint);
	directionVector.Subtract(pCameraPoint);
	directionVector.Normalize();
	ray.Cast(this, pCameraPoint, &directionVector, m_pCamera->GetMaxDepth());
	GColor c = ray.m_color.GetGColor();
	return c;
}

// -----------------------------------------------------------------------------

GRayTraceLight::GRayTraceLight(GRayTraceReal r, GRayTraceReal g, GRayTraceReal b, GRayTraceReal jitter)
: m_color(1, r, g, b), m_jitter(jitter)
{
}

/*virtual*/ GRayTraceLight::~GRayTraceLight()
{
}


// -----------------------------------------------------------------------------


GRayTraceDirectionalLight::GRayTraceDirectionalLight(GRayTraceReal dx, GRayTraceReal dy, GRayTraceReal dz, GRayTraceReal r, GRayTraceReal g, GRayTraceReal b, GRayTraceReal jitter)
: GRayTraceLight(r, g, b, jitter), m_direction(dx, dy, dz)
{
}

/*virtual*/ GRayTraceDirectionalLight::~GRayTraceDirectionalLight()
{
}

/*virtual*/ void GRayTraceDirectionalLight::ComputeColorContribution(GRayTraceScene* pScene, GRayTraceRay* pRay, GRayTraceMaterial* pMaterial)
{
	// Check if the point is in a shadow
	GRayTraceReal distance;
	if(pRay->FindClosestIntersection(pScene, &pRay->m_collisionPoint, &m_direction, &distance))
		return;

	// Compute diffuse component of the color
	GRayTraceColor diffuse(pMaterial->GetColor(GRayTraceMaterial::Diffuse));
	diffuse.Multiply(MAX((GRayTraceReal)0, m_direction.DotProduct(&pRay->m_normalVector)));

	// Compute specular component of the color
	GRayTraceReal mag = pow(MAX((GRayTraceReal)0, pRay->m_reflectionVector.DotProduct(&m_direction)), pMaterial->GetSpecularExponent());
	GRayTraceColor specular(pMaterial->GetColor(GRayTraceMaterial::Specular));
	specular.Multiply(mag);

	// Combine and multiply by light intensity
	diffuse.Add(&specular);
	diffuse.Multiply(&m_color);
	pRay->m_color.Add(&diffuse);
}

// -----------------------------------------------------------------------------


GRayTracePointLight::GRayTracePointLight(GRayTraceReal x, GRayTraceReal y, GRayTraceReal z, GRayTraceReal r, GRayTraceReal g, GRayTraceReal b, GRayTraceReal jitter)
: GRayTraceLight(r, g, b, jitter), m_position(x, y, z)
{
}

/*virtual*/ GRayTracePointLight::~GRayTracePointLight()
{
}

/*virtual*/ void GRayTracePointLight::ComputeColorContribution(GRayTraceScene* pScene, GRayTraceRay* pRay, GRayTraceMaterial* pMaterial)
{
	// Check if the point is in a shadow
	GRayTraceVector lightDirection(&m_position);
	lightDirection.Subtract(&pRay->m_collisionPoint);
	double distsqared = lightDirection.GetMagnitudeSquared();
	lightDirection.Normalize();
	GRayTraceReal distance;
	if(pRay->FindClosestIntersection(pScene, &pRay->m_collisionPoint, &lightDirection, &distance))
		return;

	// Compute diffuse component of the color
	GRayTraceColor diffuse(pMaterial->GetColor(GRayTraceMaterial::Diffuse));
	diffuse.Multiply(MAX((GRayTraceReal)0, lightDirection.DotProduct(&pRay->m_normalVector)));

	// Compute specular component of the color
	GRayTraceReal mag = pow(MAX((GRayTraceReal)0, pRay->m_reflectionVector.DotProduct(&lightDirection)), pMaterial->GetSpecularExponent());
	GRayTraceColor specular(pMaterial->GetColor(GRayTraceMaterial::Specular));
	specular.Multiply(mag);

	// Combine and multiply by light intensity
	diffuse.Add(&specular);
	diffuse.Multiply(&m_color);
	diffuse.Multiply((GRayTraceReal)(1.0 / distsqared));
	pRay->m_color.Add(&diffuse);
}

// -----------------------------------------------------------------------------

GRayTraceMaterial::GRayTraceMaterial()
{
	SetColor(Diffuse, (GRayTraceReal).5, (GRayTraceReal).5, (GRayTraceReal).5);
	SetColor(Specular, 1, 1, 1);
	SetColor(Reflective, 1, 1, 1);
	SetColor(Transmissive, 0, 0, 0);
	SetColor(Ambient, (GRayTraceReal).1, (GRayTraceReal).1, (GRayTraceReal).1);
	SetColor(Emissive, 0, 0, 0);
	m_indexOfRefraction = 1;
	m_specularExponent = 1;
	m_glossiness = 1;
	m_translucency = 0;
}

GRayTraceMaterial::~GRayTraceMaterial()
{
}

void GRayTraceMaterial::SetColor(ColorType eType, GRayTraceReal r, GRayTraceReal g, GRayTraceReal b)
{
	m_colors[eType].Set(1,
				MAX((GRayTraceReal)0, MIN((GRayTraceReal)1, r)),
				MAX((GRayTraceReal)0, MIN((GRayTraceReal)1, g)),
				MAX((GRayTraceReal)0, MIN((GRayTraceReal)1, b))
			);
}

void GRayTraceMaterial::ComputeColor(GRayTraceScene* pScene, GRayTraceRay* pRay)
{
	// Ambient light
	pRay->m_color.Copy(pScene->GetAmbientLight());
	pRay->m_color.Multiply(GetColor(Ambient));

	// Real lights
	int nLights = pScene->GetLightCount();
	int n;
	for(n = 0; n < nLights; n++)
		pScene->GetLight(n)->ComputeColorContribution(pScene, pRay, this);
}

// -----------------------------------------------------------------------------

GRayTraceSphere::GRayTraceSphere(GRayTraceMaterial* pMaterial, GRayTraceReal x, GRayTraceReal y, GRayTraceReal z, GRayTraceReal radius)
: GRayTraceObject(pMaterial), m_center(x, y, z), m_radius(radius)
{
}

/*virtual*/ GRayTraceSphere::~GRayTraceSphere()
{
}

/*virtual*/ GRayTraceReal GRayTraceSphere::ComputeRayDistance(GRayTraceVector* pRayOrigin, GRayTraceVector* pRayDirection)
{
	GRayTraceReal b = (GRayTraceReal)2 * (
				pRayDirection->m_vals[0] * (pRayOrigin->m_vals[0] - m_center.m_vals[0]) +
				pRayDirection->m_vals[1] * (pRayOrigin->m_vals[1] - m_center.m_vals[1]) +
				pRayDirection->m_vals[2] * (pRayOrigin->m_vals[2] - m_center.m_vals[2])
			);
	GRayTraceReal c = pRayOrigin->m_vals[0] * pRayOrigin->m_vals[0] -
				(GRayTraceReal)2 * pRayOrigin->m_vals[0] * m_center.m_vals[0] +
				m_center.m_vals[0] * m_center.m_vals[0] +
				pRayOrigin->m_vals[1] * pRayOrigin->m_vals[1] -
				(GRayTraceReal)2 * pRayOrigin->m_vals[1] * m_center.m_vals[1] +
				m_center.m_vals[1] * m_center.m_vals[1] +
				pRayOrigin->m_vals[2] * pRayOrigin->m_vals[2] -
				(GRayTraceReal)2 * pRayOrigin->m_vals[2] * m_center.m_vals[2] +
				m_center.m_vals[2] * m_center.m_vals[2] -
				m_radius * m_radius;
	GRayTraceReal discriminant = b * b - (GRayTraceReal)4 * c;
	if(discriminant < 0)
		return 0;
	GRayTraceReal dist = (b + (GRayTraceReal)sqrt(discriminant)) / (-2);
	if(dist > MIN_RAY_DISTANCE)
		return dist;
	dist = (b - (GRayTraceReal)sqrt(discriminant)) / (-2);
	return dist;
}

/*virtual*/ void GRayTraceSphere::ComputeNormalVector(GRayTraceVector* pOutNormalVector, GRayTraceVector* pPoint)
{
	pOutNormalVector->Copy(pPoint);
	pOutNormalVector->Subtract(&m_center);
	pOutNormalVector->Normalize();
}

// -----------------------------------------------------------------------------

GRayTraceTriMesh::GRayTraceTriMesh(GRayTraceMaterial* pMaterial, int nPoints, int nTriangles, int nNormals, int nTextureCoords)
: GRayTraceObject(pMaterial)
{
	m_nPoints = nPoints;
	m_pPoints = new GRayTraceVector[nPoints];
	m_nTriangles = nTriangles;
	m_pTriangles = new int[3 * nTriangles];
	GAssert(nNormals == 0 || nNormals == nPoints, "why are only some normals provided?");
	m_nNormals = nNormals;
	if(m_nNormals > 0)
		m_pNormals = new GRayTraceVector[nNormals];
	else
		m_pNormals = NULL;
	m_nTextureCoords = nTextureCoords;
	m_pTextureCoords = new GRayTraceReal[2 * m_nTextureCoords];
	m_bCulling = false;
	m_nHitTriangle = -1;
}

/*virtual*/ GRayTraceTriMesh::~GRayTraceTriMesh()
{
	delete[] m_pPoints;
	delete[] m_pTriangles;
	delete[] m_pNormals;
	delete[] m_pTextureCoords;
}

void GRayTraceTriMesh::SetPoint(int nIndex, GRayTraceVector* pPoint)
{
	GAssert(nIndex >= 0 && nIndex < m_nPoints, "out of range");
	m_pPoints[nIndex] = *pPoint;
}

void GRayTraceTriMesh::SetTriangle(int nIndex, int v1, int v2, int v3)
{
	GAssert(nIndex >= 0 && nIndex < m_nTriangles, "out of range");
	int* pTri = &m_pTriangles[3 * nIndex];
	pTri[0] = v1;
	pTri[1] = v2;
	pTri[2] = v3;
}

void GRayTraceTriMesh::SetNormal(int nIndex, GRayTraceVector* pNormal)
{
	GAssert(nIndex >= 0 && nIndex < m_nNormals, "out of range");
	m_pNormals[nIndex] = *pNormal;
}

void GRayTraceTriMesh::SetTextureCoord(int nIndex, GRayTraceReal x, GRayTraceReal y)
{
	GAssert(nIndex >= 0 && nIndex < m_nTextureCoords, "out of range");
	nIndex *= 2;
	m_pTextureCoords[nIndex] = x;
	m_pTextureCoords[nIndex + 1] = y;
}

bool GRayTraceTriMesh::IsPointWithinPlanarPolygon(GRayTraceVector* pPoint, GRayTraceVector** ppVertices, int nVertices)
{
	// Find the two dimensions with the most significant component (which
	// are the dimensions with the smallest component in the normal vector)
	GAssert(nVertices >= 3, "at least three points are needed to define a planar polygon");
	GRayTraceVector plane;
	plane.ComputeTriangleNormal(ppVertices[0], ppVertices[1], ppVertices[2]);
	plane.m_vals[0] = ABS(plane.m_vals[0]);
	plane.m_vals[1] = ABS(plane.m_vals[1]);
	plane.m_vals[2] = ABS(plane.m_vals[2]);
	int d1, d2;
	if(plane.m_vals[0] >= plane.m_vals[1] && plane.m_vals[0] >= plane.m_vals[2])
	{
		d1 = 1;
		d2 = 2;
	}
	else if(plane.m_vals[1] >= plane.m_vals[0] && plane.m_vals[1] >= plane.m_vals[2])
	{
		d1 = 0;
		d2 = 2;
	}
	else
	{
		d1 = 0;
		d2 = 1;
	}

	// Count the number of times a ray shot out from the point crosses
	// a side of the polygon
	int numCrossings = 0;
	int signHolder, nextSignHolder;
	if(ppVertices[0]->m_vals[d2] - pPoint->m_vals[d2] < 0)
		signHolder = -1;
	else
		signHolder = 1;
	int nVertex, nNextVertex;
	GRayTraceReal u0, u1, v0, v1;
	for(nVertex = 0; nVertex < nVertices; nVertex++)
	{
		nNextVertex = (nVertex + 1) % nVertices;
		v1 = ppVertices[nNextVertex]->m_vals[d2] - pPoint->m_vals[d2];
		if(v1 < 0)
			nextSignHolder = -1;
		else
			nextSignHolder = 1;
		if(signHolder != nextSignHolder)
		{
			u0 = ppVertices[nVertex]->m_vals[d1] - pPoint->m_vals[d1];
			u1 = ppVertices[nNextVertex]->m_vals[d1] - pPoint->m_vals[d1];
			v0 = ppVertices[nVertex]->m_vals[d2] - pPoint->m_vals[d2];
			if(u0 - v0 * (u1 - u0) / (v1 - v0) > 0)
				numCrossings++;
		}
		signHolder = nextSignHolder;
	}
	if(numCrossings & 1)
		return true;
	else
		return false;
}

GRayTraceReal GRayTraceTriMesh::ComputeRayDistanceToTriangle(int nTriangle, GRayTraceVector* pRayOrigin, GRayTraceVector* pRayDirection)
{
	// Compute the plane equasion Ax + By + Cz + D = 0
	int* pTriangle = &m_pTriangles[3 * nTriangle];
	GRayTraceVector plane; // The plane normal is the vector (A, B, C)
	GRayTraceReal d;
	plane.ComputePlaneEquasion(&m_pPoints[pTriangle[0]], &m_pPoints[pTriangle[1]], &m_pPoints[pTriangle[2]], &d);

	// Compute distance and point of intersection
	GRayTraceReal tmp = plane.DotProduct(pRayDirection);
	if(tmp >= 0)
	{
		if(tmp == 0)
			return 0; // the ray is paralell to the plane
		if(m_bCulling)
			return 0; // the ray hits the back side of the plane
		else
		{
			// Reverse the plane normal
			plane.Multiply(-1);
			d = -d;
			tmp = -tmp;
		}
	}
	GRayTraceReal distance = -(plane.DotProduct(pRayOrigin) + d) / tmp;
	if(distance <= 0)
		return 0; // the intersection point is behind the ray origin
	GRayTraceVector point(pRayDirection);
	point.Multiply(distance);
	point.Add(pRayOrigin);

	// Determine if the intersection point is within the triangle
	GRayTraceVector* pVertices[3];
	pVertices[0] = &m_pPoints[pTriangle[0]];
	pVertices[1] = &m_pPoints[pTriangle[1]];
	pVertices[2] = &m_pPoints[pTriangle[2]];
	if(!IsPointWithinPlanarPolygon(&point, pVertices, 3))
		return 0; // the ray misses the triangle
	return distance;
}

/*virtual*/ GRayTraceReal GRayTraceTriMesh::ComputeRayDistance(GRayTraceVector* pRayOrigin, GRayTraceVector* pRayDirection)
{
	GRayTraceReal dist;
	int i;
	GRayTraceReal closest = (GRayTraceReal)1e30;
	m_nHitTriangle = -1;
	for(i = 0; i < m_nTriangles; i++)
	{
		dist = ComputeRayDistanceToTriangle(i, pRayOrigin, pRayDirection);
		if(dist < closest && dist > MIN_RAY_DISTANCE)
		{
			closest = dist;
			m_nHitTriangle = i;
		}
	}
	if(m_nHitTriangle >= 0)
		return closest;
	else
		return 0;
}

/*virtual*/ void GRayTraceTriMesh::ComputeNormalVector(GRayTraceVector* pOutNormalVector, GRayTraceVector* pPoint)
{
	int* pTriangle = &m_pTriangles[3 * m_nHitTriangle];
//	if(m_pNormals)
//	{
//		// todo: implement Phong shading
//	}
//	else
	{
		pOutNormalVector->ComputeTriangleNormal(&m_pPoints[pTriangle[0]], &m_pPoints[pTriangle[1]], &m_pPoints[pTriangle[2]]);
	}
}

