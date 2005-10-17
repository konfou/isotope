/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GDECISIONTREE_H__
#define __GDECISIONTREE_H__

class GArffRelation;
class GArffData;
class GDecisionTreeNode;


class GDecisionTree
{
protected:
	GDecisionTreeNode* m_pRoot;
	GArffRelation* m_pRelation;

public:
	// The tree is built automatically in the constructor
	GDecisionTree(GArffRelation* pRelation, GArffData* pTrainingData);

	// Makes a deep copy of another decision tree.  Also, if pInterestingNode
	// is non-NULL, then ppOutInterestingNode will return the node that is
	// a copy of pInterestingNode
	GDecisionTree(GDecisionTree* pThat, GDecisionTreeNode* pInterestingNode, GDecisionTreeNode** ppOutInterestingCopy);

	~GDecisionTree();

	// Reads the input values in pRow and sets the output values
	void Evaluate(double* pRow);

	// Print an ascii representation of the tree to stdout
	void Print();

	// Returns the number of rows that the tree correctly predicts
	// in the validation set
	int ValidateTree(GArffData* pValidationSet);

	// Performs all pruning that causes the tree to give better results
	// for the validation set
	void Prune(GArffData* pValidationSet);

protected:
	// A recursive helper method used to construct the decision tree
	void BuildNode(GDecisionTreeNode* pNode, GArffData* pData, bool* pUsedAttributes);

	// InfoGain is defined as the difference in entropy in the data
	// before and after dividing it based on the specified attribute
	double MeasureInfoGain(GArffData* pData, int nAttribute);

	// Tries pruning the children of pNode.  If that improves the tree,
	// makes the change permanent, otherwise recurses on all children of pNode
	void DeepPruneNode(GDecisionTreeNode* pNode, GArffData* pValidationSet);
};

#endif // __GDECISIONTREE_H__
