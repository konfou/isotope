/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GTrigTable.h"
#include <math.h>
#include "GMacros.h"

GPrecalculatedTrigTable::GPrecalculatedTrigTable(int nDivisions)
{
	m_nDivisions = nDivisions;
	m_dDivisionSize = (PI / 2) / nDivisions;
	m_pTable = new double[nDivisions + 1];
	int n;
	for(n = 0; n < nDivisions; n++)
		m_pTable[n] = sin(n * (PI / 2) / nDivisions);
	m_pTable[nDivisions] = 1;
}

GPrecalculatedTrigTable::~GPrecalculatedTrigTable()
{
	delete(m_pTable);
}

double GPrecalculatedTrigTable::Sin(double d)
{
	// Move d to between 0 and 2 * PI
	if(d >= (2 * PI))
		d -= (2 * PI * (int)(d / (2 * PI)));
	else if(d < 0)
		d += (2 * PI * ((int)((-d) / (2 * PI)) + 1));

	// Move d to between 0 and PI
	bool bNegate = false;
	if(d >= PI)
	{
		bNegate = true;
		d -= PI;
	}

	// Move d to between 0 and PI / 2
	if(d >= (PI / 2))
		d = PI - d;

	// Interpolate the value from the table
	int nDiv = (int)(d / m_dDivisionSize);
	if(nDiv >= m_nDivisions)
		nDiv = m_nDivisions - 1;
	double dPos = d - (nDiv * (PI / 2) / m_nDivisions);
	double dVal = dPos * m_pTable[nDiv + 1] + (1 - dPos) * m_pTable[nDiv];
	
	// Return the value
	if(bNegate)
		return -dVal;
	else
		return dVal;
}

double GPrecalculatedTrigTable::Cos(double d)
{
	return(Sin((PI / 2) - d));
}
