#include "GKNN.h"
#include "GArff.h"
#include "GArray.h"
#include <math.h>
#include "GVector.h"

GKNN::GKNN(GArffRelation* pRelation, int nNeighbors)
 : GSupervisedLearner(pRelation)
{
	m_nNeighbors = nNeighbors;
	m_pRows = new GPointerArray(256);
	int nAttributes = m_pRelation->GetAttributeCount();
	m_pScaleFactors = new double[nAttributes];
	int n;
	for(n = 0; n < nAttributes; n++)
		m_pScaleFactors[n] = 1;
}

GKNN::~GKNN()
{
	int n;
	int nRows = m_pRows->GetSize();
	for(n = 0; n < nRows; n++)
		delete((double*)m_pRows->GetPointer(n));
	delete(m_pRows);
	delete(m_pScaleFactors);
}

void GKNN::AddRow(double* pRow)
{
	int nAttributes = m_pRelation->GetAttributeCount();
	double* pData = new double[nAttributes];
	m_pRows->AddPointer(pData);
	memcpy(pData, pRow, sizeof(double) * nAttributes);
}

// todo: use a better algorithm for finding neigbors. This is a lousy algorithm for that.
void GKNN::FindNeighbors(int i, double* pRow, int* pNeighbors)
{
	double* pDistances = (double*)alloca(sizeof(double) * m_nNeighbors);
	int j, k;
	for(j = 0; j < m_nNeighbors; j++)
		pDistances[j] = 1e100;
	int nWorstNeighbor = 0;
	int nRows = m_pRows->GetSize();
	double* pRow2;
	double d;
	nWorstNeighbor = 0;
	for(j = 0; j < nRows; j++)
	{
		if(j == i)
			continue;
		pRow2 = (double*)m_pRows->GetPointer(j);
		d = m_pRelation->ComputeInputDistanceSquared(pRow, pRow2);
//		d = m_pRelation->ComputeScaledInputDistanceSquared(pRow, pRow2, m_pScaleFactors);
		if(d < pDistances[nWorstNeighbor])
		{
			pDistances[nWorstNeighbor] = d;
			pNeighbors[nWorstNeighbor] = j;
			nWorstNeighbor = 0;
			for(k = 1; k < m_nNeighbors; k++)
			{
				if(pDistances[k] > pDistances[nWorstNeighbor])
					nWorstNeighbor = k;
			}
		}
	}
	GAssert(pDistances[m_nNeighbors - 1] < 1e99, "failed to find enough neighbors");
}

void GKNN::RecomputeScaleFactors()
{
	// Compute the scale factors
	int nAttributes = m_pRelation->GetAttributeCount();
	GArffData tmp(m_pRows);
	double* pMeans = (double*)alloca(sizeof(double) * nAttributes);
	tmp.GetMeans(pMeans, nAttributes);
	tmp.GetVariance(m_pScaleFactors, pMeans, nAttributes);
	tmp.SetRows(NULL);
	int i;
	for(i = 0; i < nAttributes; i++)
	{
		if(m_pRelation->GetAttribute(i)->IsContinuous())
			m_pScaleFactors[i] = sqrt(m_pScaleFactors[i]);
		else
			m_pScaleFactors[i] = 1;
	}
}

void GKNN::Train(GArffData* pData)
{
	int nRows = pData->GetRowCount();
	double* pRow;
	int n;
	for(n = 0; n < nRows; n++)
	{
		pRow = pData->GetRow(n);
		AddRow(pRow);
	}
	RecomputeScaleFactors();
}

void GKNN::EvalEqualWeight(double* pRow)
{
	int* pNeighbors = (int*)alloca(sizeof(int) * m_nNeighbors);
	FindNeighbors(-1, pRow, pNeighbors);
	int nOutputs = m_pRelation->GetOutputCount();
	GArffAttribute* pAttr;
	int i, j, index;
	double* pRowNeighbor;
	for(i = 0; i < nOutputs; i++)
	{
		index = m_pRelation->GetOutputIndex(i);
		pAttr = m_pRelation->GetAttribute(index);
		if(pAttr->IsContinuous())
		{
			double dSum = 0;
			for(j = 0; j < m_nNeighbors; j++)
			{
				pRowNeighbor = (double*)m_pRows->GetPointer(pNeighbors[j]);
				dSum += pRowNeighbor[index];
			}
			pRow[index] = dSum / m_nNeighbors;
		}
		else
		{
			GVector v(pAttr->GetValueCount());
			for(j = 0; j < m_nNeighbors; j++)
			{
				pRowNeighbor = (double*)m_pRows->GetPointer(pNeighbors[j]);
				v.Add((int)pRowNeighbor[index], 1);
			}
			pRow[index] = (double)v.GetIndexOfBiggestValue();
		}
	}
}

