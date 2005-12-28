#ifndef __GNAIVEBAYES_H__
#define __GNAIVEBAYES_H__

#include "GLearner.h"

class GXMLTag;
class GPointerArray;
struct GNaiveBayesOutputAttr;

// A naive Bayes classifier
class GNaiveBayes : public GSupervisedLearner
{
protected:
	int m_nSampleCount;
	int m_nOutputs;
	GNaiveBayesOutputAttr** m_pOutputs;
	int m_nEquivalentSampleSize;
	double* m_pDiscretizeMins;
	double* m_pDiscretizeRanges;
	int m_nDiscretizeBuckets;

public:
	GNaiveBayes(GArffRelation* pRelation);
	GNaiveBayes(GXMLTag* pTag);
	virtual ~GNaiveBayes();

	// Adds a single training sample to the collection
	void AddTrainingSample(double* pRow);

	// Train using all the samples in a collection
	virtual void Train(GArffData* pData);

	// Evaluates and returns the confidence/probability that it is correct
	double EvalWithConfidence(double* pRow);

	// Evaluates the input values in the provided row and
	// deduce the output values
	virtual void Eval(double* pRow);

	// Serialize the internal representation
	GXMLTag* ToXml(GPointerArray* pAttrNames);

	void SetEquivalentSampleSize(int n) { m_nEquivalentSampleSize = n; }

	void ComputeDiscretizeRanges(GArffData* pData);

	void SetDiscretizeBuckets(int n) { m_nDiscretizeBuckets = n; }

protected:
	void DiscretizeRow(double* pRow);
	void UndiscretizeRow(double* pRow);
};

#endif // __GNAIVEBAYES_H__
