/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include <qlistbox.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <direct.h>
#include "ClassViewDialog.h"
#include "ProgramViewDialog.h"
#include "FileViewDialog.h"
#include "DebugDialog.h"
#include "SaveFormatDialog.h"
#include "../Engine/TagNames.h"
#include "../Engine/GCompiler.h"
#include "../Engine/Library.h"
#include "../CodeObjects/CodeObjects.h"
#include "../CodeObjects/VarRef.h"
#include "../CodeObjects/InstrArray.h"
#include "../../GClasses/GnuSDK.h"
#include "../../GClasses/GXML.h"

// Static variables (*** BAD FORM ***)
int ProgramViewDialog::s_nStartLine = 0;

ProgramViewDialog::ProgramViewDialog(COProject* pCOProject, const char* szFilename, QWidget* parent, const char* name)
 : ProgramViewDialogBase(parent, name, TRUE, 0)
{
	m_pCOProject = pCOProject;
	m_pCompiledProgram = NULL;
	m_nSelectedList = -1;
    m_szFilename = szFilename;
    RefreshLists();
	_getcwd(m_szCurrentPath, 256);
}

ProgramViewDialog::~ProgramViewDialog()
{
	delete(m_pCompiledProgram);
}

QListBox* ProgramViewDialog::GetSelectedListBox()
{
	switch(m_nSelectedList)
	{
		case 0: return ListBox1;
		case 1: return ListBox2;
		case 2: return ListBox3;
	}
	GAssert(false, "No list is selected");
	return NULL;
}

void ProgramViewDialog::SetFocus(int n)
{
	GAssert(n >= 0 && n <= 2, "Out of range");
	QListBox* pListBox;

	if(n == m_nSelectedList)
		return;

	// Clear selection in the old list
	if(m_nSelectedList >= 0 && m_nSelectedList <= 4)
	{
		pListBox = GetSelectedListBox();
		if(pListBox != ListBox1) // ListBox1 retains focus because it shows which file is selected
			pListBox->clearSelection();
	}

	// Move the arrow to the new list
	m_nSelectedList = n;
	pListBox = GetSelectedListBox();
	PixmapLabel1->move(pListBox->pos().x() + 120, pListBox->pos().y() - 30);
	if(pListBox->count() > 0)
	{
		if(pListBox->currentItem() < 0 || !pListBox->isSelected(pListBox->currentItem()))
			pListBox->setSelected(0, true);
	}
}

COFile* ProgramViewDialog::GetSelectedFile()
{
	// See which item is selected
	int nSel1 = ListBox1->currentItem();

	// Find the File object
	COFile* pFile;
	int nCount = 0;
	COFileSet* pSource = m_pCOProject->GetSource();
	for(pFile = pSource->GetFirstFile(); pFile && nCount < nSel1; pFile = pSource->GetNextFile(pFile))
		nCount++;
	if(nCount < nSel1)
	{
		GAssert(false, "Internal Error--file listbox doesn't match data");
		return NULL;
	}
	return pFile;
}

COClass* ProgramViewDialog::GetSelectedClass()
{
	// Find the selected File object
	COFile* pFile = GetSelectedFile();
	if(!pFile)
		return NULL;

	// Get Selection
	int nSel2 = ListBox2->currentItem();
	
	// Find the Class object
	return pFile->GetClass(nSel2);
}

// Add button
void ProgramViewDialog::slot_button1()
{
	switch(m_nSelectedList)
	{
		case 0: AddFile(); break;
		case 1: AddClass(); break;
		case 2: AddInterface(); break;
		default:
			GAssert(false, "Unrecognized list");
	}
}

void ProgramViewDialog_CallBackOnError(void* pThis, ErrorHolder* pErrorHolder)
{
	// Show the error
	QDialog* pParent = (QDialog*)pThis;
	ErrorDialog::ShowError(pParent, pErrorHolder);
	if(pErrorHolder->GetErrorType() == ErrorHolder::ET_CLASSICSYNTAX)
		ProgramViewDialog::s_nStartLine = ((ClassicSyntaxError*)pErrorHolder)->m_nLineWithError;
}

