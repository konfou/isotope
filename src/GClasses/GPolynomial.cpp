#include "GMacros.h"
#include "GPolynomial.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN
#include "GArff.h"

GPolynomial::GPolynomial(int nDimensions, int nControlPoints)
{
	GAssert(nDimensions > 0 && nControlPoints > 0, "Bad values");
	m_nDimensions = nDimensions;
	m_nControlPoints = nControlPoints;
	m_nCoefficients = 1;
	while(nDimensions > 0)
	{
		m_nCoefficients *= nControlPoints;
		nDimensions--;
	}
	m_pCoefficients = new double[m_nCoefficients];
	int n;
	for(n = 0; n < m_nCoefficients; n++)
		m_pCoefficients[n] = 0;
}

GPolynomial::~GPolynomial()
{
	delete(m_pCoefficients);
}

int GPolynomial::CalcIndex(int* pDegrees)
{
	int nIndex = 0;
	int n;
	for(n = m_nDimensions - 1; n >= 0; n--)
	{
		nIndex *= m_nControlPoints;
		GAssert(pDegrees[n] >= 0 && pDegrees[n] < m_nControlPoints, "out of range");
		nIndex += pDegrees[n];
	}
	return nIndex;
}

double GPolynomial::GetCoefficient(int* pDegrees)
{
	return m_pCoefficients[CalcIndex(pDegrees)];
}

void GPolynomial::SetCoefficient(int* pDegrees, double dVal)
{
	m_pCoefficients[CalcIndex(pDegrees)] = dVal;
}

double GPolynomial::Eval(double* pVariables)
{
	int* pDegrees = (int*)alloca(m_nDimensions);
	int n, i;
	for(n = 0; n < m_nDimensions; n++)
		pDegrees[n] = m_nControlPoints - 1;
	double dSum = 0;
	double dVar;
	int nCoeff;
	for(nCoeff = m_nCoefficients - 1; nCoeff >= 0; nCoeff--)
	{
		dVar = 1;
		for(n = 0; n < m_nDimensions; n++)
		{
			for(i = pDegrees[n]; i > 0; i--)
				dVar *= pVariables[n];
		}
		dVar *= m_pCoefficients[nCoeff];
		dSum += dVar;
		i = 0;
		while(true)
		{
			if(pDegrees[i]-- == 0)
			{
				pDegrees[i] = m_nControlPoints - 1;
				if(++i < m_nControlPoints)
					continue;
			}
			break;
		}
	}
	return dSum;
}

double GPolynomial::MeasureMeanSquareError(GArffRelation* pRelation, GArffData* pData, int nOutputAttr)
{
	GArffAttribute* pAttr = pRelation->GetAttribute(nOutputAttr);
	GAssert(!pAttr->IsInput(), "expected an output attribute");
	int nInputs = pRelation->GetInputCount();
	double dSum = 0;
	double* pRow;
	double* pVariables = (double*)alloca(sizeof(double) * nInputs);
	double dError;
	int n, i;
	int nCount = pData->GetRowCount();
	for(n = 0; n < nCount; n++)
	{
		pRow = pData->GetRow(n);
		for(i = 0; i < nInputs; i++)
			pVariables[i] = pRow[pRelation->GetInputIndex(i)];
		dError = pRow[nOutputAttr] - Eval(pVariables);
		dError *= dError;
		dSum += dError;
	}
	return dSum / nCount;
}

