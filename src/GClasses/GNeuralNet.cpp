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

class GNeuron;


#define INIT_THRESH .2

double randomDouble()
{
	return (double)rand() / RAND_MAX;
}



struct GSynapse
{
	double m_dWeight; // Synapse
	double m_dWeightDelta;
	GNeuron* m_pInput;
	GNeuron* m_pOutput;

	GSynapse()
	{
		m_dWeight = (randomDouble() * INIT_THRESH) - (INIT_THRESH / 2);
		m_dWeightDelta = 0;
		m_pInput = NULL;
		m_pOutput = NULL;
	}
};



class GNeuron
{
public:
	virtual double PullEvalDownStream() = 0;
	virtual double PullErrorBackUpStream() = 0;
	virtual double GetOutput() = 0;
	virtual void SetOutput(double d) = 0;
	virtual void SetError(double d) = 0;
	virtual void AjustWeights(double dLearningRate, double dMomentum) = 0;
	virtual int SerializeWeights(double* pBuffer) = 0;
	virtual int DeserializeWeights(double* pBuffer) = 0;
	virtual void AddInput(GNeuron* pNeuron) = 0;

	// For internal use only
	virtual void ConnectOutputToSynapse(GSynapse* pSynapse) = 0;
	virtual void RemapSynapse(GSynapse* pOld, GSynapse* pNew) = 0;
};



class GStandardNeuron : public GNeuron
{
protected:
	double m_dOutput; // Axon
	double m_dError;
	int m_nInputs;
	int m_nInputSpace;
	struct GSynapse* m_pInputs;
	int m_nOutputs;
	int m_nOutputSpace;
	struct GSynapse** m_pOutputs;

public:
	GStandardNeuron() : GNeuron()
	{
		m_dOutput = 1e50;
		m_dError = 1e50;
		m_nInputs = 0;
		m_nInputSpace = 0;
		m_pInputs = NULL;
		AddInput(NULL); // Add the constant 1 (for bias) input
		m_nOutputs = 0;
		m_nOutputSpace = 0;
		m_pOutputs = NULL;
	}

	virtual ~GStandardNeuron()
	{
		delete [] m_pInputs;
		delete(m_pOutputs);
	}

	virtual double PullEvalDownStream()
	{
		if(m_dOutput == 1e50)
		{
			// Sum up the weighted inputs
			double dSum = m_pInputs[0].m_dWeight;
			int n;
			for(n = 1; n < m_nInputs; n++)
				dSum += (m_pInputs[n].m_dWeight * m_pInputs[n].m_pInput->PullEvalDownStream());

			// Squash the sum
			m_dOutput = GMath::sigmoid(dSum, 1);
		}
		return m_dOutput;
	}

	virtual double PullErrorBackUpStream()
	{
		GAssert(m_dOutput != 1e50, "output was not calculated");
		if(m_dError == 1e50)
		{
			// Sum up the errors from each output
			double dSum = 0;
			int n;
			for(n = 0; n < m_nOutputs; n++)
				dSum += (m_pOutputs[n]->m_dWeight * m_pOutputs[n]->m_pOutput->PullErrorBackUpStream());

			// Multiply by derivative of squashing function
			m_dError = dSum * m_dOutput * ((double)1 - m_dOutput);
		}
		return m_dError;
	}

	virtual double GetOutput()
	{
		GAssert(m_dOutput != 1e50, "output was not calculated");
		return m_dOutput;
	}

	virtual void SetOutput(double d)
	{
		m_dOutput = d;
	}

	virtual void SetError(double d)
	{
		m_dError = d;
	}

	virtual void AjustWeights(double dLearningRate, double dMomentum)
	{
		GAssert(m_dError != 1e50, "output was not calculated");
		GSynapse* pNeuronRef;
		pNeuronRef = &m_pInputs[0];
		pNeuronRef->m_dWeightDelta *= dMomentum;
		pNeuronRef->m_dWeightDelta += (dLearningRate * m_dError);
		pNeuronRef->m_dWeight += pNeuronRef->m_dWeightDelta;
		int n;
		for(n = 1; n < m_nInputs; n++)
		{
			pNeuronRef = &m_pInputs[n];
			pNeuronRef->m_dWeightDelta *= dMomentum;
			pNeuronRef->m_dWeightDelta += (dLearningRate * m_dError * pNeuronRef->m_pInput->GetOutput());
			pNeuronRef->m_dWeight += pNeuronRef->m_dWeightDelta;
		}
	}

