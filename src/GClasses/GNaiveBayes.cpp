#include "GNaiveBayes.h"
#include "GMacros.h"
#include "GArff.h"
#include "GXML.h"
#include "GArray.h"

struct GNaiveBayesInputAttr
{
	int m_nIndex;
	int m_nValues;
	int* m_pValueCounts;

	GNaiveBayesInputAttr(GArffRelation* pRelation, int nInput)
	{
		m_nIndex = pRelation->GetInputIndex(nInput);
		GArffAttribute* pAttrInput = pRelation->GetAttribute(m_nIndex);
		GAssert(!pAttrInput->IsContinuous(), "only discreet values are supported");
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
		m_pValueCounts[nValue]++;
	}

	int Eval(double* pRow)
	{
		int nValue = (int)pRow[m_nIndex];
		return m_pValueCounts[nValue];
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

	GNaiveBayesOutputValue(GArffRelation* pRelation, int nValue)
	{
		m_nCount = 0;
		m_nInputs = pRelation->GetInputCount();
		m_pInputs = new struct GNaiveBayesInputAttr*[m_nInputs];
		int n;
		for(n = 0; n < m_nInputs; n++)
			m_pInputs[n] = new struct GNaiveBayesInputAttr(pRelation, n);
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
		double dProb = m_nCount;
		double dFac = (double)m_nInputs / m_nCount;
		int n;
		for(n = 0; n < m_nInputs; n++)
		{
			dProb *= (m_pInputs[n]->Eval(pRow) + ((double)nEquivalentSampleSize / m_pInputs[n]->m_nValues));
			dProb /= (m_nCount + nEquivalentSampleSize);
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

	GNaiveBayesOutputAttr(GArffRelation* pRelation, int nOutput)
	{
		m_nIndex = pRelation->GetOutputIndex(nOutput);
		GArffAttribute* pAttrOutput = pRelation->GetAttribute(m_nIndex);
		GAssert(!pAttrOutput->IsContinuous(), "only discreet values are supported");
		m_nValues = pAttrOutput->GetValueCount();
		m_pValues = new struct GNaiveBayesOutputValue*[m_nValues];
		int n;
		for(n = 0; n < m_nValues; n++)
			m_pValues[n] = new struct GNaiveBayesOutputValue(pRelation, n);
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
		return dBestProbability / dTotalProbability;
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
{
	m_nEquivalentSampleSize = 3;
	m_nSampleCount = 0;
	m_nOutputs = pRelation->GetOutputCount();
	m_pOutputs = new struct GNaiveBayesOutputAttr*[m_nOutputs];
	int n;
	for(n = 0; n < m_nOutputs; n++)
		m_pOutputs[n] = new struct GNaiveBayesOutputAttr(pRelation, n);
}

GNaiveBayes::GNaiveBayes(GXMLTag* pTag)
{
	m_nEquivalentSampleSize = 3;
	m_nSampleCount = atoi(pTag->GetAttribute("Samples")->GetValue());
	m_nOutputs = pTag->GetChildTagCount();
	m_pOutputs = new struct GNaiveBayesOutputAttr*[m_nOutputs];
	GXMLTag* pChildTag = pTag->GetFirstChildTag();
	int n;
	for(n = 0; n < m_nOutputs; n++)
	{
		m_pOutputs[n] = new struct GNaiveBayesOutputAttr(pChildTag);
		pChildTag = pTag->GetNextChildTag(pChildTag);
	}
}

GNaiveBayes::~GNaiveBayes()
{
	int n;
	for(n = 0; n < m_nOutputs; n++)
		delete(m_pOutputs[n]);
	delete(m_pOutputs);
}

void GNaiveBayes::AddTrainingSample(double* pRow)
{
	int n;
	for(n = 0; n < m_nOutputs; n++)
		m_pOutputs[n]->AddTrainingSample(pRow);
	m_nSampleCount++;
}

void GNaiveBayes::Train(GArffData* pData)
{
	int nCount = pData->GetRowCount();
	int n;
	double* pRow;
	for(n = 0; n < nCount; n++)
	{
		pRow = pData->GetRow(n);
		AddTrainingSample(pRow);
	}
}

double GNaiveBayes::Eval(double* pRow)
{
	GAssert(m_nSampleCount > 0, "no data");
	int n;
	double dConfidence = 1;
	for(n = 0; n < m_nOutputs; n++)
		dConfidence *= m_pOutputs[n]->Eval(pRow, m_nEquivalentSampleSize);
	return dConfidence;
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
