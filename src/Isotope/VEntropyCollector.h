/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VENTROPYCOLLECTOR_H__
#define __VENTROPYCOLLECTOR_H__

#include "ViewPort.h"

class MKeyPair;
struct GRect;

// This shows a view of server status
class VEntropyCollector : public ViewPort
{
protected:
	GImage* m_pImage;
	MKeyPair* m_pModel;

public:
	VEntropyCollector(GRect* pRect, MKeyPair* pModel);
	virtual ~VEntropyCollector();

	virtual void Draw(SDL_Surface *pScreen);
	virtual void OnChar(char c) {}
	virtual void OnMouseDown(int x, int y) {}
	virtual void OnMouseUp(int x, int y) {}
	virtual void OnMousePos(int x, int y) {}
};

#endif // __VENTROPYCOLLECTOR_H__