	virtual int SerializeWeights(double* pBuffer)
	{
		if(pBuffer)
		{
			int n;
			for(n = 0; n < m_nInputs; n++)
				pBuffer[n] = m_pInputs[n].m_dWeight;
		}
		return m_nInputs;
	}

	virtual int DeserializeWeights(double* pBuffer)
	{
		int n;
		for(n = 0; n < m_nInputs; n++)
			m_pInputs[n].m_dWeight = pBuffer[n];
		return m_nInputs;
	}

	virtual void AddInput(GNeuron* pNeuron)
	{
		if(m_nInputs >= m_nInputSpace)
		{
			// Reallocate input space
			int nInputSpace = MAX(4, m_nInputSpace * 2);
			GSynapse* pInputs = new GSynapse[nInputSpace];
			memcpy(pInputs, m_pInputs, sizeof(GSynapse) * m_nInputSpace);

			// Remap everything
			int n;
			for(n = 0; n < m_nInputs; n++)
			{
				if(m_pInputs[n].m_pInput)
					m_pInputs[n].m_pInput->RemapSynapse(&m_pInputs[n], &pInputs[n]);
			}
			delete(m_pInputs);
			m_pInputs = pInputs;
			m_nInputSpace = nInputSpace;
		}
		m_pInputs[m_nInputs].m_pInput = pNeuron;
		m_pInputs[m_nInputs].m_pOutput = this;
		if(pNeuron)
			pNeuron->ConnectOutputToSynapse(&m_pInputs[m_nInputs]);
		else
		{
			GAssert(m_nInputs == 0, "only the first input should be NULL");
		}
		m_nInputs++;
	}

	virtual void RemapSynapse(GSynapse* pOld, GSynapse* pNew)
	{
		int n;
		for(n = 0; n < m_nOutputs; n++)
		{
			if(m_pOutputs[n] == pOld)
			{
				m_pOutputs[n] = pNew;
				break;
			}
		}
	}

protected:
	virtual void ConnectOutputToSynapse(GSynapse* pSynapse)
	{
		if(m_nOutputs >= m_nOutputSpace)
		{
			// Reallocate output space
			int nOutputSpace = MAX(4, m_nOutputSpace * 2);
			GSynapse** pOutputs = new GSynapse*[nOutputSpace];
			memcpy(pOutputs, m_pOutputs, sizeof(GSynapse*) * m_nOutputSpace);
			delete(m_pOutputs);
			m_pOutputs = pOutputs;
			m_nOutputSpace = nOutputSpace;
		}
		m_pOutputs[m_nOutputs++] = pSynapse;
	}
};




// ----------------------------------------------------------------------

GNeuralNet::GNeuralNet(GArffRelation* pRelation)
{
	m_pRelation = pRelation;
	m_pNeurons = new GPointerArray(64);
	m_pBestSet = NULL;
	m_nWeightCount = 0;
	m_nInputStart = 0;
	m_nLayerStart = 0;
	m_nLayerSize = 0;
	int nOutputs = pRelation->GetOutputCount();
	AddLayer(nOutputs);

	// Default settings
	m_dLearningRate = .315;
	m_dLearningDecay = 1;
	m_dMomentum = .9;
	m_dHopefulness = .25;
	m_nMinimumEpochs = 2500;
	m_nMaximumEpochs = 50000;
	m_nEpochsPerValidationCheck = 5;
	m_dAcceptableMeanSquareError = 0.000001;
	m_nPasses = 1;
}

GNeuralNet::~GNeuralNet()
{
	int nCount = m_pNeurons->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((GNeuron*)m_pNeurons->GetPointer(n));
	delete(m_pNeurons);
	delete(m_pBestSet);
}

