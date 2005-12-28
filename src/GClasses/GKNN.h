#ifndef __GKNN_H__
#define __GKNN_H__

#include "GLearner.h"

class GPointerArray;

// Implements the K-Nearest Neighbor learning algorithm
class GKNN : public GSupervisedLearner
{
protected:
	int m_nNeighbors;
	GPointerArray* m_pRows;
	double* m_pScaleFactors;

public:
	GKNN(GArffRelation* pRelation, int nNeighbors);
	virtual ~GKNN();

	// Adds a point to the collection
	void AddRow(double* pRow);

	// Recompute the amount to scale each dimension so that all dimensions
	// have equal weight
	void RecomputeScaleFactors();

	// Add all the points in pData to the collection and recompute the
	// scale factors
	virtual void Train(GArffData* pData);

	// Evaluate with each neighbor having equal vote
	void EvalEqualWeight(double* pRow);

	// Evaluate with each neighbor having a linear vote
	void EvalLinearWeight(double* pRow);

	// Evaluates the input values in the provided row and
	// deduce the output values
	virtual void Eval(double* pRow) { EvalLinearWeight(pRow); }

	// Find the row that helps the lest with predictive accuracy (very expensive)
	int FindLeastHelpfulRow(GArffData* pTestSet);

	// Drops a point from the collection
	double* DropRow(int nRow);

	// Measure predictive accuracy against a test set
	double MeasureError(GArffData* pTestSet);

protected:
	void FindNeighbors(int i, double* pRow, int* pNeighbors);
};

#endif // __GKNN_H__
