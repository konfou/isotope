#ifndef __GNAIVEBAYES_H__
#define __GNAIVEBAYES_H__

class GArffRelation;

class GNaiveBayes
{
protected:
	GArffRelation* m_pRelation;
	int*** m_pInputCounts;
	int** m_pOutputCounts;
	int m_nSampleCount;

public:
	GNaiveBayes(GArffRelation* pRelation);
	~GNaiveBayes();

	void Train(GArffData* pData);

};

#endif // __GNAIVEBAYES_H__