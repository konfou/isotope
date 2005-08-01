#include "DebugBreakPointManager.h"
#include "../Include/GashQt.h"
#include "../Engine/InstrTable.h"
#include "../Engine/InstrSet.h"
#include "../Engine/EInstrArray.h"
#include "../Engine/EMethod.h"
#include "../CodeObjects/Instruction.h"
#include "../../GClasses/GSpinLock.h"
#include "../../GClasses/GnuSDK.h"
#include "../../GClasses/GWindows.h"
#include "../../GClasses/GPointerQueue.h"
#include "DebuggerListViewItem.h"
#include "DebugDialog.h"
#include "DebugStackManager.h"

BreakPoint::BreakPoint(BreakPointType eType, MethodRef* pMR, int nOffset)
{
	m_eType = eType;
	m_pMethodRef = new MethodRef();
	*m_pMethodRef = *pMR;
	m_nOffset = nOffset;
	GAssert(nOffset >= 0, "Out of range");
	m_bSet = false;
	m_cInstrByte = GetFuncEnum(Instr_BreakPoint);
}

BreakPoint::~BreakPoint()
{
	GAssert(!m_bSet, "Breakpoint is still set!");
	delete(m_pMethodRef);
}

bool BreakPoint::Set(GVM* pVM)
{
	if(m_bSet)
	{
		GAssert(false, "This breakpoint already set");
		return false;
	}

	// Make sure we have already disassembled this method because
	// it won't be disassemble-able after we set a breakpoint.
	GAssert(!m_pMethodRef->bVirtual, "todo: figure out how to handle virtual calls here");
	int nMethodID = m_pMethodRef->nIndex;
	EMethod* pEMethod = pVM->GetLibrary()->GetEMethod(nMethodID);
	if(!pEMethod)
	{
		GAssert(false, "No such method in this library");
		return false;
	}
	pEMethod->GetEInstrArray();
	
	// Set the breakpoint
	unsigned char c = m_cInstrByte;
	if(!pVM->SwapByte(nMethodID, m_nOffset, c, &m_cInstrByte))
	{
		GAssert(false, "Failed to set breakpoint");
		return false;
	}
	GAssert(GetInstructionStruct(m_cInstrByte), "Not a valid instruction");
	if(c == m_cInstrByte)
		return false; // another breakpoint already set here
	m_bSet = true;
	return true;
}

bool BreakPoint::UnSet(GVM* pVM)
{
	if(!m_bSet)
	{
		GAssert(false, "This breakpoint not set");
		return false;
	}
	unsigned char c = m_cInstrByte;
	GAssert(m_pMethodRef->bVirtual, "todo: figure out how to handle virtual calls here");
	int nMethodID = m_pMethodRef->nIndex;
	if(!pVM->SwapByte(nMethodID, m_nOffset, c, &m_cInstrByte))
	{
		GAssert(false, "Failed to unset breakpoint");
		return false;
	}
	m_bSet = false;
	return true;
}



// -----------------------------------------------------------------

// todo: change this to a virtual method on an inherited type
void DebuggerBreakPointHandler(void* pThis)
{
	((DebugBreakPointManager*)pThis)->OnBreakPoint();
}


DebugBreakPointManager::DebugBreakPointManager(DebugDialog* pDebugDialog, GVM* pVM)
{
	m_pDebugDialog = pDebugDialog;
	m_pVM = pVM;
	m_pWorkingLock = new GSpinLock();
	m_bHoldBreakPoint = true;
	m_bRunning = true;
	m_nBreakPointCount = 0;
	pVM->SetBreakPointHandler(this, DebuggerBreakPointHandler);
}

DebugBreakPointManager::~DebugBreakPointManager()
{
	m_pWorkingLock->Unlock();
	m_pWorkingLock->Lock("~DebugBreakPointManager");
	while(m_nBreakPointCount > 0)
		RemoveBreakPoint(0);
	m_pWorkingLock->Unlock();
	delete(m_pWorkingLock);
}

void DebugBreakPointManager::RemoveStepBreakPoints()
{
	m_pWorkingLock->Lock("OnBreakPoint1");
	int n;
	BreakPoint* pBreakPoint;
	for(n = m_nBreakPointCount - 1; n >= 0; n--)
	{
		pBreakPoint = m_pBreakPoints[n];
		if(pBreakPoint->IsStepBreakpoint())
			RemoveBreakPoint(n);
	}
	m_pWorkingLock->Unlock();
}

void DebugBreakPointManager::RemoveBreakPoint(int n)
{
	GAssert(m_pWorkingLock->IsLocked(), "Caller should hold this lock");
	GAssert(n >= 0 && n < m_nBreakPointCount, "Out of range");
	GAssert(m_bHoldBreakPoint, "can't remove breakpoints while running");
	BreakPoint* pBP = m_pBreakPoints[n];
	if(pBP->IsSet())
		pBP->UnSet(m_pVM);
	delete(pBP);
	m_nBreakPointCount--;
	m_pBreakPoints[n] = m_pBreakPoints[m_nBreakPointCount];
	m_pBreakPoints[m_nBreakPointCount] = NULL;
}

bool DebugBreakPointManager::AddBreakPoint(BreakPoint* pBP)
{
	m_pWorkingLock->Lock("AddBreakPoint");
	GAssert(m_bHoldBreakPoint, "can't set breakpoints while running");
	bool bRet = true;
	do
	{
		if(m_nBreakPointCount >= MAX_BREAKPOINTS)
		{
			delete(pBP); // we said we'd take ownership of it
			bRet = false;
			break;
		}
		m_pBreakPoints[m_nBreakPointCount] = pBP;
		m_nBreakPointCount++;
	}
	while(false);
	m_pWorkingLock->Unlock();
	return bRet;
}

