#ifndef __GMANIFOLD_H__
#define __GMANIFOLD_H__

class GArffRelation;
class GArffData;
class GPointerQueue;
class GMatrix;
class GVector;

// These algorithms are for taking data represented in many dimensions and
// compressing it into as few dimensions as possible. In each case they
// operate only on input attributes and simply take output attributes along
// as baggage. This way it makes sense to pre-process some collection of
// data with one of these algorithms before giving it to a traditional
// machine learning algorithm. Note that you must include both training and
// testing samples, even though you don't know the output values for the
// testing samples yet. That's okay because the output values don't affect
// the results of any of these algorithms, but they may be able to ajust
// the input values to something easier to learn. PCA is the simplest and
// most widely used of these algorithms, but it only operates on linear
// orthogonal axes whereas the other manifold learning algorithms should in
// theory produce more meaningful results.


// This is my own home-made manifold learning algorithm
class GSquisher
{
protected:
	int m_nDataPoints;
	int m_nDimensions;
	int m_nNeighbors;
	int m_nDataIndex;
	int m_nValueIndex;
	int m_nRecordSize;
	int m_nCurrentDimension;
	int m_nTargetDimensions;
	int m_nPass;
	int m_nSmoothingAdvantage;
	unsigned char* m_pData;
	double m_dAveNeighborDist;
	double m_dSquishingRate;
	double m_dLearningRate;
	GPointerQueue* m_pQ;

public:
	GSquisher(int nDataPoints, int nDimensions, int nNeighbors);
	~GSquisher();

	// This method initializes the squisher in preparation for iterative squishing
	void SquishBegin(int nTargetDimensions);

	// Perform one iteration of squishing
	double SquishPass(int nSeedDataPoint);

	// Squish your data into the number of dimensions specified.
	// You are repsonsible to delete the data set this returns.
	static GArffData* DoSquisher(GArffRelation* pRelation, GArffData* pData, int nTargetDimensions, int nNeighbors, int nPreludeIterations, int nIterationsSinceBest);

	// Sets the data points from a collection
	void SetData(GArffRelation* pRelation, GArffData* pData);

	// Set a single data point. For unsupervised manifold learning, bAdjustable
	// should always be true. For partially-supervised manifold learning, bAdjustable
	// should only be false if this is one of the supervised (fixed) points.
	void SetDataPoint(int n, double* pValues, bool bAdjustable);

	// Get a single (multi-dimensional) data point
	double* GetDataPoint(int n);

	// Returns the number of data points
	int GetDataPointCount() { return m_nDataPoints; }

	// Set the rate of squishing. (.99 is a good value)
	void SetSquishingRate(double d) { m_dSquishingRate = d; }

	// todo: figure out how to explain what this is
	void SetSmoothingAdvantage(int n) { m_nSmoothingAdvantage = n; }

	// for internal use only
	int DataPointSortCompare(unsigned char* pA, unsigned char* pB);

protected:
	void CalculateMetadata(int nTargetDimensions);
	int FindMostDistantNeighbor(struct GSquisherNeighbor* pNeighbors);
	double CalculateDistance(unsigned char* pA, unsigned char* pB);
	double CalculateVectorCorrelation(unsigned char* pA, unsigned char* pVertex, unsigned char* pB);
	double CalculateDataPointError(unsigned char* pDataPoint);
	int AjustDataPoint(unsigned char* pDataPoint, int nTargetDimensions, double* pError);
};



// Principle Component Analysis
class GPCA
{
protected:
	GArffRelation* m_pRelation;
	GArffData* m_pInputData;
	GArffData* m_pOutputData;

	GPCA(GArffRelation* pRelation, GArffData* pData);
public:
	~GPCA();

	// Performs principle component analysis on the data. It doesn't drop any dimensions,
	// but you can do that yourself as a post-processing step. The eigenvalues are returned
	// as well to help you decide which dimensions to drop.
	// You are repsonsible to delete the data set this returns.
	static GArffData* DoPCA(GArffRelation* pRelation, GArffData* pData, GVector* pOutEigenValues);

protected:
	void DoPCA(GVector* pOutEigenValues);
	GArffData* DropOutputData();
};




// Local Linear Embedding
class GLLE
{
protected:
	GArffRelation* m_pRelation;
	GArffData* m_pInputData;
	GArffData* m_pOutputData;
	int m_nNeighbors;
	int* m_pNeighbors;
	GMatrix* m_pWeights;

	GLLE(GArffRelation* pRelation, GArffData* pData, int nNeighbors);
public:
	~GLLE();

	// Performs LLE analysis on the data. It doesn't drop any dimensions,
	// but you can do that yourself as a post-processing step. If you measure
	// the variance of each dimension, it should be obvious which ones to drop.
	// You are repsonsible to delete the data set this returns.
	static GArffData* DoLLE(GArffRelation* pRelation, GArffData* pData, int nNeighbors);

protected:
	void FindNeighbors();
	void ComputeWeights();
	void ComputeEmbedding();
	GArffData* DropOutputData();
};


#endif // __GMANIFOLD_H__
