#ifndef __GSTABSEARCH_H__
#define __GSTABSEARCH_H__

#include "GSearch.h"

// Each iteration performs a binary (divde-and-conquer) search.
// Because the high-level divisions are typically less correlated
// with the final result than the low-level divisions, it searches
// through the space of possible "stabs" by toggling choices in
// the order from high level to low level.
class GStabSearch : public GSearch
{
protected:
	int m_nDimensions;
	unsigned int m_nMask[4];
	double m_dMin, m_dRange;
	double* m_pMins;
	double* m_pRanges;
	double* m_pVector;
	double m_dTolerance;

public:
	GStabSearch(GSearchCritic* pCritic, double dMin, double dRange);
	virtual ~GStabSearch();

	virtual void Iterate();
	void SetTolerance(double d) { m_dTolerance = d; }
};

#endif // __GSTABSEARCH_H__
