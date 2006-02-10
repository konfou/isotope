#include "GManifold.h"
#include <stdio.h>
#include "GArray.h"
#include <math.h>
#include "GPointerQueue.h"
#include "GArff.h"
#include "GMatrix.h"
#include "GVector.h"
#include "GBits.h"

struct GSquisherNeighbor
{
	unsigned char* m_pNeighbor;
	unsigned char* m_pNeighborsNeighbor;
	double m_dCosTheta;
	double m_dDistance;
};

struct GSquisherData
{
	int m_nCycle;
	bool m_bAdjustable;
};

GSquisher::GSquisher(int nDataPoints, int nDimensions, int nNeighbors)
{
	m_nDataPoints = nDataPoints;
	m_nDimensions = nDimensions;
	m_nNeighbors = nNeighbors;
	m_nDataIndex = sizeof(struct GSquisherNeighbor) * m_nNeighbors;
	m_nValueIndex = m_nDataIndex + sizeof(struct GSquisherData);
	m_nRecordSize = m_nValueIndex + sizeof(double) * m_nDimensions;
	m_pData = new unsigned char[m_nRecordSize * nDataPoints];
	m_dSquishingRate = .99;
	m_nTargetDimensions = 0;
	m_nPass = 0;
	m_dAveNeighborDist = 0;
	m_pQ = NULL;
	m_nSmoothingAdvantage = 10;
}

GSquisher::~GSquisher()
{
	delete(m_pData);
	delete(m_pQ);
}

void GSquisher::SetDataPoint(int n, double* pValues, bool bAdjustable)
{
	memcpy(&m_pData[n * m_nRecordSize + m_nValueIndex], pValues, sizeof(double) * m_nDimensions);
	struct GSquisherData* pData = (struct GSquisherData*)&m_pData[n * m_nRecordSize + m_nDataIndex];
	pData->m_bAdjustable = bAdjustable;
}

void GSquisher::SetData(GArffRelation* pRelation, GArffData* pData)
{
	GAssert(pData->GetRowCount() == m_nDataPoints, "wrong number of data points");
	int nInputs = pRelation->GetInputCount();
	double* pInputs = (double*)alloca(sizeof(double) * nInputs);
	double* pRow;
	int n, i;
	for(n = 0; n < m_nDataPoints; n++)
	{
		pRow = pData->GetRow(n);
		for(i = 0; i < nInputs; i++)
			pInputs[i] = pRow[pRelation->GetInputIndex(i)];
		SetDataPoint(n, pInputs, true);
	}
}

double* GSquisher::GetDataPoint(int n)
{
	return (double*)&m_pData[n * m_nRecordSize + m_nValueIndex];
}

int GSquisher::DataPointSortCompare(unsigned char* pA, unsigned char* pB)
{
	pA += m_nValueIndex;
	pB += m_nValueIndex;
	double dA = ((double*)pA)[m_nCurrentDimension];
	double dB = ((double*)pB)[m_nCurrentDimension];
	if(dA > dB)
		return 1;
	else if(dA < dB)
		return -1;
	return 0;
}

int GSquisher_DataPointSortCompare(void* pThis, void* pA, void* pB)
{
	return ((GSquisher*)pThis)->DataPointSortCompare((unsigned char*)pA, (unsigned char*)pB);
}

int GSquisher::FindMostDistantNeighbor(struct GSquisherNeighbor* pNeighbors)
{
	int nMostDistant = 0;
	int n;
	for(n = 1; n < m_nNeighbors; n++)
	{
		if(pNeighbors[n].m_dDistance > pNeighbors[nMostDistant].m_dDistance)
			nMostDistant = n;
	}
	return nMostDistant;
}

double GSquisher::CalculateDistance(unsigned char* pA, unsigned char* pB)
{
	pA += m_nValueIndex;
	pB += m_nValueIndex;
	double* pdA = (double*)pA;
	double* pdB = (double*)pB;
	double dTmp;
	double dSum = 0;
	int n;
	for(n = 0; n < m_nDimensions; n++)
	{
		dTmp = pdB[n] - pdA[n];
		dSum += (dTmp * dTmp);
	}
	return sqrt(dSum);
}

