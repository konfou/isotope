/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GPOLYNOMIAL_H__
#define __GPOLYNOMIAL_H__

class GArffRelation;
class GArffData;

// This represents a multi-dimensional polynomial
class GPolynomial
{
protected:
	int m_nDimensions;
	int m_nControlPoints;
	int m_nCoefficients;
	double* m_pCoefficients;

public:
	GPolynomial(int nDimensions, int nControlPoints);
	~GPolynomial();

	// Returns the coefficient at the specified degrees.  pDegrees should
	// be an array of size m_nDimensions
	double GetCoefficient(int* pDegrees);

	// Sets the coefficient at the specified degrees.  pDegrees should
	// be an array of size m_nDimensions
	void SetCoefficient(int* pDegrees, double dVal);

	// Evaluates the polynomial.  pCoordinates should be an array of size
	// m_nDimensions
	double Eval(double* pVariables);

	// Returns the mean square error of this polynomial's ability to predict
	// the specified output value of the given data
	double MeasureMeanSquareError(GArffRelation* pRelation, GArffData* pData, int nOutputAttr);

	// Generates a polynomial fitted to predict the provided data as best as possible
	static GPolynomial* FitData(GArffRelation* pRelation, GArffData* pData, int nOutputAttr, int nControlPoints);

	// Generates an optimal polynomial that uses the first "n - 1" inputs to calculate
	// threshold values for the last input such that the two halves can be fitted
	// with a polynomial as well as possible
	static GPolynomial* DivideData(GArffRelation* pRelation, GArffData* pData, int nOutputAttr, int nControlPoints);

	// This uses the multi-dimensional version of Newton's polynomial algorithm.
	// pControlPoints should point to an array of size (nControlPoints * nDimensions)
	// and pValues should point to an array of size (nControlPoints ^ nDimensions)
	// where "^" means "to the power of"
	// For example, to specify the points: (x1, y1, z1), (x2, y1, z1), ... (x2, y2, z2)
	// nDimensions would be 3, nControlPoints would be 2, pControlPoints would be
	// { x1, x2, y1, y2, z1, z2 }, and pValues would be
	// { f(x1, y1, z1), f(x2, y1, z1), f(x1, y2, z1), f(x2, y2, z1), ... f(x2, y2, z2) }
//	static GPolynomial* Newton(int nDimensions, int nControlPoints, double* pControlPoints, double* pValues);

protected:
	int CalcIndex(int* pDegrees);
	double DivideAndMeasureError(GArffRelation* pRelation, GArffData* pData, int nOutputAttr);
};


#endif // __GPOLYNOMIAL_H__
