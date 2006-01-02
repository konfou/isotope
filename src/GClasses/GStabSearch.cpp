#include "GStabSearch.h"
#include "GBits.h"
#include <math.h>
#include <stdio.h>

GStabSearch::GStabSearch(GSearchCritic* pCritic, double dMin, double dRange)
: GSearch(pCritic)
{
	m_nMask[0] = 0;
	m_nMask[1] = 0;
	m_nMask[2] = 0;
	m_nMask[3] = 0;
	m_dMin = dMin;
	m_dRange = dRange;
	m_dTolerance = .000001;
	m_nDimensions = pCritic->GetVectorSize();
	m_pMins = new double[m_nDimensions];
	m_pRanges = new double[m_nDimensions];
	m_pVector = new double[m_nDimensions];
}

/*virtual*/ GStabSearch::~GStabSearch()
{
	delete(m_pMins);
	delete(m_pRanges);
	delete(m_pVector);
}

/*virtual*/ void GStabSearch::Iterate()
{
	// Start at the global scope
	int i;
	for(i = 0; i < m_nDimensions; i++)
	{
		m_pMins[i] = m_dMin;
		m_pRanges[i] = m_dRange;
	}

	// Do a binary-search stab
	int nDim = 0;
	int nMaskPos = 0;
	double dError1, dError2;
	while(true)
	{
		for(i = 0; i < m_nDimensions; i++)
			m_pVector[i] = m_pMins[i] + .5 * m_pRanges[i];
		m_pVector[nDim] = m_pMins[nDim] + .25 * m_pRanges[nDim];
		dError1 = m_pCritic->Critique(m_pVector);
		m_pVector[nDim] = m_pMins[nDim] + .75 * m_pRanges[nDim];
		dError2 = m_pCritic->Critique(m_pVector);
		if(m_nMask[nMaskPos / 32] & (1 << (nMaskPos % 32)))
		{
			if(dError2 >= dError1)
				m_pMins[nDim] += .5 * m_pRanges[nDim];
		}
		else
		{
			if(dError2 < dError1)
				m_pMins[nDim] += .5 * m_pRanges[nDim];
		}
		m_pRanges[nDim] *= .5;
		if(++nDim >= m_nDimensions)
			nDim = 0;
		if(m_pRanges[nDim] < m_dTolerance)
			break;
		if(++nMaskPos >= 128)
			nMaskPos = 0;
	}

	// Increment the mask
	int n = 0;
	while(++m_nMask[n] == 0)
		n++;
}
