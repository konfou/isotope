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
#include "GMath.h"
#include "GMacros.h"
#include "GArray.h"
#include "GBits.h"

class GNeuron;


#define INIT_THRESH .15
#define OUTPUT_MIN .1
#define OUTPUT_MIDDLE .5
#define OUTPUT_RANGE .8
#define INPUT_MIN -.7
#define INPUT_RANGE 1.4


struct GSynapse
{
	double m_dWeight; // Synapse
	double m_dWeightDelta;
	GNeuron* m_pInput;
	GNeuron* m_pOutput;

	GSynapse()
	{
		m_dWeight = (GBits::GetRandomDouble() * INIT_THRESH) - (INIT_THRESH / 2);
		m_dWeightDelta = 0;
		m_pInput = NULL;
		m_pOutput = NULL;
	}
};



class GNeuron
{
public:
	GNeuron() {}
	virtual ~GNeuron() {}

	virtual double PullEvalDownStream() = 0;
	virtual double PullErrorBackUpStream() = 0;
	virtual double GetOutput() = 0;
	virtual void SetOutput(double d) = 0;
	virtual void SetError(double d) = 0;
	virtual void AjustWeights(double dLearningRate, double dMomentum) = 0;
	virtual int SerializeWeights(double* pBuffer) = 0;
	virtual int DeserializeWeights(double* pBuffer) = 0;
	virtual void AddInput(GNeuron* pNeuron) = 0;
	virtual void Print() = 0;
	virtual void BatchUpdateDeltas(double dLearningRate) = 0;
	virtual void BatchUpdateWeights() = 0;

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

