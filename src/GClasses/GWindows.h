/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GWINDOWS_H__
#define __GWINDOWS_H__

#ifdef WIN32

#include <windows.h>

class GWindow;
class GInput;

// This class is for Windows-only functions
class GWindows
{
public:
	// This allows Windows to unload its message stack.  It is a
	// good idea to call this frequently when you are in a big loop
	// so Windows can multi-task properly and the user can still
	// have control of his computer.
	static void YieldToWindows();

	// This spawns execution of another executeable or batch file.
	// if bWait is true, it will wait until the spawned child
	// finishes execution to return.  If bWait is false, both
	// programs will continue running in separate threads.
	static int SpawnApp(char* szFilename, bool bWait);

	// This calls the Windows standard dialog for getting a filename
	// for opening or saving.
	static int GetOpenFilename(HWND hWnd, char *message, char *mask, char *bufr);
	static int GetSaveFilename(HWND hWnd, char *message, char *mask, char *bufr);

	// This displays a little dialog to get an integer from the user.
	// The GInput parameter must already be initialized and have keyboard
	// enabled.
	static int GetInt(GWindow* pWindow, GInput* pInput, char* szMsg);

	// This destroys a file completely so it can't be recovered
	static bool ShredFile(const char* szFilename);

	// This first checks to see if a directory exists and
	// makes it if it doesn't.  This is better than most
	// directory making commands because it will make as
	// many nested directories as necessary without complaining.
	static void MakeDir(const char* szPath);

	// This sets the value *addr to true and returns whatever value
	// it was before it was set to true.  It both tests and sets
	// atomically so this function can beused to write syncronization
	// primitives.
	static bool TestAndSet(bool* addr);
};
#endif // WIN32


#endif // __GWINDOWS_H__
