#ifndef __LPS_H__
#define __LPS_H__

class GPointerArray;
class LPSTask;
class LPSSubstantiveProcess;

// A tube must be at least this full of successful evidence before
// we will consider a learner to have achieved the attainment
#define ATTAINMENT_EVIDENCE_THRESHOLD .85



// Represents all that is known about the learner's thinking abilities
class LPSLearnerModel
{
protected:
	GPointerArray* m_pSubstantiveProcesses;

public:
	LPSLearnerModel();
	~LPSLearnerModel();

	void PushData(const char* szSubstantiveProcess, double dAbilityLevel, double dEvidence, bool bSuccessful);
	int GetSubstantiveProcessCount();
	LPSSubstantiveProcess* GetSubstantiveProcess(int n);
};



// Represents a specific thinking ability
class LPSSubstantiveProcess
{
protected:
	GPointerArray* m_pAttainments;
	char* m_szName;

public:
	LPSSubstantiveProcess(const char* szName);
	~LPSSubstantiveProcess();

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
	LPSSubstantiveProcess* m_pSubstantiveProcess; // The substantive process on which this attainment lies
	LPSEvidence* m_pTube; // Points to the head of a linked list of evidence about whether or not the learner has attained to this ability level

public:
	LPSAttainment(LPSSubstantiveProcess* pSubstantiveProcess, double dAbilityLevel);
	~LPSAttainment();

	LPSSubstantiveProcess* GetSubstantiveProcess() { return m_pSubstantiveProcess; }
	double GetAbilityLevel() { return m_dAbilityLevel; }
	void PushEvidence(double dEvidence, bool bSuccessful);
	double GetTotalSuccessfulEvidence();
};



#endif // __LPS_H__
