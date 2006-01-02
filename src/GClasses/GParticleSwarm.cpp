#include "GParticleSwarm.h"
#include "GBits.h"
#include <string.h>

GParticleSwarm::GParticleSwarm(GSearchCritic* pCritic, int nPopulation, double dMin, double dRange)
: GSearch(pCritic)
{
	m_dLearningRate = .2;
	m_nDimensions = pCritic->GetVectorSize();
	m_nPopulation = nPopulation;
	m_pPositions = new double[m_nPopulation * m_nDimensions];
	m_pVelocities = new double[m_nPopulation * m_nDimensions];
	m_pBests = new double[m_nPopulation * m_nDimensions];
	m_pErrors = new double[m_nPopulation];
	int n, i;
	for(i = 0; i < m_nPopulation; i++)
	{
		for(n = 0; n < m_nDimensions; n++)
		{
			m_pPositions[m_nDimensions * i + n] = GBits::GetRandomDouble() * dRange + dMin;
			m_pVelocities[m_nDimensions * i + n] = GBits::GetRandomDouble() * dRange + dMin;
			m_pBests[m_nDimensions * i + n] = m_pPositions[m_nDimensions * i + n];
		}
		m_pErrors[i] = 1e100;
	}
	m_nGlobalBest = 0;
}

/*virtual*/ GParticleSwarm::~GParticleSwarm()
{
	delete(m_pErrors);
	delete(m_pBests);
	delete(m_pVelocities);
	delete(m_pPositions);
}

/*virtual*/ void GParticleSwarm::Iterate()
{
	// Advance
	int i, j, nPos;
	int n = m_nPopulation * m_nDimensions;
	for(i = 0; i < n; i++)
		m_pPositions[i] += m_pVelocities[i];

	// Critique the current spots and find the global best
	double dError;
	double dGlobalBest = 1e100;
	for(i = 0; i < m_nPopulation; i++)
	{
		nPos = m_nDimensions * i;
		dError = m_pCritic->Critique(&m_pPositions[nPos]);
		if(dError < m_pErrors[i])
		{
			m_pErrors[i] = dError;
			memcpy(&m_pBests[nPos], &m_pPositions[nPos], sizeof(double) * m_nDimensions);
		}
		if(m_pErrors[i] < dGlobalBest)
		{
			dGlobalBest = m_pErrors[i];
			m_nGlobalBest = i;
		}
	}

	// Update velocities
	nPos = 0;
	n = m_nDimensions * m_nGlobalBest;
	for(i = 0; i < m_nPopulation; i++)
	{
		for(j = 0; j < m_nDimensions; j++)
		{
			m_pVelocities[nPos + j] += m_dLearningRate * GBits::GetRandomDouble() * (m_pBests[nPos + j] - m_pPositions[nPos + j]) + m_dLearningRate * GBits::GetRandomDouble() * (m_pPositions[n + j] - m_pPositions[nPos + j]);
		}
		nPos += m_nDimensions;
	}
}
