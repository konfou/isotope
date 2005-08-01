/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include <qpixmap.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qapplication.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>
#include <malloc.h>
#include <direct.h>
#include "DebugDialog.h"
#include "HardCodedImages.h"
#include "DebuggerListViewItem.h"
#include "DebuggerVariableItem.h"
#include "DebugSourceManager.h"
#include "DebugBreakPointManager.h"
#include "DebugStackManager.h"
#include "../../GClasses/GnuSDK.h"
#include "../../GClasses/GWindows.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GString.h"
#include "../../GClasses/GSpinLock.h"
#include "../CodeObjects/Class.h"
#include "../CodeObjects/Method.h"
#include "../CodeObjects/Call.h"
#include "../CodeObjects/Instruction.h"
#include "../CodeObjects/Project.h"
#include "../CodeObjects/Variable.h"
#include "../CodeObjects/InstrArray.h"
#include "../Engine/InstrTable.h"
#include "../Engine/EInstrArray.h"
#include "../Engine/GVMStack.h"
#include "../Engine/EMethod.h"
#include "../Engine/InstrSet.h"
#include "../Engine/EClass.h"
#include "../Include/GashQt.h"


Library* BuildFile(ErrorHandler* pErrorHandler, const char* szFilename, const char* szLibrariesPath);
void GetLibrariesPath(char* szLibrariesPath, const char* szAppPath);

void CommandDebug(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv, CallBackGetter* pCBG)
{
	// Make a Libraries path
	char szLibrariesPath[512];
	GetLibrariesPath(szLibrariesPath, szAppPath);

	// Convert parameters to primitives
	GAssert(argc >= 3, "not enough args");
	const char* szFilename = argv[2];
	PrimitiveString* pParams = NULL;
	Primitive** ppParams = NULL;
	int nParamCount = argc - 3;
	if(nParamCount > 0)
	{
		ppParams = (Primitive**)_alloca(nParamCount * sizeof(Primitive**));
		pParams = new PrimitiveString[nParamCount];
		int n;
		for(n = 0; n < nParamCount; n++)
		{
			GString s;
			s.Copy(argv[n + 3]);
			pParams[n].SetValue(s.GetString());
			ppParams[n] = &pParams[n];
		}
	}

	// Build it
	Library* pLibrary = BuildFile(pErrorHandler, szFilename, szLibrariesPath);
	if(!pLibrary)
	{
		delete [] pParams;
		return;
	}

	// Find "main" proc
	MethodRef mr;
	if(!pLibrary->FindMainProc(&mr))
	{
		GlobalError errorHolder;
		errorHolder.SetError(&Error::NO_MAIN_PROC);
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		delete(pLibrary);
		return;
	}

	{ // the VM needs to be deleted before the library
		// Determine the path of the file for the chroot jail
		char szDrive[10];
		char szDir[512];
		_splitpath(szFilename, szDrive, szDir, NULL, NULL);
		char szPath[512];
		_makepath(szPath, szDrive, szDir, NULL, NULL);

		// Set up engine
		int n;
		GVM vm(pLibrary, pCBG, pErrorHandler, szPath);

		// Set up params
		VarHolder** params = NULL;
		if(nParamCount > 0)
		{
			params = (VarHolder**)_alloca(nParamCount * sizeof(VarHolder*));
			for(n = 0; n < nParamCount; n++)
				params[n] = new VarHolder(&vm, ppParams[n]);
		}

		// Debug it
		DebugDialog::DebugProc(&vm, &mr, NULL, params, nParamCount, szLibrariesPath); // todo: use current source

		// Clean up
		for(n = 0; n < nParamCount; n++)
			delete(params[n]);
		delete [] pParams;
	}

	delete(pLibrary);
}


// ----------------------------
//
//  Construction / attaching to the VM
//  (these methods are called before the VM thread is started)
//
// ----------------------------

