#ifndef __DEBUGBREAKPOINTMANAGER_H__
#define __DEBUGBREAKPOINTMANAGER_H__

#define MAX_BREAKPOINTS 32

class GVM;
class GSpinLock;
class DebugListViewItem;
class DebugDialog;
struct MethodRef;

class BreakPoint
{
public:
	enum BreakPointType
	{
		BPT_STEP,
		BPT_PERSIST,
	};

protected:
	struct MethodRef* m_pMethodRef;
	int m_nOffset;
	unsigned char m_cInstrByte;
	BreakPointType m_eType;
	bool m_bSet;

public:
	BreakPoint(BreakPointType eType, MethodRef* pMR, int nOffset);
	virtual ~BreakPoint();

	bool Set(GVM* pVM);
	bool UnSet(GVM* pVM);
	bool IsSet() { return m_bSet; }
	bool IsStepBreakpoint() { return m_eType == BPT_STEP ? true : false; }
	MethodRef* GetMethodRef() { return m_pMethodRef; }
	int GetOffset() { return m_nOffset; }
};




class DebugBreakPointManager
{
protected:
	DebugDialog* m_pDebugDialog;
	GVM* m_pVM;
	BreakPoint* m_pBreakPoints[MAX_BREAKPOINTS];
	GSpinLock* m_pWorkingLock; // protects m_bRunning and the breakpoint lists
	int m_nBreakPointCount;
	bool m_bHoldBreakPoint;
	bool m_bRunning;

public:
	DebugBreakPointManager(DebugDialog* pDebugDialog, GVM* pVM);
	virtual ~DebugBreakPointManager();

	void RemoveBreakPoint(int n);
	bool AddBreakPoint(BreakPoint* pBP);
	void SetStepBreakPointAtOffset(int nMethodID, int nOffset);
	void ClearAllBreakPoints();
	bool Continue();
	void OnBreakPoint();
	bool SingleStepIn();
	bool StepIn();
	void SingleStepOver();
	void StepOver();
	void StepOut();

protected:
	void RemoveStepBreakPoints();
	void WalkPastCurrentBreakPoint();
	int FindBreakPoint(int nMethodID, int nOffset);
};

#endif // __DEBUGBREAKPOINTMANAGER_H__
