/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "TestDialog.h"
#include "../Test/Test.h"
#include "../Test/ClassTests.h"
#include "../Test/EngineTests.h"
#include <qmessagebox.h>
#include <qmultilineedit.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include "../../GClasses/GWindows.h"

void LogOutputFunc(void* pThis, const wchar_t* wszMessage)
{
	((TestDialog*)pThis)->Log(wszMessage);
}

#define TEST_COUNT 2

TestDialog::TestDialog(QWidget* parent, const char* name, const char* szLibrariesPath, const char* szAppPath)
: TestDialogBase(parent, name, TRUE, 0)
{
	m_pTests = new Test*[TEST_COUNT];
	m_pTests[0] = new EngineTests(this, LogOutputFunc, szAppPath);
	m_pTests[1] = new ClassTests(this, LogOutputFunc, szAppPath);
	int n;
	for(n = 0; n < TEST_COUNT; n++)
	{
		m_pTests[n]->SetLibrariesPath(szLibrariesPath);
	}
}

TestDialog::~TestDialog()
{
	int n;
	for(n = 0; n < TEST_COUNT; n++)
		delete(m_pTests[n]);
	delete [] m_pTests;
}

void TestDialog::slot_slider_value_changed()
{

}

void TestDialog::slot_button_clear()
{
	MultiLineEdit1->clear();
}

void TestDialog::slot_button_start()
{
	// Count the total tests
	char szBuff[256];
	QString s;
	int nTotalTestCount = 0;
	int n;
	for(n = 0; n < TEST_COUNT; n++)
		nTotalTestCount += m_pTests[n]->GetTestCount();
	
	// Run the tests
	int nTest = 0;
	ProgressBar1->setTotalSteps(nTotalTestCount);
	ProgressBar1->setProgress(0);
	for(n = 0; n < TEST_COUNT; n++)
	{
		// Output category
		m_pTests[n]->GetCategoryName(szBuff, 256);
		s = "Category: ";
		s += szBuff;
		MultiLineEdit1->insertLine(s);
		MultiLineEdit1->insertLine(QString("--------------------------------------------"));

		// Run the tests in the category
		int nTestCount = m_pTests[n]->GetTestCount();
		int i;
		for(i = 0; i < nTestCount; i++)
		{
			RunSingleTest(nTest, false);
			nTest++;
			ProgressBar1->setProgress(nTest);
#ifdef _WIN32
			GWindows::YieldToWindows();
#endif // _WIN32
			Sleep(0);
		}
		MultiLineEdit1->insertLine(QString(""));
	}
}

int TestDialog::GetSingleTestNumber()
{
	QString s = LineEdit1->text();
	return atoi(s.latin1());
}

void TestDialog::RunSingleTest(int nTestNumber, bool bDebug)
{
	int nTest = nTestNumber;
	int nCategory = 0;
	while(nTest >= m_pTests[nCategory]->GetTestCount())
	{
		nTest -= m_pTests[nCategory]->GetTestCount();
		nCategory++;
		if(nCategory >= TEST_COUNT)
		{
			MultiLineEdit1->insertLine(QString("Invalid test number"));
			return;
		}
	}
	if(bDebug)
	{
		m_pTests[nCategory]->DebugTest(nTest);
	}
	else
	{
		char szTmp[256];
		itoa(nTestNumber, szTmp, 10);
		char szBuff[256];
		bool bPass = m_pTests[nCategory]->RunTest(nTest);
		m_pTests[nCategory]->GetTestName(nTest, szBuff, 256);
		QString s = szTmp;
		s += "- ";
		s += szBuff;
		int nLen = s.length();
		for( ; nLen < 50; nLen++)
			s += " ";
		if(bPass)
			s += "Passed";
		else
			s += "Failed!!!";
		MultiLineEdit1->insertLine(s);
	}

}

void TestDialog::slot_button_run()
{
	int nTest = GetSingleTestNumber();
	RunSingleTest(nTest, false);
}

void TestDialog::slot_button_debug()
{
	int nTest = GetSingleTestNumber();
	RunSingleTest(nTest, true);
}

void TestDialog::Log(const wchar_t* wszMessage)
{
	MultiLineEdit1->insertLine(QString((const QChar*)wszMessage, wcslen(wszMessage)));
}

void TestDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'S': slot_button_start(); break;
		case 'C': slot_button_clear(); break;
		case 'R': slot_button_run(); break;
		case 'D': slot_button_debug(); break;
		case Key_Escape: // fall through
		case 'Q':	reject(); break;
	}
}


