/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GNeuralNet.h"
#include "GArff.h"
#include "../GClasses/GMath.h"
#include "../GClasses/GMacros.h"
#include "../GClasses/GArray.h"

struct GNeuralNetLayer
{
	int m_nNodes;
	double* m_pWeights;
	double* m_pWeightDeltas;
	double* m_pOutputs;
	double* m_pError;

	GNeuralNetLayer(int nNodes, GNeuralNetLayer* pPrevLayer)
	{
		m_nNodes = nNodes;
		if(pPrevLayer)
		{
			m_pWeights = new double[nNodes * pPrevLayer->m_nNodes];
			m_pWeightDeltas = new double[nNodes * pPrevLayer->m_nNodes];
		}
		else
		{
			m_pWeights = NULL;
			m_pWeightDeltas = NULL;
		}
		m_pOutputs = new double[nNodes];
		m_pError = new double[nNodes];
	}

	GNeuralNetLayer(GNeuralNetLayer* pThat, GNeuralNetLayer* pPrevLayer)
	{
		m_nNodes = pThat->m_nNodes;
		if(pPrevLayer)
			m_pWeights = new double[m_nNodes * pPrevLayer->m_nNodes];
		else
			m_pWeights = NULL;
		m_pWeightDeltas = NULL;
		m_pOutputs = new double[m_nNodes];
		m_pError = NULL;
	}

	~GNeuralNetLayer()
	{
		delete(m_pError);
		delete(m_pOutputs);
		delete(m_pWeights);
		delete(m_pWeightDeltas);
	}

	void CopyWeights(GNeuralNetLayer* pThat, GNeuralNetLayer* pPrevLayer)
	{
		GAssert(m_nNodes == pThat->m_nNodes, "different sizes");
		int nCount = m_nNodes * pPrevLayer->m_nNodes;
		int n;
		for(n = 0; n < nCount; n++)
			m_pWeights[n] = pThat->m_pWeights[n];
	}
};


GNeuralNet::GNeuralNet(GArffRelation* pRelation)
{
	m_pRelation = pRelation;
	m_pLayers = new GPointerArray(8);
	m_pBest = NULL;
	int nInputs = pRelation->GetInputCount();
	int nOutputs = pRelation->GetOutputCount();
	m_pLayers->AddPointer(new GNeuralNetLayer(nOutputs, NULL));
	m_nBiggestLayer = nInputs;
	if(nOutputs > m_nBiggestLayer)
		m_nBiggestLayer = nOutputs;
	m_dInitThresh = .1;
	m_dSigmoidSteepness = 1;
	m_dLearningRate = .15;
	m_dLearningDecay = 1;
	m_dMomentum = .15;
	m_dHopefulness = .3;
	m_nMinimumIterations = 2500;
	m_nMaximumIterations = 0x7fffffff;
	m_nIterationsPerValidationCheck = 19;
	m_dAcceptableMeanSquareError = 0.00001;
	m_nPasses = 1;
}

GNeuralNet::~GNeuralNet()
{
	int nCount = m_pLayers->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((GNeuralNetLayer*)m_pLayers->GetPointer(n));
	delete(m_pLayers);
	delete(m_pBest);
}

void GNeuralNet::AddHiddenLayer(int nNodes)
{
	GNeuralNetLayer* pPrevLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(m_pLayers->GetSize() - 1);
	m_pLayers->AddPointer(new GNeuralNetLayer(nNodes, pPrevLayer));
	if(nNodes > m_nBiggestLayer)
		m_nBiggestLayer = nNodes;
}

double randomDouble()
{
	return (double)rand() / RAND_MAX;
}

void GNeuralNet::EvaluateHelper(double* pRow, GPointerArray* pLayers)
{
	// Copy inputs into a working buffer
	int nLayer = pLayers->GetSize() - 1;
	GNeuralNetLayer* pIn = (GNeuralNetLayer*)pLayers->GetPointer(nLayer);
	GNeuralNetLayer* pOut;
	int nCount = pIn->m_nNodes;
	GAssert(nCount == m_pRelation->GetInputCount(), "node count mismatch");
	int n, i;
	for(n = 0; n < nCount; n++)
		pIn->m_pOutputs[n] = pRow[m_pRelation->GetInputIndex(n)];

	// Propagate over all the layers
	double dSum;
	while(nLayer > 0)
	{
		nLayer--;
		pOut = (GNeuralNetLayer*)pLayers->GetPointer(nLayer);

		// Calculate each output value
		int nIndex = 0;
		for(n = 0; n < pOut->m_nNodes; n++)
		{
			// Sum up the weighted inputs
			dSum = 0;
			for(i = 0; i < pIn->m_nNodes; i++)
			{
				dSum += pIn->m_pWeights[nIndex] * pIn->m_pOutputs[i];
				nIndex++;
			}

			// Squash the sum
			pOut->m_pOutputs[n] = GMath::sigmoid(dSum, m_dSigmoidSteepness);
		}

		// Move to the next layer
		pIn = pOut;
	}

	// Copy the outputs back into the row
	nCount = pOut->m_nNodes;
	GAssert(nCount == m_pRelation->GetOutputCount(), "node count mismatch");
	for(n = 0; n < nCount; n++)
		pRow[m_pRelation->GetOutputIndex(n)] = pOut->m_pOutputs[n];
}

