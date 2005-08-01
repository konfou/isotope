/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "SplashScreen.h"
#include <windows.h>

SplashScreen::SplashScreen()
{
	m_hFont = CreateFont(
	   64,                        // nHeight
	   0,                         // nWidth
	   0,                         // nEscapement
	   0,                         // nOrientation
	   0/*FW_BOLD*/,              // nWeight
	   FALSE,                     // bItalic
	   FALSE,                     // bUnderline
	   0,                         // cStrikeOut
	   ANSI_CHARSET,              // nCharSet
	   OUT_DEFAULT_PRECIS,        // nOutPrecision
	   CLIP_DEFAULT_PRECIS,       // nClipPrecision
	   DEFAULT_QUALITY,           // nQuality
	   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
	   "Lucida Calligraphy");     // lpszFacename
	m_hDC = GetDC(NULL);
    SetBkMode(m_hDC, TRANSPARENT);
    m_hOldFont = (HFONT)SelectObject(m_hDC, m_hFont);
    m_nScreenWidth = GetDeviceCaps(m_hDC, HORZRES);
    m_nScreenHeight = GetDeviceCaps(m_hDC, VERTRES);
    m_nAnimationWidth = 220;
    m_nAnimationHeight = 50;
    m_hOldDC = CreateCompatibleDC(m_hDC);
    m_hOldBitmap = CreateCompatibleBitmap(m_hDC, m_nAnimationWidth, m_nAnimationHeight);
    m_hOldOldBitmap = (HBITMAP)SelectObject(m_hOldDC, m_hOldBitmap);
    m_nFrame = 0;
}

SplashScreen::~SplashScreen()
{
    SelectObject(m_hOldDC, m_hOldOldBitmap);
	SelectObject(m_hDC, m_hOldFont);
}

void SplashScreen::DoIt()
{
    while(DoFrame())
        Sleep(20);
}

bool SplashScreen::DoFrame()
{
    // Some constants
    int nStartX = m_nScreenWidth / 2 - 58;
    int nStartY = 0;
    int nFinishX = nStartX;
    int nFinishY = m_nScreenHeight / 2 - 115;
    int nFrameCount = 15;
    
    // Restore old rect
    if(m_nFrame > 0)
        BitBlt(m_hDC, m_animationRect.left, m_animationRect.top, m_nAnimationWidth, m_nAnimationHeight, m_hOldDC, 0, 0, SRCCOPY);

    if(m_nFrame < nFrameCount)
    {
        // Move animation rect
	    m_animationRect.left = ((nFrameCount - m_nFrame) * nStartX + (m_nFrame * nFinishX)) / nFrameCount;
	    m_animationRect.top = ((nFrameCount - m_nFrame) * nStartY + (m_nFrame * nFinishY)) / nFrameCount;
        m_animationRect.right = m_animationRect.left + m_nAnimationWidth;
        m_animationRect.bottom = m_animationRect.top + m_nAnimationHeight;
	    const char* szMessage = "Gash";
	    SetTextColor(m_hDC, RGB(0, 0, 0));

        // Make a copy of the rect
        BitBlt(m_hOldDC, 0, 0, m_nAnimationWidth, m_nAnimationHeight, m_hDC, m_animationRect.left, m_animationRect.top, SRCCOPY);

        // Draw the animation
        DrawText(m_hDC, szMessage, strlen(szMessage), &m_animationRect, 0);

        m_nFrame++;

        // Not done yet
        return true;
    }

    // All done
    return false;
}
