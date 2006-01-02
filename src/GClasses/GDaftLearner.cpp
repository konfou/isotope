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
#include "GAVLTree.h"
#include "GHashTable.h"
#include <math.h>
#include "GPointerQueue.h"
#include "GMatrix.h"

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
	pNNFit->SetRunEpochs(16000);
	pNNFit->SetMaximumEpochs(50000);
	pNNFit->Train(pData, pData);

printf("Subtracting predicted results...\n");
	// Subtract the predicted results
	int nAttributeCount = m_pRelation->GetAttributeCount();
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