double GSquisher::CalculateVectorCorrelation(unsigned char* pA, unsigned char* pVertex, unsigned char* pB)
{
	pA += m_nValueIndex;
	pVertex += m_nValueIndex;
	pB += m_nValueIndex;
	double* pdA = (double*)pA;
	double* pdV = (double*)pVertex;
	double* pdB = (double*)pB;
	double dDotProd = 0;
	double dMagA = 0;
	double dMagB = 0;
	double dA, dB;
	int n;
	for(n = 0; n < m_nDimensions; n++)
	{
		dA = pdA[n] - pdV[n];
		dB = pdB[n] - pdV[n];
		dDotProd += (dA * dB);
		dMagA += (dA * dA);
		dMagB += (dB * dB);
	}
	if(dDotProd == 0)
		return 0;
	double dCorrelation = dDotProd / (sqrt(dMagA) * sqrt(dMagB));
	GAssert(dCorrelation > -2 && dCorrelation < 2, "out of range");
	return dCorrelation;
}

void GSquisher::CalculateMetadata(int nTargetDimensions)
{
	// Calculate how far we need to look to have a good chance of finding the
	// nearest neighbors
	int nDimSpan = (int)(pow((double)m_nDataPoints, (double)1 / nTargetDimensions) * 2);
	if(nDimSpan < m_nNeighbors * 2)
		nDimSpan = m_nNeighbors * 2;

	// Find the nearest neighbors
	GPointerArray arr(m_nDataPoints);
	int n, i, j;
	for(n = 0; n < m_nDataPoints; n++)
		arr.AddPointer(&m_pData[n * m_nRecordSize]);

	// Initialize everybody's neighbors
	struct GSquisherNeighbor* pNeighbors;
	for(n = 0; n < m_nDataPoints; n++)
	{
		pNeighbors = (struct GSquisherNeighbor*)arr.GetPointer(n);
		for(i = 0; i < m_nNeighbors; i++)
		{
			pNeighbors[i].m_pNeighbor = NULL;
			pNeighbors[i].m_pNeighborsNeighbor = NULL;
			pNeighbors[i].m_dDistance = 1e100;
		}
	}

	// Find the nearest neighbors
	int nMostDistantNeighbor;
	int nStart, nEnd;
	double dDistance;
	unsigned char* pCandidate;
	for(m_nCurrentDimension = 0; m_nCurrentDimension < m_nDimensions; m_nCurrentDimension++)
	{
		// Sort on the current dimension
		arr.Sort(GSquisher_DataPointSortCompare, this);

		// Do a pass in this dimension for every data point to search for nearest neighbors
		for(n = 0; n < m_nDataPoints; n++)
		{
			// Check all the data points that are close in this dimension
			pNeighbors = (struct GSquisherNeighbor*)arr.GetPointer(n);
			nMostDistantNeighbor = FindMostDistantNeighbor(pNeighbors);
			nStart = MAX(0, n - nDimSpan);
			nEnd = MIN(m_nDataPoints - 1, n + nDimSpan);
			for(i = nStart; i <= nEnd; i++)
			{
				if(i == n)
					continue;
				pCandidate = (unsigned char*)arr.GetPointer(i);
				dDistance = CalculateDistance((unsigned char*)pNeighbors, pCandidate);
				if(dDistance < pNeighbors[nMostDistantNeighbor].m_dDistance)
				{
					// Check to see if this is already a neighbor
					for(j = 0; j < m_nNeighbors; j++)
					{
						if(pNeighbors[j].m_pNeighbor == pCandidate)
							break;
					}
					if(j == m_nNeighbors)
					{
						// Make this a neighbor
						pNeighbors[nMostDistantNeighbor].m_pNeighbor = pCandidate;
						pNeighbors[nMostDistantNeighbor].m_dDistance = dDistance;
						nMostDistantNeighbor = FindMostDistantNeighbor(pNeighbors);
					}
				}
			}
		}
	}

	// For each data point, find the most co-linear of each neighbor's neighbors
	m_dAveNeighborDist = 0;
	struct GSquisherData* pData;
	struct GSquisherNeighbor* pNeighborsNeighbors;
	double dCosTheta;
	for(n = 0; n < m_nDataPoints; n++)
	{
		pNeighbors = (struct GSquisherNeighbor*)arr.GetPointer(n);
		pData = (struct GSquisherData*)(((unsigned char*)pNeighbors) + m_nDataIndex);
		pData->m_nCycle = -1;
		for(i = 0; i < m_nNeighbors; i++)
		{
			m_dAveNeighborDist += pNeighbors[i].m_dDistance;
			pNeighborsNeighbors = (struct GSquisherNeighbor*)pNeighbors[i].m_pNeighbor;
			pCandidate = pNeighborsNeighbors[0].m_pNeighbor;
			dCosTheta = CalculateVectorCorrelation((unsigned char*)pNeighbors, (unsigned char*)pNeighborsNeighbors, pCandidate);
			pNeighbors[i].m_dCosTheta = dCosTheta;
			pNeighbors[i].m_pNeighborsNeighbor = pCandidate;
			for(j = 1; j < m_nNeighbors; j++)
			{
				pCandidate = pNeighborsNeighbors[j].m_pNeighbor;
				dCosTheta = CalculateVectorCorrelation((unsigned char*)pNeighbors, (unsigned char*)pNeighborsNeighbors, pCandidate);
				if(dCosTheta < pNeighbors[i].m_dCosTheta)
				{
					pNeighbors[i].m_dCosTheta = dCosTheta;
					pNeighbors[i].m_pNeighborsNeighbor = pCandidate;
				}
			}
		}
	}

	m_dAveNeighborDist /= (m_nDataPoints * m_nNeighbors);
	m_dLearningRate = m_dAveNeighborDist * 10;
}

