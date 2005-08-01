#include "DebugStackManager.h"
#include "../Engine/GVMStack.h"
#include "../Engine/EInstrArray.h"
#include "../Engine/TagNames.h"
#include "../Engine/EMethod.h"
#include "../Include/GashEngine.h"
#include "../CodeObjects/Call.h"
#include "../CodeObjects/Variable.h"
#include "../../GClasses/GXML.h"

DebugStackManager::DebugStackManager(GVM* pVM)
{
	m_pVM = pVM;
	m_nMethodID = -1;
	m_nOffset = -1;
	m_nDisplayLayer = 0;
	m_nDisplayMethodID = -1;
	m_nDisplayOffset = -1;
	m_pDisplayVarNames = NULL;
	m_nDisplayVarNamesBufferSize = 0;
	m_nDisplayVarCount = 0;
	m_nDisplayLayerStackTop = 0;
	m_pDisplayParamNames = NULL;
	m_nDisplayParamCount = 0;
	m_nDisplayParamNamesBufferSize = 0;
	m_pCallStack = new GCallStackLayerArray(64);
}

DebugStackManager::~DebugStackManager()
{
	delete(m_pCallStack);
}

void DebugStackManager::RefreshCurrentLocation()
{
	m_pVM->GetCallStack(m_pCallStack);
	GAssert(m_pCallStack->GetSize() > 0, "That can't be right");
	m_nOffset = m_pCallStack->GetLayer(0)->nOffset;
	m_nMethodID = m_pCallStack->GetLayer(0)->nMethodID;
	m_nDisplayOffset = m_pCallStack->GetLayer(m_nDisplayLayer)->nOffset;
	m_nDisplayMethodID = m_pCallStack->GetLayer(m_nDisplayLayer)->nMethodID;
	struct CallStackLayer* pLayer = m_pCallStack->GetLayer(m_nDisplayLayer);
	m_nDisplayLayerStackTop = pLayer->nStackTop;
	RefreshDisplayVarNames();
}

int DebugStackManager::GetCurrentStackSize()
{
	return m_pCallStack->GetSize();
}

EMethod* DebugStackManager::GetLayerEMethod(int nLayer)
{
	int nMethodID = m_pCallStack->GetLayer(nLayer)->nMethodID;
	EMethod* pEMethod = m_pVM->GetLibrary()->GetEMethod(nMethodID);
	return pEMethod;
}

const char* DebugStackManager::GetStackLayerMethodName(int nLayer)
{
	EMethod* pEMethod = GetLayerEMethod(nLayer);
	return pEMethod->GetName();
}

EInstrArray* DebugStackManager::GetCurrentEInstrArray()
{
	EMethod* pEMethod = GetLayerEMethod(0);
	return pEMethod->GetEInstrArray();
}

COInstruction* DebugStackManager::GetCurrentInstruction()
{
	EInstrArray* pEInstrArray = GetCurrentEInstrArray();
	int nInstr = pEInstrArray->FindInstrByOffset(m_nOffset);
	return pEInstrArray->GetCOInstruction(nInstr);
}

