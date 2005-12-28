#include "GNaiveBayes.h"
#include "GMacros.h"
#include "GArff.h"
#include "GXML.h"
#include "GArray.h"
#include <math.h>

struct GNaiveBayesInputAttr
{
	int m_nIndex;
	int m_nValues;
	int* m_pValueCounts;

	GNaiveBayesInputAttr(GArffRelation* pRelation, int nInput, int nDiscretizeBuckets)
	{
		m_nIndex = pRelation->GetInputIndex(nInput);
		GArffAttribute* pAttrInput = pRelation->GetAttribute(m_nIndex);
		if(pAttrInput->IsContinuous())
			m_nValues = nDiscretizeBuckets;
		else
			m_nValues = pAttrInput->GetValueCount();
		m_pValueCounts = new int[m_nValues];
		memset(m_pValueCounts, '\0', sizeof(int) * m_nValues);
	}

	GNaiveBayesInputAttr(GXMLTag* pTag, int nIndex)
	{
		m_nIndex = nIndex;
		m_nValues = pTag->GetAttributeCount();
		m_pValueCounts = new int[m_nValues];
		char szTmp1[33];
		szTmp1[0] = 'v';
		GXMLAttribute* pAttr;
		int n;
		for(n = 0; n < m_nValues; n++)
		{
			itoa(n, szTmp1 + 1, 10);
			pAttr = pTag->GetAttribute(szTmp1);
			m_pValueCounts[n] = atoi(pAttr->GetValue());
		}
	}

	~GNaiveBayesInputAttr()
	{
		delete(m_pValueCounts);
	}

	void AddTrainingSample(double* pRow)
	{
		int nValue = (int)pRow[m_nIndex];
		if(nValue >= 0 && nValue < m_nValues)
			m_pValueCounts[nValue]++;
	}

	int Eval(double* pRow)
	{
		int nValue = (int)pRow[m_nIndex];
		if(nValue >= 0 && nValue <= m_nValues)
			return m_pValueCounts[nValue];
		else
			return 0;
	}

	GXMLTag* ToXml(const char* szName)
	{
		GXMLTag* pTag = new GXMLTag(szName);
		char szTmp1[33];
		char szTmp2[32];
		//itoa(m_nIndex, szTmp2, 10);
		//pTag->AddAttribute(new GXMLAttribute("Index", szTmp2));
		szTmp1[0] = 'v';
		int n;
		for(n = 0; n < m_nValues; n++)
		{
			itoa(n, szTmp1 + 1, 10);
			itoa(m_pValueCounts[n], szTmp2, 10);
			pTag->AddAttribute(new GXMLAttribute(szTmp1, szTmp2));
		}
		return pTag;
	}
};

// --------------------------------------------------------------------

struct GNaiveBayesOutputValue
{
	int m_nCount;
	int m_nInputs;
	struct GNaiveBayesInputAttr** m_pInputs;

	GNaiveBayesOutputValue(GArffRelation* pRelation, int nValue, int nDiscretizeBuckets)
	{
		m_nCount = 0;
		m_nInputs = pRelation->GetInputCount();
		m_pInputs = new struct GNaiveBayesInputAttr*[m_nInputs];
		int n;
		for(n = 0; n < m_nInputs; n++)
			m_pInputs[n] = new struct GNaiveBayesInputAttr(pRelation, n, nDiscretizeBuckets);
	}

	GNaiveBayesOutputValue(GXMLTag* pTag)
	{
		m_nCount = atoi(pTag->GetAttribute("Count")->GetValue());
		m_nInputs = pTag->GetChildTagCount();
		m_pInputs = new struct GNaiveBayesInputAttr*[m_nInputs];
		GXMLTag* pChildTag = pTag->GetFirstChildTag();
		int n;
		for(n = 0; n < m_nInputs; n++)
		{
			m_pInputs[n] = new struct GNaiveBayesInputAttr(pChildTag, n);
			pChildTag = pTag->GetNextChildTag(pChildTag);
		}
	}

	~GNaiveBayesOutputValue()
	{
		int n;
		for(n = 0; n < m_nInputs; n++)
			delete(m_pInputs[n]);
		delete(m_pInputs);
	}

	void AddTrainingSample(double* pRow)
	{
		int n;
		for(n = 0; n < m_nInputs; n++)
			m_pInputs[n]->AddTrainingSample(pRow);
		m_nCount++;
	}

	double Eval(double* pRow, int nEquivalentSampleSize)
	{
		double dProb = log((double)m_nCount);
		int n;
		for(n = 0; n < m_nInputs; n++)
		{
			dProb += log(
							(
								(double)m_pInputs[n]->Eval(pRow) + 
								((double)nEquivalentSampleSize / m_pInputs[n]->m_nValues)
							) / 
							(m_nCount + nEquivalentSampleSize)
						);
		}
		return dProb;
	}

