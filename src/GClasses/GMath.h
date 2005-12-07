/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GMATH_H__
#define __GMATH_H__

#include <math.h>

class GMath
{
public:
	// Converts an analog value in the range 0-1 to a digital value
	inline static int analogToDigital(double dVal, int nValues)
	{
		return (int)(dVal * nValues);
	}

	// Converts a digital value to analog.  Typically the digital
	// value will be discreet, but for applications of interpolation
	// it may be non-discreet, so it's a double instead of an int.
	inline static double digitalToAnalog(double nVal, int nValues)
	{
		return (.5 + nVal) / nValues;
	}


	// The sigmoid function.  It goes through the points
	// (-inf, 0) and (inf, 1) with a slope of 0 and through
	// (0, .5) with a slope related to steepness
	inline static double sigmoid(double x, double steepness)
	{
		return (double)1 / (exp(-steepness * x) + 1);
	}

	// This evaluates the derivative of the sigmoid function
	inline static double sigmoidDerivative(double x, double steepness)
	{
		double d = sigmoid(x, steepness);
		return steepness * d * ((double)1 - d);
	}

	// Calculates a function that always goes through (0, 0)
	// and (1, 1) with a slope of 0, and (0.5, 0.5) with a slope
	// that has some relation to steepness.  Here's an ascii-art
	// representation of the function:
	//                _---(1,1) 
	//               /
	//              /(0.5, 0.5)
	//            _/
	//    (0,0)---
	// This function is a derived from the Butterworth function,
	// but it's a little bit different.
	inline static double smoothedIdentity(double x, double steepness)
	{
		if(x > 1)
			return 1 / (1 + pow(1 - (x / 2), steepness));
		else
			return 1 - (1 / (1 + pow(x / 2, steepness)));
	}

	// The gamma function
	static double gamma(double x);

	// The gaussian function
	inline static double gaussian(double x)
	{
		return exp((x * x) / (-2));
	}

	// This implements Newton's method for determining a
	// polynomial f(t) that goes through all the control points
	// pFuncValues at pTValues.  (You could then convert to a
	// Bezier curve to get a Bezier curve that goes through those
	// points.)  The polynomial coefficients are put in pFuncValues
	// in the form c0 + c1*t + c2*t*t + c3*t*t*t + ...
	static void NewtonPolynomial(const double* pTValues, double* pFuncValues, int nPoints);
};

#endif // __GMATH_H__
