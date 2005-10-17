/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "stdio.h"
#include "GDaftLearner.h"
#include "GPolynomial.h"
#include "GMacros.h"
#include "GImage.h"
#include "GArff.h"
#include "GNeuralNet.h"
#include "GArray.h"

#include <math.h>
#include "GPointerQueue.h"

class GDaftNode
{
public:
	double m_dMin;
	double m_dRange;
	double m_dDivide;
	GNeuralNet* m_pNNFit;
	GDaftNode* m_pNegative;
	GDaftNode* m_pPositive;
bool m_bUsed;

	GDaftNode(double dMin, double dRange, GNeuralNet* pNNFit)
	{
		m_dMin = dMin;
		m_dRange = dRange;
		m_dDivide = 0;
		m_pNNFit = pNNFit;
		m_pNegative = NULL;
		m_pPositive = NULL;
	}

	~GDaftNode()
	{
		delete(m_pNegative);
		delete(m_pPositive);
		delete(m_pNNFit);
	}
};



GDaftLearner::GDaftLearner(GArffRelation* pRelation)
{
	m_pRelation = pRelation;
	m_pRoot = NULL;
	m_pTrainingData = NULL;
	m_dAcceptableError = .01;
	m_dGoodDivideWeight = .02;
	GAssert(pRelation->GetOutputCount() == 1, "Currently only one output supported");

	// Create the divide relation
	int nInputCount = pRelation->GetInputCount();
	GAssert(nInputCount > 1, "GDaftLearner requires at least two inputs");
	m_pDivideRelation = new GArffRelation();
	int n;
	for(n = 0; n < nInputCount - 1; n++)
	{
		GArffAttribute* pAttr = pRelation->GetAttribute(pRelation->GetInputIndex(n));
		m_pDivideRelation->AddAttribute(pAttr->NewCopy());
	}
	GArffAttribute* pAttr = pRelation->GetAttribute(pRelation->GetInputIndex(nInputCount - 1))->NewCopy();
	pAttr->SetIsInput(false);
	m_pDivideRelation->AddAttribute(pAttr);
}

GDaftLearner::~GDaftLearner()
{
	delete(m_pRoot);
	delete(m_pDivideRelation);
}

void GDaftLearner::Train(GArffData* pData)
{
	m_pRoot = BuildBranch(pData, 1, 0);
}

void GDaftLearner::DivideData(double dDivide, GArffData* pData, GArffData* pPositive, GArffData* pNegative, int nDepth)
{
	int nAttr = nDepth % m_pRelation->GetInputCount();
	int nRowCount = pData->GetRowCount();
	int n;
	double* pRow;
	for(n = 0; n < nRowCount; n++)
	{
		pRow = pData->GetRow(n);
		if(pRow[nAttr] >= dDivide)
			pPositive->AddRow(pRow);
		else
			pNegative->AddRow(pRow);
	}
}

int DoublePointerComparer(void* pThis, void* pA, void* pB)
{
	double dA = *(double*)pA;
	double dB = *(double*)pB;
	if(dA > dB)
		return 1;
	else if(dA < dB)
		return -1;
	else
		return 0;
}

double GDaftLearner::FindBestDivision(GArffData* pData, int nDepth)
{
	GPointerArray tmp(pData->GetRowCount());
	int nAttr = nDepth % m_pRelation->GetInputCount();
	double* pRow;
	int n;
	for(n = pData->GetRowCount() - 1; n >= 0; n--)
	{
		pRow = pData->GetRow(n);
		tmp.AddPointer(&pRow[nAttr]);
	}
	tmp.Sort(DoublePointerComparer, NULL);
	double* pMedian = (double*)tmp.GetPointer(tmp.GetSize() / 2);
	return *pMedian;
}


