#include "GGenetic.h"
#include "GMacros.h"
#include "GBits.h"
#include "GNeuralNet.h"
#include "GArff.h"

GGeneticBits::GGeneticBits(int nPopulation, int nBits)
{
	m_nPopulation = nPopulation;
	m_nBits = nBits;
	m_nUintsPerSample = (nBits - 1) / BITS_PER_UINT + 1;
	int nSize = nPopulation * m_nUintsPerSample;
	m_pData = new unsigned int[nSize];
	m_pData2 = new unsigned int[nSize];
	int n;
	for(n = 0; n < nSize; n++)
		m_pData[n] = GBits::GetRandomUint();
	m_pFitness = new double[nPopulation];
	m_nBestCurrentCandidate = 0;
}

GGeneticBits::~GGeneticBits()
{
	delete(m_pData);
	delete(m_pData2);
	delete(m_pFitness);
}

void GGeneticBits::InvertBit(int nRow, int nBit)
{
	unsigned int* pInt = &m_pData[m_nUintsPerSample * nRow + nBit / BITS_PER_UINT];
	unsigned int nMask = 1 << (nBit % BITS_PER_UINT);
	(*pInt) ^= nMask;
}

void GGeneticBits::CrossOver(unsigned int* pOutChild, int nMaternalBits, unsigned int* pMother, unsigned int* pFather)
{
	int nUints = 0;
	while(nMaternalBits >= (int)BITS_PER_UINT && nUints < (int)m_nUintsPerSample)
	{
		*(pOutChild++) = *(pMother++);
		pFather++;
		nMaternalBits -= BITS_PER_UINT;
		nUints++;
	}
	if(nUints >= m_nUintsPerSample)
		return;
	unsigned int nMask = (1 << nMaternalBits) - 1;
	*pOutChild = nMask & *pMother;
	*(pOutChild++) |= ((~nMask) & *(pFather++));
	nUints++;
	while(nUints < m_nUintsPerSample)
	{
		*(pOutChild++) = *(pFather++);
		nUints++;
	}
}

void GGeneticBits::MeasureEverybodysFitness()
{
	// Measure everyone's fitness
	double dBest = 0;
	double d;
	int n;
	for(n = 0; n < m_nPopulation; n++)
	{
		d = MeasureFitness(&m_pData[m_nUintsPerSample * n]);
		if(d > dBest)
		{
			m_nBestCurrentCandidate = n;
			dBest = d;
		}
		m_pFitness[n] = d;
	}
}

double GGeneticBits::SumFitness()
{
	int n;
	for(n = 1; n < m_nPopulation; n++)
		m_pFitness[n] += m_pFitness[n - 1];
	return m_pFitness[m_nPopulation - 1];
}

int GGeneticBits::Find(double dSumFitness)
{
	int nMin = 0;
	int nMax = m_nPopulation - 1;
	GAssert(nMax > nMin, "out of range");
	int nMid;
	while(true)
	{
		nMid = (nMin + nMax) / 2;
		if(m_pFitness[nMid] > dSumFitness)
			nMax = nMid;
		else if(m_pFitness[nMid] <= dSumFitness)
			nMin = nMid + 1;
		if(nMin >= nMax)
			return nMax;
	}
}

void GGeneticBits::DoFitnessProportionateSelection(double dSurvivalRate, double dMutationRate, int nBitsPerCrossOverPoint)
{
	// Select survivors with probability proportional to fitness
	MeasureEverybodysFitness();
	double dTotal = SumFitness();
	int nSurvivors = (int)(m_nPopulation * dSurvivalRate);
	GAssert(nSurvivors >= 0 && nSurvivors < m_nPopulation, "out of range");
	double d;
	int n, i, nFather, nMother, nCrossOverPoint;
	for(i = 0; i < nSurvivors; i++)
	{
		d = GBits::GetRandomDouble() * dTotal;
		n = Find(d);
		memcpy(&m_pData2[m_nUintsPerSample * i], &m_pData[m_nUintsPerSample * n], m_nUintsPerSample * sizeof(unsigned int));
	}

	// The very best candidate also survives
//	memcpy(&m_pData2[m_nUintsPerSample * i], &m_pData[m_nUintsPerSample * m_nBestCurrentCandidate], m_nUintsPerSample * sizeof(unsigned int));
//	i++;

	// Make one totally random child to keep the system from getting wedged
	for(n = 0; n < m_nUintsPerSample; n++)
		m_pData2[m_nUintsPerSample * i + n] = GBits::GetRandomUint();
	i++;

	// Offspring fill the remaining slots
	for( ; i < m_nPopulation; i++)
	{
		nFather = rand() % m_nPopulation;
		nMother = rand() % m_nPopulation;
		nCrossOverPoint = (rand() % (m_nBits / nBitsPerCrossOverPoint)) * nBitsPerCrossOverPoint;
		CrossOver(&m_pData2[m_nUintsPerSample * i], nBitsPerCrossOverPoint, &m_pData[m_nUintsPerSample * nMother], &m_pData[m_nUintsPerSample * nFather]);
	}

	// Swap in the new generation
	unsigned int* pData = m_pData;
	m_pData = m_pData2;
	m_pData2 = pData;

	// Do mutations
	int nMutations = (int)(m_nPopulation * dMutationRate);
	for(i = 0; i < nMutations; i++)
		InvertBit(rand() % m_nPopulation, rand() % m_nBits);
}

