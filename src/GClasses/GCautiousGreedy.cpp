#include "GCautiousGreedy.h"
#include "GBits.h"
#include <math.h>
#include <stdio.h>

GCautiousGreedySearch::GCautiousGreedySearch(GSearchCritic* pCritic)
: GSearch(pCritic)
{
	m_nDimensions = pCritic->GetVectorSize();
	m_nCurrentDim = 0;
	m_pVector = new double[m_nDimensions];
	m_pStepSizes = new double[m_nDimensions];
	m_dChangeFactor = .93;
	int i;
	for(i = 0; i < m_nDimensions; i++)
	{
		m_pVector[i] = 0;
		m_pStepSizes[i] = 1;
	}
	m_dError = pCritic->Critique(m_pVector);
}

/*virtual*/ GCautiousGreedySearch::~GCautiousGreedySearch()
{
	delete(m_pVector);
	delete(m_pStepSizes);
}

/*virtual*/ void GCautiousGreedySearch::Iterate()
{
	m_pVector[m_nCurrentDim] += m_pStepSizes[m_nCurrentDim];
	double dError = m_pCritic->Critique(m_pVector);
	if(dError >= m_dError)
	{
		m_pVector[m_nCurrentDim] -= m_pStepSizes[m_nCurrentDim];
		m_pVector[m_nCurrentDim] -= m_pStepSizes[m_nCurrentDim];
		dError = m_pCritic->Critique(m_pVector);
		if(dError >= m_dError)
			m_pVector[m_nCurrentDim] += m_pStepSizes[m_nCurrentDim];
	}
	if(dError >= m_dError)
		m_pStepSizes[m_nCurrentDim] *= m_dChangeFactor;
	else
	{
		m_pStepSizes[m_nCurrentDim] /= m_dChangeFactor;
		m_dError = dError;
	}
	if(++m_nCurrentDim >= m_nDimensions)
		m_nCurrentDim = 0;
}	