/*
GNeuralNet* GDaftLearner::FitData(GArffData* pData, int nHiddenNodes, int nMaxIterations)
{
	GNeuralNet* pNNDivide = new GNeuralNet(m_pDivideRelation);
	pNNDivide->AddHiddenLayer(nHiddenNodes);
	pNNDivide->SetAcceptableMeanSquareError(.005);
	pNNDivide->SetLearningDecay(.999);
	pNNDivide->SetLearningRate(.3);
	pNNDivide->SetMaximumIterations(nMaxIterations);
	pNNDivide->SetMinimumIterations(350);
	pNNDivide->Train(pData, pData);
	return pNNDivide;
}

double DaftLearnerCriticFunc(void* pThis, GNeuralNet* pNeuralNet)
{
	return ((GDaftLearner*)pThis)->CriticizeDivision(pNeuralNet);
}

void GDaftLearner::DivideData(GNeuralNet* pNNDivide, GArffData* pData, GArffData* pPositive, GArffData* pNegative)
{
	int nInputCount = m_pRelation->GetInputCount();
	double* pSample = (double*)alloca(sizeof(double) * nInputCount);
	int nRowCount = pData->GetRowCount();
	int n, i;
	for(n = 0; n < nRowCount; n++)
	{
		double* pRow = pData->GetRow(n);
		for(i = 0; i < nInputCount - 1; i++)
			pSample[i] = pRow[m_pRelation->GetInputIndex(i)];
		pNNDivide->EvaluateTraining(pSample);
		if(pRow[m_pRelation->GetInputIndex(nInputCount - 1)] >= pSample[nInputCount - 1])
			pPositive->AddRow(pRow);
		else
			pNegative->AddRow(pRow);
	}
}

double GDaftLearner::CriticizeDivision(GNeuralNet* pNeuralNet)
{
	int nRowCount = m_pTrainingData->GetRowCount();
	GArffData pos(nRowCount);
	GArffData neg(nRowCount);
	DivideData(pNeuralNet, m_pTrainingData, &pos, &neg);
	int nPosCount = pos.GetRowCount();
	int nNegCount = neg.GetRowCount();
	if(nPosCount == 0 || nNegCount == 0)
	{
		pos.DropAllRows();
		neg.DropAllRows();
printf("[!]");
		return 1e10;
	}
	double dRatio = (nPosCount >= nNegCount ? nPosCount / nNegCount : nNegCount / nPosCount) - 1;
	Holder<GNeuralNet*> hPos(FitData(&pos, 2, 1000));
	Holder<GNeuralNet*> hNeg(FitData(&neg, 2, 1000));
	double dPos = hPos.Get()->GetMeanSquareError(m_pTrainingData);
	double dNeg = hNeg.Get()->GetMeanSquareError(m_pTrainingData);
	pos.DropAllRows();
	neg.DropAllRows();
	double dResult = dPos * dPos + dNeg * dNeg + dRatio * m_dGoodDivideWeight;
printf("[%f]", dResult);
	return dResult;
}

GNeuralNet* GDaftLearner::FindBestDivision(GArffData* pData)
{
	m_pTrainingData = pData;
	GNeuralNet* pNNDivide = new GNeuralNet(m_pDivideRelation);
	//pNNDivide->AddHiddenLayer(3);
	pNNDivide->SetAcceptableMeanSquareError(.001);
	pNNDivide->SetLearningDecay(.997);
	pNNDivide->SetLearningRate(6);
	pNNDivide->SetMaximumIterations(150000);
	pNNDivide->SetMinimumIterations(5000);



	//pNNDivide->TrainByCritic(DaftLearnerCriticFunc, this);
	GArffData tmp(3);
	int nAttributeCount = m_pDivideRelation->GetAttributeCount();
	GAssert(m_pRelation->GetInputCount() == nAttributeCount, "mismatch");
	int i;
	for(i = 0; i < 4; i++)
	{
		int nRandRow = rand() % pData->GetRowCount();
		double* pSrcRow = pData->GetRow(nRandRow);
		double* pNewRow = new double[nAttributeCount];
		int n;
		for(n = 0; n < nAttributeCount; n++)
			pNewRow[n] = pSrcRow[m_pRelation->GetInputIndex(n)];
		tmp.AddRow(pNewRow);
	}
	pNNDivide->Train(&tmp, &tmp);




	m_pTrainingData = NULL;
	return pNNDivide;
}
*/

#define NORMALIZE_MIN .1
#define NORMALIZE_RANGE .8

