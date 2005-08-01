/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GHARDFONT_H__
#define __GHARDFONT_H__

void GSmallHardFont_GetCharCoords(char c, int* pX, int* pY, int* pWidth, int* pHeight);
void GHardFont_GetCharCoords(char c, int* pX, int* pY, int* pWidth, int* pHeight);
unsigned int GHardFont_GetWhiteOnBlack(int x, int y);
unsigned int GHardFont_GetBlackOnWhite(int x, int y);
unsigned int GHardFont_GetAlphaBlended(int x, int y, unsigned int nColor, unsigned int nBackground);
unsigned int GSmallHardFont_GetAlphaBlended(int x, int y, unsigned int nColor, unsigned int nBackground);

#endif // __GHARDFONT_H__
