/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "wassaildialogbase.h"

class WassailDialog : public WassailDialogBase
{
	Q_OBJECT

protected:
	const char* m_pAppPath;
	char* m_pLibrariesDir;

#ifdef _DEBUG
	QLabel* TextLabelTestSuite;
	QLabel* TextLabelRebuildMainLib;
#endif // _DEBUG

public:
	WassailDialog(QWidget* parent, const char* name, const char* szAppPath);
	virtual ~WassailDialog();

	virtual void slot_button1();
	virtual void slot_button2();
	virtual void slot_button3();
	virtual void slot_button4();
	virtual void slot_button5();
	virtual void slot_button6();
	virtual void slot_go();

protected:
	virtual void keyPressEvent(QKeyEvent* e);
	void DoTestSuite();
	void RebuildMainLib();
	void FindLibraryFolder(const char* szAppPath);
};

