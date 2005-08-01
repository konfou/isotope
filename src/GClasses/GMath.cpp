/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GMath.h"
#include "string.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

/*static*/ double GMath::gamma(double x)
{
    int i, k, m;
    double ga, gr, r, z;

    static double g[] =
	{
        1.0,
        0.5772156649015329,
       -0.6558780715202538,
       -0.420026350340952e-1,
        0.1665386113822915,
       -0.421977345555443e-1,
       -0.9621971527877e-2,
        0.7218943246663e-2,
       -0.11651675918591e-2,
       -0.2152416741149e-3,
        0.1280502823882e-3,
       -0.201348547807e-4,
       -0.12504934821e-5,
        0.1133027232e-5,
       -0.2056338417e-6,
        0.6116095e-8,
        0.50020075e-8,
       -0.11812746e-8,
        0.1043427e-9,
        0.77823e-11,
       -0.36968e-11,
        0.51e-12,
       -0.206e-13,
       -0.54e-14,
        0.14e-14
	};

    if(x > 171.0)
		return 1e308; // This value is an overflow flag.
    if(x == (int)x)
	{
        if(x > 0.0)
		{
            ga = 1.0;               // use factorial
            for (i = 2; i < x; i++)
			{
				ga *= i;
            }
         }
         else
            ga = 1e308;
     }
     else 
	 {
        if(fabs(x) > 1.0) 
		{
            z = fabs(x);
            m = (int)z;
            r = 1.0;
            for (k = 1; k <= m; k++)
                r *= (z - k);
            z -= m;
        }
        else
            z = x;
        gr = g[24];
        for (k = 23; k >= 0; k--)
            gr = gr * z + g[k];
        ga = 1.0 / (gr*z);
        if(fabs(x) > 1.0)
		{
            ga *= r;
            if (x < 0.0)
                ga = -3.14159265358979323846 / (x * ga * sin(3.14159265358979323846 * x));
        }
    }
    return ga;
}

// This implements Newton's method for determining a
// polynomial f(t) that goes through all the control points
// pFuncValues at pTValues.  (You could then convert to a
// Bezier curve to get a Bezier curve that goes through those
// points.)  The polynomial coefficients are put in pFuncValues
// in the form c0 + c1*t + c2*t*t + c3*t*t*t + ...
/*static*/ void GMath::NewtonPolynomial(const double* pTValues, double* pFuncValues, int nPoints)
{
	// Calculate the coefficients to Newton's blending functions
	double* pNC = (double*)alloca(nPoints * sizeof(double));
	memcpy(pNC, pFuncValues, nPoints * sizeof(double));
	int n, i;
	for(n = 1; n < nPoints; n++)
	{
		for(i = nPoints - n - 1; i >= 0; i--)
		{
			pNC[n + i] -= pNC[n + i - 1];
			pNC[n + i] /= (pTValues[n + i] - pTValues[i]);
		}
	}

	// Accumulate into polynomial coefficients
	double* pBlending = (double*)alloca(nPoints * sizeof(double));
	for(n = 1; n < nPoints; n++)
	{
		pBlending[n] = 0;
		pFuncValues[n] = 0;
	}
	pBlending[0] = 1;
	pFuncValues[0] = pNC[0];
	for(n = 1; n < nPoints; n++)
	{
		for(i = n; i > 0; i--)
			pBlending[i] -= pTValues[n - 1] * pBlending[i - 1];
		for(i = 0; i <= n; i++)
			pFuncValues[n - i] += pNC[n] * pBlending[i];
	}
}

