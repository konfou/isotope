/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GArff.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GMacros.h"
#include "../GClasses/GMath.h"
#include <math.h>
#include "GBits.h"
#include "GMatrix.h"

GArffRelation::GArffRelation()
{
	m_szName = NULL;
	m_pAttributes = new GPointerArray(32);
	m_nInputCount = -1;
	m_pInputIndexes = NULL;
	m_nOutputCount = -1;
	m_pOutputIndexes = NULL;
}

GArffRelation::~GArffRelation()
{
	int n;
	int nCount;
	nCount = m_pAttributes->GetSize();
	for(n = 0; n < nCount; n++)
		delete((GArffAttribute*)m_pAttributes->GetPointer(n));
	delete(m_pAttributes);
	delete(m_szName);
	delete(m_pInputIndexes);
	delete(m_pOutputIndexes);
}

void GArffRelation::AddAttribute(GArffAttribute* pAttr)
{
	m_pAttributes->AddPointer(pAttr);
}

GArffRelation* ParseError(int nLine, const char* szProblem)
{
	GAssert(false, szProblem);
	return NULL;
}

int GArffRelation::CountContinuousAttributes()
{
	int n;
	int nAttributes = GetAttributeCount();
	int nCount = 0;
	for(n = 0; n < nAttributes; n++)
	{
		GArffAttribute* pAttr = GetAttribute(n);
		if(pAttr->IsContinuous())
			nCount++;
	}
	return nCount;
}

/*static*/ GArffRelation* GArffRelation::ParseFile(GArffData** ppOutData, const char* szFile, int nLen)
{
	// Parse the relation name
	int nPos = 0;
	int nLine = 1;
	Holder<GArffRelation*> hRelation(new GArffRelation());
	GArffRelation* pRelation = hRelation.Get();
	while(true)
	{
		// Skip Whitespace
		while(nPos < nLen && szFile[nPos] <= ' ')
		{
			if(szFile[nPos] == '\n')
				nLine++;
			nPos++;
		}
		if(nPos >= nLen)
			return ParseError(nLine, "Expected @RELATION");

		// Check for comments
		if(szFile[nPos] == '%')
		{
			for(nPos++; szFile[nPos] != '\n' && nPos < nLen; nPos++)
			{
			}
			continue;
		}

		// Parse Relation
		if(nLen - nPos < 9 || strnicmp(&szFile[nPos], "@RELATION", 9) != 0)
			return ParseError(nLine, "Expected @RELATION");
		nPos += 9;

		// Skip Whitespace
		while(szFile[nPos] <= ' ' && nPos < nLen)
		{
			if(szFile[nPos] == '\n')
				nLine++;
			nPos++;
		}
		if(nPos >= nLen)
			return ParseError(nLine, "Expected relation name");

		// Parse Name
		int nNameStart = nPos;
		while(szFile[nPos] > ' ' && nPos < nLen)
			nPos++;
		pRelation->m_szName = new char[nPos - nNameStart + 1];
		memcpy(pRelation->m_szName, &szFile[nNameStart], nPos - nNameStart);
		pRelation->m_szName[nPos - nNameStart] = '\0';
		break;
	}

	// Parse the attribute section
	while(true)
	{
		// Skip Whitespace
		while(nPos < nLen && szFile[nPos] <= ' ')
		{
			if(szFile[nPos] == '\n')
				nLine++;
			nPos++;
		}
		if(nPos >= nLen)
			return ParseError(nLine, "Expected @ATTRIBUTE or @DATA");

		// Check for comments
		if(szFile[nPos] == '%')
		{
			for(nPos++; szFile[nPos] != '\n' && nPos < nLen; nPos++)
			{
			}
			continue;
		}

		// Check for @DATA
		if(nLen - nPos < 5) // 10 = strlen("@DATA")
			return ParseError(nLine, "Expected @DATA");
		if(strnicmp(&szFile[nPos], "@DATA", 5) == 0)
		{
			nPos += 5;
			break;
		}

		// Parse @ATTRIBUTE
		if(nLen - nPos < 10) // 10 = strlen("@ATTRIBUTE")
			return ParseError(nLine, "Expected @ATTRIBUTE");
		if(strnicmp(&szFile[nPos], "@ATTRIBUTE", 10) != 0)
			return ParseError(nLine, "Expected @ATTRIBUTE or @DATA");
		nPos += 10;
		GArffAttribute* pAttr = GArffAttribute::Parse(&szFile[nPos], nLen - nPos);
		if(!pAttr)
			return ParseError(nLine, "Problem with attribute");
		pRelation->m_pAttributes->AddPointer(pAttr);

		// Move to next line
		for(nPos++; szFile[nPos] != '\n' && nPos < nLen; nPos++)
		{
		}
	}

	// Parse the data section
	Holder<GArffData*> hData(new GArffData(256));
	GArffData* pData = hData.Get();
	while(true)
	{
		// Skip Whitespace
		while(nPos < nLen && szFile[nPos] <= ' ')
		{
			if(szFile[nPos] == '\n')
				nLine++;
			nPos++;
		}
		if(nPos >= nLen)
			break;

		// Check for comments
		if(szFile[nPos] == '%')
		{
			for(nPos++; szFile[nPos] != '\n' && nPos < nLen; nPos++)
			{
			}
			continue;
		}

		// Parse the data line
		double* pRow = pRelation->ParseDataRow(&szFile[nPos], nLen - nPos);
		if(!pRow)
			return ParseError(nLine, "Problem with data line");
		pData->AddRow(pRow);

		// Move to next line
		for(nPos++; szFile[nPos] != '\n' && nPos < nLen; nPos++)
		{
		}
		continue;
	}

	*ppOutData = hData.Drop();
	return hRelation.Drop();
}

