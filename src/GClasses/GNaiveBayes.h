#ifndef __GNAIVEBAYES_H__
#define __GNAIVEBAYES_H__

class GArffRelation;
class GArffData;
class GXMLTag;
class GPointerArray;
struct GNaiveBayesOutputAttr;

// A naive Bayes classifier
class GNaiveBayes
{
protected:
	int m_nSampleCount;
	int m_nOutputs;
	GNaiveBayesOutputAttr** m_pOutputs;
	int m_nEquivalentSampleSize;

public:
	GNaiveBayes(GArffRelation* pRelation);
	GNaiveBayes(GXMLTag* pTag);
	~GNaiveBayes();

	// Adds a single training sample to the collection
	void AddTrainingSample(double* pRow);

	// Train using all the samples in a collection
	void Train(GArffData* pData);

	// Evaluate
	double Eval(double* pRow);

	// Serialize the internal representation
	GXMLTag* ToXml(GPointerArray* pAttrNames);

	void SetEquivalentSampleSize(int n) { m_nEquivalentSampleSize = n; }
};

#endif // __GNAIVEBAYES_H__