double GSquisher::CalculateDataPointError(unsigned char* pDataPoint)
{
	double dError = 0;
	double dDist;
	double dTheta;
	struct GSquisherNeighbor* pNeighbors = (struct GSquisherNeighbor*)pDataPoint;
	struct GSquisherData* pDataPointData = (struct GSquisherData*)(pDataPoint + m_nDataIndex);
	struct GSquisherData* pNeighborData;
	int n;
	for(n = 0; n < m_nNeighbors; n++)
	{
		dDist = CalculateDistance(pDataPoint, pNeighbors[n].m_pNeighbor);
		dDist -= pNeighbors[n].m_dDistance;
		dDist /= MAX(m_dAveNeighborDist, 1e-10);
		dDist *= dDist;
		dTheta = CalculateVectorCorrelation(pDataPoint, pNeighbors[n].m_pNeighbor, pNeighbors[n].m_pNeighborsNeighbor);
		dTheta -= pNeighbors[n].m_dCosTheta;
		dTheta *= dTheta;
		dTheta = MAX((double)0, dTheta - .01);
		pNeighborData = (struct GSquisherData*)(pNeighbors[n].m_pNeighbor + m_nDataIndex);
		if(pNeighborData->m_nCycle != pDataPointData->m_nCycle && pNeighborData->m_bAdjustable)
		{
			dDist /= m_nSmoothingAdvantage;
			dTheta /= m_nSmoothingAdvantage;
		}
		dError += (2 * dDist + dTheta);
	}
	return dError * dError;
}

int GSquisher::AjustDataPoint(unsigned char* pDataPoint, int nTargetDimensions, double* pError)
{
	bool bMadeProgress = true;
	double* pValues = (double*)(pDataPoint + m_nValueIndex);
	double dErrorBase = CalculateDataPointError(pDataPoint);
	double dError = 0;
	int n, nSteps;
	for(nSteps = 0; bMadeProgress; nSteps++)
	{
		bMadeProgress = false;
		for(n = 0; n < nTargetDimensions; n++)
		{
			pValues[n] += m_dLearningRate;
			dError = CalculateDataPointError(pDataPoint);
			if(dError >= dErrorBase)
			{
				pValues[n] -= (m_dLearningRate + m_dLearningRate);
				dError = CalculateDataPointError(pDataPoint);
			}
			if(dError >= dErrorBase)
				pValues[n] += m_dLearningRate;
			else
			{
				dErrorBase = dError;
				bMadeProgress = true;
			}
		}
	}
	*pError = dError;
	return nSteps - 1; // the -1 is to undo the last incrementor
}

