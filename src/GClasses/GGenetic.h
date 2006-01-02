#ifndef __GGENETIC_H__
#define __GGENETIC_H__

#include "GSearch.h"

class GArffRelation;
class GArffData;
class GNeuralNet;
class GEvolutionarySearchHelper;


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

	// The critic
	virtual double MeasureFitness(unsigned int* pBits) = 0;

	// Returns a single row of bits from the population
	unsigned int* GetRow(int nRow) { return &m_pData[nRow * m_nUintsPerSample]; }

	// Performs a single evolutionary generation using fitness proportionate
	// selection
	void DoFitnessProportionateSelection(double dSurvivalRate, double dMutationRate, int nBitsPerCrossOverPoint);

	// Performs a single evolutionary generation using tournament selection
	void DoTournamentSelection(double dProbThatMoreFitSurvives, double dSurvivalRate, double dMutationRate, int nBitsPerCrossOverPoint);

protected:
	double SumFitness();
	void CrossOver(unsigned int* pOutChild, int nMaternalBits, unsigned int* pMother, unsigned int* pFather);
	void InvertBit(int nRow, int nBit);
	int Find(double dSumFitness);
	void MeasureEverybodysFitness();
};





class GEvolutionarySearch : public GSearch
{
friend class GEvolutionarySearchHelper;
protected:
	GEvolutionarySearchHelper* m_pHelper;
	double m_dProbThatMoreFitSurvives;
	double m_dSurvivalRate;
	double m_dMutationRate;
	int m_nBitsPerCrossOverPoint;

public:
	GEvolutionarySearch(GSearchCritic* pCritic, int nPopulation, int nBitsPerWeight);
	virtual ~GEvolutionarySearch();

	virtual void Iterate();

	// d should be a value between .5 and 1. A larger value indicates a more
	// greedy search.
	void SetProbThatMoreFitSurvives(double d) { m_dProbThatMoreFitSurvives = d; }

	// d should be a value between 0 and 1.
	void SetSurvivalRate(double d) { m_dSurvivalRate = d; }

	// d should be a value between 0 and 1
	void SetMutationRate(double d) { m_dMutationRate = d; }

	// n should be between 1 and nBitsPerWeight inclusively. 1 indicates that
	// a crossover can happen at any point. nBitsPerWeight would mean it can
	// only happen at weight boundaries.
	void SetBitsPerCrossOverPoint(int n) { m_nBitsPerCrossOverPoint = n; }

protected:
	// This method is called by GEvolutionarySearchHelper to access the critic
	double Critique(double* pVector);
};


#endif // __GGENETIC_H__
