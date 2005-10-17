/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "../../GClasses/GnuSDK.h"
#include <windows.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <direct.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include "wassaildialog.h"
#include "SplashScreen.h"
#include "FileViewDialog.h"
#include "TestDialog.h"
#include "importcppimpl.h"
#include "../Engine/GCompiler.h"
#include "../Engine/Disassembler.h"
#include "../Include/GashEngine.h"
#include "../Include/GashQt.h"
#include "../CodeObjects/Project.h"
#include "../../GClasses/GXML.h"
#include "UsageDialog.h"
#include "../../GClasses/GString.h"
#include "../../GClasses/GWindows.h"
#include "GetStringDialog.h"

void ShowError(QWidget* pThis, ErrorHolder* pErrorHolder)
{
	ConsoleWindow cw(pThis, "Error", true);
	GString gs;
	pErrorHolder->ToString(&gs);
	cw.print(gs.GetString(), gs.GetLength());
	cw.setCursorPos(0, 0);
	cw.exec();
}

class GuiErrorHandler : public ErrorHandler
{
protected:
	QWidget* m_pThis;

public:
	GuiErrorHandler(QWidget* pThis)
		: ErrorHandler()
	{
		m_pThis = pThis;
	}

	virtual void OnError(ErrorHolder* pErrorHolder)
	{
		ShowError(m_pThis, pErrorHolder);
	}
};


void ShowGashGUI(const char* szAppPath, int argc, char** argv)
{
	QApplication app(argc, argv);
	WassailDialog dialog(NULL, "Gash", szAppPath);
	app.setMainWidget(&dialog);
	dialog.exec();
}

WassailDialog::WassailDialog(QWidget* parent, const char* name, const char* szAppPath)
	: WassailDialogBase(parent, name, TRUE, 0)
{
    //TextLabel2->hide();
	//QTimer::singleShot(200, this, SLOT(slot_go()));
	m_pAppPath = szAppPath;
	FindLibraryFolder(szAppPath);

#ifdef _DEBUG
	// Test Suite
    TextLabelTestSuite = new QLabel( this, "TextLabelTestSuite" );
    TextLabelTestSuite->setGeometry( QRect( 140, 70, 190, 26 ) ); 
    TextLabelTestSuite->setProperty( "text", tr( "T- Test Suite" ) );
    TextLabelTestSuite->setProperty( "alignment", int( QLabel::AlignCenter ) );

	// Rebuild Main Lib
    TextLabelRebuildMainLib = new QLabel( this, "TextLabelTestSuite" );
    TextLabelRebuildMainLib->setGeometry( QRect( 140, 90, 190, 26 ) ); 
    TextLabelRebuildMainLib->setProperty( "text", tr( "M- Rebuild Main Lib" ) );
    TextLabelRebuildMainLib->setProperty( "alignment", int( QLabel::AlignCenter ) );
#endif // _DEBUG
}

WassailDialog::~WassailDialog()
{
	delete(m_pLibrariesDir);
}

void WassailDialog::FindLibraryFolder(const char* szAppPath)
{
	const char* szSubPath = "./";
	m_pLibrariesDir = new char[strlen(szAppPath) + strlen(szSubPath) + 1];
	strcpy(m_pLibrariesDir, szAppPath);
	strcat(m_pLibrariesDir, szSubPath);
#ifdef WIN32
	int i;
	for(i = 0; m_pLibrariesDir[i] != '\0'; i++)
	{
		if(m_pLibrariesDir[i] == '/')
			m_pLibrariesDir[i] = '\\';
	}
#endif
}

void WassailDialog::slot_go()
{
    SplashScreen ss;
    ss.DoIt();
    TextLabel2->show();
}

// Run
void WassailDialog::slot_button1()
{
	QString s = QFileDialog::getOpenFileName("", "*.xlib;*.gash;*.cmd", NULL, "GetOpenFilename", "Select a file to run");
	if(s.length() < 1)
		return;
	char* szFilename = (char*)s.latin1();
	{
		char szDrive[32];
		char szDir[512];
		char szDirectory[512];
		_splitpath(szFilename, szDrive, szDir, NULL, NULL);
		_makepath(szDirectory, szDrive, szDir, NULL, NULL);
		chdir(szDirectory);
	}
	GashQtCallBackGetter cbg;
	GuiErrorHandler errorHandler(this);
	char* szArgs[4];
	szArgs[0] = "gash";
	szArgs[1] = "run";
	szArgs[2] = (char*)s.latin1();
	szArgs[3] = NULL;
	CommandRun(&errorHandler, m_pAppPath, 3, szArgs, &cbg);
}

// Documentation
void WassailDialog::slot_button2()
{
	ShellExecute(NULL, NULL, "Doc\\index.html", NULL, NULL, SW_SHOW);
	ShellExecute(NULL, NULL, "..\\Doc\\index.html", NULL, NULL, SW_SHOW);
}

