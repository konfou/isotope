/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VLOADING_H__
#define __VLOADING_H__

#include "ViewPort.h"

class MLoadingDialog;

struct GRect;

class VLoading : public ViewPort
{
protected:
	GImage* m_pImage;
	int m_nLeft;
	int m_nTop;
	MLoadingDialog* m_pDialog;
	bool m_dirty;
	float m_fudgeFactor;
	double m_dPrevProgressTime;

public:
	VLoading(GRect* pRect, const char* szUrl);
	virtual ~VLoading();

	virtual void Draw(SDL_Surface *pScreen);
	void SetUrl(const char* szUrl);
	void SetProgress(float f);

protected:
	void RefreshEntireImage();
};

#endif // __VLOADING_H__
