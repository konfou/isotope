/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GDecisionTree.h"
#include "GArff.h"
#include "../GClasses/GMacros.h"

//#define DEBUGLOG

#ifdef DEBUGLOG
#define dbglog0(a)			fprintf(stderr, a)
#define dbglog1(a,b)		fprintf(stderr, a, b)
#define dbglog2(a,b,c)		fprintf(stderr, a, b, c)
#define dbglog3(a,b,c,d)	fprintf(stderr, a, b, c, d)
#else // DEBUGLOG
#define dbglog0(a)		((void)0)
#define dbglog1(a,b)	((void)0)
#define dbglog2(a,b,c)	((void)0)
#define dbglog3(a,b,d)	((void)0)
#endif // !DEBUGLOG




class GDecisionTreeNode
{
friend class GDecisionTree;
protected:
	int m_nAttribute;
	int m_nChildren;
	int m_nSampleSize;
	GDecisionTreeNode** m_ppChildren;
	double* m_pOutputValues;

public:
	GDecisionTreeNode()
	{
		m_nAttribute = -1;
		m_nChildren = 0;
		m_nSampleSize = 0;
		m_ppChildren = NULL;
		m_pOutputValues = NULL;
	}

	~GDecisionTreeNode()
	{
		if(m_ppChildren)
		{
			int n;
			for(n = 0; n < m_nChildren; n++)
				delete(m_ppChildren[n]);
			delete(m_ppChildren);
		}
		delete(m_pOutputValues);
	}

	GDecisionTreeNode* DeepCopy(GArffRelation* pRelation, GDecisionTreeNode* pInterestingNode, GDecisionTreeNode** ppOutInterestingCopy)
	{
		GDecisionTreeNode* pNewNode = new GDecisionTreeNode();
		pNewNode->m_nAttribute = m_nAttribute;
		pNewNode->m_nChildren = m_nChildren;
		pNewNode->m_nSampleSize = m_nSampleSize;
		if(m_ppChildren)
		{
			GAssert(!m_pOutputValues, "Can't have children and output values");
			pNewNode->m_pOutputValues = NULL;
			pNewNode->m_ppChildren = new GDecisionTreeNode*[m_nChildren];
			int n;
			for(n = 0; n < m_nChildren; n++)
				pNewNode->m_ppChildren[n] = m_ppChildren[n]->DeepCopy(pRelation, pInterestingNode, ppOutInterestingCopy);
		}
		else
		{
			GAssert(m_pOutputValues, "expected output values");
			pNewNode->m_ppChildren = NULL;
			int nCount = pRelation->GetOutputCount();
			pNewNode->m_pOutputValues = new double[nCount];
			int n;
			for(n = 0; n < nCount; n++)
				pNewNode->m_pOutputValues[n] = m_pOutputValues[n];
		}
		if(this == pInterestingNode)
			*ppOutInterestingCopy = pNewNode;
		return pNewNode;
	}

	void Print(GArffRelation* pRelation, int nSpaces, const char* szValue)
	{
		int n;
		for(n = 0; n < nSpaces; n++)
			printf("  ");
		if(m_ppChildren)
		{
			GArffAttribute* pAttr = pRelation->GetAttribute(m_nAttribute);
			printf("%s -> %s?\n", szValue, pAttr->GetName());
			for(n = 0; n < m_nChildren; n++)
				m_ppChildren[n]->Print(pRelation, nSpaces + 1, pAttr->GetValue(n));
		}
		else
		{
			int nCount = pRelation->GetOutputCount();
			printf("%s -> ", szValue);
			for(n = 0; n < nCount; n++)
			{
				GArffAttribute* pAttr = pRelation->GetAttribute(pRelation->GetOutputIndex(n));
				if(n > 0)
					printf(", ");
				printf("%s=%s", pAttr->GetName(), pAttr->GetValue((int)m_pOutputValues[n]));
			}
			printf("\n");
		}
	}

