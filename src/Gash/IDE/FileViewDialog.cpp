/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "FileViewDialog.h"
#include <qmessagebox.h>
#include <stdio.h>
#include <io.h>
#include "../../GClasses/GMacros.h"
#include <qmultilineedit.h>
#include <qradiobutton.h>
#include <qevent.h>
#include <qpushbutton.h>

FileViewDialog::FileViewDialog(QWidget* parent, const char* name, const char* szFilename, int nStartLine)
: FileViewDialogBase(parent, name, TRUE, 0)
{
    m_szFilename = NULL;
    MultiLineEdit1->setDefaultTabStop(4);
    if(!LoadFile(szFilename))
    {
		QMessageBox::information(this, "Error loading file", "Failed to load the file--aborting");
        reject();
    }
	MultiLineEdit1->setCursorPosition(nStartLine, 0);
	MultiLineEdit1->setFocus();
}

FileViewDialog::~FileViewDialog()
{
    delete(m_szFilename);
}

// OK button
void FileViewDialog::slot_button1()
{
    int nLines = MultiLineEdit1->numLines();
    QString s = MultiLineEdit1->textLine(0);
   
    if(!m_szFilename)
    {
		QMessageBox::information(this, "Error saving file", "No known filename.");
        return;
    }
    FILE* pFile = fopen(m_szFilename, "w");
    if(!pFile)
    {
		QMessageBox::information(this, "Error saving file", "Couldn't open file for writing.");
        return;
    }
    int n;
    const char* szLine;
    for(n = 0; n < nLines; n++)
    {
        s = MultiLineEdit1->textLine(n);
        szLine = s.latin1();
        fputs(szLine, pFile);
        fputs("\n", pFile);
    }
    fclose(pFile);
    accept();
}

// Cancel button
void FileViewDialog::slot_button2()
{
    if(MultiLineEdit1->edited())
    {
	    int nAnswer = QMessageBox::information( this, "Save first?", "Do you want to save your changes?",
                                          "&Yes", "&No", "&Cancel", 0, 2);
        if(nAnswer == 1)
        {
            slot_button1();
            return;
        }
        if(nAnswer == 2)
            return;
    }
    reject();
}

void FileViewDialog::SetFilename(const char* szFilename)
{
    delete(m_szFilename);
    m_szFilename = new char[strlen(szFilename) + 1];
    strcpy(m_szFilename, szFilename);
}

void FileViewDialog::DeleteString(char* szString, int nChars)
{
	int n;
	for(n = 0; szString[n] != '\0'; n++)
		szString[n] = szString[n + nChars];
	szString[n] = '\0';
}

void FileViewDialog::CleanupLine(char* szLine)
{
	// Convert spaces to tabs
	int n;
	for(n = 0; szLine[n] != '\0'; n++)
	{
		if(szLine[n] == ' ' && szLine[n + 1] == ' ' &&
			szLine[n + 2] == ' ' && szLine[n + 3] == ' ')
		{
			szLine[n] = '\t';
			DeleteString(&szLine[n + 1], 3);
		}
	}

	// Remove trailing whitespace
	while(n > 0 && szLine[n - 1] <= ' ')
		szLine[--n] = '\0';
}

bool FileViewDialog::LoadFile(const char* szFilename)
{
	char pBuf[4096];
	FILE* pFile = fopen(szFilename, "rb");
	if(!pFile)
	{
		GAssert(false, "Failed to open file");
		return false;
	}
	while(true)
	{
		if(!fgets(pBuf, 4095, pFile))
			break;
		pBuf[4095] = '\0';
		CleanupLine(pBuf);
		MultiLineEdit1->insertLine(QString(pBuf));
	}
	fclose(pFile);
	MultiLineEdit1->setEdited(FALSE);
	SetFilename(szFilename);
	return true;
}

void FileViewDialog::resizeEvent(QResizeEvent *pEvent)
{
	int nWidth = pEvent->size().width();
	int nHeight = pEvent->size().height();
	MultiLineEdit1->resize(nWidth - 20, nHeight - 60);
	PushButton1->move(PushButton1->x(), nHeight - 40);
	PushButton2->move(PushButton2->x(), nHeight - 40);
	FileViewDialogBase::resizeEvent(pEvent);
}
