/*
	Copyright (C) 1999, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GnuSDK.h"
#include "GDsound.h"

GMidi::GMidi(HWND hWnd)
{
	m_hWnd = hWnd;
}

BOOL GMidi::Play(const char *Filename)
{
	if(Filename == NULL) return FALSE;
	char buffer[256];

	sprintf(buffer, "open %s type sequencer alias MUSIC", Filename);

	if (mciSendString("close all", NULL, 0, NULL) != 0)
	{
		return(FALSE);
	}

	if (mciSendString(buffer, NULL, 0, NULL) != 0)
	{
		return(FALSE);
	}

	if (mciSendString("play MUSIC from 0 notify", NULL, 0, m_hWnd) != 0)
	{
		return(FALSE);
	}

	return TRUE;
}

GMidi::Stop()
{
	if (mciSendString("close all", NULL, 0, NULL) != 0)
	{
		return(FALSE);
	}   

	return TRUE;
}

BOOL GMidi::Pause()
{
	// Pause if we're not already paused
	if (mciSendString("stop MUSIC", NULL, 0, NULL) != 0)
	{
		return(FALSE);
	}

	return TRUE;
}

BOOL GMidi::Resume()
{
	// Resume midi
	if (mciSendString("play MUSIC notify", NULL, 0, m_hWnd) != 0)
	{
		return(FALSE);
	}

	return TRUE;
}

BOOL GMidi::Restart()
{
	// Replay midi
	if (mciSendString("play MUSIC from 0 notify", NULL, 0, m_hWnd) != 0)
	{
		return(FALSE);
	}

	return TRUE;
}
