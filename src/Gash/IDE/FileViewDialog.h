/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __FILEVIEWDIALOG_H__
#define __FILEVIEWDIALOG_H__

#include "FileViewDialogBase.h"

class FileViewDialog : public FileViewDialogBase
{
protected:
    char* m_szFilename;

public:
    FileViewDialog(QWidget* parent, const char* name, const char* szFilename, int nStartLine);
    virtual ~FileViewDialog();

    virtual void slot_button1();
    virtual void slot_button2();

protected:
    bool LoadFile(const char* szFilename);
    void SetFilename(const char* szFilename);
	static void DeleteString(char* szString, int nChars);
	static void CleanupLine(char* szLine);

	virtual void resizeEvent(QResizeEvent *pEvent);	
};

#endif __FILEVIEWDIALOG_H__