/*static*/ double* GArffRelation::ParseDataRow(const char* szFile, int nLen)
{
	char szBuf[512];
	int nAttributeCount = GetAttributeCount();
	Holder<double*> hData(new double[nAttributeCount]);
	double* pData = hData.Get();
	GArffAttribute* pAttr;
	int n;
	for(n = 0; n < nAttributeCount; n++)
	{
		// Eat whitespace
		while(nLen > 0 && *szFile <= ' ')
		{
			if(*szFile == '\n')
				return NULL;
			szFile++;
			nLen--;
		}
		if(nLen < 1)
			return NULL;

		// Parse the next value
		pAttr = GetAttribute(n);
		int nPos;
		for(nPos = 0; nPos < nLen && szFile[nPos] != ',' && szFile[nPos] != '\n'; nPos++)
		{
		}
		int nEnd;
		for(nEnd = nPos; nEnd > 0 && szFile[nEnd - 1] <= ' '; nEnd--)
		{
		}
		memcpy(szBuf, szFile, nEnd);
		szBuf[nEnd] = '\0';
		if(strcmp(szBuf, "?") == 0)
			pData[n] = -1;
		else if(pAttr->IsContinuous())
		{
			// Parse a continuous value
			if(szBuf[0] == '.' || szBuf[0] == '-' || (szBuf[0] >= '0' && szBuf[0] <= '9'))
				pData[n] = atof(szBuf);
			else
				return NULL;
		}
		else
		{
			// Parse an enumerated value
			int nVal = pAttr->FindEnumeratedValue(szBuf);
			if(nVal < 0)
				return NULL;
			pData[n] = nVal;
		}

		// Advance past the attribute
		if(nPos < nLen)
			nPos++;
		while(nPos > 0)
		{
			szFile++;
			nPos--;
			nLen--;
		}
	}
	return hData.Drop();
}

int GArffRelation::GetAttributeCount()
{
	return m_pAttributes->GetSize();
}

GArffAttribute* GArffRelation::GetAttribute(int n)
{
	return (GArffAttribute*)m_pAttributes->GetPointer(n);
}

void GArffRelation::CountInputs()
{
	m_nInputCount = 0;
	m_nOutputCount = 0;
	int n;
	int nCount = GetAttributeCount();
	GArffAttribute* pAttr;
	for(n = 0; n < nCount; n++)
	{
		pAttr = GetAttribute(n);
		if(pAttr->IsInput())
			m_nInputCount++;
		else
			m_nOutputCount++;
	}
	GAssert(m_nInputCount > 0, "no inputs");
	//GAssert(m_nOutputCount > 0, "no outputs");
	delete(m_pInputIndexes);
	delete(m_pOutputIndexes);
	m_pInputIndexes = new int[m_nInputCount];
	m_pOutputIndexes = new int[m_nOutputCount];
	int nIn = 0;
	int nOut = 0;
	for(n = 0; n < nCount; n++)
	{
		pAttr = GetAttribute(n);
		if(pAttr->IsInput())
			m_pInputIndexes[nIn++] = n;
		else
			m_pOutputIndexes[nOut++] = n;
	}
}

int GArffRelation::GetInputCount()
{
	if(m_nInputCount < 0)
		CountInputs();
	return m_nInputCount;
}

int GArffRelation::GetOutputCount()
{
	if(m_nOutputCount < 0)
		CountInputs();
	return m_nOutputCount;
}

int GArffRelation::GetInputIndex(int n)
{
	if(!m_pInputIndexes)
		CountInputs();
	GAssert(n >= 0 && n < m_nInputCount, "out of range");
	return m_pInputIndexes[n];
}

int GArffRelation::GetOutputIndex(int n)
{
	if(!m_pOutputIndexes)
		CountInputs();
	GAssert(n >= 0 && n < m_nOutputCount, "out of range");
	return m_pOutputIndexes[n];
}

double GArffRelation::MeasureTotalOutputInfo(GArffData* pData)
{
	double dInfo = 0;
	int nOutputs = GetOutputCount();
	int n, nIndex;
	GArffAttribute* pAttr;
	for(n = 0; n < nOutputs; n++)
	{
		nIndex = GetOutputIndex(n);
		pAttr = GetAttribute(nIndex);
		if(pAttr->IsContinuous())
			dInfo += pData->ComputeVariance(pData->ComputeMean(nIndex), nIndex);
		else
			dInfo += pData->MeasureEntropy(this, nIndex);
	}
	return dInfo;
}

