#include "GBlindGreedy.h"
#include "GBits.h"
#include <math.h>
#include <stdio.h>

GBlindGreedySearch::GBlindGreedySearch(GSearchCritic* pCritic, double dMin, double dRange)
: GSearch(pCritic)
{
	m_dRange = dRange;
	m_dConservativeness = 4;
	m_nDimensions = pCritic->GetVectorSize();
	m_pVector = new double[m_nDimensions];
	m_pTest = new double[m_nDimensions];
	int i;
	for(i = 0; i < m_nDimensions; i++)
		m_pVector[i] = GBits::GetRandomDouble() * dRange + dMin;
	m_dError = pCritic->Critique(m_pVector);
}

/*virtual*/ GBlindGreedySearch::~GBlindGreedySearch()
{
	delete(m_pVector);
	delete(m_pTest);
}

/*virtual*/ void GBlindGreedySearch::Iterate()
{
	// Pick a new spot to try
	int i;
	for(i = 0; i < m_nDimensions; i++)
	{
		if(rand() & 1)
			m_pTest[i] = m_pVector[i] + (m_dRange * pow(GBits::GetRandomDouble(), m_dConservativeness));
		else
			m_pTest[i] = m_pVector[i] - (m_dRange * pow(GBits::GetRandomDouble(), m_dConservativeness));
	}

	// Critique the current spots and find the global best
	double dError = m_pCritic->Critique(m_pTest);
	if(dError < m_dError)
	{
		double* pTmp = m_pTest;
		m_pTest = m_pVector;
		m_pVector = pTmp;
		m_dError = dError;
	}
}