DebugDialog::DebugDialog(const char* name, GVM* pVM)
: DebugDialogBase(NULL, name, true, 0)
{
	// Take Parameters
	m_pVM = pVM;

	// Current Location
	m_pCurrentItem = NULL;

	// Synchronization
	m_pInvokeSpinLock = new GSpinLock();
	m_bInDestructor = false;
	m_bUpdateInvoked = false;

	// Other data initial values
	m_bDebuggeeThreadRunning = false;
	m_invokeAction = IA_NONE;
	m_pGuiFunc = NULL;
	m_pGuiFuncParam = NULL;
	m_pSourceManager = new DebugSourceManager(pVM->GetLibrary());

	// Add current dir to search path
	char szBuff[256];
	getcwd(szBuff, 256);
	m_pSourceManager->AddSourceSearchPath(szBuff);
/*
	// Add main lib project to search path
	strcpy(szBuff, szLibrariesPath);
	int n = strlen(szBuff);
	if(szBuff[n - 1] == '/' || szBuff[n - 1] == '\\')
	{
		szBuff[n - 1] = '\0';
		n--;
	}
	while(n > 0 && szBuff[n - 1] != '/' && szBuff[n - 1] != '\\')
	{
		szBuff[n - 1] = '\0';
		n--;
	}
	strcat(szBuff, "MainLib");
	m_pSourceManager->AddSourceSearchPath(szBuff);
*/
	// Managers
	m_pBreakPointManager = new DebugBreakPointManager(this, pVM);
	m_pStackManager = new DebugStackManager(pVM);

	// Pixmaps
	m_pBreakPointPixmap = new QPixmap((const char**)BreakPointImageData);
	m_pInstrPointerPixmap = new QPixmap((const char**)InstrPointerImageData);
	m_pInstrOnBreakPointPixmap = new QPixmap((const char**)InstrOnBreakPointImageData);
	m_pBlankSpotPixmap = new QPixmap((const char**)BlankSpotImageData);

	MultiLineEdit1->setDefaultTabStop(4);

	// Prepare Variable ListVew
	while(ListView2_2->columns() > 0)
		ListView2_2->removeColumn(0);
	ListView2_2->addColumn("#", 35);
	ListView2_2->addColumn("Name", 100);
	ListView2_2->addColumn("Value", 150);
	ListView2_2->addColumn("Class", 150);
	ListView2_2->addColumn("Address", 100);
	ListView2_2->setSorting(-1);

	m_bDisassemblySourceRefreshed = false;

	// Attach to VM
	pVM->SetGUIHandler(DebuggerGUIHandler, this);
}

DebugDialog::~DebugDialog()
{
	// Wait for the VM thread to stop
	m_bInDestructor = true;
	m_pVM->Terminate();
	while(m_bDebuggeeThreadRunning)
	{
		m_pBreakPointManager->Continue();
		Sleep(0);
		GWindows::YieldToWindows();
	}
	m_pVM->SetBreakPointHandler(NULL, NULL); // todo: restore previous bp handler
	delete(m_pBreakPointManager);
	delete(m_pStackManager);

	// Pixmaps
	delete(m_pBreakPointPixmap);
	delete(m_pInstrPointerPixmap);
	delete(m_pInstrOnBreakPointPixmap);
	delete(m_pBlankSpotPixmap);

	// Synchronization
	delete(m_pInvokeSpinLock);

	delete(m_pSourceManager);
}

struct DebuggeeThreadStruct
{
	GVM* pVM;
	DebugDialog* pDialog;
};

unsigned int DebuggeeThread(void* pParameter);

/*static*/ bool DebugDialog::DebugMe(GVM* pVM, COProject* pCOProject, BreakPoint* pBP)
{
	DebugDialog dialog("Debugger", pVM);
	if(pBP)
	{
		if(!dialog.m_pBreakPointManager->AddBreakPoint(pBP))
		{
			GAssert(false, "Failed to set breakpoint");
		}
	}

	// Switch the VM to another thread
	unsigned int nID;
	DebuggeeThreadStruct dts;
	dts.pDialog = &dialog;
	dts.pVM = pVM;
	HANDLE hThread = (void*)CreateThread( // _beginthreadex(
							NULL,
							0,
							(unsigned long (__stdcall *)(void *))DebuggeeThread,
							&dts,
							0,
							(unsigned long*)&nID
							);
	if(hThread == BAD_HANDLE)
		return false;
	dialog.m_hGUIThread = GetCurrentThread();
	dialog.m_hVMThread = hThread;
	dialog.exec();
	return true;
}

