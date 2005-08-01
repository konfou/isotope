/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __PROGRAMVIEW_H__
#define __PROGRAMVIEW_H__

#include "ProgramViewDialogBase.h"

class COFile;
class COClass;
class COProject;
class Library;

class ProgramViewDialog : public ProgramViewDialogBase
{
protected:
	COProject* m_pCOProject;
	Library* m_pCompiledProgram;
	int m_nSelectedList;
	char m_szCurrentPath[256];
    const char* m_szFilename;

public:
	static int s_nStartLine;

	ProgramViewDialog(COProject* pCOProject, const char* szFilename, QWidget* parent,  const char* name);
	virtual ~ProgramViewDialog();

	void OnBreakPoint();

    virtual void slot_button1();
    virtual void slot_button2();
    virtual void slot_button3();
    virtual void slot_button7();
    virtual void slot_button8();
    virtual void slot_button9();
	virtual void slot_button9_2();
    virtual void slot_button10();
	virtual void slot_list1SelectionChange();
    virtual void slot_list1clicked();
    virtual void slot_list2clicked();
    virtual void slot_list3clicked();

protected:
	COFile* GetSelectedFile();
	COClass* GetSelectedClass();
	void RefreshLists();
	bool BuildToMem();
	void RunProgram(bool bDebug);
	void SetFocus(int n);
	virtual void keyPressEvent(QKeyEvent* e);
	void AddFile();
	void AddClass();
	void AddInterface();
	void EditFile();
	void EditClass();
	void EditInterface();
	void Delete_File();
	void DeleteClass();
	void DeleteInterface();
	QListBox* GetSelectedListBox();
    void Goodbye();
	void arrowUp();
	void arrowDown();

};

#endif __PROGRAMVIEW_H__
