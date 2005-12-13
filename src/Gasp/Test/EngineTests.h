/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __ENGINETESTS_H__
#define __ENGINETESTS_H__

#include "Test.h"
#include "../CodeObjects/Method.h"

class EMethodSignature;

class EngineTests : public Test
{
protected:
	Test* m_pTests;

public:
	EngineTests(void* pParent, LogFunc pLogFunc, const char* szAppPath) : Test(pParent, pLogFunc, szAppPath) {}
	virtual ~EngineTests() {}

	virtual int GetTestCount();
	virtual bool RunTest(int n);
	virtual void DebugTest(int n);
	virtual void GetTestName(int nTest, char* szBuffer, int nBufferSize);
	virtual void GetCategoryName(char* szBuffer, int nBufferSize);

private:
	bool RunGaspTest(const char* szFilename, bool bPositive, bool bDebug);
};

#endif // __ENGINETESTS_H__