struct DebugProcBreakPointData
{
	GVM* pVM;
	COProject* pCOProject;
	BreakPoint* pBP;
	const char* szLibrariesPath;
};

void DebugProcBreakPointHandler(void* pThis)
{
	struct DebugProcBreakPointData* pData = (struct DebugProcBreakPointData*)pThis;
	if(!DebugDialog::DebugMe(pData->pVM, pData->pCOProject, pData->pBP))
	{
		GAssert(false, "Failed to launch debugger thread");
	}
}

/*static*/ void DebugDialog::DebugProc(GVM* pVM, MethodRef* pMR, COProject* pCOProject, VarHolder** pParams, int nParams, const char* szLibrariesPath)
{
	unsigned char cBreakPoint = GetFuncEnum(Instr_BreakPoint);
	struct DebugProcBreakPointData dpbpd;
	pVM->SetBreakPointHandler(&dpbpd, DebugProcBreakPointHandler);
	dpbpd.pCOProject = pCOProject;
	dpbpd.pVM = pVM;
	dpbpd.pBP = new BreakPoint(BreakPoint::BPT_STEP, pMR, 0);
	dpbpd.szLibrariesPath = szLibrariesPath;
	if(!dpbpd.pBP->Set(pVM))
	{
        QMessageBox::information(NULL, "Error", "Error setting breakpoint at beginning of procedure.  Aborting.");
		delete(dpbpd.pBP);
		return;
	}
	pVM->Call(pMR, pParams, nParams); // (The VM will immediately hit the breakpoint we just set)
}

// ----------------------------
//
//  Methods that run in the VM thread
//
// ----------------------------

void DebugDialog::OnBreak()
{
	m_pStackManager->RefreshCurrentLocation();

	// Invoke the update in the GUI thread and wait until it's done
	m_bUpdateInvoked = true;
	Invoke(IA_UPDATE);
	int nSafety = 400;
	while(m_bUpdateInvoked && --nSafety)
	{
		Sleep(10);
#ifdef _WIN32
		GWindows::YieldToWindows();
#endif // _WIN32
	}
	m_bUpdateInvoked = false;
}

void DebugDialog::OnContinue()
{
	Invoke(IA_SETRUNNING);
}

void DebuggerGUIHandler(void* pHandlerParam, GuiFunc pFuncToCall, void* pFuncParam)
{
	DebugDialog* pThis = (DebugDialog*)pHandlerParam;
	GAssert(pThis->m_pGuiFunc == NULL && pThis->m_pGuiFuncParam == NULL, "Gui Func already set");
	pThis->m_pGuiFunc = pFuncToCall;
	pThis->m_pGuiFuncParam = pFuncParam;
	pThis->Invoke(DebugDialog::IA_CALL_FUNC_IN_GUI_THREAD);
	while(pThis->m_pGuiFuncParam != NULL)
	{
		Sleep(0);
		GWindows::YieldToWindows();
	}
}

unsigned int DebuggeeThread(void* pParameter)
{
	DebuggeeThreadStruct* pThreadStruct = (DebuggeeThreadStruct*)pParameter;
	GVM* pVM = pThreadStruct->pVM;
	DebugDialog* pDialog = pThreadStruct->pDialog;
	pDialog->m_bDebuggeeThreadRunning = true;
	pVM->ContinueInNewThread();
	if(!pDialog->m_bInDestructor)
		pDialog->Invoke(DebugDialog::IA_ALL_DONE);
	pDialog->m_bDebuggeeThreadRunning = false; // must be last value to change because other thread is looping on this value
	return TRUE;
}