void ProgramViewDialog::AddFile()
{
	char szFilename[256];
	if(GStuff::GetOpenFilename(NULL, "Choose a File to Add/Create", "*.p", szFilename) != IDOK)
		return;

    if(GStuff::DoesFileExist(szFilename))
    {
        // Test to see if it's an XML file
		const char* szErrorMessage;
		int nErrorOffset;
		int nErrorLine;
		int nErrorColumn;
		GXMLTag* pXMLTag = GXMLTag::FromFile(szFilename, &szErrorMessage, &nErrorOffset, &nErrorLine, &nErrorColumn);
		bool bValidXMLFile;
        if(pXMLTag)
		{
			bValidXMLFile = true;
			delete(pXMLTag);
		}
		else
			bValidXMLFile = false;

        // Add the file to the project
        COFile* pFile = new COFile(bValidXMLFile ? COFile::SYNTAX_XML : COFile::SYNTAX_CLASSIC, szFilename);
        m_pCOProject->GetSource()->AddFile(pFile);
        m_pCOProject->GetSource()->SetModified(true);
        pFile->SetModified(false);
        if(!m_pCOProject->SaveChanges())
		{
			GAssert(false, "Error saving changes--todo: handle this case");
		}

		// Reload the entire project
		if(m_pCOProject->ReloadSources(m_szFilename, this, ProgramViewDialog_CallBackOnError))
			RefreshLists();
		else
			reject(); // Unrecoverable error
	}
    else
    {
        // Create a new empty file (it won't get saved as a file until you save your changes)
        COFile* pFile = new COFile(COFile::SYNTAX_CLASSIC, szFilename);
        m_pCOProject->GetSource()->AddFile(pFile);
        m_pCOProject->GetSource()->SetModified(true);
        pFile->SetModified(true);
    }
    RefreshLists();
}

// Edit button
void ProgramViewDialog::slot_button2()
{
	switch(m_nSelectedList)
	{
	case 0: EditFile(); break;
	case 1: EditClass(); break;
	case 2: EditInterface(); break;
	default:
		GAssert(false, "unrecognized list");
	}
}

void ProgramViewDialog::AddClass()
{
    COFile* pFile = GetSelectedFile();
    if(!pFile)
    {
        QMessageBox::information(this, "No file selected", "You must select a file to add a class to first");
        return;
	}
	int n = 0;
	char szClassName[64];
	const char* pre = "New Class ";
	int preLen = strlen(pre);
	strcpy(szClassName, pre);
	while(true)
	{
		n++;
		if(n > 1)
		{
			szClassName[preLen - 1] = ' ';
			itoa(n, szClassName + preLen, 10);
		}
		else
			szClassName[preLen - 1] = '\0';
		if(m_pCOProject->FindClass(szClassName))
			continue;
		break;
	}
	COClass* pNewClass = new COClass(szClassName, m_pCOProject->m_pObject, pFile, m_pCOProject);

	// Add a "New" procedure and "Init" method to the class
	COMethod* pNewProc = new COMethod(METHOD_NAME_NEW, pNewClass, true, m_pCOProject);
	COVariable* pNewVar1 = new COVariable("Dest", pNewClass, false, false);
	pNewProc->AddParameter(pNewVar1);
	GPointerArray* pParams1 = new GPointerArray(1);
	pParams1->AddPointer(new COVarRef(pNewVar1));
	pNewProc->GetInstructions()->AddInstr(new COMethodCall(m_pCOProject->m_pNew, pNewProc->GetInstructions(), pParams1));
	COMethod* pInitMethod = new COMethod("Init", pNewClass, false, m_pCOProject); // todo: unmagic the name
	pInitMethod->AddParameter(new COVariable(VAL_THIS, pNewClass, false, false));
	pNewClass->AddMethod(pInitMethod);
	GPointerArray* pParams2 = new GPointerArray(1);
	pParams2->AddPointer(new COVarRef(pNewVar1));
	pNewProc->GetInstructions()->AddInstr(new COMethodCall(pInitMethod, pNewProc->GetInstructions(), pParams2));
	pNewClass->AddProcedure(pNewProc);

	pFile->AddClass(pNewClass);
	pFile->SetModified(true);
	ClassViewDialog dialog(m_pCOProject, pNewClass, this, "Class View", true);
	dialog.exec();
	RefreshLists();
}

