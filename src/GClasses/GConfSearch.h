#ifndef __GCONFSEARCH_H__
#define __GCONFSEARCH_H__

#include "GSearch.h"

class GStringHeap;
class GAVLTree;
class GMatrix;

// This is still an experimental algorithm. It's designed to search complex
// hypersurfaces by optimizing between exploration and exploitation using
// the intersection of Gaussians extending from known points as a guide
class GConfidenceSearch : public GSearch
{
protected:
	int m_nDimensions;
	GStringHeap* m_pHeap;
	GAVLTree* m_pPriorityQueue;
	GMatrix* m_pMatrix;
	double m_dThoroughness;

public:
	GConfidenceSearch(GSearchCritic* pCritic, double dMin, double dRange, double dThoroughness);
	~GConfidenceSearch();

	void Iterate();

protected:
	double* AddPoint(double* pPoint);
	double MeasureDistanceSquared(double* pPoint1, double* pPoint2);
	void AddSearchArea(double** ppNeighbors);
	double ComputeOptimisticSearchPointError(double* pSearchPoint, double* pNeighbor);
};

#endif // __GCONFSEARCH_H__