void GNeuralNet::AddLayer(int nNodes)
{
	int nPrevLayerStart = m_nLayerStart;
	int nPrevLayerSize = m_nLayerSize;
	m_nLayerStart = m_pNeurons->GetSize();
	m_nLayerSize = nNodes;
	int n, i;
	for(n = 0; n < nNodes; n++)
	{
		GStandardNeuron* pNewNeuron = new GStandardNeuron();
		m_pNeurons->AddPointer(pNewNeuron);
		for(i = 0; i < nPrevLayerSize; i++)
		{
			GNeuron* pOldNeuron = (GNeuron*)m_pNeurons->GetPointer(nPrevLayerStart + i);
			pOldNeuron->AddInput(pNewNeuron);
		}
	}
}

double GNeuralNet::MeasureError(GArffData* pData, double* pSample)
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
		Eval(pSample);
		for(i = 0; i < nOutputs; i++)
		{
			nIndex = m_pRelation->GetOutputIndex(i);
			d = pRow[nIndex] - pSample[nIndex];
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
	return MeasureError(pData, pSample);
}

void GNeuralNet::UpdateBestWeights()
{
	int n;
	int nCount = m_pNeurons->GetSize();
	GNeuron* pNeuron;
	if(!m_pBestSet)
	{
		// Allocate a buffer for the weights
		m_nWeightCount = 0;
		for(n = 0; n < nCount; n++)
		{
			pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
			m_nWeightCount += pNeuron->SerializeWeights(NULL);
		}
		m_pBestSet = new double[m_nWeightCount];
	}

	// Serialize the weights
	int nPos = 0;
	for(n = 0; n < nCount; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		nPos += pNeuron->SerializeWeights(&m_pBestSet[nPos]);
	}
	GAssert(nPos == m_nWeightCount, "serialization size inconsistent");
}

void GNeuralNet::RestoreBestWeights()
{
	int n;
	int nCount = m_pNeurons->GetSize();
	GNeuron* pNeuron;
	int nPos = 0;
	for(n = 0; n < nCount; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		nPos += pNeuron->DeserializeWeights(&m_pBestSet[nPos]);
	}
	GAssert(nPos == m_nWeightCount, "serialization size inconsistent");
}
/*
void GNeuralNet::RandomlyTweakWeights(double dAmount)
{
	int nCount = m_pNeurons->GetSize();
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
*/

void GNeuralNet::Eval(double* pRow)
{
	// Clear the outputs of all non-input neurons
	int n, nIndex;
	GNeuron* pNeuron;
	for(n = 0; n < m_nInputStart; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		pNeuron->SetOutput(1e50);
	}

	// Copy inputs into input neurons
	int nInputs = m_pRelation->GetInputCount();
	int nCount = m_nInputStart + nInputs;
	GAssert(nCount == m_pNeurons->GetSize(), "neurons added after input neurons?");
	int nInput = 0;
	for( ; n < nCount; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		nIndex = m_pRelation->GetInputIndex(nInput);
		pNeuron->SetOutput(pRow[nIndex]);
		nInput++;
	}

	// Pull the evaluation downstream to the output nodes
	int nOutputs = m_pRelation->GetOutputCount();
	for(n = 0; n < nOutputs; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		nIndex = m_pRelation->GetOutputIndex(n);
		pRow[nIndex] = pNeuron->PullEvalDownStream();
	}
}

void GNeuralNet::Criticize(double* pModel)
{
	// Calculate the error on all output nodes
	double dOutput;
	int n, nIndex;
	GNeuron* pNeuron;
	int nOutputs = m_pRelation->GetOutputCount();
	for(n = 0; n < nOutputs; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		nIndex = m_pRelation->GetOutputIndex(n);
		dOutput = pNeuron->GetOutput();
		pNeuron->SetError((pModel[nIndex] - dOutput) * dOutput * (1.0 - dOutput));
	}

	// Clear the error on the rest of the nodes
	int nInputs = m_pRelation->GetInputCount();
	for( ; n <= m_nInputStart; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		pNeuron->SetError(1e50);
	}

	// Backpropagate the error (we only need to call PullErrorBackUpStream
	// on one input neuron because any input is connected to all the nodes
	// in the next layer and we don't need the error value for the inputs
	pNeuron->PullErrorBackUpStream();
}