void DebugDialog::Invoke(InvokeAction action)
{
	// See if there's already a message in the inbox
	int nGiveUp = 0;
	int nBoredness = 0;
	while(true)
	{
		m_pInvokeSpinLock->Lock("Invoke");
		if(m_invokeAction == IA_NONE)
			break;
		m_pInvokeSpinLock->Unlock();
		if(++nBoredness > 25)
		{
			if(++nGiveUp > 10)
			{
				GAssert(false, "The other thread isn't getting it's messages");
				return;
			}

			// yell at the app to check its inbox for the old message
			nBoredness = 0;
			QEvent* pEvent = new QEvent(QEvent::User);
			qApp->/*postEvent*/sendEvent(this, pEvent);
		}
		Sleep(10);
		GWindows::YieldToWindows();
	}

	// tell the app to check its inbox for this message
	m_invokeAction = action;
	m_pInvokeSpinLock->Unlock();
	QEvent* pEvent = new QEvent(QEvent::User);
	qApp->sendEvent(this, pEvent);
}

// ----------------------------
//
//  Methods that run in the GUI thread
//
// ----------------------------

void DebugDialog::OnStartRunning()
{
	TextLabel1_2->setText("Running");
	ListView2_2->clear();
	ListBox1->clear();
}

void DebugDialog::Update()
{
	// Refresh Instructions
	RefreshInstructions();

	// We now have enough information to find the next step instruction, so we can turn off the m_bUpdateInvoked flag
	m_bUpdateInvoked = false;

	// Change status
	TextLabel1_2->setText("Paused");

	// Refresh Call Stack
	RefreshCallStack();

	// Refresh Variable Stack
	RefreshVariableStack();
}

void DebugDialog::RefreshCallStack()
{
	ListBox1->clear();
	int nCount = m_pStackManager->GetCurrentStackSize();
	const char* pName;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pName = m_pStackManager->GetStackLayerMethodName(n);
		if(pName)
			ListBox1->insertItem(QString(pName));
		else
			ListBox1->insertItem(QString("<Method with no name>"));
	}
}

void DebugDialog::RefreshVariableStack()
{
	// Variables
	ListView2_2->clear();
	int nCount = m_pStackManager->GetDisplayVarCount();
	int n;
	DebugVariableItem* pPrev = NULL;
	for(n = 0; n < nCount; n++)
	{
		const char* szVarName = m_pStackManager->GetDisplayVarName(n);
		EVar* pVar = m_pStackManager->GetDisplayVarValue(n);
		if(pVar->eObType == VT_LOCAL_POINTER)
			continue; // Skip StartLocals
		GAssert(pVar->eObType == VT_OB_REF, "not a variable");
		pPrev = AddVariable(n, NULL, pPrev, szVarName, pVar->pOb);
	}

	// Params
	nCount = m_pStackManager->GetDisplayParamCount();
	for(n = 0; n < nCount; n++)
	{
		const char* szParamName = m_pStackManager->GetDisplayParamName(n);
		EVar* pParam = m_pStackManager->GetDisplayParamValue(n);
		if(pParam->eObType != VT_OB_REF)
		{
			pPrev = AddVariable(n, NULL, pPrev, "<error>", NULL);
			continue;
		}
		pPrev = AddVariable(n, NULL, pPrev, szParamName, pParam->pOb);
	}
}

DebugVariableItem* DebugDialog::AddVariable(int n, DebugVariableItem* pParent, DebugVariableItem* pPrev, const char* szName, GObject* pObject)
{
	DebugVariableItem* pNewItem;
	if(pParent)
		pNewItem = new DebugVariableItem(n, pParent, pPrev, pObject, szName, m_pVM->GetLibrary());
	else
		pNewItem = new DebugVariableItem(n, ListView2_2, pPrev, pObject, szName, m_pVM->GetLibrary());
	return pNewItem;
}

void DebugDialog::EnsureVisible(DebugListViewItem* pItem)
{
	DebugListViewItem* pParent = pItem;
	while(true)
	{
		pParent = (DebugListViewItem*)pParent->parent();
		if(!pParent)
			break;
		if(!pParent->isOpen())
			pParent->setOpen(true);
	}
}

