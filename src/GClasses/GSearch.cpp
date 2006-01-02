#include "GSearch.h"
#include <string.h>

GSearchCritic::GSearchCritic(int nVectorSize)
{
	m_nVectorSize = nVectorSize;
	m_dBestError = 1e100;
	m_pBestYet = new double[nVectorSize];
	int i;
	for(i = 0; i < nVectorSize; i++)
		m_pBestYet[i] = 0;
}

GSearchCritic::~GSearchCritic()
{
	delete(m_pBestYet);
}

double GSearchCritic::Critique(double* pVector)
{
	double dError = ComputeError(pVector);
	if(dError < m_dBestError)
	{
		m_dBestError = dError;
		memcpy(m_pBestYet, pVector, sizeof(double) * m_nVectorSize);
	}
	return dError;
}

// -------------------------------------------------------

GSearch::GSearch(GSearchCritic* pCritic)
{
	m_pCritic = pCritic;
}

/*virtual*/ GSearch::~GSearch()
{
}
