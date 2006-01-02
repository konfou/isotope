#include "GMacros.h"
#include "GGenetic.h"
#include "GNeuralNet.h"
#include "GArff.h"
#include "GBits.h"

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
		d *= d;
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
	if(nSurvivors > m_nPopulation)
		nSurvivors = m_nPopulation;
	GAssert(nSurvivors >= 0, "out of range");
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
//	for(n = 0; n < m_nUintsPerSample; n++)
//		m_pData2[m_nUintsPerSample * i + n] = GBits::GetRandomUint();
//	i++;

	// Offspring fill the remaining slots
	for( ; i < m_nPopulation; i++)
	{
		nFather = rand() % m_nPopulation;
		nMother = rand() % m_nPopulation;
		nCrossOverPoint = (rand() % (m_nBits / nBitsPerCrossOverPoint)) * nBitsPerCrossOverPoint;
		CrossOver(&m_pData2[m_nUintsPerSample * i], nCrossOverPoint, &m_pData[m_nUintsPerSample * nMother], &m_pData[m_nUintsPerSample * nFather]);
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
	int i, j, nFather, nMother, nCrossOverPoint;
	for(i = 0; i < nSurvivors; i++)
	{
		j = rand() % m_nPopulation;
		if(GBits::GetRandomDouble() > dProbThatMoreFitSurvives)
		{
			// Set j to the less fit candidate
			if(m_pFitness[i] < m_pFitness[j])
				j = i;
		}
		else
		{
			// Set j to the more fit candidate
			if(m_pFitness[i] > m_pFitness[j])
				j = i;
		}
		memcpy(&m_pData2[m_nUintsPerSample * i], &m_pData[m_nUintsPerSample * j], m_nUintsPerSample * sizeof(unsigned int));
	}

	// The very best candidate also survives
//	memcpy(&m_pData2[m_nUintsPerSample * i], &m_pData[m_nUintsPerSample * m_nBestCurrentCandidate], m_nUintsPerSample * sizeof(unsigned int));
//	i++;

	// Make one totally random child to keep the system from getting wedged
//	for(n = 0; n < m_nUintsPerSample; n++)
//		m_pData2[m_nUintsPerSample * i + n] = GBits::GetRandomUint();
//	i++;

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
	{
		n |= (pBits[i + 1] << (BITS_PER_UINT - j));
	}

	// Mask away the extra bits
	unsigned int mask = ~0;
	mask >>= (BITS_PER_UINT - nLength);
	n &= mask;

	// Convert from Gray code to a value
	n = GBits::GrayCodeToBinary(n);

	// Convert to a double that ranges from 0 to 1
	return (double)n / (((unsigned int)1 << nLength) - 1);
}

// -----------------------------------------------------------------------

class GEvolutionarySearchHelper : public GGeneticBits
{
protected:
	GEvolutionarySearch* m_pParent;
	int m_nBitsPerWeight;
	int m_nWeightCount;
	double* m_pWeights;

public:
	GEvolutionarySearchHelper(GEvolutionarySearch* pParent, int nPopulation, int nBitsPerWeight, int nVectorSize);
	~GEvolutionarySearchHelper();

	virtual double MeasureFitness(unsigned int* pBits);
	void SetWeightsFromBits(unsigned int* pBits);
};


GEvolutionarySearchHelper::GEvolutionarySearchHelper(GEvolutionarySearch* pParent, int nPopulation, int nBitsPerWeight, int nVectorSize)
: GGeneticBits(nPopulation, nBitsPerWeight * nVectorSize)
{
	m_pParent = pParent;
	m_nBitsPerWeight = nBitsPerWeight;
	m_nWeightCount = nVectorSize;
	m_pWeights = new double[m_nWeightCount];
}

GEvolutionarySearchHelper::~GEvolutionarySearchHelper()
{
	delete(m_pWeights);
}

void GEvolutionarySearchHelper::SetWeightsFromBits(unsigned int* pBits)
{
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
}

/*virtual*/ double GEvolutionarySearchHelper::MeasureFitness(unsigned int* pBits)
{
	SetWeightsFromBits(pBits);
	double dError = m_pParent->Critique(m_pWeights);
	return (1.0 / (dError + .001)); // todo: is this a good measure of fitness?
}

// -----------------------------------------------------------------------

GEvolutionarySearch::GEvolutionarySearch(GSearchCritic* pCritic, int nPopulation, int nBitsPerWeight)
: GSearch(pCritic)
{
	m_dProbThatMoreFitSurvives = .9;
	m_dSurvivalRate = .5;
	m_dMutationRate = .6;
	m_nBitsPerCrossOverPoint = nBitsPerWeight;
	m_pHelper = new GEvolutionarySearchHelper(this, nPopulation, nBitsPerWeight, pCritic->GetVectorSize());
}

/*virtual*/ GEvolutionarySearch::~GEvolutionarySearch()
{
	delete(m_pHelper);
}

/*virtual*/ void GEvolutionarySearch::Iterate()
{
	m_pHelper->DoTournamentSelection(m_dProbThatMoreFitSurvives, m_dSurvivalRate, m_dMutationRate, m_nBitsPerCrossOverPoint);
}

double GEvolutionarySearch::Critique(double* pVector)
{
	return m_pCritic->Critique(pVector);
}
