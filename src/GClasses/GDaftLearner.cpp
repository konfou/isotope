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

class GDaftNode
{
public:
	double m_dMin;
	double m_dRange;
	GNeuralNet* m_pNNFit;
	GNeuralNet* m_pNNDivide;
	GDaftNode* m_pNegative;
	GDaftNode* m_pPositive;

	GDaftNode(double dMin, double dRange, GNeuralNet* pNNFit)
	{
		m_dMin = dMin;
		m_dRange = dRange;
		m_pNNFit = pNNFit;
		m_pNNDivide = NULL;
		m_pNegative = NULL;
		m_pPositive = NULL;
	}

	~GDaftNode()
	{
		delete(m_pNegative);
		delete(m_pPositive);
		delete(m_pNNDivide);
		delete(m_pNNFit);
	}
};



GDaftLearner::GDaftLearner(GArffRelation* pRelation)
{
	m_pRelation = pRelation;
	m_pRoot = NULL;
	m_pTrainingData = NULL;
	m_dAcceptableError = .01;
	m_dGoodDivideWeight = .01;
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
	m_pRoot = BuildBranch(pData, 1);
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

GNeuralNet* GDaftLearner::FitData(GArffData* pData, int nHiddenNodes, int nMaxIterations)
{
	GNeuralNet* pNNDivide = new GNeuralNet(m_pDivideRelation);
	pNNDivide->AddHiddenLayer(nHiddenNodes);
	pNNDivide->SetAcceptableMeanSquareError(.001);
	pNNDivide->SetLearningDecay(.996);
	pNNDivide->SetLearningRate(6);
	pNNDivide->SetMaximumIterations(nMaxIterations);
	pNNDivide->SetMinimumIterations(350);
	pNNDivide->Train(pData, pData);
	return pNNDivide;
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
		return 1e10;
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
	pNNDivide->AddHiddenLayer(3);
	pNNDivide->SetAcceptableMeanSquareError(.001);
	pNNDivide->SetLearningDecay(.996);
	pNNDivide->SetLearningRate(6);
	pNNDivide->SetMaximumIterations(1000);
	pNNDivide->SetMinimumIterations(500);
	pNNDivide->TrainByCritic(DaftLearnerCriticFunc, this);
	m_pTrainingData = NULL;
	return pNNDivide;
}

#define NORMALIZE_MIN .1
#define NORMALIZE_RANGE .8

GDaftNode* GDaftLearner::BuildBranch(GArffData* pData, double dScale)
{
printf("Normalizing...\n");
	// Normalize the data
	int nOutputIndex = m_pRelation->GetOutputIndex(0);
	double dMin;
	double dRange;
	pData->GetMinAndRange(nOutputIndex, &dMin, &dRange);
	pData->Normalize(nOutputIndex, dMin, dRange, NORMALIZE_MIN, NORMALIZE_RANGE);

printf("Fitting...\n");
	// Fit the data
	GNeuralNet* pNNFit = new GNeuralNet(m_pRelation);
	pNNFit->AddHiddenLayer(5);
	pNNFit->SetAcceptableMeanSquareError(.001);
	pNNFit->SetLearningDecay(.996);
	pNNFit->SetLearningRate(6);
	pNNFit->SetMaximumIterations(500000);
	pNNFit->SetMinimumIterations(1500);
	pNNFit->Train(pData, pData);

printf("Subtracting predicted results...\n");
	// Subtract the predicted results
	int nAttributeCount = m_pRelation->GetAttributeCount();
	int nInputs = m_pRelation->GetInputCount();
	double* pSample = (double*)alloca(sizeof(double) * nAttributeCount);
	double* pRow;
	int nRowCount = pData->GetRowCount();
	int n;
	for(n = 0; n < nRowCount; n++)
	{
		pRow = pData->GetRow(n);
		memcpy(pSample, pRow, sizeof(double) * nAttributeCount);
		pNNFit->Evaluate(pSample);
		pRow[nOutputIndex] -= pSample[nOutputIndex];
	}

	// Make the node
	GDaftNode* pNode = new GDaftNode(dMin, dRange, pNNFit);
	pData->GetMinAndRange(nOutputIndex, &dMin, &dRange);
	double dNewScale = dScale * dRange / NORMALIZE_RANGE;
	if(dNewScale <= m_dAcceptableError)
		return pNode;

printf("Dividing %d...", nRowCount);
	// Divide the data
	GNeuralNet* pNNDivide = FindBestDivision(pData);
	GArffData pos(nRowCount);
	GArffData neg(nRowCount);
	DivideData(pNNDivide, pData, &pos, &neg);
	if(pos.GetRowCount() * 15 < neg.GetRowCount() || neg.GetRowCount() * 15 < pos.GetRowCount())
	{
		// Couldn't divide it, so bail out
		delete(pNNDivide);
		return pNode;
	}
	pNode->m_pNNDivide = pNNDivide;

printf("Recursing... (%d, %d)\n", pos.GetRowCount(), neg.GetRowCount());
	// Recurse
	pNode->m_pPositive = BuildBranch(&pos, dNewScale);
	pNode->m_pNegative = BuildBranch(&neg, dNewScale);
	pos.DropAllRows();
	neg.DropAllRows();
	return pNode;
}

void GDaftLearner::Eval(double* pRow)
{
	int nInputCount = m_pRelation->GetInputCount();
	double* pSample = (double*)alloca(sizeof(double) * nInputCount);
	EvalHelper(pRow, m_pRoot, pSample);
}

void GDaftLearner::EvalHelper(double* pRow, GDaftNode* pNode, double* pSample)
{
	int nOutputIndex = m_pRelation->GetOutputIndex(0);
	double dVal;
	if(pNode->m_pNNDivide)
	{
		int nInputCount = m_pRelation->GetInputCount();
		int i;
		for(i = 0; i < nInputCount - 1; i++)
			pSample[i] = pRow[m_pRelation->GetInputIndex(i)];
		pNode->m_pNNDivide->Evaluate(pSample);
		if(pRow[m_pRelation->GetInputIndex(nInputCount - 1)] >= pSample[nInputCount - 1])
			EvalHelper(pRow, pNode->m_pPositive, pSample);
		else
			EvalHelper(pRow, pNode->m_pNegative, pSample);
		dVal = pRow[nOutputIndex];
	}
	else
		dVal = 0;
	pNode->m_pNNFit->Evaluate(pRow);
	dVal += GArffData::Normalize(pRow[nOutputIndex], NORMALIZE_MIN, NORMALIZE_RANGE, pNode->m_dMin, pNode->m_dRange);
	pRow[nOutputIndex] = dVal;
}