void GNeuralNet::Train(GArffData* pTrainingData, GArffData* pValidationData)
{
	GAssert(m_nInputStart == 0, "already trained");
	GAssert(m_nMinimumEpochs < m_nMaximumEpochs, "conflicting settings");

	// Normalize all the output values since the sigmoids can only handle between 0 and 1
	int nOutputCount = m_pRelation->GetOutputCount();
	double* pTrainingMinAndRanges = (double*)alloca(sizeof(double) * 2 * nOutputCount);
	double* pValidationMinAndRanges = NULL;
	int n, i;
	for(n = 0; n < nOutputCount; n++)
	{
		i = m_pRelation->GetOutputIndex(n);
		pTrainingData->GetMinAndRange(i, &pTrainingMinAndRanges[2 * n], &pTrainingMinAndRanges[2 * n + 1]);
		if(pTrainingMinAndRanges[2 * n + 1] < .00001)
			pTrainingMinAndRanges[2 * n + 1] = .00001;
		pTrainingData->Normalize(i, pTrainingMinAndRanges[2 * n], pTrainingMinAndRanges[2 * n + 1], .1, .8);
	}
	if(pValidationData != pTrainingData)
	{
		pValidationMinAndRanges = (double*)alloca(sizeof(double) * 2 * nOutputCount);
		for(n = 0; n < nOutputCount; n++)
		{
			i = m_pRelation->GetOutputIndex(n);
			pValidationData->GetMinAndRange(i, &pValidationMinAndRanges[2 * n], &pValidationMinAndRanges[2 * n + 1]);
			pValidationData->Normalize(i, pValidationMinAndRanges[2 * n], pValidationMinAndRanges[2 * n + 1], .1, .8);
		}
	}

	// Add the input layer
	m_nInputStart = m_pNeurons->GetSize();
	AddLayer(m_pRelation->GetInputCount());

	// Do the passes
	double dBestError = 1e20;
	double* pSample = (double*)alloca(sizeof(double) * m_pRelation->GetAttributeCount());
	double* pRow;
	GNeuron* pNeuron;
	int nRowCount = pTrainingData->GetRowCount();
	int nAttributeCount = m_pRelation->GetAttributeCount();
	int nPass;
	for(nPass = 0; nPass < m_nPasses; nPass++)
	{
		// Initialize all the nodes in the working net to small random values
		GAssert(m_nPasses == 1, "only one pass currently supported--todo: reinitialize weights to small random values");

		// Perform training cycles
		int nEpochsSinceValidationCheck = 0;
		int nBestEpoch = 0;
		int nEpochs;
		for(nEpochs = 0; true; nEpochs++)
		{
//printf("Epoch: %d\n", nEpochs);
			// Train with each of the training examples (one epoch)
			for(n = 0; n < nRowCount; n++)
			{
				// Compute output for this sample and update the weights
				pRow = pTrainingData->GetRow(n);
				memcpy(pSample, pRow, sizeof(double) * nAttributeCount);
				Eval(pSample);

				// Backpropagate the error
				Criticize(pRow);

				// Ajust the weights
				for(i = 0; i < m_nInputStart; i++)
				{
					pNeuron = (GNeuron*)m_pNeurons->GetPointer(i);
					pNeuron->AjustWeights(m_dLearningRate, m_dMomentum);
				}
			}
			m_dLearningRate *= m_dLearningDecay;

			// Check for termination condition
			nEpochsSinceValidationCheck++;
			if(nEpochsSinceValidationCheck >= m_nEpochsPerValidationCheck)
			{
				nEpochsSinceValidationCheck = 0;
				if(nEpochs > m_nMinimumEpochs)
				{
					int nValidationCount = pValidationData->GetRowCount();
					double dMeanSquareError = MeasureError(pValidationData, pSample);
//printf("Epoch: %d\tError=%lf\n", nEpochs, dBestError);
					if(dMeanSquareError < dBestError)
					{
						// Found a new best set of weights
						dBestError = dMeanSquareError;
						nBestEpoch = nEpochs;
						UpdateBestWeights();
						if(dMeanSquareError <= m_dAcceptableMeanSquareError)
							break;
					}
					else
					{
						// Test for termination condition
						if((double)(nEpochs - nBestEpoch) / nEpochs >= m_dHopefulness)
						{
							m_nMinimumEpochs = nEpochs; // If there's another pass, make sure it does at least this many iterations
							break;
						}
					}
					if(nEpochs >= m_nMaximumEpochs)
						break;
				}
			}
		}
	}
	if(dBestError < 1e10)
		RestoreBestWeights();
	else
	{
		GAssert(dBestError < 1e10, "Total failure!");
	}

	// Put the data back the way it was
	for(n = 0; n < nOutputCount; n++)
	{
		i = m_pRelation->GetOutputIndex(n);
		pTrainingData->Normalize(i, .1, .8, pTrainingMinAndRanges[2 * n], pTrainingMinAndRanges[2 * n + 1]);
	}
	pTrainingData->DiscretizeNonContinuousOutputs(m_pRelation);
	if(pValidationData != pTrainingData)
	{
		for(n = 0; n < nOutputCount; n++)
		{
			i = m_pRelation->GetOutputIndex(n);
			pValidationData->Normalize(i, .1, .8, pValidationMinAndRanges[2 * n], pValidationMinAndRanges[2 * n + 1]);
		}
		pValidationData->DiscretizeNonContinuousOutputs(m_pRelation);
	}
}