double GArffRelation::ComputeInputDistanceSquared(double* pRow1, double* pRow2)
{
	double dSum = 0;
	double d;
	int n, nIndex;
	for(n = 0; n < m_nInputCount; n++)
	{
		nIndex = GetInputIndex(n);
		if(GetAttribute(nIndex)->IsContinuous())
		{
			d = pRow2[nIndex] - pRow1[nIndex];
			dSum += (d * d);
		}
		else
		{
			if(pRow2[nIndex] != pRow1[nIndex])
				dSum += 1;
		}
	}
	return dSum;
}

double GArffRelation::ComputeScaledInputDistanceSquared(double* pRow1, double* pRow2, double* pInputScales)
{
	double dSum = 0;
	double d;
	int n, nIndex;
	for(n = 0; n < m_nInputCount; n++)
	{
		nIndex = GetInputIndex(n);
		if(GetAttribute(nIndex)->IsContinuous())
		{
			d = pRow2[nIndex] * pInputScales[n] - pRow1[nIndex] * pInputScales[n];
			dSum += (d * d);
		}
		else
		{
			if(pRow2[nIndex] != pRow1[nIndex])
				dSum += pInputScales[n];
		}
	}
	return dSum;
}

// ------------------------------------------------------------------



GArffAttribute::GArffAttribute()
{
	m_szName = NULL;
	m_nValues = 0;
	m_szValues = NULL;
	m_bIsInput = true;
}

GArffAttribute::GArffAttribute(bool bIsInput, int nValues, const char** szValues)
{
	m_szName = NULL;
	m_nValues = nValues;
	if(nValues == 0)
		m_szValues = NULL;
	else
	{
		if(szValues)
		{
			m_szValues = new char*[nValues];
			int n;
			for(n = 0; n < nValues; n++)
			{
				m_szValues[n] = new char[strlen(szValues[n]) + 1];
				strcpy(m_szValues[n], szValues[n]);
			}
		}
		else
			m_szValues = NULL;
	}
	m_bIsInput = bIsInput;
}

GArffAttribute::~GArffAttribute()
{
	delete(m_szName);
	if(m_szValues)
	{
		int n;
		for(n = 0; n < m_nValues; n++)
			delete(m_szValues[n]);
		delete(m_szValues);
	}
}

GArffAttribute* GArffAttribute::NewCopy()
{
	return new GArffAttribute(m_bIsInput, m_nValues, (const char**)m_szValues);
}

/*static*/ GArffAttribute* GArffAttribute::Parse(const char* szFile, int nLen)
{
	// Eat whitespace
	while(nLen > 0 && *szFile <= ' ')
	{
		if(*szFile == '\n')
			return NULL;
		szFile++;
		nLen--;
	}
	if(nLen < 1)
		return NULL;

	// Parse the name
	Holder<GArffAttribute*> hAttr(new GArffAttribute());
	GArffAttribute* pAttr = hAttr.Get();
	int nQuotes = 0;
	if(szFile[0] == '\'' || szFile[0] == '"')
		nQuotes = 1;
	int nPos = 1;
	for( ; nPos < nLen && (szFile[nPos] > ' ' || nQuotes > 0); nPos++)
	{
		if(szFile[nPos] == '\'' || szFile[nPos] == '"')
			nQuotes--;
	}
	pAttr->m_szName = new char[nPos + 1];
	memcpy(pAttr->m_szName, szFile, nPos);
	pAttr->m_szName[nPos] = '\0';

	// Eat whitespace
	while(nPos < nLen && szFile[nPos] <= ' ')
	{
		if(szFile[nPos] == '\n')
			return NULL;
		nPos++;
	}
	if(nPos >= nLen)
		return NULL;

	// Check for CONTINUOUS
	if(nLen - nPos >= 10 && strnicmp(&szFile[nPos], "CONTINUOUS", 10) == 0)
		return hAttr.Drop();
	if(nLen - nPos >= 7 && strnicmp(&szFile[nPos], "NUMERIC", 7) == 0)
		return hAttr.Drop();
	if(nLen - nPos >= 4 && strnicmp(&szFile[nPos], "REAL", 4) == 0)
		return hAttr.Drop();

	// Parse the values
	if(szFile[nPos] != '{')
		return NULL;
	nPos++;

	// Count the values
	int nCount = 1;
	int n;
	for(n = nPos; szFile[n] != '{' && szFile[n] != '\n' && n < nLen; n++)
	{
		if(szFile[n] == ',')
			nCount++;
	}

	// Parse the values
	pAttr->m_szValues = new char*[nCount];
	pAttr->m_nValues = nCount;
	int nValue = 0;
	for(n = nPos; szFile[n] != '}' && szFile[n] != '\n' && n < nLen; n++)
	{
		if(szFile[n] == ',')
		{
			int nStart = nPos;
			int nEnd = n;
			while(nStart < nEnd && szFile[nStart] <= ' ')
				nStart++;
			while(nStart < nEnd && szFile[nEnd - 1] <= ' ')
				nEnd--;
			pAttr->m_szValues[nValue] = new char[nEnd - nStart + 1];
			memcpy(pAttr->m_szValues[nValue], &szFile[nStart], nEnd - nStart);
			(pAttr->m_szValues[nValue])[nEnd - nStart] = '\0';
			nPos = n + 1;
			nValue++;
		}
	}
	int nStart = nPos;
	int nEnd = n;
	while(nStart < nEnd && szFile[nStart] <= ' ')
		nStart++;
	while(nStart < nEnd && szFile[nEnd - 1] <= ' ')
		nEnd--;
	pAttr->m_szValues[nValue] = new char[nEnd - nStart + 1];
	memcpy(pAttr->m_szValues[nValue], &szFile[nStart], nEnd - nStart);
	(pAttr->m_szValues[nValue])[nEnd - nStart] = '\0';
	if(szFile[n] != '}')
		return NULL;

	return hAttr.Drop();
}