	GXMLTag* ToXml(GPointerArray* pAttrNames)
	{
		GXMLTag* pTag = new GXMLTag("OutputValue");
		char szTmp[32];
		itoa(m_nCount, szTmp, 10);
		pTag->AddAttribute(new GXMLAttribute("Count", szTmp));
		const char* szAttrName;
		int n;
		for(n = 0; n < m_nInputs; n++)
		{
			GAssert(m_pInputs[n]->m_nIndex == n, "index mismatch");
			szAttrName = (const char*)pAttrNames->GetPointer(n);
			pTag->AddChildTag(m_pInputs[n]->ToXml(szAttrName));
		}
		return pTag;
	}
};

// --------------------------------------------------------------------

struct GNaiveBayesOutputAttr
{
	int m_nIndex;
	int m_nValues;
	struct GNaiveBayesOutputValue** m_pValues;

	GNaiveBayesOutputAttr(GArffRelation* pRelation, int nOutput, int nDiscretizeBuckets)
	{
		m_nIndex = pRelation->GetOutputIndex(nOutput);
		GArffAttribute* pAttrOutput = pRelation->GetAttribute(m_nIndex);
		if(pAttrOutput->IsContinuous())
			m_nValues = nDiscretizeBuckets;
		else
			m_nValues = pAttrOutput->GetValueCount();
		m_pValues = new struct GNaiveBayesOutputValue*[m_nValues];
		int n;
		for(n = 0; n < m_nValues; n++)
			m_pValues[n] = new struct GNaiveBayesOutputValue(pRelation, n, nDiscretizeBuckets);
	}

	GNaiveBayesOutputAttr(GXMLTag* pTag)
	{
		m_nIndex = atoi(pTag->GetAttribute("Index")->GetValue());
		m_nValues = pTag->GetChildTagCount();
		m_pValues = new struct GNaiveBayesOutputValue*[m_nValues];
		GXMLTag* pChildTag = pTag->GetFirstChildTag();
		int n;
		for(n = 0; n < m_nValues; n++)
		{
			m_pValues[n] = new struct GNaiveBayesOutputValue(pChildTag);
			pChildTag = pTag->GetNextChildTag(pChildTag);
		}
	}

	~GNaiveBayesOutputAttr()
	{
		int n;
		for(n = 0; n < m_nValues; n++)
			delete(m_pValues[n]);
		delete m_pValues;
	}

	void AddTrainingSample(double* pRow)
	{
		int nValue = (int)pRow[m_nIndex];
		if(nValue >= 0 && nValue < m_nValues)
			m_pValues[nValue]->AddTrainingSample(pRow);
	}

	double Eval(double* pRow, int nEquivalentSampleSize)
	{
		double dTotalProbability = 0;
		double dBestProbability = 0;
		double dProb;
		int nBestOutputValue = -1;
		int n;
		for(n = 0; n < m_nValues; n++)
		{
			dProb = m_pValues[n]->Eval(pRow, nEquivalentSampleSize);
			if(nBestOutputValue < 0 || dProb > dBestProbability)
			{
				nBestOutputValue = n;
				dBestProbability = dProb;
			}
			dTotalProbability += dProb;
		}
		pRow[m_nIndex] = (double)nBestOutputValue;
		return exp(dBestProbability - dTotalProbability);
	}

	GXMLTag* ToXml(GPointerArray* pAttrNames)
	{
		GXMLTag* pTag = new GXMLTag("OutputAttr");
		char szTmp[32];
		itoa(m_nIndex, szTmp, 10);
		pTag->AddAttribute(new GXMLAttribute("Index", szTmp));
		int n;
		for(n = 0; n < m_nValues; n++)
			pTag->AddChildTag(m_pValues[n]->ToXml(pAttrNames));
		return pTag;
	}
};

// --------------------------------------------------------------------

GNaiveBayes::GNaiveBayes(GArffRelation* pRelation)
: GSupervisedLearner(pRelation)
{
	m_nEquivalentSampleSize = 3;
	m_nSampleCount = 0;
	m_nOutputs = pRelation->GetOutputCount();
	m_pOutputs = new struct GNaiveBayesOutputAttr*[m_nOutputs];
	int n;
	if(pRelation->CountContinuousAttributes() > 0)
	{
		int nAttributes = m_pRelation->GetAttributeCount();
		m_pDiscretizeMins = new double[nAttributes];
		m_pDiscretizeRanges = new double[nAttributes];
		for(n = 0; n < nAttributes; n++)
		{
			m_pDiscretizeMins[n] = 0;
			m_pDiscretizeRanges[n] = 1;
		}
		m_nDiscretizeBuckets = 10;
	}
	else
	{
		m_pDiscretizeMins = NULL;
		m_pDiscretizeRanges = NULL;
		m_nDiscretizeBuckets = 0;
	}
	for(n = 0; n < m_nOutputs; n++)
		m_pOutputs[n] = new struct GNaiveBayesOutputAttr(pRelation, n, m_nDiscretizeBuckets);
}

