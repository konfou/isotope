/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GNEURALNET_H__
#define __GNEURALNET_H__

class GArffRelation;
class GArffData;
class GPointerArray;


class GNeuralNet
{
protected:
	GArffRelation* m_pRelation;
	GPointerArray* m_pLayers;
	GPointerArray* m_pBest;
	double m_dInitThresh;
	int m_nBiggestLayer;
	int m_nMinimumIterations;
	int m_nMaximumIterations;
	int m_nIterationsPerValidationCheck;
	int m_nPasses;
	double m_dSigmoidSteepness;
	double m_dLearningRate;
	double m_dMomentum;
	double m_dHopefulness;
	double m_dAcceptableMeanSquareError;

public:
	GNeuralNet(GArffRelation* pRelation);
	~GNeuralNet();

	// Adds a layer to the network.  (The input and output layers
	// are implicit, so you only need to add the hidden layers
	// before calling Train.)  The first hidden layer you add will
	// be ajacent to the output layer.  The last hidden layer you
	// add will be ajacent to the input layer.  Conceptually,
	// there is a layer of weights between each layer of nodes, so
	// the number of weight layers will be one more than the number
	// of hidden layers you add.  It's not common to add more than
	// two hidden layers because that results in large training times.
	void AddHiddenLayer(int nNodes);

	// Sets the range for initial weight values.  Each weight
	// will be set to a random value between -Thresh/2 and Thresh/2
	void SetInitThresh(double d) { m_dInitThresh = d; }

	// Set the steepness of the sigmoid function
	void SetSigmoidSteepness(double d) { m_dSigmoidSteepness = d; }

	// Set the rate of convergence
	void SetLearningRate(double d) { m_dLearningRate = d; }

	// Momentum has the effect of speeding convergence and helping
	// the gradient descent algorithm move past some local minimums
	void SetMomentum(double d) { m_dMomentum = d; }

	// Sets a ratio that determines how long to continue training
	// the neural net.  When the ratio of the number of iterations
	// since the best results against the validation data were found
	// to the total number of iterations reaches this value, the
	// training will terminate.  (So a bigger number means to keep
	// trying for longer.  It must be smaller than 1.)
	void SetHopefulness(double d) { m_dHopefulness = d; }

	// Sets the minimum number of training iterations to perform.
	// Until this number of iterations has been reached, it won't
	// bother checking against the validation set.  (An iteration
	// is defined as a single pass through all rows in the training
	// set.)
	void SetMinimumIterations(int n) { m_nMinimumIterations = n; }

	// Sets the maximum number of training iterations to perform
	// per pass.
	void SetMaximumIterations(int n) { m_nMaximumIterations = n; }

	// Sets the number of iterations that will be performed before
	// each time the network is tested again with the validation set
	// to determine if we have a better best-set of weights, and
	// whether or not it's achieved the termination condition yet.
	// (An iteration is defined as a single pass through all rows in
	// the training set.)
	void SetIterationsPerValidationCheck(int n) { m_nIterationsPerValidationCheck = n; }

	// If the mean square error ever falls below this value, training
	// will stop.  Note that if you use this as the primary stopping
	// criteria, you will be prone to overfitting issues.  To avoid
	// overfitting, keep this number very small so it will stop based
	// on other conditions.  This is more of a safety-harness for
	// cases where overfitting is okay (ie compression) so that
	// it will stop if the results are good enough even if it can keep
	// getting better.
	void SetAcceptableMeanSquareError(double d) { m_dAcceptableMeanSquareError = d; }

	// Set the number of times to try training the network.  If
	// one pass gets stuck in some local error minimum, another pass
	// may get lucky and avoid that particular local minimum, so the
	// more passes you perform the more likely you are to find the
	// global error minimum.
	void SetPasses(int n) { m_nPasses = n; }

	// Train the network until the termination condition is met.
	// The termination condition consists of a combination of the
	// minimum itterations and the hopefulness.  If overfitting is
	// desireable (ie for a compression algorithm), the validation
	// data can be the same pointer as the training data.
	void Train(GArffData* pTrainingData, GArffData* pValidationData);

	// Reads the input values in pRow and sets the output values
	// Note that if any of the output values are expected to be
	// discreet, you must call GMath::analogToDigital to get the
	// corresponding discreet values.
	void Evaluate(double* pRow);

protected:
	void UpdateWeights(double* pRow, double* pSample);
	void EvaluateHelper(double* pRow, GPointerArray* pLayers);
	double MeasureError(GArffData* pData, double* pSample);
	void UpdateBestWeights();
};

#endif // __GNEURALNET_H__
