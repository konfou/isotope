/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MCOLLISIONMAP_H__
#define __MCOLLISIONMAP_H__

class GPointerArray;
class MCollisionMapNode;


struct FRect
{
public:
	float x, y, w, h;
};


class MCollisionMap
{
protected:
	GPointerArray* m_pSolidRects;
	MCollisionMapNode* m_pRoot;

public:
	MCollisionMap();
	~MCollisionMap();

	void AddSolidRect(FRect* pRect);
	void Compile();
	bool Check(float x, float y);

protected:
	void EstimateBestDivision(GPointerArray* pSolidRects, bool* pbX, float* pfValue, FRect* pBounds);
	bool FindBestDivision(GPointerArray* pSolidRects, bool* pbX, float* pfValue, FRect* pBounds);
	int EvaluateDivision(GPointerArray* pSolidRects, bool bX, float fValue, FRect* pBounds);
	MCollisionMapNode* CompileArea(GPointerArray* pSolidRects, FRect* pBounds);
};

#endif // __MCOLLISIONMAP_H__
