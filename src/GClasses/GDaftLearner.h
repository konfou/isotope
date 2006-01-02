/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GDAFTLEARNER_H__
#define __GDAFTLEARNER_H__

class GDaftNode;
class GArffRelation;
class GArffData;

class GStringHeap;
class GAVLTree;
class GMatrix;

// Uses the Divide And Fit Technique for learning--this is still an experimental algorithm
class GDaftLearner
{
protected:
	GArffRelation* m_pRelation;
	GArffRelation* m_pDivideRelation;
	GDaftNode* m_pRoot;
	GArffData* m_pTrainingData;
	double m_dAcceptableError;
	double m_dGoodDivideWeight;

public:
	GDaftLearner(GArffRelation* pRelation);
	~GDaftLearner();

	void SetAcceptableError(double d) { m_dAcceptableError = d; }
	void SetGoodDivideWeight(double d) { m_dGoodDivideWeight = d; }
	void Train(GArffData* pData);
	void Eval(double* pRow);

protected:
	GDaftNode* BuildBranch(GArffData* pData, double dScale, int nDepth);
	double FindBestDivision(GArffData* pData, int nDepth);
	void DivideData(double dDivide, GArffData* pData, GArffData* pPositive, GArffData* pNegative, int nDepth);
	double EvalHelper(double* pRow, GDaftNode* pNode, double* pSample, int nDepth);
};


#endif // __GDAFTLEARNER_H__