	void CountValues(int nOutput, int* pnCounts)
	{
		if(m_ppChildren)
		{
			int n;
			for(n = 0; n < m_nChildren; n++)
				m_ppChildren[n]->CountValues(nOutput, pnCounts);
		}
		else
		{
			int nVal = (int)m_pOutputValues[nOutput];
			pnCounts[nVal] += m_nSampleSize;
		}
	}

	double FindSumOutputValue(int nOutput)
	{
		if(m_ppChildren)
		{
			double dSum = 0;
			int n;
			for(n = 0; n < m_nChildren; n++)
				dSum += m_ppChildren[n]->FindSumOutputValue(nOutput);
			return dSum;
		}
		else
			return m_pOutputValues[nOutput] * m_nSampleSize;
	}

	void PruneChildren(GArffRelation* pRelation)
	{
		// Create output values by finding the most common outputs among children
		GAssert(m_ppChildren, "This is a leaf node");
		int nOutputCount = pRelation->GetOutputCount();
		m_pOutputValues = new double[nOutputCount];
		int n;
		for(n = 0; n < nOutputCount; n++)
		{
			// Count the number of occurrences of each possible value for this output attribute
			GArffAttribute* pAttr = pRelation->GetAttribute(pRelation->GetOutputIndex(n));
			int nValueCount = pAttr->GetValueCount();
			if(nValueCount <= 0)
				m_pOutputValues[n] = FindSumOutputValue(n) / m_nSampleSize;
			else
			{
				Holder<int*> hCounts(new int[nValueCount]);
				int* pnCounts = hCounts.Get();
				memset(pnCounts, '\0', sizeof(int) * nValueCount);
				CountValues(n, pnCounts);
	
				// Find the most frequent value
				int i;
				int nMax = 0;
				for(i = 1; i < nValueCount; i++)
				{
					if(pnCounts[i] > pnCounts[nMax])
						nMax = i;
				}
				m_pOutputValues[n] = (double)nMax;
			}
		}

		// Delete the children
		for(n = 0; n < m_nChildren; n++)
			delete(m_ppChildren[n]);
		delete(m_ppChildren);
		m_ppChildren = NULL;
	}
};

// -----------------------------------------------------------------

GDecisionTree::GDecisionTree(GArffRelation* pRelation, GArffData* pTrainingData)
{
	m_pRelation = pRelation;
	if(pTrainingData->GetRowCount() > 0)
	{
		m_pRoot = new GDecisionTreeNode();
		int nAttributes = m_pRelation->GetAttributeCount();
		Holder<bool*> hUsedAttributes(new bool[nAttributes]);
		bool* pUsedAttributes = hUsedAttributes.Get();
		int n;
		for(n = 0; n < nAttributes; n++)
			pUsedAttributes[n] = false;
		BuildNode(m_pRoot, pTrainingData, pUsedAttributes);
	}
	else
		m_pRoot = NULL;
}

GDecisionTree::GDecisionTree(GDecisionTree* pThat, GDecisionTreeNode* pInterestingNode, GDecisionTreeNode** ppOutInterestingCopy)
{
	m_pRelation = pThat->m_pRelation;
	m_pRoot = pThat->m_pRoot->DeepCopy(pThat->m_pRelation, pInterestingNode, ppOutInterestingCopy);
}

GDecisionTree::~GDecisionTree()
{
	delete(m_pRoot);
}

