/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include <stdio.h>
#include <direct.h>
#include <string.h>
#include "../Include/GashQt.h"
#include "../../GClasses/GnuSDK.h"
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GString.h"
#include "../Engine/Error.h"
#include <qmessagebox.h>
#include "../CodeObjects/CodeObject.h"
#include "../IDE/UsageDialog.h"
#include <qapplication.h>

char* GetApplicationPath(const char* szArg0)
{
	// Make sure the app name includes path info
	char szFullNameBuf[512];
	GetModuleFileName(NULL/*GetModuleHandle(szArg0)*/, szFullNameBuf, 512);
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

class CommandErrorHandler : public ErrorHandler
{
public:
	virtual void OnError(ErrorHolder* pErrorHolder)
	{
		ConsoleWindow cw(NULL, "Error", true);
		GetApp()->setMainWidget(&cw);
		GString s;
		pErrorHolder->ToString(&s);
		cw.print(s.GetString(), s.GetLength());
		cw.setCursorPos(0, 0);
		cw.exec();
	}
};


void ShowUsage(int argc, char* argv[])
{
	QApplication app(argc, argv);
	ConsoleWindow cw(NULL, "Usage", true);
	app.setMainWidget(&cw);
	cw.print(
		"Usage:\n"
		"\n"
		"To show this usage screen:\n"
		"   gashgui\n"
		"\n"
		"To show a graphical user interface:\n"
		"   gashgui gui\n"
		"\n"
		"To debug 'myapplication.xlib':\n"
		"   gash debug myapplication.xlib\n"
		"\n"
		"To run main in 'myapplication.xlib':\n"
		"   gash run myapplication.xlib\n"
		"\n"
		"To build and run main in 'myscript.gash':\n"
		"   gash run myscript.gash\n"
		"\n"
		"To build an xlib from the files 'a.gash', 'b.gash', and 'c.gash'\n"
		"(and generate a .proj file that lists those files):\n"
		"   gash build a.gash b.gash c.gash\n"
		"\n"
		"To build all the files listed in 'myproject.proj':\n"
		"   gash build myproject.proj\n"
		"\n"
		"To disassemble 'myapplication.xlib':\n"
		"   gash disassemble myapplication.xlib\n"
		"\n"
		"To generate a .cpp file from 'myproject.proj':\n"
		"   gash cpp myproject.proj\n"
		"\n"
	);
	cw.setCursorPos(0, 0);
	cw.exec();
}


int main(int argc, char* argv[])
{
	int nRet = 0;
	Holder<char*> hAppPath(GetApplicationPath(argv[0]));
	bool bShowUsage = false;
	if(argc < 2)
		bShowUsage = true;
	else
	{
		CommandErrorHandler eh;
		if(stricmp(argv[1], "run") == 0 && argc >= 3)
		{
			GashQtCallBackGetter cbg;
			CommandRun(&eh, hAppPath.Get(), argc, argv, &cbg);
		}
		else if(stricmp(argv[1], "cpp") == 0)
			nRet = CommandCpp(&eh, hAppPath.Get(), argc, argv) ? 0x600d : 0xbad;
		else if(stricmp(argv[1], "build") == 0 && argc >= 3)
			nRet = CommandBuild(&eh, hAppPath.Get(), argc, argv) ? 0x600d : 0xbad;
		else if(stricmp(argv[1], "gui") == 0)
			ShowGashGUI(hAppPath.Get(), argc, argv);
		else if(stricmp(argv[1], "disassemble") == 0 && argc == 3)
			nRet = CommandDisassemble(&eh, hAppPath.Get(), argc, argv) ? 0x600d : 0xbad;
		else if(stricmp(argv[1], "debug") == 0 && argc >= 3)
		{
			GashQtCallBackGetter cbg;
			CommandDebug(&eh, hAppPath.Get(), argc, argv, &cbg);
		}
		else
			bShowUsage = true;
	}

	if(bShowUsage)
		ShowUsage(argc, argv);

#ifdef _DEBUG
	if(CodeObject::s_nAllocs != CodeObject::s_nDeletes)
	{
		GAssert(false, "There are leaked CodeObjects!");	
/*		GHashTableEnumerator e(CodeObject::s_pLeakedObjects);
		while(true)
		{
			CodeObject* pLeakedObject = (CodeObject*)e.GetNextKey();
			if(!pLeakedObject)
				break;
			unsigned int nAllocNumber;
			if(!CodeObject::s_pLeakedObjects->Get(pLeakedObject, (void**)&nAllocNumber))
			{
				GAssert(false, "Hash table problem");
			}
			GAssert(false, "Leaked CodeObject");
		}*/
	}
#endif // _DEBUG


	return nRet;
}
