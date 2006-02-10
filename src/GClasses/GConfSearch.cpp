#include "GConfSearch.h"
#include "GHashTable.h"
#include "GAVLTree.h"
#include "GMatrix.h"
#include "GMacros.h"
#include <math.h>

class GConfidenceSearchSpot : public GAVLNode
{
protected:
	double* m_pPoint;
	double** m_pNeighbors;

public:
	GConfidenceSearchSpot(double* pPoint, int nNeighbors, double** ppNeighbors)
	{
		m_pPoint = pPoint;
		m_pNeighbors = new double*[nNeighbors];
		int n;
		for(n = 0; n < nNeighbors; n++)
			m_pNeighbors[n] = ppNeighbors[n];
	}

	virtual ~GConfidenceSearchSpot()
	{
		delete(m_pNeighbors);
	}

	virtual int Compare(GAVLNode* pThat)
	{
		GConfidenceSearchSpot* pOther = (GConfidenceSearchSpot*)pThat;
		if(m_pPoint[0] > pOther->m_pPoint[0])
			return 1;
		else if(m_pPoint[0] < pOther->m_pPoint[0])
			return -1;
		else
			return 0;
	}

	double* GetPoint() { return m_pPoint; }
	double** GetNeighbors() { return m_pNeighbors; }
};

GConfidenceSearch::GConfidenceSearch(GSearchCritic* pCritic, double dMin, double dRange, double dThoroughness)
: GSearch(pCritic)
{
	m_nDimensions = pCritic->GetVectorSize();
	m_pHeap = new GStringHeap(4096);
	m_pPriorityQueue = new GAVLTree();
	m_dThoroughness = dThoroughness;
	m_pMatrix = new GMatrix(m_nDimensions + 1, m_nDimensions + 1);

	// Make the initial neighbor points
	double* pPoint = (double*)alloca(sizeof(double) * (m_nDimensions + 1));
	double** ppNeighbors = (double**)alloca(sizeof(double*) * (m_nDimensions + 1));
	int n;
	for(n = 0; n < m_nDimensions; n++)
		pPoint[n + 1] = dMin;
	pPoint[0] = m_pCritic->Critique(&pPoint[1]);
	ppNeighbors[0] = AddPoint(pPoint);
	for(n = 0; n < m_nDimensions; n++)
	{
		pPoint[n + 1] = dMin + dRange * m_nDimensions;
		pPoint[0] = m_pCritic->Critique(&pPoint[1]);
		ppNeighbors[n + 1] = AddPoint(pPoint);
		pPoint[n + 1] = dMin;
	}

	// Make the initial search point
	AddSearchArea(ppNeighbors);
}

GConfidenceSearch::~GConfidenceSearch()
{
	delete(m_pHeap);
	delete(m_pPriorityQueue);
	delete(m_pMatrix);
}

double* GConfidenceSearch::AddPoint(double* pPoint)
{
	double* pNewPoint = (double*)m_pHeap->Allocate(sizeof(double) * (m_nDimensions + 1));
	memcpy(pNewPoint, pPoint, sizeof(double) * (m_nDimensions + 1));
	return pNewPoint;
}

double GConfidenceSearch::MeasureDistanceSquared(double* pPoint1, double* pPoint2)
{
	int n;
	double d;
	double dSum = 0;
	for(n = 0; n < m_nDimensions; n++)
	{
		d = pPoint1[n + 1] - pPoint2[n + 1];
		dSum += (d * d);
	}
	return dSum;
}