void ProgramViewDialog::EditFile()
{
    // Get selected file
    COFile* pFile = GetSelectedFile();
    if(!pFile)
    {
        QMessageBox::information(this, "No file selected", "You must select a file to edit first");
        return;
    }

	// Save changes
    if(!m_pCOProject->SaveChanges())
    {
        QMessageBox::information(this, "Error", "Failed to save changes!");
        return;
    }

    // See if the user wants Classic Syntax or XML
	int nAnswer = QMessageBox::information( this, "What Syntax", "Do you want to edit Classic Syntax, or XML Syntax?",
                                      "C&lassic", "&XML", "&Cancel", 0, 2);
    if(nAnswer == 2)
        return;
    bool bWantClassic = (nAnswer == 0 ? true : false);
	bool bIsClassic = (pFile->GetFileType() == COFile::FT_CLASSIC);
    if(bWantClassic != bIsClassic)
    {
        // Convert the syntax to what the user wants
        if(bWantClassic)
            pFile->SetSyntax(COFile::SYNTAX_CLASSIC);
        else
            pFile->SetSyntax(COFile::SYNTAX_XML);
        pFile->Save();
        m_pCOProject->GetSource()->SetModified(true);
        m_pCOProject->GetSource()->Save();
    }

	s_nStartLine = 0;
	while(true)
	{
	    // Open the file edit dialog
		FileViewDialog dialog(this, "File View", pFile->GetFilename(), s_nStartLine);
		if(dialog.exec() != Accepted)
			break;

		// Reload the entire project
		if(m_pCOProject->ReloadSources(m_szFilename, this, ProgramViewDialog_CallBackOnError))
		{
			RefreshLists();
			break;
		}
	}
	if(!m_pCOProject->IsSourceLoaded())
		reject(); // Had an error and didn't fix it
}

void ProgramViewDialog::EditInterface()
{
	GAssert(false, "edit interface not implemented yet");
}

void ProgramViewDialog::Delete_File()
{
    // todo: check the whole Source for dependencies on any class or method in
    //       the file and don't allow them to delete it if there is one, but
    //       do allow them to jump the the dependency spot.
    
    //Remove or Delete File Permanently
	int nAnswer = QMessageBox::information( this, "Remove file from project", "Would you like to delete the file permanently, or just remove it from the project?",
                                      "&Delete", "&Remove", "&Cancel",
                                      2,  // Enter == Cancel
                                      2);
	if(nAnswer == 2) // Cancel
		return;

	// See which item is selected
	int nSel1 = ListBox1->currentItem();
    COFile* pFile = GetSelectedFile();
    if(nSel1 < 0 || !pFile)
    {
        QMessageBox::information(this, "Nothing Selected", "You must select a file to delete first");
        return;
    }
    GAssert(false, "Not implemented yet");
    char sGettingFileName[MAX_PATH] = {'\0'};
	int n = ListBox1->currentItem();
	if(n < 0)
	{
		GAssert(false, "Nothing selected");
		return;
	}
    GAssert(false, "Sorry, not implemented yet--todo: implement this");
	// todo: set the Source file as modified
}

// Delete button
void ProgramViewDialog::slot_button3()
{
	switch(m_nSelectedList)
	{
	case 0: Delete_File(); break;
	case 1: DeleteClass(); break;
	case 2: DeleteInterface(); break;
	default:
		GAssert(false, "unrecognized list");
	}
}