void GGeneticBits::DoTournamentSelection(double dProbThatMoreFitSurvives, double dSurvivalRate, double dMutationRate, int nBitsPerCrossOverPoint)
{
	// Perform tournaments to see who survives
	MeasureEverybodysFitness();
	int nSurvivors = (int)(m_nPopulation * dSurvivalRate);
	GAssert(nSurvivors >= 0 && nSurvivors < m_nPopulation, "out of range");
	int n, i, j, nFather, nMother, nCrossOverPoint;
	for(i = 0; i < nSurvivors; i++)
	{
		j = rand() % m_nPopulation;
		if(m_pFitness[i] > m_pFitness[j])
			j = i;
		memcpy(&m_pData2[m_nUintsPerSample * i], &m_pData[m_nUintsPerSample * j], m_nUintsPerSample * sizeof(unsigned int));
	}

	// The very best candidate also survives
//	memcpy(&m_pData2[m_nUintsPerSample * i], &m_pData[m_nUintsPerSample * m_nBestCurrentCandidate], m_nUintsPerSample * sizeof(unsigned int));
//	i++;

	// Make one totally random child to keep the system from getting wedged
	for(n = 0; n < m_nUintsPerSample; n++)
		m_pData2[m_nUintsPerSample * i + n] = GBits::GetRandomUint();
	i++;

	// Offspring fill the remaining slots
	for( ; i < m_nPopulation; i++)
	{
		nFather = rand() % m_nPopulation;
		nMother = rand() % m_nPopulation;
		nCrossOverPoint = (rand() % (m_nBits / nBitsPerCrossOverPoint)) * nBitsPerCrossOverPoint;
		CrossOver(&m_pData2[m_nUintsPerSample * i], nBitsPerCrossOverPoint, &m_pData[m_nUintsPerSample * nMother], &m_pData[m_nUintsPerSample * nFather]);
	}

	// Swap in the new generation
	unsigned int* pData = m_pData;
	m_pData = m_pData2;
	m_pData2 = pData;

	// Do mutations
	int nMutations = (int)(m_nPopulation * dMutationRate);
	for(i = 0; i < nMutations; i++)
		InvertBit(rand() % m_nPopulation, rand() % m_nBits);
}

/*static*/ double GGeneticBits::BitsToDouble(unsigned int* pBits, int nIndex, int nLength)
{
	// Get the portion from the first uint
	GAssert(nLength <= (int)BITS_PER_UINT, "out of range");
	int i = (int)(nIndex / BITS_PER_UINT);
	int j = (int)(nIndex % BITS_PER_UINT);
	unsigned int n = pBits[i] >> j;

	// Union with the portion from the second uint
	if(j + nLength > (int)BITS_PER_UINT)
		n |= (pBits[i + 1] << (BITS_PER_UINT - j));

	// Mask away the extra bits
	n &= ((~0) >> (BITS_PER_UINT - nLength));

	// Convert from Gray code to a value
	n = GBits::GrayCodeToBinary(n);

	// Convert to a double that ranges from 0 to 1
	return (double)n / (((unsigned int)1 << nLength) - 1);
}

// -----------------------------------------------------------------------

GGeneticNeuralNet::GGeneticNeuralNet(int nPopulation, int nBitsPerWeight, GNeuralNet* pNN, GArffRelation* pRelation, GArffData* pValidationData, int nMaxTestSamples)
: GGeneticBits(nPopulation, nBitsPerWeight * pNN->GetWeightCount())
{
	m_nBitsPerWeight = nBitsPerWeight;
	m_nWeightCount = pNN->GetWeightCount();
	m_nMaxTestSamples = nMaxTestSamples;
	m_pWeights = new double[m_nWeightCount];
	m_pRelation = pRelation;
	m_pData = pValidationData;
	m_pSample = new double[pRelation->GetAttributeCount()];
}

GGeneticNeuralNet::~GGeneticNeuralNet()
{
	delete(m_pWeights);
	delete(m_pSample);
}

/*virtual*/ double GGeneticNeuralNet::MeasureFitness(unsigned int* pBits)
{
	// Convert the bits to a set of weights
	int nPos = 0;
	int n;
	double d;
	for(n = 0; n < m_nWeightCount; n++)
	{
		d = BitsToDouble(pBits, nPos, m_nBitsPerWeight);
		d *= 10;
		d -= 5;
		m_pWeights[n] = d;
		nPos += m_nBitsPerWeight;
	}

	// Copy the weights into the neural net
	m_pNN->SetWeights(m_pWeights);

	// Measure fitness
	double* pRow;
	GArffAttribute* pAttr;
	int nRowCount = m_pData->GetRowCount();
	int nRowSize = sizeof(double) * m_pRelation->GetAttributeCount();
	int nOutputs = m_pRelation->GetOutputCount();
	int i, j, nIndex;
	double dFitness = 0;
	for(i = MIN(m_nMaxTestSamples, nRowCount); i > 0; i--)
	{
		pRow = m_pData->GetRow(rand() % nRowCount);
		memcpy(m_pSample, pRow, nRowSize);
		m_pNN->Eval(m_pSample);
		for(j = 0; j < nOutputs; j++)
		{
			nIndex = m_pRelation->GetOutputIndex(j);
			pAttr = m_pRelation->GetAttribute(nIndex);
			if(pAttr->IsContinuous())
			{
				d = m_pSample[nIndex] - pRow[nIndex];
				dFitness += (1 / (d * d + .00001));
			}
			else
			{
				if((int)m_pSample[nIndex] == (int)pRow[nIndex])
					dFitness += 1;
			}
		}
	}
	return dFitness;
}