	virtual void Print()
	{
		int n;
		for(n = 0; n < m_nInputs; n++)
			printf("\t%lf\n", m_pInputs[n].m_dWeight);
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

	virtual void BatchUpdateDeltas(double dLearningRate)
	{
		GAssert(m_dError != 1e50, "output was not calculated");
		GSynapse* pNeuronRef;
		pNeuronRef = &m_pInputs[0];
		pNeuronRef->m_dWeightDelta += (dLearningRate * m_dError);
		int n;
		for(n = 1; n < m_nInputs; n++)
		{
			pNeuronRef = &m_pInputs[n];
			pNeuronRef->m_dWeightDelta += (dLearningRate * m_dError * pNeuronRef->m_pInput->GetOutput());
		}
	}

	virtual void BatchUpdateWeights()
	{
		GSynapse* pNeuronRef;
		pNeuronRef = &m_pInputs[0];
		pNeuronRef->m_dWeight += pNeuronRef->m_dWeightDelta;
		pNeuronRef->m_dWeightDelta = 0;
		int n;
		for(n = 1; n < m_nInputs; n++)
		{
			pNeuronRef = &m_pInputs[n];
			pNeuronRef->m_dWeight += pNeuronRef->m_dWeightDelta;
			pNeuronRef->m_dWeightDelta = 0;
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
: GSupervisedLearner(pRelation)
{
	m_pInternalRelation = NULL;
	m_pNeurons = new GPointerArray(64);
	m_pBestSet = NULL;
	m_nWeightCount = 0;
	m_nInputStart = 0;
	m_nLayerStart = 0;
	m_nLayerSize = 0;
	m_pMinAndRanges = NULL;
	MakeInternalRelationAndOutputLayer();

	// Default settings
	m_dLearningRate = .215;
	m_dLearningDecay = 1;
	m_dMomentum = .9;
	m_nRunEpochs = 4000;
	m_nMaximumEpochs = 50000;
	m_nEpochsPerValidationCheck = 5;
	m_dAcceptableMeanSquareError = 0.000001;
	m_dTrainingPortion = .65;

	// Step training
	m_pTrainingDataInternal = NULL;
	m_pValidationDataInternal = NULL;
}

GNeuralNet::~GNeuralNet()
{
	int nCount = m_pNeurons->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((GNeuron*)m_pNeurons->GetPointer(n));
	delete(m_pNeurons);
	delete(m_pBestSet);
	delete(m_pMinAndRanges);
	delete(m_pInternalRelation);
	ReleaseInternalData();
}

void GNeuralNet::ReleaseInternalData()
{
	if(m_pValidationDataInternal != m_pTrainingDataInternal)
		delete(m_pValidationDataInternal);
	delete(m_pTrainingDataInternal);
	m_pTrainingDataInternal = NULL;
	m_pValidationDataInternal = NULL;
}

void GNeuralNet::MakeInternalRelationAndOutputLayer()
{
	// Make the internal relation
	GAssert(m_pInternalRelation == NULL, "already created the internal relation");
	m_pInternalRelation = new GArffRelation();

	// Add the internal input nodes
	GArffAttribute* pAttr;
	int nValueCount;
	int nInputCount = m_pRelation->GetInputCount();
	int n, i;
	for(n = 0; n < nInputCount; n++)
	{
		pAttr = m_pRelation->GetAttribute(m_pRelation->GetInputIndex(n));
		if(pAttr->IsContinuous())
			m_pInternalRelation->AddAttribute(new GArffAttribute(true, 0, NULL));
		else
		{
			nValueCount = pAttr->GetValueCount();
			if(nValueCount <= 2)
				m_pInternalRelation->AddAttribute(new GArffAttribute(true, 0, NULL));
			else
			{
				for(i = 0; i < nValueCount; i++)
					m_pInternalRelation->AddAttribute(new GArffAttribute(true, 0, NULL));
			}
		}
	}

	// Add the internal output nodes
	int nOutputCount = m_pRelation->GetOutputCount();
	for(n = 0; n < nOutputCount; n++)
	{
		pAttr = m_pRelation->GetAttribute(m_pRelation->GetOutputIndex(n));
		if(pAttr->IsContinuous())
			m_pInternalRelation->AddAttribute(new GArffAttribute(false, 0, NULL));
		else
		{
			nValueCount = pAttr->GetValueCount();
			if(nValueCount <= 2)
				m_pInternalRelation->AddAttribute(new GArffAttribute(false, 0, NULL));
			else
			{
				for(i = 0; i < nValueCount; i++)
					m_pInternalRelation->AddAttribute(new GArffAttribute(false, 0, NULL));
			}
		}
	}

	// Make the output layer
	AddLayer(m_pInternalRelation->GetOutputCount());
}

void GNeuralNet::MakeInputLayer()
{
	GAssert(m_nInputStart == 0, "already made the input layer");
	m_nInputStart = m_pNeurons->GetSize();
	AddLayer(m_pInternalRelation->GetInputCount());
}

void GNeuralNet::InputsToInternal(double* pExternal, double* pInternal)
{
	GAssert(m_pMinAndRanges, "min and ranges not calculated yet");
	GArffAttribute* pAttr;
	int nValueCount;
	int nInputCount = m_pRelation->GetInputCount();
	int nInternalIndex = 0;
	int n, i, nExternalIndex;
	for(n = 0; n < nInputCount; n++)
	{
		nExternalIndex = m_pRelation->GetInputIndex(n);
		pAttr = m_pRelation->GetAttribute(nExternalIndex);
		if(pAttr->IsContinuous())
			pInternal[nInternalIndex++] = GArffData::Normalize(pExternal[nExternalIndex], m_pMinAndRanges[nExternalIndex + nExternalIndex], m_pMinAndRanges[nExternalIndex + nExternalIndex + 1], INPUT_MIN, INPUT_RANGE);
		else
		{
			nValueCount = pAttr->GetValueCount();
			if(nValueCount <= 2)
				pInternal[nInternalIndex++] = (pExternal[nExternalIndex] < .5 ? INPUT_MIN : INPUT_MIN + INPUT_RANGE);
			else
			{
				for(i = 0; i < nValueCount; i++)
					pInternal[nInternalIndex + i] = INPUT_MIN;
				GAssert((int)pExternal[nExternalIndex] >= 0 && (int)pExternal[nExternalIndex] < nValueCount, "out of range");
				pInternal[nInternalIndex + (int)pExternal[nExternalIndex]] = INPUT_MIN + INPUT_RANGE;
				nInternalIndex += nValueCount;
			}
		}
	}
	GAssert(nInternalIndex == m_pInternalRelation->GetInputCount(), "error");
}

void GNeuralNet::OutputsToInternal(double* pExternal, double* pInternal)
{
	GAssert(m_pMinAndRanges, "min and ranges not calculated yet");
	GArffAttribute* pAttr;
	int nValueCount;
	int nOutputCount = m_pRelation->GetOutputCount();
	int nInternalIndex = m_pInternalRelation->GetOutputIndex(0);
	int n, i, nExternalIndex;
	for(n = 0; n < nOutputCount; n++)
	{
		nExternalIndex = m_pRelation->GetOutputIndex(n);
		pAttr = m_pRelation->GetAttribute(nExternalIndex);
		if(pAttr->IsContinuous())
			pInternal[nInternalIndex++] = GArffData::Normalize(pExternal[nExternalIndex], m_pMinAndRanges[nExternalIndex + nExternalIndex], m_pMinAndRanges[nExternalIndex + nExternalIndex + 1], OUTPUT_MIN, OUTPUT_RANGE);
		else
		{
			nValueCount = pAttr->GetValueCount();
			if(nValueCount <= 2)
				pInternal[nInternalIndex++] = (pExternal[nExternalIndex] < .5 ? OUTPUT_MIN : OUTPUT_MIN + OUTPUT_RANGE);
			else
			{
				for(i = 0; i < nValueCount; i++)
					pInternal[nInternalIndex + i] = OUTPUT_MIN;
				GAssert((int)pExternal[nExternalIndex] >= 0 && (int)pExternal[nExternalIndex] < nValueCount, "out of range");
				pInternal[nInternalIndex + (int)pExternal[nExternalIndex]] = OUTPUT_MIN + OUTPUT_RANGE;
				nInternalIndex += nValueCount;
			}
		}
	}
	GAssert(nInternalIndex == m_pInternalRelation->GetAttributeCount(), "error");
}

void GNeuralNet::OutputsToExternal(double* pInternal, double* pExternal)
{
	GAssert(m_pMinAndRanges, "min and ranges not calculated yet");
	GArffAttribute* pAttr;
	int nValueCount;
	int nOutputCount = m_pRelation->GetOutputCount();
	int nInternalIndex = m_pInternalRelation->GetOutputIndex(0);
	int n, i, nExternalIndex;
	double dVal, dHighestVal;
	for(n = 0; n < nOutputCount; n++)
	{
		nExternalIndex = m_pRelation->GetOutputIndex(n);
		pAttr = m_pRelation->GetAttribute(nExternalIndex);
		if(pAttr->IsContinuous())
			pExternal[nExternalIndex] = GArffData::Normalize(pInternal[nInternalIndex++], OUTPUT_MIN, OUTPUT_RANGE, m_pMinAndRanges[nExternalIndex + nExternalIndex], m_pMinAndRanges[nExternalIndex + nExternalIndex + 1]);
		else
		{
			nValueCount = pAttr->GetValueCount();
			if(nValueCount <= 2)
				pExternal[nExternalIndex] = (pInternal[nInternalIndex++] >= OUTPUT_MIDDLE ? 1 : 0);
			else
			{
				pExternal[nExternalIndex] = 0;
				dHighestVal = pInternal[nInternalIndex++];
				for(i = 1; i < nValueCount; i++)
				{
					dVal = pInternal[nInternalIndex++];
					if(dVal > dHighestVal)
					{
						pExternal[nExternalIndex] = i;
						dHighestVal = dVal;
					}
				}
			}
		}
	}
	GAssert(nInternalIndex == m_pInternalRelation->GetAttributeCount(), "error");
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

int GNeuralNet::GetWeightCount()
{
	if(m_nWeightCount == 0)
	{
		int n;
		int nCount = m_pNeurons->GetSize();
		GNeuron* pNeuron;
		for(n = 0; n < nCount; n++)
		{
			pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
			m_nWeightCount += pNeuron->SerializeWeights(NULL);
		}
	}
	return m_nWeightCount;
}

void GNeuralNet::GetWeights(double* pOutWeights)
{
	// Serialize the weights
	int nCount = m_pNeurons->GetSize();
	int nPos = 0;
	int n;
	GNeuron* pNeuron;
	for(n = 0; n < nCount; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		nPos += pNeuron->SerializeWeights(&pOutWeights[nPos]);
	}
	GAssert(nPos == m_nWeightCount, "serialization size inconsistent");
}

void GNeuralNet::SetWeights(double* pWeights)
{
	int n;
	int nCount = m_pNeurons->GetSize();
	GNeuron* pNeuron;
	int nPos = 0;
	for(n = 0; n < nCount; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		nPos += pNeuron->DeserializeWeights(&pWeights[nPos]);
	}
	GAssert(nPos == m_nWeightCount, "serialization size inconsistent");
}

void GNeuralNet::UpdateBestWeights()
{
	if(!m_pBestSet)
		m_pBestSet = new double[GetWeightCount()];
	GetWeights(m_pBestSet);
}

void GNeuralNet::RestoreBestWeights()
{
	SetWeights(m_pBestSet);
}

void GNeuralNet::EvalInternal(double* pRow)
{
	// Clear the outputs of all non-input neurons
	GNeuron* pNeuron;
	int n;
	for(n = 0; n < m_nInputStart; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		pNeuron->SetOutput(1e50);
	}

	// Copy inputs into input neurons
	int nInputs = m_pInternalRelation->GetInputCount();
	int nCount = m_nInputStart + nInputs;
	GAssert(nCount == m_pNeurons->GetSize(), "neurons added after input neurons?");
	int nInput = 0;
	for( ; n < nCount; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		pNeuron->SetOutput(pRow[nInput++]);
	}

	// Pull the evaluation downstream to the output nodes
	int nOutputs = m_pInternalRelation->GetOutputCount();
	for(n = 0; n < nOutputs; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		pNeuron->PullEvalDownStream();
	}
}

void GNeuralNet::Eval(double* pRow)
{
	// Convert to internal data
	double* pInternalRow = (double*)alloca(sizeof(double) * m_pInternalRelation->GetAttributeCount());
	InputsToInternal(pRow, pInternalRow);

	// Do the evaluation
	EvalInternal(pInternalRow);

	// Extract the output values from the output nodes
	GNeuron* pNeuron;
	int n;
	int nOutputs = m_pInternalRelation->GetOutputCount();
	int nIndex = m_pInternalRelation->GetOutputIndex(0);
	for(n = 0; n < nOutputs; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		pInternalRow[nIndex++] = pNeuron->GetOutput();
	}

	// Convert outputs to external data
	OutputsToExternal(pInternalRow, pRow);
}

void GNeuralNet::Criticize(double* pModel)
{
	// Calculate the error on all output nodes
	GNeuron* pNeuron = NULL;
	int n;
	double dOutput;
	int nOutputs = m_pInternalRelation->GetOutputCount();
	int nIndex = m_pInternalRelation->GetOutputIndex(0);
	for(n = 0; n < nOutputs; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		dOutput = pNeuron->GetOutput();
		pNeuron->SetError((pModel[nIndex++] - dOutput) * dOutput * (1.0 - dOutput));
	}

	// Clear the error on the rest of the nodes
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

void GNeuralNet::MeasureMinAndRanges(GArffData* pTrainingData)
{
	int nAttrCount = m_pRelation->GetAttributeCount();
	delete(m_pMinAndRanges);
	m_pMinAndRanges = new double[2 * nAttrCount];
	GArffAttribute* pAttr;
	int n;
	for(n = 0; n < nAttrCount; n++)
	{
		pAttr = m_pRelation->GetAttribute(n);
		if(pAttr->IsContinuous())
		{
			pTrainingData->GetMinAndRange(n, &m_pMinAndRanges[2 * n], &m_pMinAndRanges[2 * n + 1]);
			if(m_pMinAndRanges[2 * n + 1] < .00001)
				m_pMinAndRanges[2 * n + 1] = .00001;
		}
		else
		{
			m_pMinAndRanges[2 * n] = 0;
			m_pMinAndRanges[2 * n + 1] = 0;
		}
	}
}

void GNeuralNet::ExternalToInternalData(GArffData* pExternal, GArffData* pInternal)
{
	double* pExternalRow;
	double* pInternalRow;
	int n;
	int nInternalAttributeCount = m_pInternalRelation->GetAttributeCount();
	int nRowCount = pExternal->GetRowCount();
	for(n = 0; n < nRowCount; n++)
	{
		pExternalRow = pExternal->GetRow(n);
		pInternalRow = new double[nInternalAttributeCount];
		InputsToInternal(pExternalRow, pInternalRow);
		OutputsToInternal(pExternalRow, pInternalRow);
		pInternal->AddRow(pInternalRow);
	}
}

double GNeuralNet::TrainValidate()
{
	int n, i, nIndex;
	GNeuron* pNeuron;
	double* pRow;
	double d;
	double dError = 0;
	int nCount = m_pValidationDataInternal->GetRowCount();
	int nOutputs = m_pInternalRelation->GetOutputCount();
	for(n = 0; n < nCount; n++)
	{
		pRow = m_pValidationDataInternal->GetRow(n);
		EvalInternal(pRow);
		nIndex = m_pInternalRelation->GetOutputIndex(0);
		for(i = 0; i < nOutputs; i++)
		{
			pNeuron = (GNeuron*)m_pNeurons->GetPointer(i);
			d = pRow[nIndex++] - pNeuron->GetOutput();
			d *= d;
			dError += d;
		}
	}
	dError /= (nCount * nOutputs);
	return dError;
}

void GNeuralNet::PrintNeurons()
{
	printf("-----------------\n");
	GNeuron* pNeuron;
	int n;
	for(n = 0; n < m_nInputStart; n++)
	{
		pNeuron = (GNeuron*)m_pNeurons->GetPointer(n);
		printf("Neuron %d\n", n);
		pNeuron->Print();
	}
	printf("-----------------\n");
}

void GNeuralNet::Train(GArffData* pData)
{
	int nTrainRows = (int)(m_dTrainingPortion * pData->GetRowCount());
	GArffData* pValidateData = pData->SplitBySize(nTrainRows);
	Train(pData, pValidateData);
}

int GNeuralNet::Train(GArffData* pTrainingData, GArffData* pValidationData)
{
	TrainInit(pTrainingData, pValidationData);

	// Do the epochs
	int nEpochs;
	double dBestError = 1e20;
	int nEpochsSinceValidationCheck = 0;
	int nBestEpoch = 0;
	for(nEpochs = 0; true; nEpochs++)
	{
		TrainEpoch();

		// Check for termination condition
		nEpochsSinceValidationCheck++;
		if(nEpochsSinceValidationCheck >= m_nEpochsPerValidationCheck)
		{
			nEpochsSinceValidationCheck = 0;
			double dMeanSquareError = TrainValidate();
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
				if(nEpochs - nBestEpoch >= m_nRunEpochs)
					break;
			}
			if(nEpochs >= m_nMaximumEpochs)
				break;
		}
	}
	if(dBestError < 1e20)
		RestoreBestWeights();
	else
	{
		GAssert(false, "Total failure!");
	}
	ReleaseInternalData();
	return nEpochs;
}

void GNeuralNet::TrainInit(GArffData* pTrainingData, GArffData* pValidationData)
{
	GAssert(m_nRunEpochs <= m_nMaximumEpochs, "conflicting settings");

	// Add the input layer
	MakeInputLayer();

	// Make the internal data
	MeasureMinAndRanges(pTrainingData);
	ReleaseInternalData();
	m_pTrainingDataInternal = new GArffData(pTrainingData->GetRowCount());
	ExternalToInternalData(pTrainingData, m_pTrainingDataInternal);
	if(pTrainingData == pValidationData)
		m_pValidationDataInternal = m_pTrainingDataInternal;
	else
	{
		m_pValidationDataInternal = new GArffData(pValidationData->GetRowCount());
		ExternalToInternalData(pValidationData, m_pValidationDataInternal);
	}
}

void GNeuralNet::TrainEpoch()
{
	// Do a single epoch
	double* pRow;
	GNeuron* pNeuron;
	int n, i;
	int nRowCount = m_pTrainingDataInternal->GetRowCount();
	for(n = 0; n < nRowCount; n++)
	{
		// Compute output for this row and update the weights
		pRow = m_pTrainingDataInternal->GetRow(n);
		EvalInternal(pRow);

		// Backpropagate the error
		Criticize(pRow);

		// Ajust the weights in a gradient descent manner
		for(i = 0; i < m_nInputStart; i++)
		{
			pNeuron = (GNeuron*)m_pNeurons->GetPointer(i);
			pNeuron->AjustWeights(m_dLearningRate, m_dMomentum);
		}
	}
	m_dLearningRate *= m_dLearningDecay;
	m_pTrainingDataInternal->ShuffleRows();
}

int GNeuralNet::TrainBatch(GArffData* pTrainingData, GArffData* pValidationData)
{
	TrainInit(pTrainingData, pValidationData);

	// Do the epochs
	double* pRow;
	GNeuron* pNeuron;
	int n, i, nEpochs;
	double dBestError = 1e20;
	int nRowCount = m_pTrainingDataInternal->GetRowCount();
	int nEpochsSinceValidationCheck = 0;
	int nBestEpoch = 0;
	for(nEpochs = 0; true; nEpochs++)
	{
		// Train with each of the training examples (one epoch)
		for(n = 0; n < nRowCount; n++)
		{
			// Compute output for this row and update the weights
			pRow = m_pTrainingDataInternal->GetRow(n);
			EvalInternal(pRow);

			// Backpropagate the error
			Criticize(pRow);

			// Ajust the weight delta in a gradient descent manner
			for(i = 0; i < m_nInputStart; i++)
			{
				pNeuron = (GNeuron*)m_pNeurons->GetPointer(i);
				pNeuron->BatchUpdateDeltas(m_dLearningRate);
			}
		}

		// Ajust the weights by the sum weight delta
		for(i = 0; i < m_nInputStart; i++)
		{
			pNeuron = (GNeuron*)m_pNeurons->GetPointer(i);
			pNeuron->BatchUpdateWeights();
		}
		m_dLearningRate *= m_dLearningDecay;

		// Check for termination condition
		nEpochsSinceValidationCheck++;
		if(nEpochsSinceValidationCheck >= m_nEpochsPerValidationCheck)
		{
			nEpochsSinceValidationCheck = 0;
			double dMeanSquareError = TrainValidate();
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
				if(nEpochs - nBestEpoch >= m_nRunEpochs)
					break;
			}
			if(nEpochs >= m_nMaximumEpochs)
				break;
		}
	}
	if(dBestError < 1e20)
		RestoreBestWeights();
	else
	{
		GAssert(false, "Total failure!");
	}
	ReleaseInternalData();
	return nEpochs;
}

