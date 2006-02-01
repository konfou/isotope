/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "LPS.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GXML.h"


LPSLearnerModel::LPSLearnerModel()
{
	m_pSkills = new GPointerArray(16);
}

LPSLearnerModel::~LPSLearnerModel()
{
	int nCount = m_pSkills->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((LPSThinkingSkill*)m_pSkills->GetPointer(n));
	delete(m_pSkills);
}

int LPSLearnerModel::GetThinkingSkillCount()
{
	return m_pSkills->GetSize();
}

LPSThinkingSkill* LPSLearnerModel::GetThinkingSkill(int n)
{
	return (LPSThinkingSkill*)m_pSkills->GetPointer(n);
}

void LPSLearnerModel::PushData(const char* szThinkingSkill, double dAbilityLevel, double dEvidence, bool bSuccessful)
{
	int nCount = m_pSkills->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
	{
		LPSThinkingSkill* pSkill = (LPSThinkingSkill*)m_pSkills->GetPointer(n);
		if(stricmp(pSkill->GetName(), szThinkingSkill) == 0)
		{
			pSkill->PushEvidence(dAbilityLevel, dEvidence, bSuccessful);
			break;
		}
	}
	if(n >= nCount)
		fprintf(stderr, "*** Unrecognized thinking skill: %s\n", szThinkingSkill);
}


// --------------------------------------------------------------------------------------------



LPSThinkingSkill::LPSThinkingSkill(const char* szName)
{
	m_pAttainments = new GPointerArray(32);
	m_szName = new char[strlen(szName) + 1];
	strcpy(m_szName, szName);
}

LPSThinkingSkill::~LPSThinkingSkill()
{
	int nCount = m_pAttainments->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((LPSAttainment*)m_pAttainments->GetPointer(n));
	delete(m_pAttainments);
	delete(m_szName);
}

void LPSThinkingSkill::PushEvidence(double dAbilityLevel, double dEvidence, bool bSuccessful)
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

double LPSThinkingSkill::CalculateLearnerPosition()
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



LPSAttainment::LPSAttainment(LPSThinkingSkill* pSkill, double dAbilityLevel)
{
	m_pSkill = pSkill;
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