GDaftNode* GDaftLearner::BuildBranch(GArffData* pData, double dScale, int nDepth)
{
printf("Normalizing...\n");
	// Normalize the data
	int nOutputIndex = m_pRelation->GetOutputIndex(0);
	double dMin;
	double dRange;
	pData->GetMinAndRange(nOutputIndex, &dMin, &dRange);
	if(dRange < .00001)
		dRange = .00001;
	pData->Normalize(nOutputIndex, dMin, dRange, NORMALIZE_MIN, NORMALIZE_RANGE);
	int nRowCount = pData->GetRowCount();
GNeuralNet* pNNFit = NULL;
//if(pData->GetRowCount() < 64)
{
printf("Fitting...\n");
	// Fit the data
	pNNFit = new GNeuralNet(m_pRelation);
	pNNFit->AddLayer(4);
	pNNFit->AddLayer(2);
	pNNFit->SetAcceptableMeanSquareError(.0001);
	pNNFit->SetLearningRate(.25);
	pNNFit->SetHopefulness(.66);
	pNNFit->SetMaximumEpochs(50000);
	pNNFit->SetMinimumEpochs(16000);
	pNNFit->Train(pData, pData);

printf("Subtracting predicted results...\n");
	// Subtract the predicted results
	int nAttributeCount = m_pRelation->GetAttributeCount();
	int nInputs = m_pRelation->GetInputCount();
	double* pSample = (double*)alloca(sizeof(double) * nAttributeCount);
	double* pRow;
	int n;
	for(n = 0; n < nRowCount; n++)
	{
		pRow = pData->GetRow(n);
		memcpy(pSample, pRow, sizeof(double) * nAttributeCount);
		pNNFit->Eval(pSample);
		pRow[nOutputIndex] -= pSample[nOutputIndex];
	}
}
	// Make the node
	GDaftNode* pNode = new GDaftNode(dMin, dRange, pNNFit);
//pNode->m_bUsed = (pData->GetRowCount() < 64);
	pData->GetMinAndRange(nOutputIndex, &dMin, &dRange);
	double dNewScale = dScale * dRange / NORMALIZE_RANGE;
	if(dNewScale <= m_dAcceptableError || nRowCount < 12)
		return pNode;

printf("Dividing %d...", nRowCount);
	// Divide the data
	double dDivide = FindBestDivision(pData, nDepth);
	GArffData pos(nRowCount);
	GArffData neg(nRowCount);
	DivideData(dDivide, pData, &pos, &neg, nDepth);
	if(pos.GetRowCount() * 15 < neg.GetRowCount() || neg.GetRowCount() * 15 < pos.GetRowCount())
	{
		// Couldn't divide it, so bail out
		return pNode;
	}
	pNode->m_dDivide = dDivide;

printf("Recursing... (%d, %d)\n", pos.GetRowCount(), neg.GetRowCount());
	// Recurse
	pNode->m_pPositive = BuildBranch(&pos, dNewScale, nDepth + 1);
	pNode->m_pNegative = BuildBranch(&neg, dNewScale, nDepth + 1);
	pos.DropAllRows();
	neg.DropAllRows();
	return pNode;
}

void GDaftLearner::Eval(double* pRow)
{
	int nInputCount = m_pRelation->GetInputCount();
	double* pSample = (double*)alloca(sizeof(double) * nInputCount);
	int nOutputIndex = m_pRelation->GetOutputIndex(0);
	pRow[nOutputIndex] = EvalHelper(pRow, m_pRoot, pSample, 0);
}

double GDaftLearner::EvalHelper(double* pRow, GDaftNode* pNode, double* pSample, int nDepth)
{
	int nOutputIndex = m_pRelation->GetOutputIndex(0);
	double dVal;
	if(pNode->m_pPositive)
	{
		int nInputCount = m_pRelation->GetInputCount();
		if(pRow[m_pRelation->GetInputIndex(nDepth % nInputCount)] >= pNode->m_dDivide)
			dVal = EvalHelper(pRow, pNode->m_pPositive, pSample, nDepth + 1);
		else
			dVal = EvalHelper(pRow, pNode->m_pNegative, pSample, nDepth + 1);
	}
	else
		dVal = 0;
if(pNode->m_bUsed)
{
	pNode->m_pNNFit->Eval(pRow);
	dVal += pRow[nOutputIndex];
}
	return GArffData::Normalize(dVal, NORMALIZE_MIN, NORMALIZE_RANGE, pNode->m_dMin, pNode->m_dRange);
}


// --------------------------------------------------------------------------

struct GSquisherNeighbor
{
	unsigned char* m_pNeighbor;
	unsigned char* m_pNeighborsNeighbor;
	double m_dCosTheta;
	double m_dDistance;
};

struct GSquisherData
{
	int m_nCycle;
};

