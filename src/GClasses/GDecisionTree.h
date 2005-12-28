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

#include "GLearner.h"

class GDecisionTreeNode;


class GDecisionTree : public GSupervisedLearner
{
protected:
	GDecisionTreeNode* m_pRoot;
	double m_dTrainingPortion;

public:
	// The tree is built automatically in the constructor
	GDecisionTree(GArffRelation* pRelation);

	// Makes a deep copy of another decision tree.  Also, if pInterestingNode
	// is non-NULL, then ppOutInterestingNode will return the node that is
	// a copy of pInterestingNode
	GDecisionTree(GDecisionTree* pThat, GDecisionTreeNode* pInterestingNode, GDecisionTreeNode** ppOutInterestingCopy);

	virtual ~GDecisionTree();

	// Divides the provided data into two parts, trains with one part and prunes with
	// the other. (Use SetTrainingPortion to set the ratio of the two parts.)
	virtual void Train(GArffData* pData);

	// Trains using all of the provided data and doesn't do any pruning
	void TrainWithoutPruning(GArffData* pTrainingData);

	// Specifies how much of the training data is used to build the tree. (The rest
	// is used to prune the tree.)
	void SetTrainingPortion(double d) { m_dTrainingPortion = d; }

	// Evaluates the input values in the provided row and
	// deduce the output values
	virtual void Eval(double* pRow);

	// Print an ascii representation of the tree to stdout
	void Print();

	// Performs all pruning that causes the tree to give better results
	// for the validation set
	void Prune(GArffData* pValidationSet);

protected:
	// A recursive helper method used to construct the decision tree
	void BuildNode(GDecisionTreeNode* pNode, GArffData* pData, bool* pUsedAttributes);

	// InfoGain is defined as the difference in entropy in the data
	// before and after dividing it based on the specified attribute. For
	// continuous attributes it uses the difference between the original
	// variance and the sum of the variances of the two parts after
	// dividing at the point the maximizes this value.
	double MeasureInfoGain(GArffData* pData, int nAttribute, double* pPivot);

	// Tries pruning the children of pNode.  If that improves the tree,
	// makes the change permanent, otherwise recurses on all children of pNode
	void DeepPruneNode(GDecisionTreeNode* pNode, GArffData* pValidationSet);
};

#endif // __GDECISIONTREE_H__