// This constructs the decision tree in a recursive depth-first manner
void GDecisionTree::BuildNode(GDecisionTreeNode* pNode, GArffData* pData, bool* pUsedAttributes)
{
	// Log debug stuff
	int n;
#ifdef DEBUGLOG
	dbglog1("BuildNode from %d rows\n", pData->GetRowCount());
	int nAttrCount = pRelation->GetAttributeCount();
	for(n = 0; n < pData->GetRowCount(); n++)
	{
		double* pRow = pData->GetRow(n);
		dbglog0("\t");
		int i;
		for(i = 0; i < nAttrCount; i++)
		{
			GArffAttribute* pAttr = pRelation->GetAttribute(i);
			dbglog1("%s, ", pAttr->GetValue((int)pRow[i]));
		}
		dbglog0("\n");
	}
#endif // DEBUGLOG

	// Pick the best attribute to divide on
	pNode->m_nSampleSize = pData->GetRowCount();
	GAssert(pNode->m_nSampleSize > 0, "Can't work with no data");
	double dBestGain = 0;
	int nBestAttribute = -1;
	if(!pData->IsOutputHomogenous(m_pRelation))
	{
		double dGain;
		int nAttr;
		int nInputCount = m_pRelation->GetInputCount();
		for(n = 0; n < nInputCount; n++)
		{
			nAttr = m_pRelation->GetInputIndex(n);
			if(pUsedAttributes[nAttr])
				continue;
			dGain = MeasureInfoGain(pData, nAttr);
			if(nBestAttribute < 0 || dGain > dBestGain)
			{
				dBestGain = dGain;
				nBestAttribute = nAttr;
			}
		}
	}
	Holder<double*> hMostCommonOutputs(pData->MakeSetOfMostCommonOutputs(m_pRelation));
	GAssert(hMostCommonOutputs.Get(), "Failed to get output values");
	if(nBestAttribute < 0)
	{
		// There are no input attributes left on which to divide, so this is a leaf
		dbglog0("Leaf\n");
		pNode->m_pOutputValues = hMostCommonOutputs.Drop();
		return;
	}

	// Get rid of any unknown values for the best attribute
	pData->ReplaceMissingAttributeWithMostCommonValue(m_pRelation, nBestAttribute);

	// Create child nodes
	pUsedAttributes[nBestAttribute] = true;
	pNode->m_nAttribute = nBestAttribute;
	GArffAttribute* pAttr = m_pRelation->GetAttribute(nBestAttribute);
	dbglog2("Pivot=%d (%s)\n", nBestAttribute, pAttr->GetName());
	GAssert(pAttr->IsInput(), "Expected an input");
	GArffData** ppParts = pData->SplitByAttribute(m_pRelation, nBestAttribute);
	int nChildCount = pAttr->GetValueCount();
	if(nChildCount == 0)
		nChildCount = 2; // Real-valued attributes are split in half
	pNode->m_nChildren = nChildCount;
	pNode->m_ppChildren = new GDecisionTreeNode*[nChildCount];
	for(n = 0; n < nChildCount; n++)
	{
		GDecisionTreeNode* pChildNode = new GDecisionTreeNode();
		pNode->m_ppChildren[n] = pChildNode;
		if(ppParts[n] && ppParts[n]->GetRowCount() > 0)
		{
			BuildNode(pChildNode, ppParts[n], pUsedAttributes);
			pData->Merge(ppParts[n]);
		}
		else
		{
			// There's no data for this child, so just guess the most common outputs of the parent
			pChildNode->m_nSampleSize = 0;
			GAssert(hMostCommonOutputs.Get(), "no outputs");
			pChildNode->m_pOutputValues = new double[m_pRelation->GetOutputCount()];
			memcpy(pChildNode->m_pOutputValues, hMostCommonOutputs.Get(), sizeof(double) * m_pRelation->GetOutputCount());
		}
		delete(ppParts[n]);
	}
	delete(ppParts);
	pUsedAttributes[nBestAttribute] = false;
}