void GNeuralNet::Evaluate(double* pRow)
{
	EvaluateHelper(pRow, m_pBest);
}

void GNeuralNet::EvaluateTraining(double* pRow)
{
	EvaluateHelper(pRow, m_pLayers);
}

double GNeuralNet::MeasureError(GArffData* pData, double* pSample, GPointerArray* pLayers)
{
	int n, i, nIndex;
	double* pRow;
	double d;
	double dError = 0;
	int nCount = pData->GetRowCount();
	int nAttributeCount = m_pRelation->GetAttributeCount();
	int nOutputs = m_pRelation->GetOutputCount();
	for(n = 0; n < nCount; n++)
	{
		pRow = pData->GetRow(n);
		memcpy(pSample, pRow, sizeof(double) * nAttributeCount);
		EvaluateHelper(pSample, pLayers);
		for(i = 0; i < nOutputs; i++)
		{
			nIndex = m_pRelation->GetOutputIndex(i);
			d = pSample[nIndex] - pRow[nIndex];
			d *= d;
			dError += d;
		}
	}
	dError /= (nCount * nOutputs);
	return dError;
}

double GNeuralNet::GetMeanSquareError(GArffData* pData)
{
	int nAttributeCount = m_pRelation->GetAttributeCount();
	double* pSample = (double*)alloca(sizeof(double) * nAttributeCount);
	return MeasureError(pData, pSample, m_pBest);
}

void GNeuralNet::UpdateBestWeights()
{
	int nCount = m_pLayers->GetSize();
	int n, i, nWeights;
	GNeuralNetLayer* pLayerSource;
	GNeuralNetLayer* pLayerTarget;
	GNeuralNetLayer* pPrev = (GNeuralNetLayer*)m_pLayers->GetPointer(0);
	for(n = 1; n < nCount; n++)
	{
		pLayerSource = (GNeuralNetLayer*)m_pLayers->GetPointer(n);
		pLayerTarget = (GNeuralNetLayer*)m_pBest->GetPointer(n);
		nWeights = pLayerSource->m_nNodes * pPrev->m_nNodes;
		for(i = 0; i < nWeights; i++)
			pLayerTarget->m_pWeights[i] = pLayerSource->m_pWeights[i];
		pPrev = pLayerSource;
	}
}

void GNeuralNet::RestoreBestWeights()
{
	int nCount = m_pLayers->GetSize();
	int n, i, nWeights;
	GNeuralNetLayer* pLayerSource;
	GNeuralNetLayer* pLayerTarget;
	GNeuralNetLayer* pPrev = (GNeuralNetLayer*)m_pLayers->GetPointer(0);
	for(n = 1; n < nCount; n++)
	{
		pLayerSource = (GNeuralNetLayer*)m_pLayers->GetPointer(n);
		pLayerTarget = (GNeuralNetLayer*)m_pBest->GetPointer(n);
		nWeights = pLayerSource->m_nNodes * pPrev->m_nNodes;
		for(i = 0; i < nWeights; i++)
			pLayerSource->m_pWeights[i] = pLayerTarget->m_pWeights[i];
		pPrev = pLayerSource;
	}
}

void GNeuralNet::RandomlyTweakWeights(double dAmount)
{
	int nCount = m_pLayers->GetSize();
	int n, i, nWeights;
	GNeuralNetLayer* pLayer;
	GNeuralNetLayer* pPrev = (GNeuralNetLayer*)m_pLayers->GetPointer(0);
	for(n = 1; n < nCount; n++)
	{
		pLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(n);
		nWeights = pLayer->m_nNodes * pPrev->m_nNodes;
		for(i = 0; i < nWeights; i++)
			pLayer->m_pWeights[i] += (dAmount * (randomDouble() - .5) * randomDouble() * randomDouble());
		pPrev = pLayer;
	}
}

