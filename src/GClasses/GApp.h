/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GAPP_H__
#define __GAPP_H__

typedef void (*DaemonMainFunc)(void* pArg);

class GApp
{
public:
	static void LaunchDaemon(DaemonMainFunc pDaemonMain, void* pArg);
	static char* GetApplicationPath(const char* szArg0);
	static void TurnSignalsIntoExceptions();
};

#endif // __GAPP_H__
