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

struct GRect;

class VLoading : public ViewPort
{
protected:
	GImage* m_pImage;

public:
	VLoading(GRect* pRect);
	virtual ~VLoading();

	virtual void Draw(SDL_Surface *pScreen);

protected:
	void RefreshEntireImage();
};

#endif // __VLOADING_H__