// Edit class button
void ProgramViewDialog::EditClass()
{
	COClass* pClass = GetSelectedClass();
	if(!pClass)
	{
        QMessageBox::information(this, "Nothing Selected", "First select a class to edit");
		return;
	}
	ClassViewDialog dialog(m_pCOProject, pClass, this, "Class View", false);
	dialog.exec();
    RefreshLists();
}

void ProgramViewDialog::DeleteClass()
{
	COFile* pFile = GetSelectedFile();
	
	int n = ListBox2->currentItem();
	//if this is the head class, RemoveClass takes a null parameter
	if (n == 0)
		pFile->RemoveClass(NULL);

	//set the current pointer in class to the previous one.
	pFile->RemoveClass(GetSelectedClass());
	slot_list1SelectionChange();	
}

void ProgramViewDialog::AddInterface()
{
    GAssert(false, "Add interface not implemented yet");
}

void ProgramViewDialog::DeleteInterface()
{
    GAssert(false, "Delete interface not implemented yet");
}

void ProgramViewDialog::Goodbye()
{
    if(m_pCOProject->HasChanges())
    {
	    int nAnswer = QMessageBox::information( this, "Save changes?", "You have made changes.  Would you like to save them before you quit?",
                                      "&Yes", "&No", "&Cancel",
                                      0,      // Enter == button 0
                                      2);
	    if(nAnswer == 2) // Cancel
		    return;
        if(nAnswer == 0)
            slot_button8();
    }
    accept();
}

// Save All Changes button
void ProgramViewDialog::slot_button8()
{
	_chdir(m_szCurrentPath);
    if(m_pCOProject->SaveChanges())
        QMessageBox::information(this, "Successful", "Save successful");
	else
        QMessageBox::information(this, "Error saving!!!!!", "Error saving!!!!!!!!");
}

// Build button
void ProgramViewDialog::slot_button9()
{
    if(BuildToMem())
	{
        QMessageBox::information(this, "Successful", "Build successful");
	}
}

// Run button
void ProgramViewDialog::slot_button10()
{
    RunProgram(false);
}

// To File button
void ProgramViewDialog::slot_button7()
{
    if(!m_pCompiledProgram)
    {
        QMessageBox::information(this, "You didn't build", "You must build before you can save the built program to a file");
        return;
    }

	// Get a format
	SaveFormatDialog dialog(this, "Save Format");
	if(dialog.exec() != Accepted)
		return;

	// Make the file
	switch(dialog.GetSelection())
	{
	case 0: // .xlib file
		{
			char szFilename[256];
			if(GStuff::GetSaveFilename(NULL, "Save as...", "*.xlib", szFilename) != IDOK)
				return;

			if(!m_pCompiledProgram->GetLibraryTag()->ToFile(szFilename))
				QMessageBox::information(this, "Failed to make the file", "Error");
		}
		break;
	case 1: // xml string in a .cpp file
		{
			char szFilename[256];
			if(GStuff::GetSaveFilename(NULL, "Save as...", "*.cpp", szFilename) != IDOK)
				return;

			if(!m_pCompiledProgram->SaveAsCppString(szFilename))
				QMessageBox::information(this, "Failed to make the file", "Error");
		}
		break;
	case 2: // cpp code
		{
			GAssert(false, "Not implemented yet");
		}
		break;
	default:
		GAssert(false, "Unexpected case");
		break;
	}
}

// Refresh all lists in Program view
void ProgramViewDialog::RefreshLists()
{
	// Show the list of files in the Source
	ListBox1->clear();
	ListBox2->clear();
	ListBox3->clear();
	COFile* pFile;
	COFileSet* pSource = m_pCOProject->GetSource();
	int nCount = 0;
	char szTmp[256];
	for(pFile = pSource->GetFirstFile(); pFile; pFile = pSource->GetNextFile(pFile))
	{
		pFile->GetFilenameNoPath(szTmp);
		ListBox1->insertItem(QString(szTmp));
		nCount++;
	}
	SetFocus(0);
}