void GSquisher::SquishBegin(int nTargetDimensions)
{
	GAssert(nTargetDimensions > 0 && nTargetDimensions < m_nDimensions, "out of range");
	m_nTargetDimensions = nTargetDimensions;
	m_nPass = 0;

	// Calculate metadata
	CalculateMetadata(nTargetDimensions);

	// Make the queue
	m_pQ = new GPointerQueue();
}

double GSquisher::SquishPass(int nSeedDataPoint)
{
	double* pValues;
	struct GSquisherData* pData;
	unsigned char* pDataPoint;
	struct GSquisherNeighbor* pNeighbors;
	int n, i;

	// Squish the extra dimensions
	for(n = 0; n < m_nDataPoints; n++)
	{
		pValues = (double*)(&m_pData[n * m_nRecordSize] + m_nValueIndex);
		for(i = m_nTargetDimensions; i < m_nDimensions; i++)
			pValues[i] *= m_dSquishingRate;
	}

	// Start at the seed point and correct outward in a breadth-first mannner
	m_pQ->Push(&m_pData[nSeedDataPoint * m_nRecordSize]);
	int nVisitedNodes = 0;
	int nSteps = 0;
	double dError = 0;
	double dTotalError = 0;
	while(m_pQ->GetSize() > 0)
	{
		// Check if this one has already been done
		pDataPoint = (unsigned char*)m_pQ->Pop();
		pData = (struct GSquisherData*)(pDataPoint + m_nDataIndex);
		if(pData->m_nCycle == m_nPass)
			continue;
		pData->m_nCycle = m_nPass;
		nVisitedNodes++;

		// Push all neighbors into the queue
		pNeighbors = (struct GSquisherNeighbor*)pDataPoint;
		for(n = 0; n < m_nNeighbors; n++)
			m_pQ->Push(pNeighbors[n].m_pNeighbor);

		// Ajust this data point
		if(pData->m_bAdjustable)
		{
			nSteps += AjustDataPoint(pDataPoint, m_nTargetDimensions, &dError);
			dTotalError += dError;
		}
	}
	//GAssert(nVisitedNodes * 1.25 > m_nDataPoints, "manifold appears poorly sampled");
	if(nSteps * 3 < m_nDataPoints)
		m_dLearningRate *= .9;
	else if(nSteps > m_nDataPoints * 9)
		m_dLearningRate /= .9;
//	printf("[Learning Rate: %f]", m_dLearningRate);
	m_nPass++;
	return dTotalError;
}

/*static*/ GArffData* GSquisher::DoSquisher(GArffRelation* pRelation, GArffData* pData, int nTargetDimensions, int nNeighbors, int nPreludeIterations, int nIterationsSinceBest)
{
	// Make the squisher
	int nDimensions = pRelation->GetInputCount();
	int nDataPoints = pData->GetRowCount();
	GSquisher squisher(nDataPoints, nDimensions, nNeighbors);
	squisher.SetData(pRelation, pData);

	// Do the squishing
	squisher.SquishBegin(nTargetDimensions);
	double d;
	double dBestError = 0;
	int n;
	for(n = 0; n < nPreludeIterations; n++)
		dBestError = squisher.SquishPass(rand() % nDataPoints);
	for(n = 0; n < nIterationsSinceBest; n++)
	{
		d = squisher.SquishPass(rand() % nDataPoints);
		if(d < dBestError)
		{
			dBestError = d;
			n = 0;
		}
	}

	// Convert data to an ARFF file
	int nAttributeCount = pRelation->GetAttributeCount();
	int nOutputs = pRelation->GetOutputCount();
	GArffData* pDataOut = new GArffData(nDataPoints);
	double* pRowIn;
	double* pRowOut;
	int i, nIndex;
	for(n = 0; n < nDataPoints; n++)
	{
		pRowIn = pData->GetRow(n);
		pRowOut = new double[nAttributeCount];
		pDataOut->AddRow(pRowOut);

		// Copy the output values straight over
		for(i = 0; i < nOutputs; i++)
		{
			nIndex = pRelation->GetOutputIndex(i);
			pRowOut[nIndex] = pRowIn[nIndex];
		}

		// Copy the squished input values
		pRowIn = squisher.GetDataPoint(n);
		for(i = 0; i < nDimensions; i++)
		{
			nIndex = pRelation->GetInputIndex(i);
			pRowOut[nIndex] = pRowIn[i];
		}
	}
	return pDataOut;
}