void GKNN::EvalLinearWeight(double* pRow)
{
	int* pNeighbors = (int*)alloca(sizeof(int) * m_nNeighbors);
	FindNeighbors(-1, pRow, pNeighbors);
	int nOutputs = m_pRelation->GetOutputCount();
	GArffAttribute* pAttr;
	int i, j, index;
	double d;
	double* pRowNeighbor;
	for(i = 0; i < nOutputs; i++)
	{
		index = m_pRelation->GetOutputIndex(i);
		pAttr = m_pRelation->GetAttribute(index);
		if(pAttr->IsContinuous())
		{
			double dSum = 0;
			double dTot = 0;
			for(j = 0; j < m_nNeighbors; j++)
			{
				pRowNeighbor = (double*)m_pRows->GetPointer(pNeighbors[j]);
				d = m_pRelation->ComputeInputDistanceSquared(pRow, pRowNeighbor);
//				d = m_pRelation->ComputeScaledInputDistanceSquared(pRow, pRowNeighbor, m_pScaleFactors);
				d = 1 / (d + .001);
				dSum += d * pRowNeighbor[index];
				dTot += d;
			}
			pRow[index] = dSum / dTot;
		}
		else
		{
			GVector v(pAttr->GetValueCount());
			for(j = 0; j < m_nNeighbors; j++)
			{
				pRowNeighbor = (double*)m_pRows->GetPointer(pNeighbors[j]);
				d = m_pRelation->ComputeInputDistanceSquared(pRow, pRowNeighbor);
//				d = m_pRelation->ComputeScaledInputDistanceSquared(pRow, pRowNeighbor, m_pScaleFactors);
				d = 1 / (d + .001);
				v.Add((int)pRowNeighbor[index], d);
			}
			pRow[index] = (double)v.GetIndexOfBiggestValue();
		}
	}
}

double* GKNN::DropRow(int nRow)
{
	int nCount = m_pRows->GetSize();
	double* pRow = (double*)m_pRows->GetPointer(nRow);
	m_pRows->SetPointer(nRow, m_pRows->GetPointer(nCount - 1));
	m_pRows->DeleteCell(nCount - 1);
	return pRow;
}

double GKNN::MeasureError(GArffData* pTestSet)
{
	double dError = 0;
	int nOutput, nIndex, i;
	GArffAttribute* pAttr;
	int nRows = pTestSet->GetRowCount();
	double* pRow;
	double d;
	int nRowSize = sizeof(double) * m_pRelation->GetAttributeCount();
	double* pSample = (double*)alloca(nRowSize);
	for(nOutput = 0; nOutput < m_pRelation->GetOutputCount(); nOutput++)
	{
		nIndex = m_pRelation->GetOutputIndex(nOutput);
		pAttr = m_pRelation->GetAttribute(nIndex);
		if(pAttr->IsContinuous())
		{
			double dSum = 0;
			for(i = 0; i < nRows; i++)
			{
				pRow = pTestSet->GetRow(i);
				memcpy(pSample, pRow, nRowSize);
				EvalLinearWeight(pSample);
				d = pSample[nIndex] - pRow[nIndex];
				dSum += (d * d);
			}
			dError += dSum / nRows;
		}
		else
		{
			int nWrong = 0;
			for(i = 0; i < nRows; i++)
			{
				pRow = pTestSet->GetRow(i);
				memcpy(pSample, pRow, nRowSize);
				EvalLinearWeight(pSample);
				if((int)pSample[nIndex] != (int)pRow[nIndex])
					nWrong++;
			}
			d = (double)nWrong / nRows;
			dError += d;
		}
	}
	return dError;
}

int GKNN::FindLeastHelpfulRow(GArffData* pTestSet)
{
	double dBestError = 1e100;
	int nBestIndex = 0;
	int n;
	double d;
	for(n = m_pRows->GetSize() - 1; n >= 0; n--)
	{
		double* pTemp = DropRow(n);
		d = MeasureError(pTestSet);
		m_pRows->AddPointer(pTemp);
		if(d < dBestError)
		{
			dBestError = d;
			nBestIndex = n;
		}
	}
	return nBestIndex;
}
