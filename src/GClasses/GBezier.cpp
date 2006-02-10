/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GBezier.h"
#include "GRayTrace.h"
#include "GXML.h"
#include "GMacros.h"
#include <stdlib.h>

void Point3D::Transform(const struct Transform* pTransform)
{
	double c, s, t;

	// Size
	m_vals[0] *= pTransform->dScale;
	m_vals[1] *= pTransform->dScale;
	m_vals[2] *= pTransform->dScale;

	// Roll
//	c = pTransform->GetCosRoll();
//	s = pTransform->GetSinRoll();
//	t = x;
//	x = x * c + y * s;
//	y = y * c - t * s;

	// Pitch;
	c = pTransform->GetCosPitch();
	s = pTransform->GetSinPitch();
	t = m_vals[1];
	m_vals[1] = m_vals[1] * c + m_vals[2] * s;
	m_vals[2] = m_vals[2] * c - t * s;

	// Yaw
	c = pTransform->GetCosYaw();
	s = pTransform->GetSinYaw();
	t = m_vals[2];
	m_vals[2] = m_vals[2] * c + m_vals[0] * s;
	m_vals[0] = m_vals[0] * c - t * s;

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
	t = m_vals[2];
	m_vals[2] = m_vals[2] * c - m_vals[0] * s;
	m_vals[0] = m_vals[0] * c + t * s;

	// Pitch;
	c = pTransform->GetCosPitch();
	s = pTransform->GetSinPitch();
	t = m_vals[1];
	m_vals[1] = m_vals[1] * c - m_vals[2] * s;
	m_vals[2] = m_vals[2] * c + t * s;
	// Roll
//	c = pTransform->GetCosRoll();
//	s = pTransform->GetSinRoll();
//	t = x;
//	x = x * c - y * s;
//	y = y * c + t * s;

	// Size
	m_vals[0] /= pTransform->dScale;
	m_vals[1] /= pTransform->dScale;
	m_vals[2] /= pTransform->dScale;
}

