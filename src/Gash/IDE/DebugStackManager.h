#ifndef __DEBUGSTACKMANAGER_H__
#define __DEBUGSTACKMANAGER_H__

#include "../Include/GashEngine.h"

class GVM;
class GCallStackLayerArray;
class EMethod;
class COInstruction;
class EInstrArray;

class DebugStackManager
{
protected:
	GVM* m_pVM;
	int m_nMethodID;
	int m_nOffset;
	int m_nDisplayLayer;
	int m_nDisplayMethodID;
	int m_nDisplayOffset;
	GCallStackLayerArray* m_pCallStack;
	const char** m_pDisplayVarNames;
	int m_nDisplayVarCount;
	int m_nDisplayVarNamesBufferSize;
	int m_nDisplayLayerStackTop;
	const char** m_pDisplayParamNames;
	int m_nDisplayParamCount;
	int m_nDisplayParamNamesBufferSize;

public:
	DebugStackManager(GVM* pVM);
	virtual ~DebugStackManager();

	void RefreshCurrentLocation();
	int GetCurrentMethodID() { return m_nMethodID; }
	int GetCurrentCodeOffset() { return m_nOffset; }
	int GetDisplayLayer() { return m_nDisplayLayer; }
	void SetDisplayLayer(int n) { m_nDisplayLayer = n; }
	int GetDisplayMethodID() { return m_nDisplayMethodID; }
	int GetDisplayCodeOffset() { return m_nDisplayOffset; }
	int GetCurrentStackSize();
	const char* GetStackLayerMethodName(int nLayer);
	int GetDisplayVarCount() { return m_nDisplayVarCount; }
	const char* GetDisplayVarName(int nStackPos);
	EVar* GetDisplayVarValue(int nStackPos);
	int GetDisplayParamCount() { return m_nDisplayParamCount; }
	const char* GetDisplayParamName(int nParam);
	EVar* GetDisplayParamValue(int nParam);
	COInstruction* GetCurrentInstruction();
	EInstrArray* GetCurrentEInstrArray();

protected:
	EMethod* GetLayerEMethod(int nLayer);
	void RefreshDisplayVarNames();
};

#endif // __DEBUGSTACKMANAGER_H__
