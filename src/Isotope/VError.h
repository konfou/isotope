/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VERROR_H__
#define __VERROR_H__

#include "ViewPort.h"

class VErrorDialog;

struct GRect;

class VError : public ViewPort
{
protected:
	GImage* m_pImage;
	int m_nLeft;
	int m_nTop;
	VErrorDialog* m_pDialog;
	bool m_dirty;

public:
	VError(GRect* pRect, const char* szUrl);
	virtual ~VError();

	virtual void Draw(SDL_Surface *pScreen);
	virtual void OnChar(char c) {}
	virtual void OnMouseDown(int x, int y) {}
	virtual void OnMouseUp(int x, int y) {}
	virtual void OnMousePos(int x, int y) {}

protected:
	void RefreshEntireImage();
};

#endif // __VERROR_H__
