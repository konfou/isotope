/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GPNG_H__
#define __GPNG_H__

#include <stdio.h>

class GImage;

bool LoadPng(GImage* pImage, const unsigned char* pData, int nDataSize);

// pFile should already be opened for writing binary data
bool SavePng(GImage* pImage, FILE* pFile, bool bIncludeAlphaChannel);

#endif // __GPNG_H__
