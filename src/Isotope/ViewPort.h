/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__

#include <SDL/SDL.h>
#include "../GClasses/GMacros.h"
#include "../GClasses/GImage.h"

class View;
class GWidgetGroup;
class GWidgetTextButton;

// This class represents a single layer of the main view.  (For example, if you have the game
// in the background and a menu on top of it, then the view consists of two ViewPorts.)
class ViewPort
{
protected:
	GRect m_rect;
	int m_nOpacity; // 0 = Invisible, 1-254 = Translucent, 255 = Opaque

public:
	ViewPort(GRect* pRect)
	{
		m_nOpacity = 255;
		m_rect = *pRect;
	}

	virtual ~ViewPort()
	{
	}

	virtual void Draw(SDL_Surface *pScreen) = 0;
	virtual void OnChar(char c) = 0;
	virtual void OnMouseDown(int x, int y) = 0;
	virtual void OnMouseUp(int x, int y) = 0;
	virtual void OnMousePos(int x, int y) = 0;

	int GetOpacity() { return m_nOpacity; }
	void SetOpacity(int n) { GAssert(n >= 0 && n < 256, "out of range"); m_nOpacity = n; }
	GRect* GetRect() { return &m_rect; }
	static void BlitImage(SDL_Surface* pScreen, int x, int y, GImage* pImage);
	static void StretchClipAndBlitImage(SDL_Surface* pScreen, GRect* pDestRect, GRect* pClipRect, GImage* pImage);
};


inline Uint32* getPixMem32(SDL_Surface *surface, int x, int y)
{
	return (Uint32*)((Uint8*)surface->pixels + y * surface->pitch + (x << 2));
}

inline Uint16* getPixMem16(SDL_Surface *pScreen, int x, int y)
{
	return (Uint16*)pScreen->pixels + y * pScreen->pitch / 2 + x;
}

/*
inline void putpixel32(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    *(Uint32*)((Uint8*)surface->pixels + y * surface->pitch + (x << 2)) = pixel;
}
*/

#endif // __VIEWPORT_H__