// Disassembler
void WassailDialog::slot_button3()
{
	char szFilename[256];
	if(GWindows::GetOpenFilename(NULL, "Selected an file to disassemble", "*.xlib;*.gash;*.proj", szFilename) != IDOK)
		return;

	GuiErrorHandler errorHandler(this);
	char* szArgs[4];
	szArgs[0] = "gash";
	szArgs[1] = "disassemble";
	szArgs[2] = szFilename;
	szArgs[3] = NULL;
	bool bOK = CommandDisassemble(&errorHandler, m_pAppPath, 3, szArgs);
	GAssert(bOK, "failed to disassemble--todo: handle this case");
/*
	
	
	Library* pLibrary = Library::LoadFromFile(szFilename);
	if(!pLibrary)
	{
		// todo: handle this better
		GAssert(false, "Failed to load .xlib file");
	}
	int nLen;
	char* szDisassembly = Disassembler::DisassembleLibraryToText(pLibrary, &nLen, NULL);
	FILE* pFile = fopen("disassembly.txt", "w");
	fwrite(szDisassembly, nLen, 1, pFile);
	fclose(pFile);
	delete(szDisassembly);*/
	ShellExecute(NULL, NULL, "disassembly.txt", NULL, NULL, SW_SHOW);
}

void WassailDialog_CallBackOnError(void* pThis, ErrorHolder* pErrorHolder)
{
	// Show the error
	QDialog* pParent = (QDialog*)pThis;
	ShowError(pParent, pErrorHolder);
	const char* szFilename = NULL;
	int nStartLine = 0;
	if(pErrorHolder->GetErrorType() == ErrorHolder::ET_CLASSICSYNTAX)
	{
		szFilename = ((ClassicSyntaxError*)pErrorHolder)->GetFilename();
		nStartLine = ((ClassicSyntaxError*)pErrorHolder)->m_nLineWithError;
	}
	else if(pErrorHolder->GetErrorType() == ErrorHolder::ET_PARSE)
		szFilename = ((ParseError*)pErrorHolder)->GetFilename();
	if(szFilename)
	{
		FileViewDialog dialog(pParent, "File View", szFilename, nStartLine);
		dialog.exec();
	}
}

// Debug button
void WassailDialog::slot_button4()
{
	QString s = QFileDialog::getOpenFileName("", "*.xlib;*.gash;*.cmd", NULL, "GetOpenFilename", "Select a file to run");
	if(s.length() < 1)
		return;
	GashQtCallBackGetter cbg;
	GuiErrorHandler errorHandler(this);
	char* szArgs[4];
	szArgs[0] = "gash";
	szArgs[1] = "debug";
	szArgs[2] = (char*)s.latin1();
	szArgs[3] = NULL;
	CommandDebug(&errorHandler, m_pAppPath, 3, szArgs, &cbg);
}

// Import Cpp button
void WassailDialog::slot_button5()
{
	ImportCpp dialog(this, "Import Cpp");
	dialog.exec();
}

// Build button
void WassailDialog::slot_button6()
{
	QString s = QFileDialog::getOpenFileName("", "*.gash;*.sources", NULL, "GetOpenFilename", "Select a file to build");
	if(s.length() < 1)
		return;
	GuiErrorHandler errorHandler(this);
	char* szArgs[4];
	szArgs[0] = "gash";
	szArgs[1] = "build";
	szArgs[2] = (char*)s.latin1();
	szArgs[3] = NULL;
	CommandBuild(&errorHandler, m_pAppPath, 3, szArgs);
}

void WassailDialog::DoTestSuite()
{
    TestDialog dialog(this, "Test View", m_pLibrariesDir, m_pAppPath);
	dialog.exec();
}

void WassailDialog::RebuildMainLib()
{
	_chdir(m_pAppPath);

	// Load the project
	GuiErrorHandler errorHandler(this);
	COProject* pProject = COProject::LoadProject(NULL, "../Source/Gash/MainLib/MainLib.proj", &errorHandler);
	if(!pProject)
		return;

	// Compile the project to a library
	CompileError errorHolder;
	Library* pLibrary = GCompiler::Compile(pProject, &errorHolder);
	if(!pLibrary)
	{
		ShowError(this, &errorHolder);
		delete(pProject);
		return;
	}

	// Save the library
	if(_chdir(m_pLibrariesDir) != 0)
	{
	    QMessageBox::information(NULL, "Error", "Can't change to libraries folder");
		delete(pProject);
		delete(pLibrary);
		return;
	}
	if(!pLibrary->GetLibraryTag()->ToFile("MainLib.xlib"))
	{
	    QMessageBox::information(NULL, "Error", "There was an error saving the xlib");
		delete(pProject);
		delete(pLibrary);
		return;
	}
	delete(pLibrary);
	delete(pProject);
    QMessageBox::information(NULL, "Yay!", "Successful!");
}

void WassailDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'R': slot_button1(); break;
		case 'S': slot_button2(); break;
		case 'A': slot_button3(); break;
		case 'D': slot_button4(); break;
		case 'I': slot_button5(); break;
		case 'B': slot_button6(); break;
//#ifdef _DEBUG
		case 'T': DoTestSuite(); break;
		case 'M': RebuildMainLib(); break;
//#endif // _DEBUG
		case Key_Escape: // fall through
		case 'Q': accept(); break;
	}
}

