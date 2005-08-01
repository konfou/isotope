/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MSPOT_H__
#define __MSPOT_H__

class MSpot
{
protected:
	float m_x;
	float m_y;

public:
	MSpot(float x, float y);
	~MSpot();

	void GetPos(float* x, float* y) { *x = m_x; *y = m_y; }
};

#endif // __MSPOT_H__
