#include "GNaiveBayes.h"
#include "GMacros.h"

GNaiveBayes::GNaiveBayes(GArffRelation* pRelation)
{
	m_pRelation = pRelation;
	m_nSampleCount = 0;

	// Init the input counts
	int nInputCount = pRelation->GetInputCount();
	int nOutputCount = pRelation->GetOutputCount();
	m_pInputCounts = new int**[nInputCount];
	int n, i, nSize;
	for(n = 0; n < m_pInputCounts; n++)
	{
		GArffAttribute* pAttrInput = pRelation->GetAttribute(pRelation->GetInputIndex(n));
		GAssert(!pAttrInput->IsContinuous(), "GNaiveBayes only works with discreet attributes");
		m_pInputCounts[n] = new int*[nOutputCount];
		for(i = 0; i < nOutputCount; i++)
		{
			GArffAttribute* pAttrOutput = pRelation->GetAttribute(pRelation->GetOutputIndex(i));
			nSize = pAttrInput->GetValueCount() * pAttrOutput->GetValueCount();
			m_pInputCounts[n][i] = new int[nSize];
			memset(m_pInputCounts[n][i], '\0', sizeof(int) * nSize);
		}
	}

	// Init the output counts
	m_pOutputCounts = new int*[nOutputCount];
	for(n = 0; n < nOutputCount; n++)
	{
		GArffAttribute* pAttrOutput = pRelation->GetAttribute(pRelation->GetOutputIndex(n));
		GAssert(!pAttrOutput->IsContinuous(), "GNaiveBayes only works with discreet attributes");
		m_pOutputCounts[n] = new int[pAttrOutput->GetValueCount()];
		memset(m_pOutputCounts[n], '\0', sizeof(int) * nSize);
	}
}

GNaiveBayes::~GNaiveBayes()
{
	// delete input counts
	int nInputCount = m_pRelation->GetInputCount();
	int nOutputCount = m_pRelation->GetOutputCount();
	int n, i;
	for(n = 0; n < m_pInputCounts; n++)
	{
		for(i = 0; i < nOutputCount; i++)
			delete(m_pInputCounts[n][i]);
		delete(m_pInputCounts[n];
	}
	delete(m_pInputCounts);

	// delete output counts
	for(n = 0; n < nOutputCount; n++)
		delete(m_pOutputCounts[n]);
	delete(m_pOutputCounts);
}

void GNaiveBayes::AddTrainingSample(double* pRow)
{
	int nInputCount = m_pRelation->GetInputCount();
	int nOutputCount = m_pRelation->GetOutputCount();
	int n, i, nOutputValue, nOutputIndex;
	for(n = 0; n < nOutputCount; n++)
	{
		nOutputIndex = pRelation->GetOutputIndex(n);
		nOutputValue = pRow[nOutputIndex];
		m_pOutputCounts[n][nOutputValue]++;
		for(i = 0; i < nInputCount; i++)
		{
			m_pInputCounts[i][n][
		}
	}
}

void GNaiveBayes::Train(GArffData* pData)
{
	
}