// --------------------------------------------------------------------------

GPCA::GPCA(GArffRelation* pRelation, GArffData* pData)
{
	m_pRelation = pRelation;
	m_pInputData = pData;
	m_pOutputData = NULL;
}

GPCA::~GPCA()
{
	delete(m_pOutputData);
}

/*static*/ GArffData* GPCA::DoPCA(GArffRelation* pRelation, GArffData* pData, GVector* pOutEigenValues)
{
	GPCA pca(pRelation, pData);
	pca.DoPCA(pOutEigenValues);
	return pca.DropOutputData();
}

void GPCA::DoPCA(GVector* pOutEigenValues)
{
	// Compute the eigenvectors
	GMatrix m;
	m_pInputData->ComputeCovarianceMatrix(&m, m_pRelation);
	GMatrix eigenVectors;
	m.ComputeEigenVectors(pOutEigenValues, &eigenVectors);
	m_pOutputData = new GArffData(m_pInputData->GetRowCount());
	int nRowCount = m_pInputData->GetRowCount();
	int nInputCount = m_pRelation->GetInputCount();
	int nOutputCount = m_pRelation->GetOutputCount();
	int nAttributeCount = m_pRelation->GetAttributeCount();
	double* pInputRow;
	double* pOutputRow;
	int n, i, j, nIndex;

	// Allocate space for the output
	for(n = 0; n < nRowCount; n++)
	{
		pOutputRow = new double[nAttributeCount];
		m_pOutputData->AddRow(pOutputRow);
	}

	// Compute the output
	Holder<double*> hEigenVector(new double[nInputCount]);
	double* pEigenVector = hEigenVector.Get();
	Holder<double*> hInputVector(new double[nInputCount]);
	double* pInputVector = hInputVector.Get();
	for(i = 0; i < nInputCount; i++)
	{
		nIndex = m_pRelation->GetInputIndex(i);
		for(n = 0; n < nInputCount; n++)
			pEigenVector[n] = eigenVectors.Get(i, n);
		for(n = 0; n < nRowCount; n++)
		{
			pInputRow = m_pInputData->GetRow(n);
			for(j = 0; j < nInputCount; j++)
				pInputVector[j] = pInputRow[m_pRelation->GetInputIndex(j)];
			pOutputRow = m_pOutputData->GetRow(n);
			pOutputRow[nIndex] = GVector::ComputeDotProduct(pInputVector, pEigenVector, nInputCount);
		}
	}
	for(i = 0; i < nOutputCount; i++)
	{
		for(n = 0; n < nRowCount; n++)
		{
			nIndex = m_pRelation->GetOutputIndex(i);
			pInputRow = m_pInputData->GetRow(n);
			pOutputRow = m_pOutputData->GetRow(n);
			pOutputRow[nIndex] = pInputRow[nIndex];
		}	
	}
}

GArffData* GPCA::DropOutputData()
{
	GArffData* pData = m_pOutputData;
	m_pOutputData = NULL;
	return pData;
}

// --------------------------------------------------------------------------

GLLE::GLLE(GArffRelation* pRelation, GArffData* pData, int nNeighbors)
{
	m_pRelation = pRelation;
	m_pInputData = pData;
	m_nNeighbors = nNeighbors;
	m_pNeighbors = NULL;
	m_pWeights = NULL;
	m_pOutputData = NULL;
}

GLLE::~GLLE()
{
	delete(m_pNeighbors);
	delete(m_pWeights);
	delete(m_pOutputData);
}

GArffData* GLLE::DropOutputData()
{
	GArffData* pData = m_pOutputData;
	m_pOutputData = NULL;
	return pData;
}

/*static*/ GArffData* GLLE::DoLLE(GArffRelation* pRelation, GArffData* pData, int nNeighbors)
{
	GLLE lle(pRelation, pData, nNeighbors);
	lle.FindNeighbors();
	lle.ComputeWeights();
	lle.ComputeEmbedding();
	return lle.DropOutputData();
}

