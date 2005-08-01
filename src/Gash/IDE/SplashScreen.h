/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __SPLASHSCREEN_H__
#define __SPLASHSCREEN_H__

#include <windows.h>

class SplashScreen
{
protected:
    HFONT m_hFont;
    HFONT m_hOldFont;
    HDC m_hDC;
    HDC m_hOldDC;
    HBITMAP m_hOldBitmap;
    HBITMAP m_hOldOldBitmap;
    int m_nScreenWidth;
    int m_nScreenHeight;
    int m_nFrame;
    RECT m_animationRect;
    int m_nAnimationWidth;
    int m_nAnimationHeight;

public:
    SplashScreen();
    virtual ~SplashScreen();

    void DoIt();
    bool DoFrame();
};

#endif // __SPLASHSCREEN_H__