GSquisher::GSquisher(int nDataPoints, int nDimensions, int nNeighbors)
{
	m_nDataPoints = nDataPoints;
	m_nDimensions = nDimensions;
	m_nNeighbors = nNeighbors;
	m_nDataIndex = sizeof(struct GSquisherNeighbor) * m_nNeighbors;
	m_nValueIndex = m_nDataIndex + sizeof(struct GSquisherData);
	m_nRecordSize = m_nValueIndex + sizeof(double) * m_nDimensions;
	m_pData = new unsigned char[m_nRecordSize * nDataPoints];
	m_dSquishingRate = .95;
	m_nTargetDimensions = 0;
	m_nPass = 0;
	m_dAveNeighborDist = 0;
	m_pQ = NULL;
}

GSquisher::~GSquisher()
{
	delete(m_pData);
	delete(m_pQ);
}

void GSquisher::SetDataPoint(int n, double* pValues)
{
	memcpy(&m_pData[n * m_nRecordSize + m_nValueIndex], pValues, sizeof(double) * m_nDimensions);
}

double* GSquisher::GetDataPoint(int n)
{
	return (double*)&m_pData[n * m_nRecordSize + m_nValueIndex];
}

int GSquisher::DataPointSortCompare(unsigned char* pA, unsigned char* pB)
{
	pA += m_nValueIndex;
	pB += m_nValueIndex;
	double dA = ((double*)pA)[m_nCurrentDimension];
	double dB = ((double*)pB)[m_nCurrentDimension];
	if(dA > dB)
		return 1;
	else if(dA < dB)
		return -1;
	return 0;
}

int GSquisher_DataPointSortCompare(void* pThis, void* pA, void* pB)
{
	return ((GSquisher*)pThis)->DataPointSortCompare((unsigned char*)pA, (unsigned char*)pB);
}

int GSquisher::FindMostDistantNeighbor(struct GSquisherNeighbor* pNeighbors)
{
	int nMostDistant = 0;
	int n;
	for(n = 1; n < m_nNeighbors; n++)
	{
		if(pNeighbors[n].m_dDistance > pNeighbors[nMostDistant].m_dDistance)
			nMostDistant = n;
	}
	return nMostDistant;
}

double GSquisher::CalculateDistance(unsigned char* pA, unsigned char* pB)
{
	pA += m_nValueIndex;
	pB += m_nValueIndex;
	double* pdA = (double*)pA;
	double* pdB = (double*)pB;
	double dTmp;
	double dSum = 0;
	int n;
	for(n = 0; n < m_nDimensions; n++)
	{
		dTmp = pdB[n] - pdA[n];
		dSum += (dTmp * dTmp);
	}
	return sqrt(dSum);
}

double GSquisher::CalculateVectorCorrelation(unsigned char* pA, unsigned char* pVertex, unsigned char* pB)
{
	pA += m_nValueIndex;
	pVertex += m_nValueIndex;
	pB += m_nValueIndex;
	double* pdA = (double*)pA;
	double* pdV = (double*)pVertex;
	double* pdB = (double*)pB;
	double dDotProd = 0;
	double dMagA = 0;
	double dMagB = 0;
	double dA, dB;
	int n;
	for(n = 0; n < m_nDimensions; n++)
	{
		dA = pdA[n] - pdV[n];
		dB = pdB[n] - pdV[n];
		dDotProd += (dA * dB);
		dMagA += (dA * dA);
		dMagB += (dB * dB);
	}
	return dDotProd / (sqrt(dMagA) * sqrt(dMagB));
}