void GArffAttribute::SetContinuous()
{
	if(m_szValues)
	{
		int n;
		for(n = 0; n < m_nValues; n++)
			delete(m_szValues[n]);
		delete(m_szValues);
	}
	m_szValues = NULL;
	m_nValues = 0;
}

int GArffAttribute::GetValueCount()
{
	return m_nValues;
}

const char* GArffAttribute::GetValue(int n)
{
	if(n < 0)
	{
		GAssert(n == -1, "out of range");
		return "<?>";
	}
	GAssert(n < m_nValues, "out of range");
	return m_szValues[n];
}

int GArffAttribute::FindEnumeratedValue(const char* szValue)
{
	GAssert(!IsContinuous(), "Not an enumerated attribute");
	int n;
	for(n = 0; n < m_nValues; n++)
	{
		if(strcmp(m_szValues[n], szValue) == 0)
			return n;
	}
	return -1;
}

// ------------------------------------------------------------------

GArffData::GArffData(GPointerArray* pRows)
{
	m_pRows = pRows;
}

GArffData::GArffData(int nGrowSize)
{
	m_pRows = new GPointerArray(nGrowSize);
}

GArffData::~GArffData()
{
	if(m_pRows)
	{
		int nCount = m_pRows->GetSize();
		int n;
		for(n = 0; n < nCount; n++)
			delete((double*)m_pRows->GetPointer(n));
		delete(m_pRows);
	}
}

int GArffData::GetRowCount()
{
	return m_pRows->GetSize();
}

double* GArffData::GetRow(int nRow)
{
	GAssert(nRow >= 0 && nRow < m_pRows->GetSize(), "out of range");
	return ((double*)m_pRows->GetPointer(nRow));
}

void GArffData::AddRow(double* pRow)
{
	m_pRows->AddPointer(pRow);
}

double* GArffData::DropRow(int nRow)
{
	int nCount = GetRowCount();
	double* pRow = GetRow(nRow);
	m_pRows->SetPointer(nRow, m_pRows->GetPointer(nCount - 1));
	m_pRows->DeleteCell(nCount - 1);
	return pRow;
}

void GArffData::DropAllRows()
{
	m_pRows->Clear();
}

void GArffData::ShuffleRows()
{
	// Swap every row with a randomely selected row
	int nCount = GetRowCount();
	int n, r;
	void* pTemp;
	for(n = nCount - 1; n > 0; n--)
	{
		r = rand() % n;
		pTemp = m_pRows->GetPointer(r);
		m_pRows->SetPointer(r, m_pRows->GetPointer(n));
		m_pRows->SetPointer(n, pTemp);
	}
}

double GArffData::MeasureEntropy(GArffRelation* pRelation, int nColumn)
{
	// Count the number of occurrences of each value
	GArffAttribute* pAttr = pRelation->GetAttribute(nColumn);
	GAssert(!pAttr->IsInput(), "Expected an output");
	GAssert(!pAttr->IsContinuous(), "MeasureEntropy doesn't work with continuous attributes");
	int nPossibleValues = pAttr->GetValueCount();
	int* pnCounts = (int*)alloca(nPossibleValues * sizeof(int));
	int nTotalCount = 0;
	memset(pnCounts, '\0', pAttr->GetValueCount() * sizeof(int));
	int n;
	int nRows = m_pRows->GetSize();
	for(n = 0; n < nRows; n++)
	{
		int nValue = (int)GetRow(n)[nColumn];
		if(nValue < 0)
		{
			GAssert(nValue == -1, "out of range");
			continue;
		}
		GAssert(nValue < nPossibleValues, "value out of range");
		pnCounts[nValue]++;
		nTotalCount++;
	}
	if(nTotalCount == 0)
		return 0;

	// Total up the entropy
	double dLog2 = log((double)2);
	double dEntropy = 0;
	double dRatio;
	for(n = 0; n < nPossibleValues; n++)
	{
		if(pnCounts[n] > 0)
		{
			dRatio = (double)pnCounts[n] / nTotalCount;
			dEntropy -= (dRatio * log(dRatio) / dLog2);
		}
	}
	return dEntropy;
}