void DebugStackManager::RefreshDisplayVarNames()
{
	// Find the current instruction number
	EMethod* pEMethod = GetLayerEMethod(m_nDisplayLayer);
	if(!pEMethod)
	{
		GAssert(false, "No such method in this library");
		return;
	}
	EInstrArray* pEInstrArray = pEMethod->GetEInstrArray();
	if(!pEInstrArray)
	{
		GAssert(false, "Failed to get EInstrArray");
		return;
	}
	int nInstr = pEInstrArray->FindInstrByOffset(m_nDisplayOffset);
	if(nInstr < 0)
	{
		GAssert(false, "Couldn't find instruction at that offset");
		return;
	}

	// Get a buffer to store the variable names
	m_nDisplayVarCount = pEInstrArray->CountStackDepth(nInstr);
	if(m_nDisplayVarCount > m_nDisplayVarNamesBufferSize)
	{
		delete [] m_pDisplayVarNames;
		m_nDisplayVarNamesBufferSize = (int)(m_nDisplayVarCount * 1.5); // add 50% just to avoid reallocations
		m_pDisplayVarNames = new const char*[m_nDisplayVarNamesBufferSize];
		memset(m_pDisplayVarNames, '\0', m_nDisplayVarNamesBufferSize * sizeof(const char*));
	}

	// Fill the array
	int nPos = pEInstrArray->GetStackInstr(nInstr);
	int n;
	for(n = m_nDisplayVarCount - 1; n >= 0; n--)
	{
		COInstruction* pInstruction = pEInstrArray->GetCOInstruction(nPos);
		if(pInstruction && pInstruction->GetInstructionType() == COInstruction::IT_CALL)
		{
			// Find the parameter that declared this variable
			COCall* pCall = (COCall*)pInstruction;
			int nParamCount = pCall->GetParamCount();
			int i;
			for(i = 0; i < nParamCount; i++)
			{
				COExpression* pParam = pCall->GetParam(i);
				if(pParam->GetExpressionType() == COExpression::ET_VARDECL)
				{
					// todo: handle the case where one instruction declares multiple variables
					COVariable* pVariable = (COVariable*)pParam;
					m_pDisplayVarNames[n] = pVariable->GetName();
					break;
				}
			}
			if(!m_pDisplayVarNames[n])
				m_pDisplayVarNames[n] = "<not found>";
		}
		else
			m_pDisplayVarNames[n] = "<unknown>";
		nPos = pEInstrArray->GetStackInstr(nPos);
	}

	// Get a buffer to store the param names
	m_nDisplayParamCount = pEMethod->CountParams();
	bool bStatic = (stricmp(pEMethod->GetTag()->GetName(), TAG_NAME_PROCEDURE) == 0);
	if(m_nDisplayParamCount > m_nDisplayParamNamesBufferSize)
	{
		delete [] m_pDisplayParamNames;
		m_nDisplayParamNamesBufferSize = (int)(m_nDisplayParamCount * 1.5); // add 50% just to avoid reallocations
		m_pDisplayParamNames = new const char*[m_nDisplayParamNamesBufferSize];
		memset(m_pDisplayParamNames, '\0', m_nDisplayParamNamesBufferSize * sizeof(const char*));
	}

	// Fill the array
	n = 0;
	if(!bStatic)
	{
		m_pDisplayParamNames[n] = "this";
		n++;
	}
	GXMLTag* pTag;
	for(pTag = pEMethod->GetTag()->GetFirstChildTag(); pTag && n < m_nDisplayParamCount; pTag = pEMethod->GetTag()->GetNextChildTag(pTag))
	{
		if(stricmp(pTag->GetName(), TAG_NAME_VAR) != 0)
			continue;
		GXMLAttribute* pExpAttr = pTag->GetAttribute(ATTR_EXP);
		if(pExpAttr)
		{
			const char* szParamName = pExpAttr->GetValue();
			int i;
			for(i = 0; szParamName[i] != '\0' && szParamName[i] != ':'; i++); // the trailing ';' is intentional
			if(szParamName[i] == ':')
				szParamName = &szParamName[i + 1];
			m_pDisplayParamNames[n] = szParamName;
		}
		else
			m_pDisplayParamNames[n] = "<param missing name>";
		n++;
	}
}

const char* DebugStackManager::GetDisplayVarName(int nStackPos)
{
	if(nStackPos < 0 || nStackPos >= m_nDisplayVarCount)
		return "<out of range>";
	return m_pDisplayVarNames[nStackPos];
}

EVar* DebugStackManager::GetDisplayVarValue(int nStackPos)
{
	if(nStackPos < 0 || nStackPos >= m_nDisplayVarCount)
		return NULL;
	int nPos = m_nDisplayLayerStackTop - m_nDisplayVarCount + nStackPos;
	if(nPos < 0)
		return NULL;
	EVar* pVar = m_pVM->GetVariableFromStackNotRelative(nPos);
	return pVar;
}

const char* DebugStackManager::GetDisplayParamName(int nParam)
{
	if(nParam < 0 || nParam >= m_nDisplayParamCount)
		return "<out of range>";
	return m_pDisplayParamNames[nParam];
}

EVar* DebugStackManager::GetDisplayParamValue(int nParam)
{
	if(nParam < 0 || nParam >= m_nDisplayParamCount)
		return NULL;
	int nPos = m_nDisplayLayerStackTop - (m_nDisplayVarCount + m_nDisplayParamCount + 3) + nParam;
	if(nPos < 0)
		return NULL;
	EVar* pVar = m_pVM->GetVariableFromStackNotRelative(nPos);
	return pVar;
}
