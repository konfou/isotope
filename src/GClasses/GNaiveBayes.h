#ifndef __GNAIVEBAYES_H__
#define __GNAIVEBAYES_H__

class GArffRelation;
class GArffData;
class GXMLTag;
class GPointerArray;
struct GNaiveBayesOutputAttr;

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

	void AddTrainingSample(double* pRow);
	void Train(GArffData* pData);
	double Eval(double* pRow);
	GXMLTag* ToXml(GPointerArray* pAttrNames);
	void SetEquivalentSampleSize(int n) { m_nEquivalentSampleSize = n; }
};

#endif // __GNAIVEBAYES_H__
