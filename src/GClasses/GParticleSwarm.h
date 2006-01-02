#ifndef __GPARTICLESWARM_H__
#define __GPARTICLESWARM_H__

#include "GSearch.h"

class GParticleSwarm : public GSearch
{
protected:
	double m_dLearningRate;
	int m_nDimensions;
	int m_nPopulation;
	double* m_pPositions;
	double* m_pVelocities;
	double* m_pBests;
	double* m_pErrors;
	int m_nGlobalBest;

public:
	GParticleSwarm(GSearchCritic* pCritic, int nPopulation, double dMin, double dRange);
	virtual ~GParticleSwarm();

	virtual void Iterate();

	void SetLearningRate(double d) { m_dLearningRate = d; }
};

#endif // __GPARTICLESWARM_H__
