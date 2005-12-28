#include "LPS.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GXML.h"


LPSLearnerModel::LPSLearnerModel()
{
	m_pSubstantiveProcesses = new GPointerArray(16);
}

LPSLearnerModel::~LPSLearnerModel()
{
	int nCount = m_pSubstantiveProcesses->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((LPSSubstantiveProcess*)m_pSubstantiveProcesses->GetPointer(n));
	delete(m_pSubstantiveProcesses);
}

int LPSLearnerModel::GetSubstantiveProcessCount()
{
	return m_pSubstantiveProcesses->GetSize();
}

LPSSubstantiveProcess* LPSLearnerModel::GetSubstantiveProcess(int n)
{
	return (LPSSubstantiveProcess*)m_pSubstantiveProcesses->GetPointer(n);
}

void LPSLearnerModel::PushData(const char* szSubstantiveProcess, double dAbilityLevel, double dEvidence, bool bSuccessful)
{
	int nCount = m_pSubstantiveProcesses->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
	{
		LPSSubstantiveProcess* pSubstantiveProcess = (LPSSubstantiveProcess*)m_pSubstantiveProcesses->GetPointer(n);
		if(stricmp(pSubstantiveProcess->GetName(), szSubstantiveProcess) == 0)
		{
			pSubstantiveProcess->PushEvidence(dAbilityLevel, dEvidence, bSuccessful);
			break;
		}
	}
}

// --------------------------------------------------------------------------------------------



LPSSubstantiveProcess::LPSSubstantiveProcess(const char* szName)
{
	m_pAttainments = new GPointerArray(32);
	m_szName = new char[strlen(szName) + 1];
	strcpy(m_szName, szName);
}

LPSSubstantiveProcess::~LPSSubstantiveProcess()
{
	int nCount = m_pAttainments->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((LPSAttainment*)m_pAttainments->GetPointer(n));
	delete(m_pAttainments);
	delete(m_szName);
}

void LPSSubstantiveProcess::PushEvidence(double dAbilityLevel, double dEvidence, bool bSuccessful)
{
	int nCount = m_pAttainments->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
	{
		LPSAttainment* pAttainment = (LPSAttainment*)m_pAttainments->GetPointer(n);
		if(pAttainment->GetAbilityLevel() <= dAbilityLevel)
			pAttainment->PushEvidence(dEvidence, bSuccessful);
		else
			break;
	}
}

double LPSSubstantiveProcess::CalculateLearnerPosition()
{
	int nCount = m_pAttainments->GetSize();
	int n;
	double dPrevPos = 0;
	for(n = 0; n < nCount; n++)
	{
		LPSAttainment* pAttainment = (LPSAttainment*)m_pAttainments->GetPointer(n);
		double dEvidence = pAttainment->GetTotalSuccessfulEvidence();
		if(dEvidence < ATTAINMENT_EVIDENCE_THRESHOLD)
			return dPrevPos + (pAttainment->GetAbilityLevel() - dPrevPos) * (dEvidence / ATTAINMENT_EVIDENCE_THRESHOLD);
		dPrevPos = pAttainment->GetAbilityLevel();
	}
	return 1;
}


// --------------------------------------------------------------------------------------------





LPSEvidence::LPSEvidence(bool bSuccessful, double dAmount)
{
	m_bSuccessful = bSuccessful;
	m_dAmount = dAmount;
	m_pNext = NULL;
}

LPSEvidence::~LPSEvidence()
{
	delete(m_pNext);
}



// --------------------------------------------------------------------------------------------



LPSAttainment::LPSAttainment(LPSSubstantiveProcess* pSubstantiveProcess, double dAbilityLevel)
{
	m_pSubstantiveProcess = pSubstantiveProcess;
	m_dAbilityLevel = dAbilityLevel;
	m_pTube = NULL;
}

LPSAttainment::~LPSAttainment()
{
	delete(m_pTube);
}

void LPSAttainment::PushEvidence(double dEvidence, bool bSuccessful)
{
	// Add new evidence to the tube
	LPSEvidence* pEvidence = new LPSEvidence(bSuccessful, dEvidence);
	pEvidence->m_pNext = m_pTube;
	m_pTube = pEvidence;

	// Pop any excessive evidence out the end of the tube
	double dSum = 0;
	while(pEvidence)
	{
		if(dSum + pEvidence->m_dAmount >= 1)
		{
			delete(pEvidence->m_pNext);
			pEvidence->m_pNext = NULL;
			pEvidence->m_dAmount = 1 - dSum;
			break;
		}
		dSum += pEvidence->m_dAmount;
		pEvidence = pEvidence->m_pNext;
	}
}

double LPSAttainment::GetTotalSuccessfulEvidence()
{
	double dSum = 0;
	LPSEvidence* pEvidence = m_pTube;
	while(pEvidence)
	{
		if(pEvidence->m_bSuccessful)
			dSum += pEvidence->m_dAmount;
	}
	return dSum;
}