void ProgramViewDialog::slot_list1SelectionChange()
{
	// Clear the lists
	ListBox3->clear();
	ListBox2->clear();

	// Find the new selected File object
	COFile* pFile = GetSelectedFile();
	if(!pFile)
		return;

	// Show the classes
	COClass* pClass;
	for(pClass = pFile->GetFirstClass(); pClass; pClass = pFile->GetNextClass(pClass))
	{
		ListBox2->insertItem(QString(pClass->GetName()));
	}
	
	//Show the Interfaces
	COInterface* pInterface;
	for(pInterface = pFile->GetFirstInterface(); pInterface; pInterface = pFile->GetNextInterface(pInterface))
	{
		ListBox3->insertItem(QString(pInterface->GetName()));
	}
}

bool ProgramViewDialog::BuildToMem()
{
    // Throw out the old compiled program
    delete(m_pCompiledProgram);

	// Compile the program
	CompileError errorHolder;
	m_pCompiledProgram = GCompiler::Compile(m_pCOProject, &errorHolder);
	if(!m_pCompiledProgram)
	{
		ErrorDialog::ShowError(this, &errorHolder);
		// todo: take the user to the error here
		return false;
	}
    return true;
}

void onBreakPointCB(void* pThis)
{
	((ProgramViewDialog*)pThis)->OnBreakPoint();
}

void ProgramViewDialog::OnBreakPoint()
{
	QMessageBox::information(this, "Breakpoint", "Hit a breakpoint"); // todo: give option to abort/continue/debug
}

void ProgramViewDialog::RunProgram(bool bDebug)
{
    // Make sure there is a built program in memory
    if(!m_pCompiledProgram)
    {
        QMessageBox::information(this, "You didn't build", "You must build before you can run");
        return;
    }

    // Find a Procedure named "Main" and call it
	int nMethodID = m_pCompiledProgram->FindMainProc();
    if(nMethodID >= 0)
    {
		// todo: make sure it has no params
		CallBackGetter cbg; // todo: support custom callbacks--at least pop up a message box saying its name
		GVM vm(m_pCompiledProgram, &cbg);
		//x86Compiler VM(m_pCompiledProgram, &cb, true);
		if(bDebug)
		{
			DebugDialog::DebugProc(&vm, nMethodID, m_pCOProject, NULL, 0); // todo: use current source
		}
		else
		{
			vm.SetBreakPointHandler(this, onBreakPointCB);
			vm.CallNoParams(nMethodID);
		}
    }
    else
        QMessageBox::information(this, "Can't find Main", "There wasn't a procedure named 'Main'.  How am I supposed to know which procedure to call?");
}

void ProgramViewDialog::arrowUp()
{
	QListBox* pListBox = GetSelectedListBox();
	int n = pListBox->currentItem() - 1;
	if(n >= 0 && n < (int)pListBox->count())
		pListBox->setSelected(n, true);
}

void ProgramViewDialog::arrowDown()
{
	QListBox* pListBox = GetSelectedListBox();
	int n = pListBox->currentItem() + 1;
	if(n >= 0 && n < (int)pListBox->count())
		pListBox->setSelected(n, true);
}

void ProgramViewDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'F':	SetFocus(0);	break;
		case 'C':	SetFocus(1);	break;
		case 'G':	SetFocus(2);	break;
		case Key_Escape: // fall through
		case 'Q':	Goodbye();		break;
		case 'S':	slot_button8(); break;
		case 'B':	slot_button9();	break;
		case 'T':	slot_button7();	break;
		case 'R':	slot_button10();break;
		case 'E':	slot_button2();	break;
		case 'A':	slot_button1();	break;
		case 'X':	slot_button3();	break;
		case 'D':	slot_button9_2(); break;
		case Key_Up:	arrowUp();	break;
		case Key_Down:	arrowDown();break;
	}
}

void ProgramViewDialog::slot_list1clicked()
{
	SetFocus(0);
}

void ProgramViewDialog::slot_list2clicked()
{
	SetFocus(1);
}

void ProgramViewDialog::slot_list3clicked()
{
	SetFocus(2);
}

// Debug button
void ProgramViewDialog::slot_button9_2()
{
	RunProgram(true);
}
