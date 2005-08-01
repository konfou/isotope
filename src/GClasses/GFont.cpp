/*
	Copyright (C) 1999, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GDDraw.h"

GFont::GFont(GWindow* pScreen, const char* szFilename, int nBGCol, int nLetterWid, int nLetterHgt)
: GBlock(pScreen, szFilename, nBGCol, nLetterWid, nLetterHgt)
{

}

GFont::~GFont()
{

}

void GFont::PrintString(int nXPos, int nYPos, const char* szText)
{
   int x = nXPos;
   int n;
   char c;
   for(n = 0; szText[n] != '\0'; n++)
   {
      c = szText[n];
      if(c >= '0' && c <= '9')
         c = c - '0' + 26;
      else if(c >= 'A' && c <= 'Z')
         c -= 'A';
      else if(c >= 'a' && c <= 'z')
         c -= 'a';
      else
         c = -1;
      if(c > -1)
         this->SafeDrawBlock(x, nYPos, c);
      x += m_BlockWidth;
   }
}
