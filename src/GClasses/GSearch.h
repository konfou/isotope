#ifndef __GSEARCH_H__
#define __GSEARCH_H__

// This tells the search algorithm how bad a particular spot in the search space is
class GSearchCritic
{
protected:
	int m_nVectorSize;
	double m_dBestError;
	double* m_pBestYet;

public:
	// nVectorSize is the number of dimensions in the search space
	GSearchCritic(int nVectorSize);

	virtual ~GSearchCritic();

	// Compute the error of the given vector
	double Critique(double* pVector);

	// Returns the best vector that was ever passed to the Critique method
	double* GetBestYet() { return m_pBestYet; }

	// Returns the error computed for the best vector ever passed
	// to the Critique method
	double GetBestError() { return m_dBestError; }

	// Returns the dimensionality of the vector
	int GetVectorSize() { return m_nVectorSize; }

protected:
	// Computes the error of the given vector with respect to the search space
	virtual double ComputeError(double* pVector) = 0;
};



// The goal of a search algorithm is to find a vector that
// minimizes the error that the critic reports. The GSearchCritic
// will keep track of the best vector yet found, so when you want
// the results ask the GSearchCritic.
class GSearch
{
protected:
	GSearchCritic* m_pCritic;

public:
	GSearch(GSearchCritic* pCritic);
	virtual ~GSearch();

	// Call this method in a loop to perform the search
	virtual void Iterate() = 0;
};

#endif // __GSEARCH_H__