void GNeuralNet::UpdateWeights(double* pRow, double* pSample)
{
	int i, j, k, nIndex;
	int nLayers = m_pLayers->GetSize();
	GNeuralNetLayer* pLayerIn = (GNeuralNetLayer*)m_pLayers->GetPointer(0);
	GNeuralNetLayer* pLayerOut;
	GNeuralNetLayer* pPrevLayer = NULL;
	double dErrorSum, dOutput;
	for(i = 1; i < nLayers; i++)
	{
		pLayerOut = pLayerIn;
		pLayerIn = (GNeuralNetLayer*)m_pLayers->GetPointer(i);
		for(j = 0; j < pLayerOut->m_nNodes; j++)
		{
			// Calculate the error on this node
			dOutput = pLayerOut->m_pOutputs[j];
			if(i <= 1)
			{
				nIndex = m_pRelation->GetOutputIndex(j);
				dErrorSum = pRow[nIndex] - dOutput;
			}
			else
			{
				dErrorSum = 0;
				for(k = 0; k < pPrevLayer->m_nNodes; k++)
					dErrorSum += (pLayerOut->m_pWeights[j + k * pLayerOut->m_nNodes] * pPrevLayer->m_pError[k]);
			}
			pLayerOut->m_pError[j] = m_dSigmoidSteepness * dOutput * ((double)1 - dOutput) * dErrorSum;

			// Update all the weights for this node
			nIndex = j * pLayerIn->m_nNodes;
			for(k = 0; k < pLayerIn->m_nNodes; k++)
			{
				// todo: implement decaying learning rate
				pLayerIn->m_pWeightDeltas[nIndex] *= m_dMomentum;
				pLayerIn->m_pWeightDeltas[nIndex] += (m_dLearningRate * pLayerOut->m_pError[j] * pLayerIn->m_pOutputs[k]);
				pLayerIn->m_pWeights[nIndex] += pLayerIn->m_pWeightDeltas[nIndex];
				nIndex++;
			}
		}
		pPrevLayer = pLayerOut;
	}
}

void GNeuralNet::Train(GArffData* pTrainingData, GArffData* pValidationData)
{
	GAssert(!m_pBest, "Already trained");

	// Prepare the data by making sure it all falls between 0 and 1
	pTrainingData->Analogize(m_pRelation);
	if(pValidationData != pTrainingData)
		pValidationData->Analogize(m_pRelation);

	// Add the input layer
	GNeuralNetLayer* pTmpLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(m_pLayers->GetSize() - 1);
	m_pLayers->AddPointer(new GNeuralNetLayer(m_pRelation->GetInputCount(), pTmpLayer));

	// Make layers to hold the best found set of weights
	m_pBest = new GPointerArray(8);
	int nCount = m_pLayers->GetSize();
	int n, i;
	GNeuralNetLayer* pPrevLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(0);
	m_pBest->AddPointer(new GNeuralNetLayer(pPrevLayer, NULL));
	for(n = 1; n < nCount; n++)
	{
		GNeuralNetLayer* pLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(n);
		m_pBest->AddPointer(new GNeuralNetLayer(pLayer, pPrevLayer));
		pPrevLayer = pLayer;
	}
	double dBestError = 1e20;

	// Do the passes
	double* pSample = (double*)alloca(sizeof(double) * m_pRelation->GetAttributeCount());
	double* pRow;
	nCount = pTrainingData->GetRowCount();
	int nAttributeCount = m_pRelation->GetAttributeCount();
	int nPass;
	for(nPass = 0; nPass < m_nPasses; nPass++)
	{
		// Initialize all the nodes in the working net to small random values
		pPrevLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(0);
		double dHalfThresh = m_dInitThresh / 2;
		for(n = 1; n < m_pLayers->GetSize(); n++)
		{
			GNeuralNetLayer* pLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(n);
			int nTotalWeights = pLayer->m_nNodes * pPrevLayer->m_nNodes;
			for(i = 0; i < nTotalWeights; i++)
			{
				pLayer->m_pWeights[i] = (randomDouble() * m_dInitThresh) - dHalfThresh;
				pLayer->m_pWeightDeltas[i] = 0;
			}
			pPrevLayer = pLayer;
		}

		// Perform training cycles
		int nIterationsSinceValidationCheck = 0;
		int nBestIteration = 0;
		int nItters;
		for(nItters = 0; nItters < m_nMaximumIterations; nItters++)
		{
			// Run each of the training examples
			for(n = 0; n < nCount; n++)
			{
				// Compute output for this sample and update the weights
				pRow = pTrainingData->GetRow(n);
				memcpy(pSample, pRow, sizeof(double) * nAttributeCount);
				EvaluateHelper(pSample, m_pLayers);
				UpdateWeights(pRow, pSample);
			}
			m_dLearningRate *= m_dLearningDecay;

			// Check for termination condition
			nIterationsSinceValidationCheck++;
			if(nIterationsSinceValidationCheck >= m_nIterationsPerValidationCheck)
			{
				nIterationsSinceValidationCheck = 0;
				if(nItters > m_nMinimumIterations)
				{
					int nValidationCount = pValidationData->GetRowCount();
					double dMeanSquareError = MeasureError(pValidationData, pSample, m_pLayers);
					if(dMeanSquareError < dBestError)
					{
						// Found a new best set of weights
						dBestError = dMeanSquareError;
						nBestIteration = nItters;
						UpdateBestWeights();
						if(dMeanSquareError <= m_dAcceptableMeanSquareError)
							break;
					}
					else
					{
						// Test for termination condition
						if((double)(nItters - nBestIteration) / nItters >= m_dHopefulness)
						{
							m_nMinimumIterations = nItters; // If there's another pass, make sure it does at least this many iterations
							break;
						}
					}
				}
//printf("Itters=%d\tError=%f\n", nItters++, dBestError);
			}
		}
	}
	GAssert(dBestError < 1e10, "Total failure!");

	// Put the data back the way it was
	pTrainingData->Unanalogize(m_pRelation);
	if(pValidationData != pTrainingData)
		pValidationData->Unanalogize(m_pRelation);
}

