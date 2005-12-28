/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GLEARNER_H__
#define __GLEARNER_H__

class GArffRelation;
class GArffData;


class GSupervisedLearner
{
protected:
	GArffRelation* m_pRelation;

public:
	GSupervisedLearner(GArffRelation* pRelation);
	virtual ~GSupervisedLearner();

	// Returns the relation used to construct this learner
	GArffRelation* GetRelation() { return m_pRelation; }

	// Train with the provided data
	virtual void Train(GArffData* pData) = 0;

	// Evaluates the input values in the provided row and
	// deduce the output values
	virtual void Eval(double* pRow) = 0;

	// Computes predictive accuracy (the ratio of samples that
	// are correctly classified to total samples). If there is
	// more than one output attribute, each output attribute
	// is evaluated independently. If there are continuous output
	// values, it uses 1-1/(1+(squared error)) as an estimate
	// so that a small squared error will be close to 1 (correct)
	// and a large squared error will be close to 0 (incorrect).
	double MeasurePredictiveAccuracy(GArffData* pData);

	// Computes the mean squared error. If there are multiple
	// output attributes, each one is considered independently.
	// If there are discreet output attributes, a correct
	// classification is considered to be a squared error of
	// 0 and an incorrect classification is a squared error of 1.
	double MeasureMeanSquaredError(GArffData* pData);
};

#endif // __GLEARNER_H__
