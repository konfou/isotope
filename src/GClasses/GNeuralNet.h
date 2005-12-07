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
class GNeuralNet;

typedef double (*CriticFunc)(void* pThis, GNeuralNet* pNeuralNet);

// An artificial neural network
class GNeuralNet
{
protected:
	GArffRelation* m_pInternalRelation;
	GArffRelation* m_pExternalRelation;
	GPointerArray* m_pNeurons;
	double* m_pBestSet;
	int m_nWeightCount;
	int m_nInputStart;
	int m_nLayerStart;
	int m_nLayerSize;
	double* m_pMinAndRanges;
	GArffData* m_pTrainingDataInternal;
	GArffData* m_pValidationDataInternal;

	// Settings
	int m_nRunEpochs;
	int m_nMaximumEpochs;
	int m_nEpochsPerValidationCheck;
	double m_dLearningRate;
	double m_dLearningDecay;
	double m_dMomentum;
	double m_dAcceptableMeanSquareError;

public:
	GNeuralNet(GArffRelation* pRelation);
	~GNeuralNet();

	// Adds a layer to the network.  (The input and output layers
	// are implicit, so you only need to add the hidden layers
	// before calling Train.)  The first hidden layer you add will
	// be ajacent to the output layer.  The last hidden layer you
	// add will be ajacent to the input layer.  It's not common to add
	// more than two hidden layers because that results in large
	// training times.
	void AddLayer(int nNodes);

	// Returns the number of weights in the network
	int GetWeightCount();

	// Set the rate of convergence
	void SetLearningRate(double d) { m_dLearningRate = d; }

	// Set the rate at which the learning rate decays.  (The learning
	// rate will be multiplied by this value after every pass through
	// the training data.)
	void SetLearningDecay(double d) { m_dLearningDecay = d; }

	// Momentum has the effect of speeding convergence and helping
	// the gradient descent algorithm move past some local minimums
	void SetMomentum(double d) { m_dMomentum = d; }

	// Training will terminate when this number of epochs are performed
	// without finding another best epoch for the validation set.
	void SetRunEpochs(int n) { m_nRunEpochs = n; }

	// Sets the maximum number of times per pass to train with all the
	// data.
	void SetMaximumEpochs(int n) { m_nMaximumEpochs = n; }

	// Sets the number of iterations that will be performed before
	// each time the network is tested again with the validation set
	// to determine if we have a better best-set of weights, and
	// whether or not it's achieved the termination condition yet.
	// (An iteration is defined as a single pass through all rows in
	// the training set.)
	void SetIterationsPerValidationCheck(int n) { m_nEpochsPerValidationCheck = n; }

	// If the mean square error ever falls below this value, training
	// will stop.  Note that if you use this as the primary stopping
	// criteria, you will be prone to overfitting issues.  To avoid
	// overfitting, keep this number very small so it will stop based
	// on other conditions.  This is more of a safety-harness for
	// cases where overfitting is okay (ie compression) so that
	// it will stop if the results are good enough even if it can keep
	// getting better.
	void SetAcceptableMeanSquareError(double d) { m_dAcceptableMeanSquareError = d; }

	// Train the network until the termination condition is met.
	// Returns the number of epochs required to train it.  This is sort
	// of an all-in-one method that calls TrainInit, followed by several
	// calls to TrainEpoch and TrainValidate.
	int Train(GArffData* pTrainingData, GArffData* pValidationData);

	// Same as Train except it uses batch updates instead of incremental
	// updates
	int TrainBatch(GArffData* pTrainingData, GArffData* pValidationData);

	// This must be called before you call TrainEpoch
	void TrainInit(GArffData* pTrainingData, GArffData* pValidationData);

	// Trains with a single epoch
	void TrainEpoch();

	// Measures the mean squared error against the internal validation set
	double TrainValidate();

	// Reads the input values in pRow and sets the output values
	void Eval(double* pRow);

	// Returns the Relation corresponding to the internal data. This
	// relation will contain all continuous attributes and the inputs
	// and outputs will correspond to the actual input and output
	// neurons in the network topology.
	GArffRelation* GetInternalRelation() { return m_pInternalRelation; }

	// For efficiency purposes the neural net produces an internal copy
	// of the training data with values normalized to ranges that the
	// neural net can handle. This method tells the neural net that
	// training is complete so it's okay to free up that memory.
	void ReleaseInternalData();

	// Sets all the weights from an array of doubles. The number of
	// doubles in the array can be determined by calling GetWeightCount().
	void SetWeights(double* pWeights);

	// Serializes the network weights into an array of doubles. The
	// number of doubles in the array can be determined by calling
	// GetWeightCount().
	void GetWeights(double* pOutWeights);

protected:
	// Convert all the input values to the internal representation
	void InputsToInternal(double* pExternal, double* pInternal);

	// Convert all the output values to the internal representation
	void OutputsToInternal(double* pExternal, double* pInternal);

	// Convert the internal output values to the external representation
	void OutputsToExternal(double* pInternal, double* pExternal);

	// Evaluates a row of data for the internal relation.  It doesn't
	// set any output values, it just leaves those in the output nodes
	// so it's safe to pass the original training data in to this method.
	void EvalInternal(double* pRow);

	// Measures the min and range of every attribute in the external
	// training set.  This data is used when converting continuous values
	// between the internal and external format
	void MeasureMinAndRanges(GArffData* pTrainingData);

	// Converts a collection of external data to the internal format
	void ExternalToInternalData(GArffData* pExternal, GArffData* pInternal);

	// Remembers the current weights as the best set known so far
	void UpdateBestWeights();

	// Restores the best known set of weights
	void RestoreBestWeights();

	// This computes the error on the output nodes and uses backpropagation
	// to assign the appropriate amount of error to all upstream nodes
	void Criticize(double* pModel);

	void MakeInputLayer();
	void MakeInternalRelationAndOutputLayer();
	void ReadOutput(double* pRow);
	void PrintNeurons();
	//void TestDataSet(GArffData* pData);
};

#endif // __GNEURALNET_H__
