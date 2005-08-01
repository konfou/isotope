/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __TESTDIALOG_H__
#define __TESTDIALOG_H__

#include "TestDialogBase.h"

class Test;

class TestDialog : public TestDialogBase
{ 
protected:
	Test** m_pTests;

public:
	TestDialog(QWidget* parent, const char* name, const char* szLibrariesPath, const char* szAppPath);
    virtual ~TestDialog();

    virtual void slot_slider_value_changed();
    virtual void slot_button_clear();
    virtual void slot_button_start();
	virtual void slot_button_run();
	virtual void slot_button_debug();
	void Log(const wchar_t* szMessage);
	void keyPressEvent(QKeyEvent* e);

protected:
	int GetSingleTestNumber();
	void RunSingleTest(int nTest, bool bDebug);

};

#endif // __TESTDIALOG_H__
