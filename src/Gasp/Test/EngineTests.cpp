/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "EngineTests.h"
#include "string.h"
#include "../CodeObjects/File.h"
#include "../CodeObjects/Project.h"
#include "../Include/GaspLib.h"
#include "../Engine/GCompiler.h"
#include "../Engine/Disassembler.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GFile.h"
#include <wchar.h>
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif // WIN32

struct EngineTest
{
	const char* szFilename;
	bool bPositive;
};

static struct EngineTest testTable[] = 
{
	{ "Rebuild Main Lib", true },
	{ "basictestneg.gasp", false },
	{ "basictestpos.gasp", true },
	{ "castneg.gasp", false },
	{ "castpos.gasp", true },
	{ "arraytest.gasp", true },
	{ "floattestpos.gasp", true },
	{ "interfacetest.gasp", true },
	{ "looptest.gasp", true },
	{ "poundtest.gasp", true },
	{ "serializetest.gasp", true },
	{ "stream.gasp", true },
	{ "stringtest.gasp", true },
	{ "throwtest.gasp", true },
	{ "virtualtest.gasp", true },
};


class PositiveTestErrorHandler : public ErrorHandler
{
protected:
	EngineTests* m_pEngineTests;
	bool m_bGotError;

public:
	PositiveTestErrorHandler(EngineTests* pEngineTests)
		: ErrorHandler()
	{
		m_pEngineTests = pEngineTests;
		m_bGotError = false;
	}

	virtual void OnError(ErrorHolder* pErrorHolder)
	{
		GString s;
		pErrorHolder->ToString(&s);
		m_pEngineTests->Log(80, s.GetString());

		m_bGotError = true;
	}

	bool HadError() { return m_bGotError; }
};



class NegativeTestErrorHandler : public ErrorHandler
{
protected:
	EngineTests* m_pEngineTests;
	bool m_bGotError;

public:
	NegativeTestErrorHandler(EngineTests* pEngineTests)
		: ErrorHandler()
	{
		m_pEngineTests = pEngineTests;
		m_bGotError = false;
	}

	virtual void OnError(ErrorHolder* pErrorHolder)
	{
		const wchar_t* wszMessage = pErrorHolder->m_pError->message;
		if(wcscmp(wszMessage, Error::UNHANDLED_EXCEPTION.message) != 0)
			m_pEngineTests->Log(80, wszMessage);
		m_bGotError = true;
	}

	bool HadError() { return m_bGotError; }
};


// ------------------------------------------------

bool EngineTests::RunGaspTest(const char* szFilename, bool bPositive, bool bDebug)
{
	char szTestScript[256];

	bool bRebuildMainLib = false;
	if(stricmp(szFilename, "Rebuild Main Lib") == 0)
		bRebuildMainLib = true;
	if(bRebuildMainLib)
		strcpy(szTestScript, "../src/Gasp/MainLib/make.gasp");
	else
	{
		strcpy(szTestScript, "../src/Gasp/Test/Engine/");
		strcat(szTestScript, szFilename);
	}
	char* args[3];
	args[0] = (char*)"gasp";
	args[1] = (char*)"run";
	args[2] = szTestScript;
	bool bRet;
	const char* szAppPath = GetAppPath();
	if(chdir(szAppPath) != 0)
		return false;
	if(bPositive)
	{
		GaspLibCallBackGetter cbg;
		PositiveTestErrorHandler ehp(this);
		if(!bDebug)
			CommandRun(&ehp, szAppPath, 3, args, &cbg, false);
		else
		{
			//CommandDebug(&ehp, szAppPath, 3, args, &cbg);
			GAssert(false, "temporarily disabled");
		}
		bRet = !ehp.HadError();
	}
	else
	{
		GaspLibCallBackGetter cbg;
		NegativeTestErrorHandler ehn(this);
		if(!bDebug)
			CommandRun(&ehn, szAppPath, 3, args, &cbg, true);
		else
		{
			//CommandDebug(&ehn, szAppPath, 3, args, &cbg);
			GAssert(false, "temporarily disabled");
		}
		bRet = ehn.HadError();
	}

	if(bRebuildMainLib && bRet)
		bRet = GFile::CpyFile("../src/Gasp/MainLib/MainLib.xlib", "./MainLib.xlib");

	return bRet;
}



void EngineTests::GetCategoryName(char* szBuffer, int nBufferSize)
{
	strcpy(szBuffer, "Engine Tests");
}

int EngineTests::GetTestCount()
{
	return (sizeof(testTable) / sizeof(struct EngineTest));
}

void EngineTests::GetTestName(int nTest, char* szBuffer, int nBufferSize)
{
	strcpy(szBuffer, testTable[nTest].szFilename);
}

bool EngineTests::RunTest(int nTest)
{
	return RunGaspTest(testTable[nTest].szFilename, testTable[nTest].bPositive, false);
}

void EngineTests::DebugTest(int nTest)
{
	RunGaspTest(testTable[nTest].szFilename, testTable[nTest].bPositive, true);
}

void EngineTestLoadProjectErrorCallBack(void* pThis, ErrorHolder* pErrorHolder)
{
	EngineTests* pEngineTests = (EngineTests*)pThis;
	GString s;
	pErrorHolder->ToString(&s);
	pEngineTests->Log(80, s.GetString());
}

void ShowDisassembly(EngineTests* pThis, Library* pLibrary)
{
	// Disassemble the library
	pThis->Log(10, L"Disassembling...");
	int nLen;
	char* szDisassembly = Disassembler::DisassembleLibraryToText(pLibrary, &nLen);
	FILE* pFile = fopen("c:/mike/cpp/disassembly.txt", "w");
	fwrite(szDisassembly, nLen, 1, pFile);
	fclose(pFile);
	delete(szDisassembly);
#ifdef WIN32
	ShellExecute(NULL, NULL, "c:/mike/cpp/disassembly.txt", NULL, NULL, SW_SHOW);
#endif // WIN32
}

