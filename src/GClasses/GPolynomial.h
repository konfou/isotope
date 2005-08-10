#ifndef __GPOLYNOMIAL_H__
#define __GPOLYNOMIAL_H__

class GArffRelation;
class GArffData;

// This represents a multi-dimensional polynomial
class GPolynomial
{
protected:
	int m_nDimensions;
	int m_nControlPoints;
	int m_nCoefficients;
	double* m_pCoefficients;

public:
	GPolynomial(int nDimensions, int nControlPoints);
	~GPolynomial();

	// Returns the coefficient at the specified degrees.  pDegrees should
	// be an array of size m_nDimensions
	double GetCoefficient(int* pDegrees);

	// Sets the coefficient at the specified degrees.  pDegrees should
	// be an array of size m_nDimensions
	void SetCoefficient(int* pDegrees, double dVal);

	// Evaluates the polynomial.  pCoordinates should be an array of size
	// m_nDimensions
	double Eval(double* pVariables);

	// Returns the mean square error of this polynomial's ability to predict
	// the specified output value of the given data
	double MeasureMeanSquareError(GArffRelation* pRelation, GArffData* pData, int nOutputAttr);

	// Generates a polynomial fitted to predict the provided data as best as possible
	static GPolynomial* FitData(GArffRelation* pRelation, GArffData* pData, int nOutputAttr, int nControlPoints);

	// Generates an optimal polynomial that uses the first "n - 1" inputs to calculate
	// threshold values for the last input such that the two halves can be fitted
	// with a polynomial as well as possible
	static GPolynomial* DivideData(GArffRelation* pRelation, GArffData* pData, int nOutputAttr, int nControlPoints);

protected:
	int CalcIndex(int* pDegrees);
	double DivideAndMeasureError(GArffRelation* pRelation, GArffData* pData, int nOutputAttr);
};


#endif // __GPOLYNOMIAL_H__