void GSquisher::CalculateMetadata(int nTargetDimensions)
{
	// Calculate how far we need to look to have a good chance of finding the
	// nearest neighbors
	int nDimSpan = (int)(pow((double)m_nDataPoints, (double)1 / nTargetDimensions) * 2);
	GAssert(nDimSpan > m_nNeighbors, "These numbers aren't going to work very well");

	// Find the nearest neighbors
	GPointerArray arr(m_nDataPoints);
	int n, i, j;
	for(n = 0; n < m_nDataPoints; n++)
		arr.AddPointer(&m_pData[n * m_nRecordSize]);

	// Initialize everybody's neighbors
	struct GSquisherNeighbor* pNeighbors;
	for(n = 0; n < m_nDataPoints; n++)
	{
		pNeighbors = (struct GSquisherNeighbor*)arr.GetPointer(n);
		for(i = 0; i < m_nNeighbors; i++)
		{
			pNeighbors[i].m_pNeighbor = NULL;
			pNeighbors[i].m_pNeighborsNeighbor = NULL;
			pNeighbors[i].m_dDistance = 1e100;
		}
	}

	// Find the nearest neighbors
	int nMostDistantNeighbor;
	int nStart, nEnd;
	double dDistance;
	unsigned char* pCandidate;
	for(m_nCurrentDimension = 0; m_nCurrentDimension < m_nDimensions; m_nCurrentDimension++)
	{
		// Sort on the current dimension
		arr.Sort(GSquisher_DataPointSortCompare, this);

		// Do a pass in this dimension for every data point to search for nearest neighbors
		for(n = 0; n < m_nDataPoints; n++)
		{
			// Check all the data points that are close in this dimension
			pNeighbors = (struct GSquisherNeighbor*)arr.GetPointer(n);
			nMostDistantNeighbor = FindMostDistantNeighbor(pNeighbors);
			nStart = MAX(0, n - nDimSpan);
			nEnd = MIN(m_nDataPoints - 1, n + nDimSpan);
			for(i = nStart; i <= nEnd; i++)
			{
				if(i == n)
					continue;
				pCandidate = (unsigned char*)arr.GetPointer(i);
				dDistance = CalculateDistance((unsigned char*)pNeighbors, pCandidate);
				if(dDistance < pNeighbors[nMostDistantNeighbor].m_dDistance)
				{
					// Check to see if this is already a neighbor
					for(j = 0; j < m_nNeighbors; j++)
					{
						if(pNeighbors[j].m_pNeighbor == pCandidate)
							break;
					}
					if(j == m_nNeighbors)
					{
						// Make this a neighbor
						pNeighbors[nMostDistantNeighbor].m_pNeighbor = pCandidate;
						pNeighbors[nMostDistantNeighbor].m_dDistance = dDistance;
						nMostDistantNeighbor = FindMostDistantNeighbor(pNeighbors);
					}
				}
			}
		}
	}

	// For each data point, find the most co-linear of each neighbor's neighbors
	m_dAveNeighborDist = 0;
	struct GSquisherData* pData;
	struct GSquisherNeighbor* pNeighborsNeighbors;
	double dCosTheta;
	for(n = 0; n < m_nDataPoints; n++)
	{
		pNeighbors = (struct GSquisherNeighbor*)arr.GetPointer(n);
		pData = (struct GSquisherData*)(((unsigned char*)pNeighbors) + m_nDataIndex);
		pData->m_nCycle = -1;
		for(i = 0; i < m_nNeighbors; i++)
		{
			m_dAveNeighborDist += pNeighbors[i].m_dDistance;
			pNeighborsNeighbors = (struct GSquisherNeighbor*)pNeighbors[i].m_pNeighbor;
			pCandidate = pNeighborsNeighbors[0].m_pNeighbor;
			dCosTheta = CalculateVectorCorrelation((unsigned char*)pNeighbors, (unsigned char*)pNeighborsNeighbors, pCandidate);
			pNeighbors[i].m_dCosTheta = dCosTheta;
			pNeighbors[i].m_pNeighborsNeighbor = pCandidate;
			for(j = 1; j < m_nNeighbors; j++)
			{
				pCandidate = pNeighborsNeighbors[j].m_pNeighbor;
				dCosTheta = CalculateVectorCorrelation((unsigned char*)pNeighbors, (unsigned char*)pNeighborsNeighbors, pCandidate);
				if(dCosTheta < pNeighbors[i].m_dCosTheta)
				{
					pNeighbors[i].m_dCosTheta = dCosTheta;
					pNeighbors[i].m_pNeighborsNeighbor = pCandidate;
				}
			}
		}
	}

	m_dAveNeighborDist /= (m_nDataPoints * m_nNeighbors);
	m_dLearningRate = m_dAveNeighborDist * 10;
}

