/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __LPS_H__
#define __LPS_H__

class GPointerArray;
class LPSTask;
class LPSThinkingSkill;

// A tube must be at least this full of successful evidence before
// we will consider a learner to have achieved the attainment
#define ATTAINMENT_EVIDENCE_THRESHOLD .85



// Represents all that is known about the learner's thinking abilities
class LPSLearnerModel
{
protected:
	GPointerArray* m_pSkills;

public:
	LPSLearnerModel();
	~LPSLearnerModel();

	void PushData(const char* szThinkingSkill, double dAbilityLevel, double dEvidence, bool bSuccessful);
	int GetThinkingSkillCount();
	LPSThinkingSkill* GetThinkingSkill(int n);
};



// Represents a specific thinking skill
class LPSThinkingSkill
{
protected:
	GPointerArray* m_pAttainments;
	char* m_szName;

public:
	LPSThinkingSkill(const char* szName);
	~LPSThinkingSkill();

	void PushEvidence(double dAbilityLevel, double dEvidence, bool bSuccessful);
	double CalculateLearnerPosition();
	const char* GetName() { return m_szName; }
};



// Represents an indication of whether or not the learner has achieved some attainment
class LPSEvidence
{
friend class LPSAttainment;
protected:
	bool m_bSuccessful; // Whether or not the learner was successful at the attempt
	double m_dAmount; // How much evidence this represents (from 0 to 1)
	LPSEvidence* m_pNext; // The next older piece of evidence in the tube

public:
	LPSEvidence(bool bSuccessful, double dAmount);
	~LPSEvidence();
};



// Represents a difficulty/ability level on a specific substantive process.  A learner
// is determined to have reached an attainment when there exists sufficient evidence
// that the learner can consistently solve problems that require  use of that thinking
// ability with at least this level of difficulty/ability.
class LPSAttainment
{
protected:
	double m_dAbilityLevel; // The location (from 0 to 1) along the substantive process pathway
	LPSThinkingSkill* m_pSkill; // The substantive process on which this attainment lies
	LPSEvidence* m_pTube; // Points to the head of a linked list of evidence about whether or not the learner has attained to this ability level

public:
	LPSAttainment(LPSThinkingSkill* pSkill, double dAbilityLevel);
	~LPSAttainment();

	LPSThinkingSkill* GetThinkingSkill() { return m_pSkill; }
	double GetAbilityLevel() { return m_dAbilityLevel; }
	void PushEvidence(double dEvidence, bool bSuccessful);
	double GetTotalSuccessfulEvidence();
};



#endif // __LPS_H__