void GLLE::FindNeighbors()
{
	// Allocate space for the neighbors
	delete(m_pNeighbors);
	m_pNeighbors = new int[m_nNeighbors * m_pInputData->GetRowCount()];

	// For now we'll just brute force it--todo: use a better neighbor finding technique
	double* pBestDistances = (double*)alloca(sizeof(double) * m_nNeighbors);
	int nRowCount = m_pInputData->GetRowCount();
	double* pRow1;
	double* pRow2;
	double d;
	int i, j, k, nWorstBestDistance;
	for(i = 0; i < nRowCount; i++)
	{
		// Reset the best distances
		for(j = 0; j < m_nNeighbors; j++)
			pBestDistances[j] = 1e100;
		nWorstBestDistance = 0;

		// Check every other point
		pRow1 = m_pInputData->GetRow(i);
		for(j = 0; j < nRowCount; j++)
		{
			if(j == i)
				continue;
			pRow2 = m_pInputData->GetRow(j);
			d = m_pRelation->ComputeInputDistanceSquared(pRow1, pRow2);
			if(d < pBestDistances[nWorstBestDistance])
			{
				pBestDistances[nWorstBestDistance] = d;
				m_pNeighbors[i * m_nNeighbors + nWorstBestDistance] = j;

				// Find new worst of the best distances
				nWorstBestDistance = 0;
				for(k = 1; k < m_nNeighbors; k++)
				{
					if(pBestDistances[k] > pBestDistances[nWorstBestDistance])
						nWorstBestDistance = k;
				}
			}
		}
		GAssert(pBestDistances[nWorstBestDistance] < 1e99, "Failed to find enough neighbors");
	}

//for(i = 0; i < m_nNeighbors * m_pInputData->GetRowCount(); i++)
//printf("%d\n", m_pNeighbors[i]);
}

void GLLE::ComputeWeights()
{
	int nDimensions = m_pRelation->GetInputCount();
	int nRowCount = m_pInputData->GetRowCount();
	double* pVec = (double*)alloca(sizeof(double) * m_nNeighbors);
	m_pWeights = new GMatrix(nRowCount, nRowCount); // todo: this should be a sparse matrix
	double* pRowNeighbor;
	double* pRow;
	double dSum;
	int n, i, j, nIndex;
	for(n = 0; n < nRowCount; n++)
	{
		// Create a matrix of all the neighbors normalized around the origin
		GMatrix z(m_nNeighbors, nDimensions);
		pRow = m_pInputData->GetRow(n);
		for(i = 0; i < m_nNeighbors; i++)
		{
			pRowNeighbor = m_pInputData->GetRow(m_pNeighbors[n * m_nNeighbors + i]);
			for(j = 0; j < nDimensions; j++)
			{
				nIndex = m_pRelation->GetInputIndex(j);
				z.Set(i, j, pRowNeighbor[nIndex] - pRow[nIndex]);
			}
		}

		// Compute local covariance (sort of, it's actually relative to the original point, not the mean, so it's not really true covariance)
		GMatrix transpose;
		transpose.Copy(&z);
		transpose.Transpose();
		GMatrix covariance;
		covariance.Multiply(&z, &transpose);
		GAssert(covariance.GetRowCount() == covariance.GetColumnCount(), "expected a square matrix");
		GAssert(covariance.GetRowCount() == m_nNeighbors, "unexpected size");

		// if the number of neighbors is more than the number of dimensions then the covariance will not be full rank so we need to regularize it
		if(m_nNeighbors > nDimensions)
		{
			double dReg = covariance.ComputeTrace() * .001;
			int nRows = covariance.GetRowCount();
			for(i = 0; i < nRows; i++)
				covariance.Set(i, i, covariance.Get(i, i) + dReg);
		}

		// Compute the weights
		for(i = 0; i < m_nNeighbors; i++)
			pVec[i] = 1;
		covariance.Solve(pVec);

		// Normalize the weights (so they sum to one)
		dSum = 0;
		for(i = 0; i < m_nNeighbors; i++)
			dSum += pVec[i];
		for(i = 0; i < nRowCount; i++)
			m_pWeights->Set(n, i, 0);
		for(i = 0; i < m_nNeighbors; i++)
			m_pWeights->Set(n, m_pNeighbors[n * m_nNeighbors + i], pVec[i] / dSum);
	}
/*	printf("Weights:\n");
	m_pWeights->Print();
	printf("\n");*/
}

