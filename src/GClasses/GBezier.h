/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GBEZIER_H__
#define __GBEZIER_H__

#include <math.h>
#include "GMacros.h"

class GXMLTag;

struct Point3D
{
public:
	double m_vals[3];

	Point3D()
	{
		m_vals[0] = 0;
		m_vals[1] = 0;
		m_vals[2] = 0;
	}

	Point3D(double x, double y, double z)
	{
		m_vals[0] = x;
		m_vals[1] = y;
		m_vals[2] = z;
	}

	inline double GetDistanceSquared(const Point3D* pThat) const
	{
		return (pThat->m_vals[0] - m_vals[0]) * (pThat->m_vals[0] - m_vals[0]) +
			(pThat->m_vals[1] - m_vals[1]) * (pThat->m_vals[1] - m_vals[1]) +
			(pThat->m_vals[2] - m_vals[2]) * (pThat->m_vals[2] - m_vals[2]);
	}

	inline double GetDistanceFromOriginSquared() const
	{
		return (m_vals[0] * m_vals[0] + m_vals[1] * m_vals[1] + m_vals[2] * m_vals[2]);
	}

	inline void Add(const Point3D* pThat)
	{
		m_vals[0] += pThat->m_vals[0];
		m_vals[1] += pThat->m_vals[1];
		m_vals[2] += pThat->m_vals[2];
	}

	inline void Subtract(const Point3D* pThat)
	{
		m_vals[0] -= pThat->m_vals[0];
		m_vals[1] -= pThat->m_vals[1];
		m_vals[2] -= pThat->m_vals[2];
	}

	inline void Multiply(double mag)
	{
		m_vals[0] *= mag;
		m_vals[1] *= mag;
		m_vals[2] *= mag;
	}

	inline void DotProduct(const Point3D* pThat)
	{
		m_vals[0] *= pThat->m_vals[0];
		m_vals[1] *= pThat->m_vals[1];
		m_vals[2] *= pThat->m_vals[2];
	}

	inline void GetLatLon(double* pdLat, double* pdLon) const
	{
		*pdLat = atan2(m_vals[1], sqrt(m_vals[0] * m_vals[0] + m_vals[2] * m_vals[2]));
		*pdLon = atan2(m_vals[0], m_vals[2]);
		if((*pdLon) < (-PI / (double)2))
			(*pdLon) += (2 * PI);
	}

	void Transform(const struct Transform* pTransform);
	void Untransform(const struct Transform* pTransform);
	void FromXML(GXMLTag* pTag);
	void ToXML(GXMLTag* pTag);
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
/*
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
*/
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







struct GBezierPoint;

// Represents a Bezier curve
class GBezier
{
protected:
	int m_nControlPoints;
	struct GBezierPoint* m_pPoints;

public:
	GBezier(int nControlPoints);
	GBezier(GBezier* pThat);
	~GBezier();

	// Returns the number of control points in this curve (which is always
	// one more than the degree of the curve).
	int GetControlPointCount();

	// Returns a point on the curve
	void GetPoint(double t, struct Point3D* pOutPoint);

	// Returns a control point
	void GetControlPoint(struct Point3D* pOutPoint, double* pOutWeight, int n);

	// Sets a control point
	void SetControlPoint(int n, struct Point3D* pPoint, double weight);
	GBezier* Copy();

	// Increases the degree (and number of control points) of the curve by one
	// without changing the curve.  (Only the control points are changed.)
	void ElevateDegree();

	// Crops the curve.  If bTail is true, only the end of the curve remains.  If
	// bTail is false, only the beginning of the curve remains.
	void GetSegment(double t, bool bTail);

	// Returns the tangeant to the curve at t=0
	void DerivativeAtZero(struct Point3D* pOutPoint);

	// todo: this method is not reliable--there's a bug in this method somewhere
	double CurvatureAtZero();

	// Example: If you have the three equasions:  x=1+2t+3t*t, y=4+5t+6t*t, z=7+8t+9t*t then you would
	//          pass in this array of points: { (1, 4, 7), (2, 5, 8), (3, 6, 9) } to get the equivalent Bezier curve
	static GBezier* FromPolynomial(struct Point3D* pCoefficients, int nCoefficients);

	// This expects you to pass in a pointer to an array of struct Point3D of size m_nControlPoints
	void ToPolynomial(struct Point3D* pOutCoefficients);

protected:
	struct GBezierPoint* GBezier::deCasteljau(double t, bool bTail);

};





struct GNurbsPoint;

// NURBS = Non Uniform Rational B-Spline
// Periodic = closed loop
class GNurbs
{
protected:
	int m_nControlPoints;
	int m_nDegree;
	struct GNurbsPoint* m_pPoints;
	bool m_bPeriodic;

public:
	GNurbs(int nControlPoints, int nDegree, bool periodic);
	GNurbs(GNurbs* pThat);
	~GNurbs();

	// Returns the number of control points
	int GetControlPointCount() { return m_nControlPoints; }
	
	// Returns a control point and the associated weight
	void GetControlPoint(struct Point3D* pOutPoint, double* pOutWeight, int n);
	
	// Set a control point location and its weight
	void SetControlPoint(int n, struct Point3D* pPoint, double weight);
	
	double GetKnotInterval(int n);
	void SetKnotInterval(int n, double dInterval);
	
	// Create a Bezier curve that exactly fits a portion of this curve.
	// todo: what are acceptable values for nInterval?
	GBezier* GetBezier(int nInterval);

	// 0 <= dRatio <= 1, (for example, .5 means to insert the knot in the center of the interval)
	void InsertKnotPeriodic(int nInterval, double dRatio);

protected:
	void GetPointCircular(struct GBezierPoint* pOutPoint, int n);
	double GetKnotValue(int n);
	void CalculateBezierControlPoint(struct GBezierPoint* pOutPoint, int nInterval, int nControlPoint);
	void GetNewKnotPeriodic(struct GBezierPoint* pA, struct GBezierPoint* pB, int n, int nControlPoint, double dRatio);
};



#endif // __GBEZIER_H__
