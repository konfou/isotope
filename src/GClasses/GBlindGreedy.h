#ifndef __GBLINDGREEDY_H__
#define __GBLINDGREEDY_H__

#include "GSearch.h"

// At each iteration this algorithm moves in a random direction
// from the best point ever found. The size of the step is a
// random number raised to a constant power (called the
// conservativeness).
class GBlindGreedySearch : public GSearch
{
protected:
	double m_dRange;
	double m_dConservativeness;
	int m_nDimensions;
	double* m_pVector;
	double* m_pTest;
	double m_dError;

public:
	GBlindGreedySearch(GSearchCritic* pCritic, double dMin, double dRange);
	virtual ~GBlindGreedySearch();

	virtual void Iterate();

	// d should be greater than 1. A bigger value will cause it to usually
	// take small steps and only rarely take big steps. A smaller value
	// will cause it to take big steps more frequently.
	void SetConservativeness(double d) { m_dConservativeness = d; }
};

#endif // __GBLINDGREEDY_H__