void GConfidenceSearch::AddSearchArea(double** ppNeighbors)
{
	// Make sure the thoroughness value isn't too small
	double d, distSquared;
	int row, col;
	double dFac = m_dThoroughness * m_dThoroughness;
	for(row = 0; row <= m_nDimensions; row++)
	{
		for(col = 0; col <= m_nDimensions; col++)
		{
			if(col == row)
				continue;
			if(ppNeighbors[row][0] < ppNeighbors[col][0])
				continue;
			distSquared = MeasureDistanceSquared(ppNeighbors[row], ppNeighbors[col]);
			d = ppNeighbors[row][0] * exp(dFac * distSquared / (-2));
			if(d <= ppNeighbors[col][0])
				continue;
			dFac = 2 * log(2 * ppNeighbors[row][0] / ppNeighbors[col][0]) / distSquared;
			m_dThoroughness = sqrt(dFac);
// todo: remove this check
GAssert(ppNeighbors[row][0] * exp(dFac * distSquared / (-2)) < ppNeighbors[col][0] * .6, "didn't work");
printf("###### Thoroughness-> %f\n", m_dThoroughness);
		}
	}

	// Find the intersection point of (m_nDimensions + 1) Gaussians centered and scaled to each neighbor
	double* pIntersectPoint = (double*)m_pHeap->Allocate(sizeof(double) * (m_nDimensions + 1));
	double* pVector = (double*)alloca(sizeof(double) * (m_nDimensions + 1));
	for(row = 0; row <= m_nDimensions; row++)
		pVector[row] = 0;
	double* pNeighbor;
	double dVal;
	for(row = 0; row <= m_nDimensions; row++)
	{
		dVal = 0;
		pNeighbor = ppNeighbors[row];
		for(col = 0; col < m_nDimensions; col++)
		{
			m_pMatrix->Set(row, col, dFac * pNeighbor[1 + col]);
			dVal += (pNeighbor[1 + col] * pNeighbor[1 + col]);
		}
		m_pMatrix->Set(row, m_nDimensions, -1);
		pVector[row] = (dFac * dVal / 2) - log(pNeighbor[0]);
	}
	m_pMatrix->Solve(pVector);
	memcpy(&pIntersectPoint[1], pVector, sizeof(double) * m_nDimensions);

	// Compute the optimistic error
	pIntersectPoint[0] = ComputeOptimisticSearchPointError(pIntersectPoint, ppNeighbors[0]);

	// todo: remove this check
#ifdef _DEBUG
	if(m_nDimensions > 1)
	{
		double dVal2 = ComputeOptimisticSearchPointError(pIntersectPoint, ppNeighbors[1]);
		GAssert(dVal2 - pIntersectPoint[0] < .001 && dVal2 - pIntersectPoint[0] > -.001, "point is not true intersection of Gaussians");
		if(m_nDimensions > 2)
		{
			double dVal3 = ComputeOptimisticSearchPointError(pIntersectPoint, ppNeighbors[2]);
			GAssert(dVal3 - pIntersectPoint[0] < .001 && dVal3 - pIntersectPoint[0] > -.001, "point is not true intersection of Gaussians");
		}
	}
#endif // _DEBUG

	// Add a new search spot to the priority queue
printf("(%f, %f)->%f    OptimisticError=%f\n", ppNeighbors[0][1], ppNeighbors[1][1], pIntersectPoint[1], pIntersectPoint[0]);
	GConfidenceSearchSpot* pSearchSpot = new GConfidenceSearchSpot(pIntersectPoint, m_nDimensions + 1, ppNeighbors);
	m_pPriorityQueue->Insert(pSearchSpot);
}

double GConfidenceSearch::ComputeOptimisticSearchPointError(double* pSearchPoint, double* pNeighbor)
{
	// find the square of the distance between the two points
	double dSum = 0;
	double d;
	int n;
	for(n = 1; n <= m_nDimensions; n++)
	{
		d = pSearchPoint[n] - pNeighbor[n];
		dSum += (d * d);
	}

	// Compute the position on the Gaussian
	return pNeighbor[0] * exp((m_dThoroughness * m_dThoroughness * dSum) / (-2));
}

void GConfidenceSearch::Iterate()
{
	// Get the next spot from the priority queue
	GConfidenceSearchSpot* pSpot = (GConfidenceSearchSpot*)m_pPriorityQueue->Unlink(0);
	GAssert(pSpot, "the queue should never be empty");

	// Ask the critic for the actual error of this spot
	double* pPoint = pSpot->GetPoint();
	pPoint[0] = m_pCritic->Critique(&pPoint[1]);

	// Push the new search spots into the queue
	double* pOldNeighbor;
	double** ppNeighbors = pSpot->GetNeighbors();
	int n;
	for(n = 0; n <= m_nDimensions; n++)
	{
		pOldNeighbor = ppNeighbors[n];
		ppNeighbors[n] = pPoint;
		AddSearchArea(ppNeighbors);
		ppNeighbors[n] = pOldNeighbor;
	}
	delete(pSpot);
}
