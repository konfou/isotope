/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GTRIGTABLE_H__
#define __GTRIGTABLE_H__

class GPrecalculatedTrigTable
{
public:
	// nDivisions is the number of data points between 0 and PI / 2
	GPrecalculatedTrigTable(int nDivisions);
	virtual ~GPrecalculatedTrigTable();

	// d is in radians of course.
	// It will interpolate values not in the table
	double Sin(double d);

	// d is in radians of course.
	// It will interpolate values not in the table
	double Cos(double d);

protected:
	double* m_pTable;
	double m_dDivisionSize;
	int m_nDivisions;
};

#endif // __GTRIGTABLE_H__