void GNeuralNet::TrainByCritic(CriticFunc pCritic, void* pThis)
{
	GAssert(!m_pBest, "Already trained");

	// Add the input layer
	GNeuralNetLayer* pTmpLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(m_pLayers->GetSize() - 1);
	m_pLayers->AddPointer(new GNeuralNetLayer(m_pRelation->GetInputCount(), pTmpLayer));

	// Make layers to hold the best found set of weights
	m_pBest = new GPointerArray(8);
	int nCount = m_pLayers->GetSize();
	int n, i;
	GNeuralNetLayer* pPrevLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(0);
	m_pBest->AddPointer(new GNeuralNetLayer(pPrevLayer, NULL));
	for(n = 1; n < nCount; n++)
	{
		GNeuralNetLayer* pLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(n);
		m_pBest->AddPointer(new GNeuralNetLayer(pLayer, pPrevLayer));
		pPrevLayer = pLayer;
	}
	double dBestError = 1e20;

	// Do the passes
	double dMeanSquareError;
	int nPass;
	for(nPass = 0; nPass < m_nPasses; nPass++)
	{
		// Initialize all the nodes in the working net to small random values
		pPrevLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(0);
		double dHalfThresh = m_dInitThresh / 2;
		for(n = 1; n < m_pLayers->GetSize(); n++)
		{
			GNeuralNetLayer* pLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(n);
			int nTotalWeights = pLayer->m_nNodes * pPrevLayer->m_nNodes;
			for(i = 0; i < nTotalWeights; i++)
			{
				pLayer->m_pWeights[i] = (randomDouble() * m_dInitThresh) - dHalfThresh;
				pLayer->m_pWeightDeltas[i] = 0;
			}
			pPrevLayer = pLayer;
		}

		// Perform training cycles
		int nBestIteration = 0;
		int nItters;
		for(nItters = 0; nItters < m_nMaximumIterations; nItters++)
		{
			// Check for a new best set of weights
			dMeanSquareError = pCritic(pThis, this); 
			if(dMeanSquareError < dBestError)
			{
				dBestError = dMeanSquareError;
				nBestIteration = nItters;
				UpdateBestWeights();
				if(dMeanSquareError <= m_dAcceptableMeanSquareError)
					break;
			}
			else
				RestoreBestWeights();

			// Check for termination condition
			if(nItters > m_nMinimumIterations)
			{
				// Test for termination condition
				if((double)(nItters - nBestIteration) / nItters >= m_dHopefulness)
				{
					m_nMinimumIterations = nItters; // If there's another pass, make sure it does at least this many iterations
					break;
				}
			}
//printf("Itters=%d\tError=%f\n", nItters++, dBestError);

			// Try tweaking the weights
			RandomlyTweakWeights(m_dLearningRate);
			m_dLearningRate *= m_dLearningDecay;
		}
	}
	GAssert(dBestError < 1e10, "Total failure!");
}
