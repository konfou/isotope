/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GApp.h"
#include <stdio.h>
#ifdef WIN32
#	include <windows.h>
#	include <direct.h>
#else // WIN32
#	include <unistd.h>
#endif // !WIN32
#include "GMacros.h"

/*static*/ void GApp::LaunchDaemon(DaemonMainFunc pDaemonMain, void* pArg)
{
#ifdef WIN32
	// Windows isn't POSIX compliant and it has its own process system that
	// isn't really friendly to launching daemons.  You're supposed to create
	// a "service", but I don't know how to do that (and I'm too lazy to learn
	// something that can't generalize off a proprietary platform) so let's
	// just launch it like a normal app and be happy with that.
	pDaemonMain(pArg);
#else // WIN32
#	ifdef _DEBUG
	// Don't fork the process in debug mode because that just makes it hard to debug
	pDaemonMain(pArg);
#	else // _DEBUG	
	// Fork the process
	int pid = fork();
	if(pid < 0)
		throw "Error forking the daemon process";
	if(pid == 0)
	{
		// Drop my process group leader and become my own process group leader
		// (so the process isn't terminated when the group leader is killed)
		setsid();

		// Get off any mounted drives so that they can be unmounted without
		// killing the daemon
		chdir("/");

		// Launch the daemon
		pDaemonMain(pArg);
	}
#	endif // _DEBUG
#endif // !WIN32
}

/*static*/ char* GApp::GetApplicationPath(const char* szArg0)
{
	// Make sure the app name includes path info
	char szFullNameBuf[512];
#ifdef WIN32
	GetModuleFileName(NULL/*GetModuleHandle(szArg0)*/, szFullNameBuf, 512);
#else // WIN32
	strcpy(szFullNameBuf, szArg0);
#endif // !WIN32
	int nFullLen = strlen(szFullNameBuf);
	const char* szFilename;
	if(nFullLen > 0)
		szFilename = szFullNameBuf;
	else
	{
		GAssert(false, "failed to get full name of executing assembly");
		szFilename = szArg0;
	}

	// Find the last slash in szFilename
	int n = strlen(szFilename);
	for(n--; n >= 0; n--)
	{
		if(szFilename[n] == '/' || szFilename[n] == '\\')
			break;
	}
	const char* szFilePart = szFilename + n + 1;
	char szAppPath[512];
	if(n >= 0)
	{
		memcpy(szAppPath, szFilename, n);
		szAppPath[n] = '\0';
	}
	else
		getcwd(szAppPath, 512);

	// Copy to an allocated buffer and append a slash if necessary
	int nLen = strlen(szAppPath);
	bool bAddSlash = true;
	if(szAppPath[nLen - 1] == '/' || szAppPath[nLen - 1] == '\\')
		bAddSlash = false;
	char* szApplicationPath = new char[nLen + 1 + (bAddSlash ? 1 : 0)];
	strcpy(szApplicationPath, szAppPath);
	if(bAddSlash)
	{
#ifdef WIN32
		szApplicationPath[nLen] = '\\';
#else
		szApplicationPath[nLen] = '/';
#endif
		szApplicationPath[nLen + 1] = '\0';
	}
	return szApplicationPath;
}

void onSigSegV(int n)
{
	throw "A memory access violation occurred.  The most common cause is an attempt to dereference null";
}

void onSigInt(int n)
{
	throw "The program was interrupted when the user pressed Ctrl-C";
}

void onSigQuit(int n)
{
	throw "The program was interrupted with SIGQUIT";
}

void onSigTstp(int n)
{
	throw "The program was interrupted with SIGTSTP";
}

void onSigAbrt(int n)
{
	throw "An unhandled exception was thrown";
}

/*static*/ void GApp::TurnSignalsIntoExceptions()
{
#ifndef WIN32
	signal(SIGSEGV, onSigSegV);
	signal(SIGINT, onSigInt);
	signal(SIGQUIT, onSigQuit);
	signal(SIGTSTP, onSigTstp);
	signal(SIGABRT, onSigAbrt);
#endif // !WIN32
}