void GLLE::ComputeEmbedding()
{
	//  Subtract the weights from the identity matrix
	int row, col;
	int nRowCount = m_pInputData->GetRowCount();
	for(row = 0; row < nRowCount; row++)
	{
		for(col = 0; col < nRowCount; col++)
			m_pWeights->Set(row, col, -m_pWeights->Get(row, col));
	}
	for(row = 0; row < nRowCount; row++)
		m_pWeights->Set(row, row, m_pWeights->Get(row, row) + 1);

	// Compute the cost matrix
	GMatrix transposed;
	transposed.Copy(m_pWeights);
	transposed.Transpose();
	GMatrix m;
	m.Multiply(&transposed, m_pWeights);
//	printf("Cost Matrix:\n");
//	m.Print();
//	printf("%f\t%f\n%f\t%f\n", m.Get(0, 0), m.Get(0, m.GetColumnCount() - 1), m.Get(m.GetRowCount() - 1, 0), m.Get(m.GetRowCount() - 1, m.GetColumnCount()));
//	printf("\n");

	// Compute eigen vectors
	GMatrix mEigenVectors;
	GVector vEigenValues;
/*
printf("Cost Matrix\n");
m.PrintCorners(2);
*/
	m.ComputeEigenVectors(&vEigenValues, &mEigenVectors);
//printf("Eigen Vectors\n");
//mEigenVectors.PrintCorners(2);
//mEigenVectors.Print();
/*
	// Print the eigen values and corresponding vectors
	printf("Eigen values and vectors\n");
	for(row = 0; row < nRowCount; row++)
	{
		printf("%f\n", vEigenValues.Get(row));
		//for(col = 0; col < nRowCount; col++)
		//	printf("\t%f\n", mEigenVectors.Get(row, col));
	}
*/
	// Allocate space for the output data
	m_pOutputData = new GArffData(nRowCount);
	int nAttributes = m_pRelation->GetAttributeCount();
	int nInputs = m_pRelation->GetInputCount();
	for(row = 0; row < nRowCount; row++)
		m_pOutputData->AddRow(new double[nAttributes]);

	// Compute the transformed data by dividing the eigen vectors by the square root of the eigen values
	double d;
	int nColumns = nInputs;
	//GAssert(nColumns < nRowCount, "Not enough data to compute values");
	int nEigen = 1;//nRowCount - 2;
	double* pInputRow;
	double* pOutputRow;
	int nIndex;
	d = sqrt((double)nRowCount);
	for(col = 0; col < nColumns; col++)
	{
/*		if(vEigenValues.Get(nEigen) >= 0)
			d = 1 / sqrt(vEigenValues.Get(nEigen));
		else
			d = 1 / (-sqrt(-vEigenValues.Get(nEigen)));*/
		nIndex = m_pRelation->GetInputIndex(col);
		for(row = 0; row < nRowCount; row++)
		{
			pOutputRow = m_pOutputData->GetRow(row);
			pOutputRow[nIndex] = mEigenVectors.Get(nEigen, row) * d;
		}
		nEigen++;//--;
	}
	int nOutputs = m_pRelation->GetOutputCount();
	for(col = 0; col < nOutputs; col++)
	{
		nIndex = m_pRelation->GetOutputIndex(col);
		for(row = 0; row < nRowCount; row++)
		{
			pInputRow = m_pInputData->GetRow(row);
			pOutputRow = m_pOutputData->GetRow(row);
			pOutputRow[nIndex] = pInputRow[nIndex];
		}
	}
/*
	// Print the final data
	printf("Final data:\n");
	for(row = 0; row < nRowCount; row++)
	{
		pOutputRow = m_pOutputData->GetRow(row);
		printf("%f", pOutputRow[0]);
		for(col = 1; col < nInputDimensions; col++)
			printf("\t%f", pOutputRow[col]);
		printf("\n");
	}
*/
}
