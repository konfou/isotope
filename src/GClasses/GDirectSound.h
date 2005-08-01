/*
	Copyright (C) 1999, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GDIRECTSOUND_H__
#define __GDIRECTSOUND_H__

#include "GDSound.h"

// Used by the GWave class
class GDirectSound
{
public:
	GDirectSound(void);
	~GDirectSound(void);

	BOOL Init(HWND hWnd);
	BOOL GetCaps(void);

public:
	LPDIRECTSOUND m_lpDS;
	DSCAPS m_DSCaps;
};

#endif // __GDIRECTSOUND_H__