double GDecisionTree::MeasureInfoGain(GArffData* pData, int nAttribute)
{
	// Measure initial entropy
	double dGain = m_pRelation->TotalEntropyOfAllOutputs(pData);
	if(dGain == 0)
		return 0;

	// Seperate by attribute values and measure difference in entropy
	GArffAttribute* pAttr = m_pRelation->GetAttribute(nAttribute);
	GAssert(pAttr->IsInput(), "expected an input attribute");
	if(pAttr->IsContinuous())
		return 0; // todo: support continuous values
	int nRowCount = pData->GetRowCount();
	GArffData** ppParts = pData->SplitByAttribute(m_pRelation, nAttribute);
	int nCount = pAttr->GetValueCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		dGain -= ((double)ppParts[n]->GetRowCount() / nRowCount) * m_pRelation->TotalEntropyOfAllOutputs(ppParts[n]);
		pData->Merge(ppParts[n]);
		delete(ppParts[n]);
	}
	delete(ppParts);
	GAssert(pData->GetRowCount() == nRowCount, "Didn't reassemble data correctly");
	return dGain;
}

void GDecisionTree::Evaluate(double* pRow)
{
	GAssert(m_pRoot, "No tree constructed");
	GDecisionTreeNode* pNode = m_pRoot;
	GArffAttribute* pAttr;
	int nVal;
	while(pNode->m_ppChildren)
	{
		pAttr = m_pRelation->GetAttribute(pNode->m_nAttribute);
		GAssert(pAttr->IsInput(), "expected an input");
		nVal = (int)pRow[pNode->m_nAttribute];
		if(nVal < 0)
		{
			GAssert(nVal == -1, "out of range");
			nVal = rand() % pAttr->GetValueCount();
		}
		GAssert(nVal < pAttr->GetValueCount(), "value out of range");
		pNode = pNode->m_ppChildren[nVal];
	}
	GAssert(pNode->m_pOutputValues, "Leaf node has no output values");
	int n;
	int nOutputCount = m_pRelation->GetOutputCount();
	for(n = 0; n < nOutputCount; n++)
		pRow[m_pRelation->GetOutputIndex(n)] = pNode->m_pOutputValues[n];
}

void GDecisionTree::Print()
{
	m_pRoot->Print(m_pRelation, 0, "All");
}

int GDecisionTree::ValidateTree(GArffData* pValidationSet)
{
	double* pTest = (double*)alloca(sizeof(double) * m_pRelation->GetAttributeCount());
	int nSuccesses = 0;
	int n;
	for(n = 0; n < pValidationSet->GetRowCount(); n++)
	{
		double* pRow = pValidationSet->GetRow(n);
		int i;
		int nCount = m_pRelation->GetInputCount();
		for(i = 0; i < nCount; i++)
		{
			int nAttr = m_pRelation->GetInputIndex(i);
			pTest[nAttr] = pRow[nAttr];
		}
		Evaluate(pTest);
		nCount = m_pRelation->GetOutputCount();
		bool bOK = true;
		for(i = 0; i < nCount; i++)
		{
			int index = m_pRelation->GetOutputIndex(i);
			if(pTest[index] != pRow[index])
			{
				bOK = false;
				break;
			}
		}
		if(bOK)
			nSuccesses++;
	}
	return nSuccesses;
}

void GDecisionTree::DeepPruneNode(GDecisionTreeNode* pNode, GArffData* pValidationSet)
{
	if(!pNode->m_ppChildren)
		return;
	int n;
	for(n = 0; n < pNode->m_nChildren; n++)
		DeepPruneNode(pNode->m_ppChildren[n], pValidationSet);
	GDecisionTreeNode* pNodeCopy;
	GDecisionTree tmp(this, pNode, &pNodeCopy);
	pNodeCopy->PruneChildren(m_pRelation);
	int nOriginalScore = ValidateTree(pValidationSet);
	int nPrunedScore = tmp.ValidateTree(pValidationSet);
	if(nPrunedScore >= nOriginalScore)
		pNode->PruneChildren(m_pRelation);
}

void GDecisionTree::Prune(GArffData* pValidationSet)
{
	DeepPruneNode(m_pRoot, pValidationSet);
}
