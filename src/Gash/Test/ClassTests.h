/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __CLASSTESTS_H__
#define __CLASSTESTS_H__

#include "Test.h"

class ClassTests : public Test
{
protected:
	Test* m_pTests;

public:
	ClassTests(void* pParent, LogFunc pLogFunc, const char* szAppPath) : Test(pParent, pLogFunc, szAppPath) {}
	virtual ~ClassTests() {}

	virtual int GetTestCount();
	virtual bool RunTest(int n);
	virtual void DebugTest(int n);
	virtual void GetTestName(int nTest, char* szBuffer, int nBufferSize);
	virtual void GetCategoryName(char* szBuffer, int nBufferSize);
};

#endif // __CLASSTESTS_H__
