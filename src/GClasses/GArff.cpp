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
#include <malloc.h>
#include <math.h>

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
		if(pAttr->IsContinuous())
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
	GAssert(m_nOutputCount > 0, "no outputs");
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

double GArffRelation::TotalEntropyOfAllOutputs(GArffData* pData)
{
	double dEntropy = 0;
	int nOutputs = GetOutputCount();
	int n;
	for(n = 0; n < nOutputs; n++)
		dEntropy += pData->MeasureEntropy(this, GetOutputIndex(n));
	return dEntropy;
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
		m_szValues = new char*[nValues];
		int n;
		for(n = 0; n < nValues; n++)
		{
			m_szValues[n] = new char[strlen(szValues[n]) + 1];
			strcpy(m_szValues[n], szValues[n]);
		}
	}
	m_bIsInput = bIsInput;
}

GArffAttribute::~GArffAttribute()
{
	delete(m_szName);
	int n;
	for(n = 0; n < m_nValues; n++)
		delete(m_szValues[n]);
	delete(m_szValues);
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
	int nPos;
	for(nPos = 1; nPos < nLen && szFile[nPos] > ' '; nPos++)
	{
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

int GArffAttribute::GetValueCount()
{
	return m_nValues;
}

const char* GArffAttribute::GetValue(int n)
{
	GAssert(n >= 0 && n < m_nValues, "out of range");
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

GArffData::GArffData(int nGrowSize)
{
	m_pRows = new GPointerArray(nGrowSize);
}

GArffData::~GArffData()
{
	int nCount = m_pRows->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((double*)m_pRows->GetPointer(n));
	delete(m_pRows);
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
		GAssert(nValue >= 0 && nValue < nPossibleValues, "value out of range");
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

int DoubleRefComparer(void* pA, void* pB)
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
	if(nCount > 0)
	{
		// Split by each discreet attribute value
		GArffData** ppParts = new GArffData*[nCount];
		int n;
		for(n = 0; n < nCount; n++)
			ppParts[n] = SplitByPivot(nAttribute, (double)n);
		GAssert(GetRowCount() == 0, "some data out of range");
		return ppParts;
	}
	else
	{
		// Find the median
		GPointerArray arr(GetRowCount());
		int nRows = GetRowCount();
		int n;
		for(n = 0; n < nRows; n++)
		{
			double* pRow = GetRow(n);
			arr.AddPointer(&pRow[nAttribute]);
		}
		arr.Sort(DoubleRefComparer);
		double dMedian = *(double*)arr.GetPointer(nRows / 2);

		// Split the data
		GArffData** ppParts = new GArffData*[2];
		ppParts[0] = SplitByPivot(nAttribute, dMedian);
		ppParts[1] = new GArffData(MAX(8, GetRowCount()));
		while(GetRowCount() > 0)
			ppParts[1]->AddRow(DropRow(0));
		return ppParts;
	}
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

void GArffData::Analogize(GArffRelation* pRelation)
{
	int nCount = GetRowCount();
	int n, i;
	double* pRow;
	for(i = 0; i < pRelation->GetAttributeCount(); i++)
	{
		GArffAttribute* pAttr = pRelation->GetAttribute(i);
		if(pAttr->IsContinuous())
		{
			for(n = 0; n < nCount; n++)
			{
				pRow = GetRow(n);
				if(pRow[i] < .00001 || pRow[i] > .99999)
				{
					const char* szMessage = "Sigmoids can't output this value, so it won't work with GNeuralNet";
					GAssert(false, szMessage);
					throw szMessage;
				}
			}
		}
		else
		{
			for(n = 0; n < nCount; n++)
			{
				pRow = GetRow(n);
				pRow[i] = GMath::digitalToAnalog(pRow[i], pAttr->GetValueCount());
			}
		}
	}
}

void GArffData::Unanalogize(GArffRelation* pRelation)
{
	int nCount = GetRowCount();
	int n, i;
	double* pRow;
	for(i = 0; i < pRelation->GetAttributeCount(); i++)
	{
		GArffAttribute* pAttr = pRelation->GetAttribute(i);
		if(!pAttr->IsContinuous())
		{
			for(n = 0; n < nCount; n++)
			{
				pRow = GetRow(n);
				pRow[i] = GMath::analogToDigital(pRow[i], pAttr->GetValueCount());
			}
		}
	}
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
	dVal -= dInputMin;
	dVal /= dInputRange;
	dVal *= dOutputRange;
	dVal += dOutputMin;
	return dVal;
}