GArffData* GArffData::SplitByPivot(int nColumn, double dPivot)
{
	GArffData* pNewSet = new GArffData(MAX(8, GetRowCount()));
	double* pRow;
	int n;
	for(n = 0; n < GetRowCount(); n++)
	{
		pRow = GetRow(n);
		if(pRow[nColumn] <= dPivot)
		{
			pNewSet->AddRow(DropRow(n));
			n--;
		}
	}
	return pNewSet;
}

int DoubleRefComparer(void* pThis, void* pA, void* pB)
{
	if(*(double*)pA > *(double*)pB)
		return 1;
	if(*(double*)pA < *(double*)pB)
		return -1;
	return 0;
}

GArffData** GArffData::SplitByAttribute(GArffRelation* pRelation, int nAttribute)
{
	GArffAttribute* pAttr = pRelation->GetAttribute(nAttribute);
	GAssert(pAttr->IsInput(), "Expected an input");
	int nCount = pAttr->GetValueCount();
	GAssert(nCount > 0, "Only discreet values are supported");
	GArffData** ppParts = new GArffData*[nCount];
	int n;
	for(n = 0; n < nCount; n++)
		ppParts[n] = SplitByPivot(nAttribute, (double)n);
	GAssert(GetRowCount() == 0, "some data out of range");
	return ppParts;
}

GArffData* GArffData::SplitBySize(int nRows)
{
	GAssert(nRows >= 0 && nRows <= GetRowCount(), "out of range");
	GArffData* pNewSet = new GArffData(MAX(8, GetRowCount() - nRows));
	while(GetRowCount() > nRows)
		pNewSet->AddRow(DropRow(nRows));
	return pNewSet;
}

void GArffData::Merge(GArffData* pData)
{
	while(pData->GetRowCount() > 0)
		AddRow(pData->DropRow(0));
}

void GArffData::DiscretizeNonContinuousOutputs(GArffRelation* pRelation)
{
	int nOutputs = pRelation->GetOutputCount();
	int n, nIndex, i, nValueCount, nVal;
	int nRowCount = GetRowCount();
	double* pRow;
	for(n = 0; n < nOutputs; n++)
	{
		nIndex = pRelation->GetOutputIndex(n);
		GArffAttribute* pAttr = pRelation->GetAttribute(nIndex);
		if(pAttr->IsContinuous())
			continue;
		nValueCount = pAttr->GetValueCount();
		for(i = 0; i < nRowCount; i++)
		{
			pRow = GetRow(i);
			nVal = (int)(pRow[nIndex] - .5);
			if(nVal < 0)
				nVal = 0;
			else if(nVal >= nValueCount)
				nVal = nValueCount - 1;
			pRow[nIndex] = (double)nVal;
		}
	}
}

double GArffData::ComputeMean(int nAttribute)
{
	double dMean = 0;
	int nRowCount = GetRowCount();
	double* pRow;
	int i;
	for(i = 0; i < nRowCount; i++)
	{
		pRow = GetRow(i);
		dMean += pRow[nAttribute];
	}
	return dMean / nRowCount;
}

void GArffData::GetMeans(double* pOutMeans, int nAttributes)
{
	int n;
	for(n = 0; n < nAttributes; n++)
		pOutMeans[n] = 0;
	int nRowCount = GetRowCount();
	double* pRow;
	int i;
	for(i = 0; i < nRowCount; i++)
	{
		pRow = GetRow(i);
		for(n = 0; n < nAttributes; n++)
			pOutMeans[n] += pRow[n];
	}
	for(n = 0; n < nAttributes; n++)
		pOutMeans[n] /= nRowCount;
}

double GArffData::ComputeVariance(double dMean, int nAttribute)
{
	double dVariance = 0;
	double* pRow;
	double d;
	int i;
	int nRowCount = GetRowCount();
	for(i = 0; i < nRowCount; i++)
	{
		pRow = GetRow(i);
		d = pRow[nAttribute] - dMean;
		dVariance += (d * d);
	}
	return dVariance / nRowCount;
}

void GArffData::GetVariance(double* pOutVariance, double* pMeans, int nAttributes)
{
	int n;
	for(n = 0; n < nAttributes; n++)
		pOutVariance[n] = 0;
	int nRowCount = GetRowCount();
	double* pRow;
	int i;
	for(i = 0; i < nRowCount; i++)
	{
		pRow = GetRow(i);
		for(n = 0; n < nAttributes; n++)
			pOutVariance[n] += ((pRow[n] - pMeans[n]) * (pRow[n] - pMeans[n]));
	}
	for(n = 0; n < nAttributes; n++)
		pOutVariance[n] /= nRowCount;
}

int GArffData::RemoveOutlyers(double dStandardDeviations, int nAttributes)
{
	int nOutlyers = 0;
	double* pMeans = (double*)alloca(sizeof(double) * nAttributes);
	double* pVariance = (double*)alloca(sizeof(double) * nAttributes);
	GetMeans(pMeans, nAttributes);
	GetVariance(pVariance, pMeans, nAttributes);
	int n, i;
	for(n = 0; n < nAttributes; n++)
		pVariance[n] = sqrt(pVariance[n]);
	double* pRow;
	int nRowCount = GetRowCount();
	for(i = nRowCount - 1; i >= 0; i--)
	{
		pRow = GetRow(i);
		for(n = 0; n < nAttributes; n++)
		{
			if(ABS(pRow[n] - pMeans[n]) > dStandardDeviations * pVariance[n])
			{
				delete(DropRow(i));
				nOutlyers++;
				break;
			}
		}
	}
	return nOutlyers;
}