void Point3D::FromXML(GXMLTag* pTag)
{
	GXMLAttribute* pAttr;
	pAttr = pTag->GetAttribute("x");
	if(pAttr)
		m_vals[0] = atof(pAttr->GetValue());
	pAttr = pTag->GetAttribute("y");
	if(pAttr)
		m_vals[1] = atof(pAttr->GetValue());
	pAttr = pTag->GetAttribute("z");
	if(pAttr)
		m_vals[2] = atof(pAttr->GetValue());
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

void Point3D::ToXML(GXMLTag* pTag)
{
	char szBuff[256];
	pTag->AddAttribute(new GXMLAttribute("x", dtoa(m_vals[0], szBuff)));
	pTag->AddAttribute(new GXMLAttribute("y", dtoa(m_vals[1], szBuff)));
	pTag->AddAttribute(new GXMLAttribute("z", dtoa(m_vals[2], szBuff)));
}


// --------------------------------------------------------------------



void Transform::FromXML(GXMLTag* pTag)
{
	offset.FromXML(pTag);
	GXMLAttribute* pAttr;
	pAttr = pTag->GetAttribute("Scale");
	if(pAttr)
		dScale = atof(pAttr->GetValue());
//	pAttr = pTag->GetAttribute("Roll");
//	if(pAttr)
//		dRoll = atof(pAttr->GetValue());
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
//	if(dRoll != 0)
//		pTag->AddAttribute(new GXMLAttribute("Roll", dtoa(dScale, szBuff)));
	if(dPitch != 0)
		pTag->AddAttribute(new GXMLAttribute("Pitch", dtoa(dScale, szBuff)));
	if(dYaw != 0)
		pTag->AddAttribute(new GXMLAttribute("Yaw", dtoa(dScale, szBuff)));
}


// --------------------------------------------------------------------


struct GBezierPoint
{
	struct Point3D point;
	double weight;

	inline void Multiply(double dMag)
	{
		point.Multiply(dMag);
		weight *= dMag;
	}

	inline void Add(GBezierPoint* pPoint)
	{
		point.Add(&pPoint->point);
		weight += pPoint->weight;
	}
};

GBezier::GBezier(int nControlPoints)
{
	m_nControlPoints = nControlPoints;
	m_pPoints = new GBezierPoint[nControlPoints];
}

GBezier::GBezier(GBezier* pThat)
{
	m_nControlPoints = pThat->m_nControlPoints;
	m_pPoints = new GBezierPoint[m_nControlPoints];
	struct Point3D point;
	double weight;
	int n;
	for(n = 0; n < m_nControlPoints; n++)
	{
		pThat->GetControlPoint(&point, &weight, n);
		SetControlPoint(n, &point, weight);
	}
}

GBezier::~GBezier()
{
	delete(m_pPoints);
}

GBezier* GBezier::Copy()
{
	GBezier* pNewBezier = new GBezier(m_nControlPoints);
	struct Point3D point;
	double weight;
	int n;
	for(n = 0; n < m_nControlPoints; n++)
	{
		GetControlPoint(&point, &weight, n);
		pNewBezier->SetControlPoint(n, &point, weight);
	}
	return pNewBezier;
}

// Determine the point on the rational Bezier curve using the Horner algorithm
void GBezier::GetPoint(double t, struct Point3D* pOutPoint)
{
	int n = m_nControlPoints - 1;
	if(t == 1.0)
	{
		// Special case because the calcualtion for "u" below can't handle the case where t is 1
		*pOutPoint = m_pPoints[n].point;
		return;
	}
	double fw = m_pPoints[n].weight;
	pOutPoint->m_vals[0] = m_pPoints[n].point.m_vals[0] * fw;
	pOutPoint->m_vals[1] = m_pPoints[n].point.m_vals[1] * fw;
	pOutPoint->m_vals[2] = m_pPoints[n].point.m_vals[2] * fw;
	double u = t / (1.0 - t); // t/(1-t) in "monomialization trick" 
	double bc = 1.0; // binomial coefficient
	double den = 1.0; // denominator in P(t)/(1-t)^n
	double aux; // auxiliary variable - efficiency

	int i;
	for (i = n - 1; i >= 0; i--)
	{
		bc = (bc * (i + 1)) / (n - i); // new binomial coefficient 
		aux = bc * m_pPoints[i].weight;
		pOutPoint->m_vals[0] = pOutPoint->m_vals[0] * u + m_pPoints[i].point.m_vals[0] * aux;
		pOutPoint->m_vals[1] = pOutPoint->m_vals[1] * u + m_pPoints[i].point.m_vals[1] * aux;
		pOutPoint->m_vals[2] = pOutPoint->m_vals[2] * u + m_pPoints[i].point.m_vals[2] * aux;
		fw = fw * u + aux;
		den *= (1.0 - t);
	}

	pOutPoint->m_vals[0] /= fw;
	pOutPoint->m_vals[1] /= fw;
	pOutPoint->m_vals[2] /= fw;
}

int GBezier::GetControlPointCount()
{
	return m_nControlPoints;
}

void GBezier::GetControlPoint(struct Point3D* pOutPoint, double* pOutWeight, int n)
{
	GAssert(n >= 0 && n <= m_nControlPoints, "out of range");
	*pOutPoint = m_pPoints[n].point;
	*pOutWeight = m_pPoints[n].weight;
}

void GBezier::SetControlPoint(int n, struct Point3D* pPoint, double weight)
{
	GAssert(n >= 0 && n <= m_nControlPoints, "out of range");
	m_pPoints[n].point = *pPoint;
	m_pPoints[n].weight = weight;
}

void GBezier::ElevateDegree()
{
	// Allocate the new control points and set the first and last one
	GBezierPoint* pNewControlPoints = new GBezierPoint[m_nControlPoints + 1];
	pNewControlPoints[0] = m_pPoints[0];
	pNewControlPoints[m_nControlPoints] = m_pPoints[m_nControlPoints - 1];

	// Convert from weighted (rational) 3D control points to integral (non-rational) 4D control points
	int n;
	for(n = 0; n < m_nControlPoints; n++)
		m_pPoints[n].point.Multiply(m_pPoints[n].weight);

	// Determine the new intermediate control points
	struct GBezierPoint p1;
	struct GBezierPoint p2;
	for(n = 1; n < m_nControlPoints; n++)
	{
		// Find the new 4D control point
		double dFac = (double)n / (m_nControlPoints);
		p1 = m_pPoints[n - 1];
		p1.Multiply(dFac);
		p2 = m_pPoints[n];
		p2.Multiply(1.0 - dFac);
		pNewControlPoints[n] = p1;
		pNewControlPoints[n].Add(&p2);

		// Convert back to a weighted (rational) 3D point
		pNewControlPoints[n].point.Multiply(1.0 / pNewControlPoints[n].weight);
	}

	// Swap in the new control points
	delete(m_pPoints);
	m_pPoints = pNewControlPoints;
	m_nControlPoints++;
}

// Returns a 2D triangular array of 4D control points determined by the deCasteljau algorithm
struct GBezierPoint* GBezier::deCasteljau(double t, bool bTail)
{
	// Convert from weighted (rational) 3D control points to integral (non-rational) 4D control points
	int n;
	for(n = 0; n < m_nControlPoints; n++)
		m_pPoints[n].point.Multiply(m_pPoints[n].weight);

	// Populate the biggest row with the current control points
	GBezierPoint* pDeCasteljauPoints = (GBezierPoint*)alloca(sizeof(GBezierPoint) * m_nControlPoints * (m_nControlPoints + 1) / 2);
	int y = m_nControlPoints - 1;
	n = (y * (y + 1)) / 2;
	int x;
	for(x = 0; x < m_nControlPoints; x++)
		pDeCasteljauPoints[n + x] = m_pPoints[x];

	// Calculate the rest of the points
	struct GBezierPoint p1;
	struct GBezierPoint p2;
	for(y--; y >= 0; y--)
	{
		n = (y * (y + 1)) / 2;
		for(x = 0; x <= y; x++)
		{
			p1 = pDeCasteljauPoints[n + x + y + 1];
			p2 = pDeCasteljauPoints[n + x + y + 2];
			p1.Multiply(1.0 - t);
			p2.Multiply(t);
			pDeCasteljauPoints[n + x] = p1;
			pDeCasteljauPoints[n + x].Add(&p2);
		}
	}

	// Extract the segment control points and convert back to weighted (rational) 3D points
	GBezierPoint* pSegment = new GBezierPoint[m_nControlPoints];
	for(y = 0; y < m_nControlPoints; y++)
	{
		if(bTail)
			n = (y * (y + 1)) / 2 + y;
		else
			n = ((m_nControlPoints - 1 - y) * (m_nControlPoints - y)) / 2;
		pSegment[y] = pDeCasteljauPoints[n];
		pSegment[y].point.Multiply(1 / pSegment[y].weight);
	}
	return pSegment;
}

void GBezier::GetSegment(double t, bool bTail)
{
	GBezierPoint* pNewControlPoints = deCasteljau(t, bTail);
	delete(m_pPoints);
	m_pPoints = pNewControlPoints;
}

void GBezier::DerivativeAtZero(struct Point3D* pOutPoint)
{
	GAssert(m_nControlPoints > 1, "undefined with only one point");
	*pOutPoint = m_pPoints[1].point;
	pOutPoint->Subtract(&m_pPoints[0].point);
	double d = m_pPoints[1].weight * (m_nControlPoints - 1) / m_pPoints[0].weight;
	pOutPoint->Multiply(d);
}

double GBezier::CurvatureAtZero()
{
	if(m_nControlPoints < 3)
		return 0;

	struct Point3D vec1 = m_pPoints[1].point;
	vec1.Subtract(&m_pPoints[0].point);
	double aSquared = vec1.GetDistanceFromOriginSquared();
	vec1.Multiply(1.0 / sqrt(aSquared));

	struct Point3D vec2 = m_pPoints[2].point;
	vec2.Subtract(&m_pPoints[1].point);
	double dSquared = vec2.GetDistanceFromOriginSquared();
	vec2.DotProduct(&vec1);
	double h = sqrt(dSquared - vec2.GetDistanceFromOriginSquared());

	return m_pPoints[0].weight * m_pPoints[2].weight / (m_pPoints[1].weight * m_pPoints[1].weight) * h / aSquared * (m_nControlPoints - 2) / (m_nControlPoints - 1);
}

void PascalsTriangle(int* pOutRow, int nRow)
{
	GAssert(nRow > 0, "n must be >= 1");
	int i, j;
	for(i = 0; i < nRow; i++)
	{
		pOutRow[i] = 1;
		for(j = i - 1; j > 0; j--)
			pOutRow[j] = pOutRow[j - 1] + pOutRow[j];
	}
}

double DoPolarEquasion(int n, double* pPolarCoords, int nPolarCoords)
{
	GAssert(nPolarCoords >= n, "nPolarCoords must be bigger than n");
	int* pPermutations = (int*)alloca(n * sizeof(int));
	int i;
	for(i = 0; i < n; i++)
		pPermutations[i] = n - 1 - i;
	int pos;
	double dSum = 0;
	int nPermutations = 0;
	while(true)
	{
		nPermutations++;
		double dTemp = 1;
		for(i = 0; i < n; i++)
		{
			GAssert(pPermutations[i] < nPolarCoords, "internal error");
			dTemp *= pPolarCoords[pPermutations[i]];
		}
		dSum += dTemp;
		for(pos = 0; pos < n && pPermutations[pos] >= nPolarCoords - 1 - pos; pos++)
		{
		}
		if(pos >= n)
			break;
		pPermutations[pos]++;
		for( ; pos > 0; pos--)
			pPermutations[pos - 1] = pPermutations[pos] + 1;
	}
	return dSum / nPermutations;
}

void CalculateBernsteinCoefficient(Point3D* pOutPoint, double* pPolarCoords, struct Point3D* pPolyCoeff, int nPolyCoeff)
{
	pOutPoint->m_vals[0] = 0;
	pOutPoint->m_vals[1] = 0;
	pOutPoint->m_vals[2] = 0;
	double tmp;
	int n;
	for(n = 0; n < nPolyCoeff; n++)
	{
		tmp = DoPolarEquasion(n, pPolarCoords, nPolyCoeff - 1);
		pOutPoint->m_vals[0] += pPolyCoeff[n].m_vals[0] * tmp;
		pOutPoint->m_vals[1] += pPolyCoeff[n].m_vals[1] * tmp;
		pOutPoint->m_vals[2] += pPolyCoeff[n].m_vals[2] * tmp;
	}
}

/*static*/ GBezier* GBezier::FromPolynomial(struct Point3D* pCoefficients, int nCoefficients)
{
	double* pPolarCoords = (double*)alloca((nCoefficients - 1) * sizeof(double));
	int n;
	for(n = 0; n < nCoefficients - 1; n++)
		pPolarCoords[n] = 0;
	GBezier* pNewBezier = new GBezier(nCoefficients);
	struct Point3D point;
	CalculateBernsteinCoefficient(&point, pPolarCoords, pCoefficients, nCoefficients);
	pNewBezier->SetControlPoint(0, &point, 1);
	for(n = 0; n < nCoefficients - 1; n++)
	{
		pPolarCoords[n] = 1;
		CalculateBernsteinCoefficient(&point, pPolarCoords, pCoefficients, nCoefficients);
		pNewBezier->SetControlPoint(n + 1, &point, 1);
	}
	return pNewBezier;
}

void GBezier::ToPolynomial(struct Point3D* pOutCoefficients)
{
	int n;
	for(n = 0; n < m_nControlPoints; n++)
	{
		pOutCoefficients[n] = m_pPoints[n].point;
		GAssert(m_pPoints[n].weight = m_pPoints[0].weight, "Rational (weighted) Bezier curves can't be represented as a polynomial--pretending all weights are the same");
	}
	int i;
	for(i = 1; i < m_nControlPoints; i++)
	{
		for(n = m_nControlPoints - 1; n >= i; n--)
			pOutCoefficients[n].Subtract(&pOutCoefficients[n - 1]);
	}
	int* pPascalsTriangle = (int*)alloca(m_nControlPoints * sizeof(int));
	PascalsTriangle(pPascalsTriangle, m_nControlPoints);
	for(n = 0; n < m_nControlPoints; n++)
		pOutCoefficients[n].Multiply((double)pPascalsTriangle[n]);
}

// ---------------------------------------------------------------------------------------------------

struct GNurbsPoint
{
	struct Point3D point;
	double weight;
	double knotInterval;
};

GNurbs::GNurbs(int nControlPoints, int nDegree, bool periodic)
{
	GAssert(m_nControlPoints > m_nDegree, "You must have more control points than the degree");
	m_nControlPoints = nControlPoints;
	m_nDegree = nDegree;
	m_pPoints = new GNurbsPoint[nControlPoints];
	m_bPeriodic = periodic;
}

GNurbs::GNurbs(GNurbs* pThat)
{
	m_pPoints = new GNurbsPoint[pThat->m_nControlPoints];
	m_nDegree = pThat->m_nDegree;
	m_nControlPoints = pThat->m_nControlPoints;
	int n;
	for(n = 0; n < m_nControlPoints; n++)
		m_pPoints[n] = pThat->m_pPoints[n];
	m_bPeriodic = pThat->m_bPeriodic;
}

GNurbs::~GNurbs()
{
	delete(m_pPoints);
}

void GNurbs::GetControlPoint(struct Point3D* pOutPoint, double* pOutWeight, int n)
{
	*pOutPoint = m_pPoints[n].point;
	*pOutWeight = m_pPoints[n].weight;
}

void GNurbs::SetControlPoint(int n, struct Point3D* pPoint, double weight)
{
	m_pPoints[n].point = *pPoint;
	m_pPoints[n].weight = weight;
}

double GNurbs::GetKnotInterval(int n)
{
	return m_pPoints[n].knotInterval;
}

void GNurbs::SetKnotInterval(int n, double dInterval)
{
	m_pPoints[n].knotInterval = dInterval;
}

// This maps both positive and negative numbers to a positive modulus of n
inline int WrapAround(int i, int n)
{
	return ((i % n) + n) % n;
}

void GNurbs::GetPointCircular(struct GBezierPoint* pOutPoint, int n)
{
	int nControlPoint = WrapAround(n, m_nControlPoints);
	pOutPoint->point = m_pPoints[nControlPoint].point;
	pOutPoint->weight = m_pPoints[nControlPoint].weight;
}

double GNurbs::GetKnotValue(int n)
{
	if(m_bPeriodic)
	{
		if(n < 0)
		{
			double dSum = 0;
			int i;
			for(i = -1; i >= n; i--)
			{
				int nKnotIndex = WrapAround(i, m_nControlPoints);
				dSum -= m_pPoints[nKnotIndex].knotInterval;
			}
			return dSum;
		}
		else if(n > 0)
		{
			double dSum = 0;
			int i;
			for(i = 0; i < n; i++)
				dSum += m_pPoints[i % m_nControlPoints].knotInterval;
			return dSum;
		}
		else
			return 0;
	}
	else
	{
		if(n <= 0)
			return 0;
		if(n > m_nControlPoints)
			n = m_nControlPoints;
		double dSum = 0;
		int i;
		for(i = 0; i < n; i++)
			dSum += m_pPoints[i].knotInterval;
		return dSum;
	}
}

void MixPoints(struct GBezierPoint* pA, const struct GBezierPoint* pB, double dA, double dTarget, double dB)
{
	struct GBezierPoint temp = *pB;
	temp.Multiply(dTarget - dA);
	pA->Multiply(dB - dTarget);
	pA->Add(&temp);
	pA->Multiply(1.0 / (dB - dA));
}

void GNurbs::CalculateBezierControlPoint(struct GBezierPoint* pOutPoint, int nInterval, int nBezierControlPoint)
{
	GAssert(nBezierControlPoint < m_nDegree - 1, "This method won't get the last control point in the Bezier curve.  Use the first control point of the next interval instead (since they're the same point).");
	struct GBezierPoint* pPoints = (struct GBezierPoint*)alloca(m_nDegree * sizeof(struct GBezierPoint));
	int nStartPoint = nInterval - (m_nDegree >> 1);
	int n;
	for(n = 0; n < m_nDegree; n++)
		GetPointCircular(&pPoints[n], nStartPoint + n);
	int nStartKnotPos = nStartPoint - (m_nDegree >> 1);
	int nTargetKnotPos = nInterval;
	int nEndKnotPos = nStartKnotPos + m_nDegree;
	for(n = m_nDegree - 1; n > 0; n--)
	{
		if(nBezierControlPoint >= n)
			nTargetKnotPos = nInterval + 1;
		double dTargetKnot = GetKnotValue(nTargetKnotPos);
		int i;
		for(i = 0; i < n; i++)
		{
			double dAKnot = GetKnotValue(nStartKnotPos + i);
			double dBKnot = GetKnotValue(nEndKnotPos + i);
			GAssert(dAKnot <= dTargetKnot && dTargetKnot <= dBKnot, "Something seems wrong here");
			MixPoints(&pPoints[i], &pPoints[i + 1], dAKnot, dTargetKnot, dBKnot);
		}
		nStartKnotPos++;
	}
	*pOutPoint = pPoints[0];
}


GBezier* GNurbs::GetBezier(int nInterval)
{
	GBezier* pBez = new GBezier(m_nDegree + 1);
	struct GBezierPoint point;
	int n;
	for(n = 0; n < m_nDegree; n++)
	{
		CalculateBezierControlPoint(&point, nInterval, n);
		pBez->SetControlPoint(n, &point.point, point.weight);
	}
	CalculateBezierControlPoint(&point, (nInterval + 1) % m_nControlPoints, 0);
	pBez->SetControlPoint(m_nDegree, &point.point, point.weight);
	return pBez;
}

// This will mess up the value at pB and will return the answer in pA
void GNurbs::GetNewKnotPeriodic(struct GBezierPoint* pA, struct GBezierPoint* pB, int n, int nControlPoint, double dRatio)
{
	double dSum = 0;
	double dMin = 0;
	double dMax = 0;
	int i;
	nControlPoint -= ((m_nDegree - 1) >> 1);
	for(i = 0; i < m_nDegree; i++)
	{
		if(i == m_nDegree - 1 - n)
			dMin = dSum;
		dSum += GetKnotInterval(WrapAround(nControlPoint + i, m_nControlPoints));
		if(i == m_nDegree - 1 - n)
			dMax = dSum;
	}
	dMin /= dSum;
	dMax /= dSum;
	dRatio = dMin * (1.0 - dRatio) + dMax * dRatio;
	pA->Multiply(1.0 - dRatio);
	pB->Multiply(dRatio);
	pA->Add(pB);
}

void GNurbs::InsertKnotPeriodic(int nInterval, double dRatio)
{
	GAssert(m_bPeriodic, "This function is not implemented for non-periodic NURBS");
	struct GNurbsPoint* pNewPoints = new struct GNurbsPoint[m_nControlPoints + 1];
	int nStart = nInterval - (m_nDegree >> 1);
	struct GBezierPoint a;
	struct GBezierPoint b;
	double dNewKnotInterval;
	int n;
	for(n = 0; n < m_nDegree; n++)
	{
		int nControlPoint = WrapAround(n + nStart, m_nControlPoints);
		int nControlPoint2 = WrapAround(n + nStart + 1, m_nControlPoints);
		GetControlPoint(&a.point, &a.weight, nControlPoint);
		GetControlPoint(&b.point, &b.weight, nControlPoint2);
		GetNewKnotPeriodic(&a, &b, n, nControlPoint, dRatio);
		if(n < (m_nDegree >> 1) - 1)
			dNewKnotInterval = GetKnotInterval(nControlPoint2);
		else if(n > (m_nDegree >> 1))
			dNewKnotInterval = GetKnotInterval(nControlPoint);
		else if(n == (m_nDegree >> 1) - 1)
			dNewKnotInterval = dRatio * GetKnotInterval(nControlPoint2);
		else
			dNewKnotInterval = (1.0 - dRatio) * GetKnotInterval(nControlPoint2);
		pNewPoints[n].point = a.point;
		pNewPoints[n].weight = a.weight;
		pNewPoints[n].knotInterval = dNewKnotInterval;
	}
	for( ; n < m_nControlPoints + 1; n++)
		pNewPoints[n] = m_pPoints[WrapAround(nStart + n, m_nControlPoints)];
	delete(m_pPoints);
	m_pPoints = pNewPoints;
	m_nControlPoints++;
}