void DebugDialog::RefreshInstructions()
{
	// todo: move all this logic into StackManager
	Library* pLibrary = m_pVM->GetLibrary();
	if(!pLibrary)
	{
		GAssert(false, "No Library");
		return;
	}
	EMethod* pEMethod = pLibrary->GetEMethod(m_pStackManager->GetDisplayMethodID());
	if(!pEMethod)
	{
		GAssert(false, "No such method in this library");
		return;
	}
	RefreshSource(pEMethod);
	EInstrArray* pEInstrArray = pEMethod->GetEInstrArray();
	if(!pEInstrArray)
	{
		GAssert(false, "Failed to get EInstrArray for current method");
		return;
	}
	if(!pEInstrArray->HaveSymbolicInfo())
		m_pSourceManager->MakeSymbols(pEInstrArray, pEMethod);
	int nInstr = pEInstrArray->FindInstrByOffset(m_pStackManager->GetDisplayCodeOffset());
	if(nInstr < 0)
	{
		GAssert(false, "Couldn't find an instruction at that offset");
		return;
	}
	COInstruction* pInstruction = pEInstrArray->GetCOInstruction(nInstr);
	if(!pInstruction)
		return; // todo: indicate that there is no corresponding instruction to the current location

	// Select the current disassembly line
	int nLine = pEInstrArray->GetDisassemblyStartLine() + nInstr;
	MultiLineEdit2->setCursorPosition(nLine - 1, 6, FALSE);
	MultiLineEdit2->setCursorPosition(nLine - 1, 35, TRUE);

	// Select the current instruction
	nLine = pInstruction->GetLineNumber();
	int nColumn, nWidth;
	pInstruction->GetColumnAndWidth(&nColumn, &nWidth);
	MultiLineEdit1->setCursorPosition(nLine - 1, nColumn - 1, FALSE);
	MultiLineEdit1->setCursorPosition(nLine - 1, nColumn - 1 + nWidth, TRUE);
}

void DebugDialog::RefreshSource(EMethod* pEMethod)
{
	// Refresh Source
	GXMLTag* pTypeTag = pEMethod->GetClass()->GetTag();
	if(!pTypeTag)
	{
		GAssert(false, "Couldn't get type tag");
		return;
	}
	if(pTypeTag == m_pSourceManager->GetCurrentTypeTag())
		return;
	GXMLAttribute* pSourceAttr = pTypeTag->GetAttribute(ATTR_SOURCE);
	if(!pSourceAttr)
	{
		GAssert(false, "Type tag has no Source attribute--todo: default to disassembly");
		return;
	}
	const char* szSourceActual;
	const char* szSource = m_pSourceManager->GetSourceFile(pSourceAttr->GetValue(), &szSourceActual);
	if(!szSource)
	{
		GAssert(false, "Couldn't load source file--todo: default to disassembly");
		return;
	}
	if(m_pSourceManager->IsCurrentSource(szSourceActual))
		return;
	MultiLineEdit1->setText(QString(szSource));
	m_pSourceManager->SetCurrentSource(szSourceActual);
	m_pSourceManager->SetCurrentTypeTag(pTypeTag);

	// Refresh Disassembly
	if(!m_bDisassemblySourceRefreshed)
	{
		MultiLineEdit2->setText(QString(m_pSourceManager->GetDisassembly()));
		m_bDisassemblySourceRefreshed = true;
	}
}

COInstruction* DebugDialog::GetCurrentInstruction()
{
	return m_pStackManager->GetCurrentInstruction();
}

int DebugDialog::GetCurrentMethodID()
{
	 return m_pStackManager->GetCurrentMethodID();
}

int DebugDialog::GetCurrentCodeOffset()
{
	 return m_pStackManager->GetCurrentCodeOffset();
}

EInstrArray* DebugDialog::GetCurrentEInstrArray()
{
	return m_pStackManager->GetCurrentEInstrArray();
}

