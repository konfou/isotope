#ifndef __GCAUTIOUSGREEDY_H__
#define __GCAUTIOUSGREEDY_H__

#include "GSearch.h"

// At each iteration this algorithm moves in only one
// dimension. If the situation doesn't improve it tries
// the opposite direction. If both directions are worse,
// it decreases the step size for that dimension, otherwise
// it increases the step size for that dimension.
class GCautiousGreedySearch : public GSearch
{
protected:
	int m_nDimensions;
	int m_nCurrentDim;
	double* m_pStepSizes;
	double* m_pVector;
	double m_dError;
	double m_dChangeFactor;

public:
	GCautiousGreedySearch(GSearchCritic* pCritic);
	virtual ~GCautiousGreedySearch();

	virtual void Iterate();

	// d should be a value between 0 and 1
	void SetChangeFactor(double d) { m_dChangeFactor = d; }
};

#endif // __GCAUTIOUSGREEDY_H__