void GArffData::GetMinAndRange(int nAttribute, double* pMin, double* pRange)
{
	int nCount = GetRowCount();
	GAssert(nCount > 0, "No data");
	double* pRow = GetRow(0);
	double dMin = pRow[nAttribute];
	double dMax = dMin;
	int n;
	for(n = 1; n < nCount; n++)
	{
		pRow = GetRow(n);
		if(pRow[nAttribute] < dMin)
			dMin = pRow[nAttribute];
		if(pRow[nAttribute] > dMax)
			dMax = pRow[nAttribute];
	}
	*pMin = dMin;
	*pRange = dMax - dMin;
}

void GArffData::Normalize(int nAttribute, double dInputMin, double dInputRange, double dOutputMin, double dOutputRange)
{
	GAssert(dInputRange > 0, "divide by zero");
	int nCount = GetRowCount();
	double* pRow;
	double dScale = dOutputRange / dInputRange;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pRow = GetRow(n);
		pRow[nAttribute] -= dInputMin;
		pRow[nAttribute] *= dScale;
		pRow[nAttribute] += dOutputMin;
	}
}

/*static*/ double GArffData::Normalize(double dVal, double dInputMin, double dInputRange, double dOutputMin, double dOutputRange)
{
	GAssert(dInputRange > 0, "divide by zero");
	dVal -= dInputMin;
	dVal /= dInputRange;
	dVal *= dOutputRange;
	dVal += dOutputMin;
	return dVal;
}

double* GArffData::MakeSetOfMostCommonOutputs(GArffRelation* pRelation)
{
	int nOutputs = pRelation->GetOutputCount();
	double* pOutputs = new double[nOutputs];
	double* pRow;
	int nVal;
	int nIndex;
	int n;
	for(n = 0; n < nOutputs; n++)
	{
		nIndex = pRelation->GetOutputIndex(n);
		GArffAttribute* pAttr = pRelation->GetAttribute(nIndex);
		if(pAttr->IsContinuous())
		{
			// Find the mean output value
			int i;
			int nCount = GetRowCount();
			double dSum = 0;
			for(i = 0; i < nCount; i++)
			{
				pRow = GetRow(i);
				dSum += pRow[n];
			}
			pOutputs[n] = dSum / nCount;
		}
		else
		{
			// Init the counts to zero
			int nCount = pAttr->GetValueCount();
			Holder<int*> hCounts(new int[nCount]);
			int* pCounts = hCounts.Get();
			memset(pCounts, '\0', sizeof(int) * nCount);

			// Count occurrences of each output value
			int i;
			nCount = GetRowCount();
			for(i = 0; i < nCount; i++)
			{
				pRow = GetRow(i);
				nVal = (int)pRow[nIndex];
				if(nVal < 0)
				{
					GAssert(nVal == -1, "out of range");
					continue;
				}
				pCounts[nVal]++;
			}

			// Find the most common output value
			nCount = pAttr->GetValueCount();			
			int nMaxCount = pCounts[0];
			int nBestValue = 0;
			for(i = 1; i < nCount; i++)
			{
				if(pCounts[i] > nMaxCount)
				{
					nBestValue = i;
					nMaxCount = pCounts[i];
				}
			}

			// Set the value
			pOutputs[n] = (double)nBestValue;
		}
	}
	return pOutputs;
}

bool GArffData::IsOutputHomogenous(GArffRelation* pRelation)
{
	int nRowCount = GetRowCount();
	if(nRowCount <= 0)
		return true;
	int nOutputs = pRelation->GetOutputCount();
	int n, i, nIndex, nVal, nTmp;
	double* pRow;
	double dVal;
	for(i = 0; i < nOutputs; i++)
	{
		nIndex = pRelation->GetOutputIndex(i);
		GArffAttribute* pAttr = pRelation->GetAttribute(nIndex);
		if(pAttr->IsContinuous())
		{
			pRow = GetRow(0);
			dVal = pRow[nIndex];
			for(n = 1; n < nRowCount; n++)
			{
				pRow = GetRow(n);
				if(pRow[nIndex] != dVal)
					return false;
			}
		}
		else
		{
			for(n = 0; n < nRowCount; n++)
			{
				pRow = GetRow(n);
				nVal = (int)pRow[nIndex];
				if(nVal >= 0)
				{
					n++;
					break;
				}
			}
			for( ; n < nRowCount; n++)
			{
				pRow = GetRow(n);
				nTmp = (int)pRow[nIndex];
				if(nTmp != nVal && nTmp >= 0)
					return false;
			}
		}
	}
	return true;
}