QPixmap* DebugDialog::GetWhichPixmap(int nOffset)
{
/*
	m_pWorkingLock->Lock("GetWhichPixmap");
	bool bBreakPoint = false;
	bool bInstrPointer = false;
	if(FindBreakPoint(m_nMethodID, nOffset) >= 0)
		bBreakPoint = true;
	if(nOffset == m_nOffset)
		bInstrPointer = true;
	m_pWorkingLock->Unlock();
	if(bInstrPointer)
	{
		if(bBreakPoint)
			return m_pInstrOnBreakPointPixmap;
		else
			return m_pInstrPointerPixmap;
	}
	else if(bBreakPoint)
		return m_pBreakPointPixmap;
	else
		return m_pBlankSpotPixmap;
*/
	return NULL;
}
/*
// returns the instruction number of the next AsmInstruction (the one with the sentinel offset)
int DebugDialog::AddAsmInstructions(DebugListViewItem* pParent, Disassembly* pDisassembly, int nInstr, int nSize)
{
	QPixmap* pPixmap;
	InstrNode* pInstr;
	DebugListViewItem* pPrev = NULL;
	InstrNode* pAsmInstructions = pDisassembly->GetInstrArray();
	int nAsmInstrCount = pDisassembly->GetInstrCount();
	int nSentinelOffset = pAsmInstructions[nInstr].m_nCodePos + nSize;
	while(nInstr < nAsmInstrCount && pAsmInstructions[nInstr].m_nCodePos < nSentinelOffset)
	{
		pInstr = &pAsmInstructions[nInstr];
		pPixmap = GetWhichPixmap(pInstr->m_nCodePos);
		if(pParent)
			pPrev = new DebugListViewItem(pParent, pPrev, pInstr, "todo:comment", pPixmap);
		else
			pPrev = new DebugListViewItem(ListView2, pPrev, pInstr, "todo:comment", pPixmap);
		if(pInstr->m_nCodePos == m_nOffset && m_pCurrentItem == NULL)
			m_pCurrentItem = pPrev;
		nInstr++;
	}
	return nInstr;
}

DebugListViewItem* DebugDialog::AddCOInstruction(DebugListViewItem* pParent, DebugListViewItem* pPrev, COInstruction* pInstr, COInstruction* pNextInstr, Disassembly* pDisassembly, int* pnCurrentAsmInstr)
{
	// Create the node for this instruction
	QPixmap* pPixmap = GetWhichPixmap(pInstr->GetOffset());
	DebugListViewItem* pNewItem;
	if(pParent)
		pNewItem = new DebugListViewItem(pParent, pPrev, pInstr, pPixmap, m_pProject); // todo: set proper pixmap
	else
		pNewItem = new DebugListViewItem(ListView2, pPrev, pInstr, pPixmap); // todo: set proper pixmap
	if(pInstr->GetOffset() == m_nOffset && m_pCurrentItem == NULL)
		m_pCurrentItem = pNewItem;

	// Add children
	COInstrArray* pInstructions = pInstr->GetChildInstructions();
	if(pInstructions)
	{
		int nCount = pInstructions->GetInstrCount();
		int n;
		DebugListViewItem* pPrevItem = NULL;
		for(n = 0; n < nCount; n++)
			pPrevItem = AddCOInstruction(pNewItem, pPrevItem, pInstructions->GetInstr(n), ((n + 1 >= nCount) ? NULL : pInstructions->GetInstr(n + 1)), pDisassembly, pnCurrentAsmInstr);
	}
	else
	{
		// todo: figure out how to display the asm sub-instructions for if/while commands
		InstrNode* pAsmInstructions = pDisassembly->GetInstrArray();
		int nAsmInstrCount = pDisassembly->GetInstrCount();
		while(*pnCurrentAsmInstr < nAsmInstrCount && pAsmInstructions[*pnCurrentAsmInstr].m_nCodePos < pInstr->GetOffset())
			(*pnCurrentAsmInstr)++;
		if(*pnCurrentAsmInstr < nAsmInstrCount)
			*pnCurrentAsmInstr = AddAsmInstructions(pNewItem, pDisassembly, *pnCurrentAsmInstr, pInstr->GetSize());
		else
			GAssert(false, "Problem finding asm instructions for this instruction");
	}
	return pNewItem;
}
*/

// Step In button
void DebugDialog::slot_button1()
{
	if(TabWidget3_2->currentPageIndex() == 1) // if you're looking at the disassembly
		m_pBreakPointManager->SingleStepIn();
	else
		m_pBreakPointManager->StepIn();
}

// Step Over button
void DebugDialog::slot_button2()
{
	if(TabWidget3_2->currentPageIndex() == 1) // if you're looking at the disassembly
		m_pBreakPointManager->SingleStepOver();
	else
		m_pBreakPointManager->StepOver();
}