double GSquisher::CalculateDataPointError(unsigned char* pDataPoint)
{
	double dError = 0;
	double dDist;
	double dTheta;
	struct GSquisherNeighbor* pNeighbors = (struct GSquisherNeighbor*)pDataPoint;
	struct GSquisherData* pDataPointData = (struct GSquisherData*)(pDataPoint + m_nDataIndex);
	struct GSquisherData* pNeighborData;
	int n;
	for(n = 0; n < m_nNeighbors; n++)
	{
		dDist = CalculateDistance(pDataPoint, pNeighbors[n].m_pNeighbor);
		dDist -= pNeighbors[n].m_dDistance;
		dDist /= m_dAveNeighborDist;
		dDist *= dDist;
		dTheta = CalculateVectorCorrelation(pDataPoint, pNeighbors[n].m_pNeighbor, pNeighbors[n].m_pNeighborsNeighbor);
		dTheta -= pNeighbors[n].m_dCosTheta;
		dTheta *= dTheta;
		dTheta = MAX((double)0, dTheta - .01);
		pNeighborData = (struct GSquisherData*)(pNeighbors[n].m_pNeighbor + m_nDataIndex);
		if(pNeighborData->m_nCycle != pDataPointData->m_nCycle)
		{
			dDist /= 6;
			dTheta /= 6;
		}
		dError += (2 * dDist + dTheta);
	}
	return dError * dError;
}

int GSquisher::AjustDataPoint(unsigned char* pDataPoint, int nTargetDimensions)
{
	bool bMadeProgress = true;
	double* pValues = (double*)(pDataPoint + m_nValueIndex);
	double dErrorBase = CalculateDataPointError(pDataPoint);
	double dError;
	int n, nSteps;
	for(nSteps = 0; bMadeProgress; nSteps++)
	{
		bMadeProgress = false;
		for(n = 0; n < nTargetDimensions; n++)
		{
			pValues[n] += m_dLearningRate;
			dError = CalculateDataPointError(pDataPoint);
			if(dError >= dErrorBase)
			{
				pValues[n] -= (m_dLearningRate + m_dLearningRate);
				dError = CalculateDataPointError(pDataPoint);
			}
			if(dError >= dErrorBase)
				pValues[n] += m_dLearningRate;
			else
			{
				dErrorBase = dError;
				bMadeProgress = true;
			}
		}
	}
	return nSteps - 1; // the -1 is to undo the last incrementor
}

void GSquisher::SquishBegin(int nTargetDimensions)
{
	GAssert(nTargetDimensions > 0 && nTargetDimensions < m_nDimensions, "out of range");
	m_nTargetDimensions = nTargetDimensions;
	m_nPass = 0;

	// Calculate metadata
	CalculateMetadata(nTargetDimensions);

	// Make the queue
	m_pQ = new GPointerQueue();
}

void GSquisher::SquishPass()
{
	double* pValues;
	struct GSquisherData* pData;
	unsigned char* pDataPoint;
	struct GSquisherNeighbor* pNeighbors;
	int n, i;

	// Squish the extra dimensions
	for(n = 0; n < m_nDataPoints; n++)
	{
		pValues = (double*)(&m_pData[n * m_nRecordSize] + m_nValueIndex);
		for(i = m_nTargetDimensions; i < m_nDimensions; i++)
			pValues[i] *= m_dSquishingRate;
	}

	// Pick a random data point and correct outward in a breadth-first mannner
	int nSeedDataPoint = m_nDataPoints / 2; //(int)(GetRandUInt() % m_nDataPoints);
	m_pQ->Push(&m_pData[nSeedDataPoint * m_nRecordSize]);
	int nVisitedNodes = 0;
	int nSteps = 0;
	while(m_pQ->GetSize() > 0)
	{
		// Check if this one has already been done
		pDataPoint = (unsigned char*)m_pQ->Pop();
		pData = (struct GSquisherData*)(pDataPoint + m_nDataIndex);
		if(pData->m_nCycle == m_nPass)
			continue;
		pData->m_nCycle = m_nPass;
		nVisitedNodes++;

		// Push all neighbors into the queue
		pNeighbors = (struct GSquisherNeighbor*)pDataPoint;
		for(n = 0; n < m_nNeighbors; n++)
			m_pQ->Push(pNeighbors[n].m_pNeighbor);

		// Ajust this data point
		nSteps += AjustDataPoint(pDataPoint, m_nTargetDimensions);
	}
	GAssert(nVisitedNodes * 1.25 > m_nDataPoints, "manifold appears poorly sampled");
	if(nSteps * 3 < m_nDataPoints)
		m_dLearningRate *= .9;
	else if(nSteps > m_nDataPoints * 9)
		m_dLearningRate /= .9;
	printf("[Learning Rate: %f]", m_dLearningRate);
	m_nPass++;
}