void GArffData::RandomlyReplaceMissingData(GArffRelation* pRelation)
{
	int n, i, j;
	int nRowCount = GetRowCount();
	int nAttrCount = pRelation->GetAttributeCount();
	int nMaxValues = 0;
	int nValues;
	int nVal;
	int nSum;
	int nRand;
	int* pCounts = NULL;
	double* pRow;
	GArffAttribute* pAttr;
	for(i = 0; i < nAttrCount; i++)
	{
		// Make a buffer to hold the counts
		pAttr = pRelation->GetAttribute(i);
		if(pAttr->IsContinuous())
			continue;
		nValues = pAttr->GetValueCount();
		if(nValues > nMaxValues)
		{
			delete(pCounts);
			nMaxValues = pAttr->GetValueCount() + 3;
			pCounts = new int[nMaxValues];
		}

		// Count the number of each value
		memset(pCounts, '\0', sizeof(int) * nValues);
		for(n = 0; n < nRowCount; n++)
		{
			nVal = (int)GetRow(n)[i];
			if(nVal >= 0)
			{
				GAssert(nVal < nValues, "out of range");
				pCounts[nVal]++;
			}
			else
			{
				GAssert(nVal == -1, "out of range");
			}
		}

		// Sum the value counts
		nSum = 0;
		for(n = 0; n < nValues; n++)
			nSum += pCounts[n];

		// Replace the missing values
		for(n = 0; n < nRowCount; n++)
		{
			pRow = GetRow(n);
			nVal = (int)pRow[i];
			if(nVal < 0)
			{
				nRand = (int)(GBits::GetRandomUint() % nSum);
				for(j = 0; ; j++)
				{
					GAssert(j < nValues, "internal inconsistency");
					nRand -= pCounts[j];
					if(nRand < 0)
					{
						pRow[i] = (double)j;
						break;
					}
				}
			}
		}
	}
}

void GArffData::ReplaceMissingAttributeWithMostCommonValue(GArffRelation* pRelation, int nAttribute)
{
	GArffAttribute* pAttr = pRelation->GetAttribute(nAttribute);
	if(pAttr->IsContinuous())
		return; // missing values are currently only supported for discreet values
	int nValues = pAttr->GetValueCount();
	int* pCounts = (int*)alloca(sizeof(int) * nValues);
	memset(pCounts, '\0', sizeof(int) * nValues);
	double* pRow;
	int nRowCount = GetRowCount();
	int n, nVal;
	for(n = 0; n < nRowCount; n++)
	{
		pRow = GetRow(n);
		nVal = (int)pRow[nAttribute];
		if(nVal < 0)
			continue;
		GAssert(nVal < nValues, "out of range");
		pCounts[nVal]++;
	}
	int nBest = 0;
	for(n = 1; n < nValues; n++)
	{
		if(pCounts[n] > pCounts[nBest])
			nBest = n;
	}
	for(n = 0; n < nRowCount; n++)
	{
		pRow = GetRow(n);
		nVal = (int)pRow[nAttribute];
		if(nVal < 0)
		{
			pRow[nAttribute] = (double)nBest;
		}
	}
}

void GArffData::Print(int nAttributes)
{
	int nRows = GetRowCount();
	double* pRow;
	int n, i;
	for(n = 0; n < nRows; n++)
	{
		pRow = GetRow(n);
		printf("%f", pRow[0]);
		for(i = 1; i < nAttributes; i++)
			printf("\t%f", pRow[i]);
		printf("\n");
	}
}

int ComputeMinimumVariancePivotComparer(void* pThis, void* pA, void* pB)
{
	int nAttr = (int)pThis;
	double* pdA = (double*)pA;
	double* pdB = (double*)pB;
	if(pdA[nAttr] > pdB[nAttr])
		return 1;
	else
		return -1;
}

double GArffData::ComputeMinimumVariancePivot(int nAttr)
{
	int nRows = GetRowCount();
	GPointerArray arr(nRows);
	int n;
	for(n = 0; n < nRows; n++)
		arr.AddPointer(GetRow(n));
	arr.Sort(ComputeMinimumVariancePivotComparer, (void*)nAttr);
	double dBestPivotScore = 1e100;
	double dBestPivot = 0;
	double dPivot, d;
	double* pRow1;
	double* pRow2;
	double dMean1, dMean2, dVar1, dVar2;
	int nCount1, nCount2, i;
	for(n = nRows - 2; n >= 0; n--)
	{
		// Try a pivot
		pRow1 = (double*)arr.GetPointer(n);
		pRow2 = (double*)arr.GetPointer(n + 1);
		dPivot = (pRow1[nAttr] + pRow2[nAttr]) / 2;

		// Compute the mean of each half
		dMean1 = 0;
		dMean2 = 0;
		nCount1 = 0;
		nCount2 = 0;
		for(i = 0; i < nRows; i++)
		{
			pRow1 = GetRow(i);
			if(pRow1[nAttr] < dPivot)
			{
				nCount1++;
				dMean1 += pRow1[nAttr];
			}
			else
			{
				nCount2++;
				dMean2 += pRow1[nAttr];
			}
		}
		dMean1 /= nCount1;
		dMean2 /= nCount2;

		// Compute the variance of each half
		dVar1 = 0;
		dVar2 = 0;
		for(i = 0; i < nRows; i++)
		{
			pRow1 = GetRow(i);
			if(pRow1[nAttr] < dPivot)
			{
				d = pRow1[nAttr] - dMean1;
				dVar1 += (d * d);
			}
			else
			{
				d = pRow2[nAttr] - dMean2;
				dVar2 += (d * d);
			}
		}
		dVar1 /= nCount1;
		dVar2 /= nCount2;
		d = dVar1 + dVar2;
		
		// See if we've got a new best score
		if(d < dBestPivotScore)
		{
			dBestPivotScore = d;
			dBestPivot = dPivot;
		}
	}
	return dBestPivot;
}

