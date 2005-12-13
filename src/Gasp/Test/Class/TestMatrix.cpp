/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "TestMatrix.h"
#include "../../../GClasses/GMatrix.h"
#include "../ClassTests.h"

// *** This only tests:
// Multiply,
// Copy,
// Get,
// Set,
// GetDeterminant,
bool TestMatrix(ClassTests* pThis)
{
	GMatrix m1(5, 5);
	m1.Set(1, 1, 3);	m1.Set(1, 2, 2);	m1.Set(1, 3, 7);	m1.Set(1, 4, 2);	m1.Set(1, 5, 1);
	m1.Set(2, 1, 4);	m1.Set(2, 2, 6);	m1.Set(2, 3, 2);	m1.Set(2, 4, 6);	m1.Set(2, 5, 6);
	m1.Set(3, 1, 0);	m1.Set(3, 2, 2);	m1.Set(3, 3, 3);	m1.Set(3, 4, 2);	m1.Set(3, 5, 9);
	m1.Set(4, 1, 0);	m1.Set(4, 2, 3);	m1.Set(4, 3, 3);	m1.Set(4, 4, 3);	m1.Set(4, 5, 9);
	m1.Set(5, 1, 0);	m1.Set(5, 2, 6);	m1.Set(5, 3, 3);	m1.Set(5, 4, 2);	m1.Set(5, 5, 5);
	if(m1.GetDeterminant() != -960)
		return false;
	GMatrix m2(3, 3);
	m2.Copy(&m1);
	m2.Set(3, 3, 8.1f);
	m2.Set(3, 4, 8.8f);
	GMatrix m3(8, 8);
	m3.Multiply(&m1, &m2);
	if(m3.Get(5, 4) != 78.4)
		return false;
	return true;
}