double GPolynomial::DivideAndMeasureError(GArffRelation* pRelation, GArffData* pData, int nOutputAttr)
{
	int nCount = pData->GetRowCount();
	GArffData d1(nCount);
	GArffData d2(nCount);
	int nInputs = pRelation->GetInputCount();
	GAssert(m_nDimensions == nInputs - 1, "unexpected number of inputs");
	double* pInputs = (double*)alloca(sizeof(double) * (nInputs - 1));
	int n, i;
	double dThresh;
	for(n = 0; n < nCount; n++)
	{
		double* pRow = pData->GetRow(n);
		for(i = 0; i < nInputs - 1; i++)
			pInputs[i] = pRow[pRelation->GetInputIndex(i)];
		dThresh = Eval(pInputs);
		if(dThresh >= 0)
			d1.AddRow(pRow);
		else
			d2.AddRow(pRow);
	}
	double dError1 = 1e100;
	double dError2 = 1e100;
	if(d1.GetRowCount() * 5 >= d2.GetRowCount() && d2.GetRowCount() * 5 >= d1.GetRowCount())
	{
		Holder<GPolynomial*> hPoly1(FitData(pRelation, &d1, nOutputAttr, m_nControlPoints));
		dError1 = hPoly1.Get()->MeasureMeanSquareError(pRelation, &d1, nOutputAttr);
		Holder<GPolynomial*> hPoly2(FitData(pRelation, &d2, nOutputAttr, m_nControlPoints));
		dError2 = hPoly2.Get()->MeasureMeanSquareError(pRelation, &d2, nOutputAttr);
	}
	dError1 *= dError1;
	dError2 *= dError2;
	d1.DropAllRows();
	d2.DropAllRows();
	return dError1 + dError2;
}

/*static*/ GPolynomial* GPolynomial::FitData(GArffRelation* pRelation, GArffData* pData, int nOutputAttr, int nControlPoints)
{
	GArffAttribute* pAttr = pRelation->GetAttribute(nOutputAttr);
	GAssert(!pAttr->IsInput(), "expected an output attribute");
	int nInputs = pRelation->GetInputCount();
	GPolynomial* pPolynomial = new GPolynomial(nInputs, nControlPoints);
	double dBestError = pPolynomial->MeasureMeanSquareError(pRelation, pData, nOutputAttr);
	double dError;
	double dStep;
	int nCoefficients = pPolynomial->m_nCoefficients;
	int n;
	bool bGotOne;
	for(dStep = 1000; dStep > .000001; dStep *= .95)
	{
		bGotOne = false;
		for(n = 0; n < nCoefficients; n++)
		{
			pPolynomial->m_pCoefficients[n] += dStep;
			dError = pPolynomial->MeasureMeanSquareError(pRelation, pData, nOutputAttr);
			if(dError >= dBestError)
			{
				pPolynomial->m_pCoefficients[n] -= dStep;
				pPolynomial->m_pCoefficients[n] -= dStep;
				dError = pPolynomial->MeasureMeanSquareError(pRelation, pData, nOutputAttr);
			}
			if(dError >= dBestError)
				pPolynomial->m_pCoefficients[n] += dStep;
			else
			{
				dBestError = dError;
				bGotOne = true;
			}
		}
		if(!bGotOne)
			dStep *= .6;
	}
	return pPolynomial;
}

/*static*/ GPolynomial* GPolynomial::DivideData(GArffRelation* pRelation, GArffData* pData, int nOutputAttr, int nControlPoints)
{
	GArffAttribute* pAttr = pRelation->GetAttribute(nOutputAttr);
	GAssert(!pAttr->IsInput(), "expected an output attribute");
	int nInputs = pRelation->GetInputCount();
	GAssert(nInputs > 1, "Not enough inputs for this technique");
	GPolynomial* pPolynomial = new GPolynomial(nInputs - 1, nControlPoints);
	double dBestError = pPolynomial->DivideAndMeasureError(pRelation, pData, nOutputAttr);
	double dError;
	double dStep;
	int nCoefficients = pPolynomial->m_nCoefficients;
	int n;
	bool bGotOne;
	for(dStep = 1000; dStep > .000001; dStep *= .95)
	{
		bGotOne = false;
		for(n = 0; n < nCoefficients; n++)
		{
			pPolynomial->m_pCoefficients[n] += dStep;
			dError = pPolynomial->DivideAndMeasureError(pRelation, pData, nOutputAttr);
			if(dError >= dBestError)
			{
				pPolynomial->m_pCoefficients[n] -= dStep;
				pPolynomial->m_pCoefficients[n] -= dStep;
				dError = pPolynomial->DivideAndMeasureError(pRelation, pData, nOutputAttr);
			}
			if(dError >= dBestError)
				pPolynomial->m_pCoefficients[n] += dStep;
			else
			{
				dBestError = dError;
				bGotOne = true;
			}
		}
		if(!bGotOne)
			dStep *= .6;
	}
	return pPolynomial;
}