// Step Out button
void DebugDialog::slot_button3()
{
	m_pBreakPointManager->StepOut();
}

// Toggle Breakpoint button
void DebugDialog::slot_button4()
{
/*	DebugListViewItem* pSelected = (DebugListViewItem*)ListView2->selectedItem();
	if(!pSelected)
	{
        QMessageBox::information(NULL, "Nothing selected", "First select a line to toggle a breakpoint at");
		return;
	}
	int nOffset;
	if(pSelected->GetType() == DebugListViewItem::IT_ASM)
	{
		InstrNode* pInstr = pSelected->GetAsmInstr();
		nOffset = pInstr->m_nCodePos;
	}
	else if(pSelected->GetType() == DebugListViewItem::IT_CO)
	{
		COInstruction* pInstr = pSelected->GetCOInstr();
		nOffset = pInstr->GetOffset();
	}
	else
	{
		GAssert(false, "unrecognized type");
		return;
	}
	m_pWorkingLock->Lock("slot_button4");
	int nCurrentBreakPoint = FindBreakPoint(m_nMethodID, nOffset);
	if(nCurrentBreakPoint >= 0)
		RemoveBreakPoint(nCurrentBreakPoint);
	else
	{
		BreakPoint* pNewBreakPoint = new BreakPoint(BreakPoint::BPT_STEP, m_nMethodID, nOffset);
		pNewBreakPoint->Set(m_pVM);
		AddBreakPoint(pNewBreakPoint);
	}
	m_pWorkingLock->Unlock();
	Update();*/
}

// Clear all BreakPoints button
void DebugDialog::slot_button5()
{
	m_pBreakPointManager->ClearAllBreakPoints();
	Update();
}

// Go button
void DebugDialog::slot_button6()
{
	m_pBreakPointManager->Continue();
}

// Quit button
void DebugDialog::slot_button7()
{
	accept();
}

void DebugDialog::slot_button8()
{

}

void DebugDialog::slot_button9()
{

}

void DebugDialog::slot_callStackSelChange()
{
	int n = ListBox1->currentItem();
	if(n < 0)
		return;
	GAssert(n < m_pStackManager->GetCurrentStackSize(), "out of range");
	m_pStackManager->SetDisplayLayer(n);
	GWindows::YieldToWindows();
	Sleep(50);
	Update();
	GWindows::YieldToWindows();
	Sleep(50);
}

void DebugDialog::keyPressEvent(QKeyEvent* e)
{
	// Note: see qnamespace.h for a list of Qt key-codes
	switch(e->key())
	{
		case 'I': slot_button1(); break;
		case 'O': slot_button2(); break;
		case 'P': slot_button3(); break;
		case 'B': slot_button4(); break;
		case 'C': slot_button5(); break;
		case 'G': slot_button6(); break;
		case Key_Escape: // fall through
		case 'Q': accept(); break;
	}
}

bool DebugDialog::event(QEvent* e)
{
	if(e->type() == QEvent::User)
	{
		m_pInvokeSpinLock->Lock("event");
		InvokeAction action = m_invokeAction;
		m_invokeAction = IA_NONE;
		m_pInvokeSpinLock->Unlock();
		switch(action)
		{
		case IA_UPDATE:
			Update();
			break;
		case IA_SETRUNNING:
			OnStartRunning();
			break;
		case IA_NONE:
			GAssert(false, "No action to perform");
			break;
		case IA_CALL_FUNC_IN_GUI_THREAD:
			m_pGuiFunc(m_pGuiFuncParam);
			m_pGuiFunc = NULL;
			m_pGuiFuncParam = NULL;
			break;
		case IA_ALL_DONE:
			accept();
			break;
		default:
			{
				GAssert(false, "Unrecognized InvokeAction");
			}
		}
	}
	return DebugDialogBase::event(e);
}

void DebugDialog::resizeEvent(QResizeEvent* pResizeEvent)
{
	const QSize* pSize = &pResizeEvent->size();
//	MyCustomWidget1->setFixedSize(pSize->width() - 110, pSize->height() - 90);
}
