#ifndef __GGENETIC_H__
#define __GGENETIC_H__

class GArffRelation;
class GArffData;
class GNeuralNet;

class GGeneticBits
{
protected:
	int m_nPopulation;
	int m_nBits;
	int m_nUintsPerSample;
	int m_nBestCurrentCandidate;
	unsigned int* m_pData;
	unsigned int* m_pData2;
	double* m_pFitness;

public:
	GGeneticBits(int nPopulation, int nBits);
	virtual ~GGeneticBits();

	// Converts the nLength bits starting at the nIndex'th bit in the pBits
	// array of unsigned integers from a Gray-code integer value to a double
	// ranging between 0 and 1. (This is for making the genetic algorithm
	// learn real values.)
	static double BitsToDouble(unsigned int* pBits, int nIndex, int nLength);

	virtual double MeasureFitness(unsigned int* pBits) = 0;

	unsigned int* GetRow(int nRow) { return &m_pData[nRow * m_nUintsPerSample]; }
	void DoFitnessProportionateSelection(double dSurvivalRate, double dMutationRate, int nBitsPerCrossOverPoint);
	void DoTournamentSelection(double dProbThatMoreFitSurvives, double dSurvivalRate, double dMutationRate, int nBitsPerCrossOverPoint);

protected:
	double SumFitness();
	void CrossOver(unsigned int* pOutChild, int nMaternalBits, unsigned int* pMother, unsigned int* pFather);
	void InvertBit(int nRow, int nBit);
	int Find(double dSumFitness);
	void MeasureEverybodysFitness();
};





class GGeneticNeuralNet : public GGeneticBits
{
protected:
	GNeuralNet* m_pNN;
	int m_nBitsPerWeight;
	int m_nWeightCount;
	int m_nMaxTestSamples;
	double* m_pWeights;
	GArffRelation* m_pRelation;
	GArffData* m_pData;
	double* m_pSample;

public:
	GGeneticNeuralNet(int nPopulation, int nBitsPerWeight, GNeuralNet* pNN, GArffRelation* pRelation, GArffData* pValidationData, int nMaxTestSamples);
	~GGeneticNeuralNet();

	virtual double MeasureFitness(unsigned int* pBits);

};

#endif // __GGENETIC_H__
