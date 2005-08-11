/*
	Copyright (C) 2006, Mike Gashler

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include <stdio.h>
#include <wchar.h>
#ifdef WIN32
#include <direct.h>
#else // WIN32
#include <unistd.h>
#endif // !WIN32
#include <string.h>
#include "../Include/GashSdl.h"
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GString.h"
#include "../../GClasses/GXML.h"
#include "../Engine/Error.h"
#include "../CodeObjects/CodeObject.h"
#include "../../SDL/SDL.h"
#include "../Test/EngineTests.h"
#include "../Test/ClassTests.h"
#include "../CodeObjects/Project.h"
#include "../Engine/GCompiler.h"
#include "../GashSDL/Editor.h"

char* GetApplicationPath(const char* szArg0)
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

class CommandErrorHandler : public ErrorHandler
{
public:
	virtual void OnError(ErrorHolder* pErrorHolder)
	{
		GString s;
		pErrorHolder->ToString(&s);
		wprintf(s.GetString());
	}
};


void ShowUsage()
{
	printf(
		"Usage:\n"
		"\n"
		"To show this usage screen:\n"
		"   gash\n"
		"\n"
		"To build and run main in 'myscript.gash':\n"
		"   gash run myscript.gash\n"
		"\n"
		"To run main in 'myapplication.xlib':\n"
		"   gash run myapplication.xlib\n"
		"\n"
		"To build an xlib from the files 'a.gash', 'b.gash', and 'c.gash'\n"
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
		"To run a set of tests:\n"
		"   gash test\n"
		"\n"
	);
}

bool RebuildMainLib(const char* szAppPath)
{
	chdir(szAppPath);

	// Load the project
	CommandErrorHandler errorHandler;
	COProject* pProject = COProject::LoadProject(NULL, "../src/Gash/MainLib/MainLib.proj", &errorHandler);
	if(!pProject)
		return false;

	// Compile the project to a library
	CompileError errorHolder;
	Holder<Library*> hLibrary(GCompiler::Compile(pProject, true, &errorHolder));
	Library* pLibrary = hLibrary.Get();
	if(!pLibrary)
	{
		errorHandler.OnError(&errorHolder);
		return false;
	}

	// Save the library
	const char* szSubPath = "../src/Gash/xlib/";
	char* pLibrariesDir = (char*)alloca(strlen(szAppPath) + strlen(szSubPath) + 1);
	strcpy(pLibrariesDir, szAppPath);
	strcat(pLibrariesDir, szSubPath);
	if(chdir(pLibrariesDir) != 0)
	{
	    fprintf(stderr, "couldn't change to libraries folder");
		return false;
	}
	if(!pLibrary->GetLibraryTag()->ToFile("MainLib.xlib"))
	{
	    fprintf(stderr, "error saving the library file");
		return false;
	}
    printf("Successfully rebuilt main lib!");
	return true;
}

void testLogFunc(void* pThis, const wchar_t* wszMessage)
{
	fwprintf(stderr, wszMessage);
}

int runTests(const char* szAppPath)
{
	// Get the tests
	EngineTests engineTests(NULL, testLogFunc, szAppPath);
	ClassTests classTests(NULL, testLogFunc, szAppPath);
	int nTestSuites = 2;
	Test* tests[2];
	tests[0] = &engineTests;
	tests[1] = &classTests;

	// Run them	
	bool bOK = true;
	char szBuf[256];
	int nCount = 0;
	int i;
	for(i = 0; i < nTestSuites; i++)
	{
		tests[i]->SetOutputLevel(0);
		tests[i]->GetCategoryName(szBuf, 256);
		fprintf(stderr, "----- %s -----\n", szBuf);
		int n;
		for(n = 0; n < tests[i]->GetTestCount(); n++)
		{
			tests[i]->GetTestName(n, szBuf, 256);
			fprintf(stderr, "%d- %s", nCount, szBuf);
			int j = 60 - strlen(szBuf) - (nCount >= 10 ? (nCount >= 100 ? 2 : 1) : 0);
			for( ; j >= 0; j--)
				fprintf(stderr, " ");
			if(tests[i]->RunTest(n))
				fprintf(stderr, "Passed\n");
			else
			{
				fprintf(stderr, "FAILED!!!\n");
				bOK = false;
			}
			nCount++;
		}
	}
	fprintf(stderr, "----- Done -----\n");
	if(bOK)
		return 0x600d;
	else
		return 0xbad;
	
}

int runSingleTest(const char* szAppPath, int nTest)
{
	// Get the tests
	EngineTests engineTests(NULL, testLogFunc, szAppPath);
	ClassTests classTests(NULL, testLogFunc, szAppPath);
	int nTestSuites = 2;
	Test* tests[2];
	tests[0] = &engineTests;
	tests[1] = &classTests;

	// Run them	
	bool bOK = true;
	char szBuf[256];
	int nCount = 0;
	int i;
	for(i = 0; i < nTestSuites; i++)
	{
		tests[i]->SetOutputLevel(0);
		tests[i]->GetCategoryName(szBuf, 256);
		int n;
		for(n = 0; n < tests[i]->GetTestCount(); n++)
		{
			if(nCount == nTest)
			{
				tests[i]->GetTestName(n, szBuf, 256);
				fprintf(stderr, "%d- %s", nCount, szBuf);
				int j = 60 - strlen(szBuf) - (nCount >= 10 ? (nCount >= 100 ? 2 : 1) : 0);
				for( ; j >= 0; j--)
					fprintf(stderr, " ");
				if(tests[i]->RunTest(n))
					fprintf(stderr, "Passed\n");
				else
				{
					fprintf(stderr, "FAILED!!!\n");
					bOK = false;
				}
			}
			nCount++;
		}
	}
	fprintf(stderr, "----- Done -----\n");
	if(bOK)
		return 0x600d;
	else
		return 0xbad;
}

int main(int argc, char* argv[])
{
	//SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
int nRet = 0;
	Holder<char*> hAppPath(GetApplicationPath(argv[0]));
	const char* szAppPath = hAppPath.Get();
	bool bShowUsage = false;
	if(argc < 2)
		bShowUsage = true;
	else
	{
		CommandErrorHandler eh;
		if(stricmp(argv[1], "run") == 0 && argc >= 3)
		{
			GashSdlCallBackGetter cbg;
			CommandRun(&eh, szAppPath, argc, argv, &cbg);
		}
		else if(stricmp(argv[1], "cpp") == 0)
			nRet = CommandCpp(&eh, szAppPath, argc, argv) ? 0x600d : 0xbad;
		else if(stricmp(argv[1], "build") == 0 && argc >= 3)
			nRet = CommandBuild(&eh, szAppPath, argc, argv) ? 0x600d : 0xbad;
		else if(stricmp(argv[1], "disassemble") == 0 && argc == 3)
			nRet = CommandDisassemble(&eh, szAppPath, argc, argv) ? 0x600d : 0xbad;
		else if(stricmp(argv[1], "mainlib") == 0)
			nRet = RebuildMainLib(szAppPath) ? 0x600d : 0xbad;
		else if(stricmp(argv[1], "edit") == 0 && argc == 3)
			EditFile(argv[2], szAppPath, &eh);
		else if(stricmp(argv[1], "test") == 0)
		{
			if(argc > 2)
				nRet = runSingleTest(szAppPath, atoi(argv[2]));
			else
				nRet = runTests(szAppPath);
		}
		else
			bShowUsage = true;
	}

	if(bShowUsage)
		ShowUsage();

#ifdef _DEBUG
	if(CodeObject::s_nAllocs != CodeObject::s_nDeletes)
	{
		GAssert(false, "There are leaked CodeObjects!");	
/*		GHashTableEnumerator e(CodeObject::s_pLeakedObjects);
		while(true)
		{
			CodeObject* pLeakedObject = (CodeObject*)e.GetNextKey();
			unsigned int nAllocNumber;
			if(!CodeObject::s_pLeakedObjects->Get(pLeakedObject, (void**)&nAllocNumber))
			{
				GAssert(false, "Hash table problem");
			}
			GAssert(false, "Leaked CodeObject");
		}*/
	}
#endif // _DEBUG

	SDL_Quit();
	return nRet;
}