double GArffData::ComputeMinimumInfoPivot(GArffRelation* pRelation, int nAttr, double* pOutputInfo)
{
	int nRows = GetRowCount();
	GPointerArray arr(nRows);
	int n;
	for(n = 0; n < nRows; n++)
		arr.AddPointer(GetRow(n));
	arr.Sort(ComputeMinimumVariancePivotComparer, (void*)nAttr);
	double dBestPivotScore = 1e100;
	double dBestPivot = 0;
	double dPivot, d;
	double* pRow1;
	double* pRow2;
	for(n = nRows - 2; n >= 0; n--)
	{
		// Try a pivot
		pRow1 = (double*)arr.GetPointer(n);
		pRow2 = (double*)arr.GetPointer(n + 1);
		dPivot = (pRow1[nAttr] + pRow2[nAttr]) / 2;

		// Split at the pivot and measure the sum info
		GArffData* pData2 = SplitByPivot(nAttr, dPivot);
		d = pRelation->MeasureTotalOutputInfo(this) + pRelation->MeasureTotalOutputInfo(pData2);
		Merge(pData2);
		delete(pData2);

		// See if we've got a new best score
		if(d < dBestPivotScore)
		{
			dBestPivotScore = d;
			dBestPivot = dPivot;
		}
	}
	*pOutputInfo = dBestPivotScore;
	return dBestPivot;
}

void GArffData::ComputeCovarianceMatrix(GMatrix* pOutMatrix, GArffRelation* pRelation)
{
	// Resize the matrix
	int nInputs = pRelation->GetInputCount();
	pOutMatrix->Resize(nInputs, nInputs);

	// Compute the deviations
	Holder<double*> hMeans(new double[nInputs]);
	double* pMeans = hMeans.Get();
	int nRowCount = GetRowCount();
	double* pRow;
	int n, i, j, nIndex;
	for(i = 0; i < nInputs; i++)
	{
		nIndex = pRelation->GetInputIndex(i);

		// Compute the mean
		double dSum = 0;
		for(n = 0; n < nRowCount; n++)
		{
			pRow = GetRow(n);
			dSum += pRow[nIndex];
		}
		pMeans[i] = dSum / nRowCount;
	}

	// Compute the covariances for half the matrix
	for(i = 0; i < nInputs; i++)
	{
		for(n = i; n < nInputs; n++)
		{
			double dSum = 0;
			for(j = 0; j < nRowCount; j++)
			{
				pRow = GetRow(j);
				dSum += ((pRow[i] - pMeans[i]) * (pRow[n] - pMeans[n]));
			}
			pOutMatrix->Set(i, n, dSum / (nRowCount - 1));
		}
	}

	// Fill out the other half of the matrix
	for(i = 1; i < nInputs; i++)
	{
		for(n = 0; n < i; n++)
			pOutMatrix->Set(i, n, pOutMatrix->Get(n, i));
	}
}

void GArffData::ComputeCoprobabilityMatrix(GMatrix* pOutMatrix, GArffRelation* pRelation, int nAttr, double noDataValue)
{
	// Resize the matrix
	GArffAttribute* pAttr = pRelation->GetAttribute(nAttr);
	int nRows = pAttr->GetValueCount();
	int nAttributes = pRelation->GetAttributeCount();
	int nCols = 0;
	int i;
	for(i = 0; i < nAttributes; i++)
	{
		GArffAttribute* pAttrCol = pRelation->GetAttribute(i);
		nCols += pAttrCol->GetValueCount();
	}
	pOutMatrix->Resize(nRows, nCols);

	// Compute the coprobabilities
	int nRowCount = GetRowCount();
	int row, col, nMatch, nTotal, nAttrCol, nVal;
	double* pRow;
	for(row = 0; row < nRows; row++)
	{
		col = 0;
		for(nAttrCol = 0; nAttrCol < nAttributes; nAttrCol++)
		{
			GArffAttribute* pAttrCol = pRelation->GetAttribute(nAttrCol);
			for(nVal = 0; nVal < pAttrCol->GetValueCount(); nVal++)
			{
				nMatch = 0;
				nTotal = 0;
				for(i = 0; i < nRowCount; i++)
				{
					pRow = GetRow(i);
					if((int)pRow[nAttrCol] == nVal)
					{
						nTotal++;
						if((int)pRow[nAttr] == row)
							nMatch++;
					}
				}
				if(nTotal == 0)
					pOutMatrix->Set(row, col, noDataValue);
				else
					pOutMatrix->Set(row, col, (double)nMatch / nTotal);
				col++;
			}
		}
		GAssert(col == nCols, "problem with columns");
	}
}
