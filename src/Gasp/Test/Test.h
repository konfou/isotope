/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef _TEST_H__
#define _TEST_H__

#include "../../GClasses/GMacros.h"

typedef void (*LogFunc)(void* pThis, const wchar_t*);

class Test
{
private:
	int m_nTestLevel;
	int m_nOutputLevel;
	LogFunc m_pLogFunc;
	const char* m_szLibrariesPath;
	const char* m_szAppPath;
	void* m_pParent;

public:
	Test(void* pParent, LogFunc pLogFunc, const char* szAppPath)
	{
		m_nTestLevel = 100;
		m_nOutputLevel = 50;
		m_pLogFunc = pLogFunc;
		m_szLibrariesPath = NULL;
		m_szAppPath = szAppPath;
		m_pParent = pParent;
	}

	virtual ~Test()
	{

	}

	virtual void GetCategoryName(char* szBuffer, int nBufferSize) = 0;
	virtual int GetTestCount() = 0;
	virtual void GetTestName(int nTest, char* szBuffer, int nBufferSize) = 0;
	virtual bool RunTest(int nTest) = 0;
	virtual void DebugTest(int nTest) = 0;

	void SetTestLevel(int nTestLevel)
	{
		GAssert(m_nTestLevel >= 0 && m_nTestLevel <= 100, "Out of range");
		m_nTestLevel = nTestLevel;
	}

	int GetTestLevel()
	{
		return m_nTestLevel;
	}

	void SetOutputLevel(int nOutputLevel)
	{
		GAssert(m_nOutputLevel >= 0 && m_nOutputLevel <= 100, "Out of range");
		m_nOutputLevel = nOutputLevel;
	}

	int GetOutputLevel()
	{
		return m_nOutputLevel;
	}

	void Log(int nLevel, const wchar_t* wszMessage)
	{
		GAssert(nLevel >= 0 && nLevel <= 100, "Out of range");
		if(nLevel >= m_nOutputLevel)
			m_pLogFunc(m_pParent, wszMessage);
	}

	const char* GetAppPath() { return m_szAppPath; }
	const char* GetLibrariesPath() { return m_szLibrariesPath; }
	void SetLibrariesPath(const char* szPath) { m_szLibrariesPath = szPath; }
};

#endif // _TEST_H__
