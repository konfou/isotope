/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __DEBUGDIALOG_H__
#define __DEBUGDIALOG_H__

#include "DebugDialogBase.h"
#include "../Include/GashEngine.h"

class COInstruction;
class InstrNode;
class COMethod;
class GVM;
class GIntArray;
class COFileSet;
class Disassembly;
class Library;
class COProject;
class GSpinLock;
class QPixmap;
class DebugListViewItem;
class DebugVariableItem;
class DebugSourceManager;
class DebugBreakPointManager;
class BreakPoint;
class DebugStackManager;
class EInstrArray;

// *** Don't call these--used internally
void DebuggerBreakPointHandler(void* pThis);
void DebuggerGUIHandler(void* pHandlerParam, GuiFunc pFuncToCall, void* pFuncParam);
unsigned int DebuggeeThread(void* pParameter);
// ***



class DebugDialog : public DebugDialogBase
{
friend void DebuggerBreakPointHandler(void* pThis);
friend unsigned int DebuggeeThread(void* pParameter);
friend void DebuggerGUIHandler(void* pHandlerParam, GuiFunc pFuncToCall, void* pFuncParam);
protected:

	enum InvokeAction
	{
		IA_NONE,
		IA_UPDATE,
		IA_SETRUNNING,
		IA_CALL_FUNC_IN_GUI_THREAD,
		IA_ALL_DONE,
	};

	GVM* m_pVM;
	bool m_bDebuggeeThreadRunning;
	InvokeAction m_invokeAction;
	DebugSourceManager* m_pSourceManager;
	DebugBreakPointManager* m_pBreakPointManager;
	DebugStackManager* m_pStackManager;

	// Synchronization
	GSpinLock* m_pInvokeSpinLock; // protects the m_invokeAction variable
	bool m_bInDestructor;
	bool m_bUpdateInvoked;

	DebugListViewItem* m_pCurrentItem;
	bool m_bDisassemblySourceRefreshed;

	// Pixmaps
	QPixmap* m_pBreakPointPixmap;
	QPixmap* m_pInstrPointerPixmap;
	QPixmap* m_pInstrOnBreakPointPixmap;
	QPixmap* m_pBlankSpotPixmap;

	// Gui handling callback stuff
	GuiFunc m_pGuiFunc;
	void* m_pGuiFuncParam;

	DebugDialog(const char* name, GVM* pVM);

public:
	// Thread Handles
	HANDLE m_hGUIThread;
	HANDLE m_hVMThread;


	virtual ~DebugDialog();

	// You must call this method in the thread that does modal dialogs (or it
	// will lock up when it tries to create the debugger dialog.)
	static void DebugProc(GVM* pVM, MethodRef* pMR, COProject* pCOProject, VarHolder** pParams, int nParams, const char* szLibrariesPath);

	// You must call this method in the thread that does modal dialogs (or it
	// will lock up when it tries to create the debugger dialog.)  If your VM is
	// running in that thread, you can call this from your breakpoint handler.
	// (pBP is an optional parameter that specifies a remove-able breakpoint that you
	// want the debugger to be aware of.  It takes ownership of pBP.)
	static bool DebugMe(GVM* pVM, COProject* pCOProject, BreakPoint* pBP);

    virtual void slot_button1();
    virtual void slot_button2();
    virtual void slot_button3();
    virtual void slot_button4();
    virtual void slot_button5();
    virtual void slot_button6();
    virtual void slot_button7();
    virtual void slot_button8();
    virtual void slot_button9();
    virtual void slot_callStackSelChange();

	void OnBreak();
	void OnContinue();

	COInstruction* GetCurrentInstruction();
	int GetCurrentMethodID();
	int GetCurrentCodeOffset();
	EInstrArray* GetCurrentEInstrArray();

protected:
	// Methods that run in the VM thread
	void RemoveStepBreakPoints();
	void WalkPastCurrentBreakPoint();
	int FindBreakPoint(int nMethodID, int nOffset);
	void Invoke(InvokeAction action);

	virtual void resizeEvent(QResizeEvent* pResizeEvent);

	// Methods that run in the UI thread
//	int AddAsmInstructions(DebugListViewItem* pParent, Disassembly* pDisassembly, int nInstr, int nSentinelOffset);
//	DebugListViewItem* AddCOInstruction(DebugListViewItem* pParent, DebugListViewItem* pPrev, COInstruction* pInstr, COInstruction* pNextInstr, Disassembly* pDisassembly, int* pnCurrentAsmInstr);
	DebugVariableItem* AddVariable(int n, DebugVariableItem* pParent, DebugVariableItem* pPrev, const char* szName, GObject* pObject);
	const char* GetMethodName(int nID);
	bool AddBreakPoint(BreakPoint* pBP);
	virtual bool event(QEvent* e);
	void Update();
	void RefreshCallStack();
	void RefreshVariableStack();
	void RefreshInstructions();
	void RefreshSource(EMethod* pEMethod);
	QPixmap* GetWhichPixmap(int nOffset);
	void OnStartRunning();
	void EnsureVisible(DebugListViewItem* pItem);
	virtual void keyPressEvent(QKeyEvent* e);
	void Continue();
};

#endif // __DEBUGDIALOG_H__