/*
void GNeuralNet::TrainByCritic(CriticFunc pCritic, void* pThis)
{
	GAssert(!m_pBest, "Already trained");

	// Add the input layer
	AddHiddenLayer(m_pRelation->GetInputCount());

	// Do the passes
	double dBestError = 1e20;
	double dMeanSquareError;
	int n, i, nPass;
	for(nPass = 0; nPass < m_nPasses; nPass++)
	{
		// Initialize all the nodes in the working net to small random values
		GNeuralNetLayer* pPrevLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(0);
		for(n = 1; n < m_pLayers->GetSize(); n++)
		{
			GNeuralNetLayer* pLayer = (GNeuralNetLayer*)m_pLayers->GetPointer(n);
			int nTotalWeights = pLayer->m_nNodes * pPrevLayer->m_nNodes;
			for(i = 0; i < nTotalWeights; i++)
			{
				pLayer->m_pWeights[i] = (randomDouble() * INIT_THRESH) - (INIT_THRESH / 2);
				pLayer->m_pWeightDeltas[i] = 0;
			}
			pPrevLayer = pLayer;
		}

		// Perform training cycles
		int nBestEpoch = 0;
		int nEpochs;
		for(nEpochs = 0; nEpochs < m_nMaximumEpochs; nEpochs++)
		{
			// Check for a new best set of weights
			dMeanSquareError = pCritic(pThis, this); 
			if(dMeanSquareError < dBestError)
			{
				dBestError = dMeanSquareError;
				nBestEpoch = nEpochs;
				UpdateBestWeights();
				if(dMeanSquareError <= m_dAcceptableMeanSquareError)
					break;
			}
			else
				RestoreBestWeights();

			// Check for termination condition
			if(nEpochs > m_nMinimumEpochs)
			{
				// Test for termination condition
				if((double)(nEpochs - nBestEpoch) / nEpochs >= m_dHopefulness)
				{
					m_nMinimumEpochs = nEpochs; // If there's another pass, make sure it does at least this many iterations
					break;
				}
			}
//printf("Itters=%d\tError=%f\n", nEpochs++, dBestError);

			// Try tweaking the weights
			RandomlyTweakWeights(m_dLearningRate);
			m_dLearningRate *= m_dLearningDecay;
		}
	}
	//GAssert(dBestError < 1e10, "Total failure!");
}
*/