GNaiveBayes::GNaiveBayes(GXMLTag* pTag)
: GSupervisedLearner(NULL)
{
	m_nEquivalentSampleSize = 3;
	m_nSampleCount = atoi(pTag->GetAttribute("Samples")->GetValue());
	m_nOutputs = pTag->GetChildTagCount();
	m_pOutputs = new struct GNaiveBayesOutputAttr*[m_nOutputs];
	GXMLTag* pChildTag = pTag->GetFirstChildTag();
	m_pDiscretizeMins = NULL;
	m_pDiscretizeRanges = NULL;
	m_nDiscretizeBuckets = 0;
	int n;
	for(n = 0; n < m_nOutputs; n++)
	{
		m_pOutputs[n] = new struct GNaiveBayesOutputAttr(pChildTag);
		pChildTag = pTag->GetNextChildTag(pChildTag);
		m_pDiscretizeMins[n] = 0;
		m_pDiscretizeRanges[n] = 1;
	}
}

GNaiveBayes::~GNaiveBayes()
{
	int n;
	for(n = 0; n < m_nOutputs; n++)
		delete(m_pOutputs[n]);
	delete(m_pOutputs);
//	delete(m_pDiscretizeMins);
//	delete(m_pDiscretizeRanges);
}

void GNaiveBayes::DiscretizeRow(double* pRow)
{
	int nAttributes = m_pRelation->GetAttributeCount();
	int i;
	GArffAttribute* pAttr;
	for(i = 0; i < nAttributes; i++)
	{
		pAttr = m_pRelation->GetAttribute(i);
		if(pAttr->IsContinuous())
			pRow[i] = GArffData::Normalize(pRow[i], m_pDiscretizeMins[i], m_pDiscretizeRanges[i], .5, m_nDiscretizeBuckets); // the .5 is so that when we cast from double to int, it will round to the nearest discreet value
	}
}

void GNaiveBayes::UndiscretizeRow(double* pRow)
{
	int nAttributes = m_pRelation->GetAttributeCount();
	int i;
	GArffAttribute* pAttr;
	for(i = 0; i < nAttributes; i++)
	{
		pAttr = m_pRelation->GetAttribute(i);
		if(pAttr->IsContinuous())
			pRow[i] = GArffData::Normalize(pRow[i], .5, m_nDiscretizeBuckets, m_pDiscretizeMins[i], m_pDiscretizeRanges[i]);
	}
}

void GNaiveBayes::AddTrainingSample(double* pRow)
{
	if(m_pDiscretizeMins)
		DiscretizeRow(pRow);
	int n;
	for(n = 0; n < m_nOutputs; n++)
		m_pOutputs[n]->AddTrainingSample(pRow);
	m_nSampleCount++;
	if(m_pDiscretizeMins)
		UndiscretizeRow(pRow);
}

void GNaiveBayes::ComputeDiscretizeRanges(GArffData* pData)
{
	int i;
	GArffAttribute* pAttr;
	int nAttributes = m_pRelation->GetAttributeCount();
	for(i = 0; i < nAttributes; i++)
	{
		pAttr = m_pRelation->GetAttribute(i);
		if(pAttr->IsContinuous())
			pData->GetMinAndRange(i, &m_pDiscretizeMins[i], &m_pDiscretizeRanges[i]);
		if(m_pDiscretizeRanges[i] < .00001)
			m_pDiscretizeRanges[i] = .00001;
	}
}

void GNaiveBayes::Train(GArffData* pData)
{
	if(m_pDiscretizeMins)
		ComputeDiscretizeRanges(pData);
	int nCount = pData->GetRowCount();
	int n;
	double* pRow;
	for(n = 0; n < nCount; n++)
	{
		pRow = pData->GetRow(n);
		AddTrainingSample(pRow);
	}
}

double GNaiveBayes::EvalWithConfidence(double* pRow)
{
	GAssert(m_nSampleCount > 0, "no data");
	int n;
	double dConfidence = 1;
	if(m_pDiscretizeMins)
		DiscretizeRow(pRow);
	for(n = 0; n < m_nOutputs; n++)
		dConfidence *= m_pOutputs[n]->Eval(pRow, m_nEquivalentSampleSize);
	if(m_pDiscretizeMins)
		UndiscretizeRow(pRow);
	return dConfidence;
}

void GNaiveBayes::Eval(double* pRow)
{
	EvalWithConfidence(pRow);
}

GXMLTag* GNaiveBayes::ToXml(GPointerArray* pAttrNames)
{
	GXMLTag* pTag = new GXMLTag("GNaiveBayes");
	char szTmp[32];
	itoa(m_nSampleCount, szTmp, 10);
	pTag->AddAttribute(new GXMLAttribute("Samples", szTmp));
	int n;
	for(n = 0; n < m_nOutputs; n++)
		pTag->AddChildTag(m_pOutputs[n]->ToXml(pAttrNames));
	return pTag;
}
