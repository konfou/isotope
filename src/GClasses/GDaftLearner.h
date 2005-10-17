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
class GImage;
class GArffRelation;
class GArffData;
class GNeuralNet;
class GPointerQueue;

// Uses the Divide And Fit Technique for learning
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




class GSquisher
{
protected:
	int m_nDataPoints;
	int m_nDimensions;
	int m_nNeighbors;
	int m_nDataIndex;
	int m_nValueIndex;
	int m_nRecordSize;
	int m_nCurrentDimension;
	int m_nTargetDimensions;
	int m_nPass;
	unsigned char* m_pData;
	double m_dAveNeighborDist;
	double m_dSquishingRate;
	double m_dLearningRate;
	GPointerQueue* m_pQ;

public:
	GSquisher(int nDataPoints, int nDimensions, int nNeighbors);
	~GSquisher();

	void SquishBegin(int nTargetDimensions);
	void SquishPass();

	void SetDataPoint(int n, double* pValues);
	double* GetDataPoint(int n);
	
	// for internal use only
	int DataPointSortCompare(unsigned char* pA, unsigned char* pB);

protected:
	void CalculateMetadata(int nTargetDimensions);
	int FindMostDistantNeighbor(struct GSquisherNeighbor* pNeighbors);
	double CalculateDistance(unsigned char* pA, unsigned char* pB);
	double CalculateVectorCorrelation(unsigned char* pA, unsigned char* pVertex, unsigned char* pB);
	double CalculateDataPointError(unsigned char* pDataPoint);
	int AjustDataPoint(unsigned char* pDataPoint, int nTargetDimensions);
};


#endif // __GDAFTLEARNER_H__