void DebugBreakPointManager::SetStepBreakPointAtOffset(int nMethodID, int nOffset)
{
	GAssert(m_bHoldBreakPoint, "can't set breakpoints while running");
	// todo: figure out what to do about virtual calls
	struct MethodRef mr;
	mr.bVirtual = false;
	mr.nIndex = nMethodID;
	BreakPoint* pNewBreakPoint = new BreakPoint(BreakPoint::BPT_STEP, &mr, nOffset);
	pNewBreakPoint->Set(m_pVM);
	AddBreakPoint(pNewBreakPoint);
}

int DebugBreakPointManager::FindBreakPoint(int nMethodID, int nOffset)
{
	GAssert(m_pWorkingLock->IsLocked(), "Caller should hold this lock");
	int n;
	for(n = 0; n < m_nBreakPointCount; n++)
	{
		GAssert(!m_pBreakPoints[n]->GetMethodRef()->bVirtual, "todo: figure out what to do about virtual calls here");
		if(m_pBreakPoints[n]->GetMethodRef()->nIndex == nMethodID &&
			m_pBreakPoints[n]->GetOffset() == nOffset)
			return n;
	}
	return -1;
}

void DebugBreakPointManager::ClearAllBreakPoints()
{
	m_pWorkingLock->Lock("slot_button5");
	while(m_nBreakPointCount > 0)
		RemoveBreakPoint(m_nBreakPointCount - 1);
	m_pWorkingLock->Unlock();
}

void DebugBreakPointManager::WalkPastCurrentBreakPoint()
{
	GAssert(m_pWorkingLock->IsLocked(), "caller should hold this lock");
	int nMethodID = m_pDebugDialog->GetCurrentMethodID();
	int nOffset = m_pDebugDialog->GetCurrentCodeOffset();
	int nBreakPoint = FindBreakPoint(nMethodID, nOffset);
	if(nBreakPoint < 0)
		return;
	BreakPoint* pCurrentBreakPoint = m_pBreakPoints[nBreakPoint];		
	bool bSet = pCurrentBreakPoint->IsSet();
	if(bSet && !pCurrentBreakPoint->UnSet(m_pVM))
	{
		GAssert(false, "Failed to unset breakpoint");
		return;
	}
	m_pVM->Step();
	if(bSet && !pCurrentBreakPoint->Set(m_pVM))
	{
		GAssert(false, "Failed to reset current breakpoint");
		return;
	}
}

bool DebugBreakPointManager::Continue()
{
	m_pWorkingLock->Lock("DebugBreakPointManager::Continue");
	if(m_bRunning)
	{
		m_pWorkingLock->Unlock();
		return false;
	}
	m_bHoldBreakPoint = false;
	m_bRunning = true;
	m_pWorkingLock->Unlock();
	return true;
}

void DebugBreakPointManager::OnBreakPoint()
{
	RemoveStepBreakPoints();
	m_pDebugDialog->OnBreak();

	// Spin on the breakpoing until m_bHoldBreakPoint is false
	m_pWorkingLock->Lock("OnBreakPoint(1)");
	while(true)
	{
		if(!m_bHoldBreakPoint)
			break;
		m_bRunning = false;
		m_pWorkingLock->Unlock();
		Sleep(10);
#ifdef _WIN32
		GWindows::YieldToWindows();
#endif // _WIN32
		m_pWorkingLock->Lock("OnBreakPoint(2)");
	}
	m_bHoldBreakPoint = true;
	GAssert(m_bRunning, "should set m_bRunning to true when you set m_bHoldBreakPoint to false");
	WalkPastCurrentBreakPoint();
	m_pWorkingLock->Unlock();
	m_pDebugDialog->OnContinue();
}

bool DebugBreakPointManager::SingleStepIn()
{
	// Step to the next assembly instruction
	m_pDebugDialog->OnContinue();
	m_pWorkingLock->Lock("StepIn");
	if(m_bRunning)
	{
		m_pWorkingLock->Unlock();
		return false;
	}
	m_pVM->Step();
	m_pWorkingLock->Unlock();
	m_pDebugDialog->OnBreak();
	return true;
}

bool DebugBreakPointManager::StepIn()
{
	// Single-Step until the symbol instruction changes
	COInstruction* pInstr = m_pDebugDialog->GetCurrentInstruction();
	COInstruction* pInstrOrig = pInstr;
	while(pInstr == pInstrOrig)
	{
		if(!SingleStepIn())
			return false;
		pInstr = m_pDebugDialog->GetCurrentInstruction();
	}
	return true;
}

void DebugBreakPointManager::SingleStepOver()
{
	GAssert(false, "todo: not implemented yet");
}

void DebugBreakPointManager::StepOver()
{
	COInstruction* pInstr = m_pDebugDialog->GetCurrentInstruction();
	GPointerQueue q;
	pInstr->GetStepOverInstructions(&q);
	int nMethodID = m_pDebugDialog->GetCurrentMethodID();
	EInstrArray* pEInstrArray = m_pDebugDialog->GetCurrentEInstrArray();
	while(q.GetSize() > 0)
	{
		COInstruction* pPossibleNext = (COInstruction*)q.Pop();
		int nAsmInstr = pPossibleNext->GetFirstAsmInstr();
		if(nAsmInstr >= 0)
		{
			int nOffset = pEInstrArray->GetOffset(nAsmInstr);
			SetStepBreakPointAtOffset(nMethodID, nOffset);
		}
	}
	StepOut();
}

void DebugBreakPointManager::StepOut()
{
	// todo: set step breakpoints
	Continue();
}
