#ifndef __GFUZZYLOGIC_H__
#define __GFUZZYLOGIC_H__

class GFuzzyLogic
{
public:
	// Computes the fuzzy NOT of a value between 0 and 1
	inline static double Not(double a) { return 1.0 - a; }

	// Computes the fuzzy AND of two values between 0 and 1
	inline static double And(double a, double b) { return a * b; }

	// Computes the fuzzy OR of two values between 0 and 1
	inline static double Or(double a, double b) { return 1.0 - ((1.0 - a) * (1.0 - b)); }

	// Computes the fuzzy XOR of two values between 0 and 1
	inline static double Xor(double a, double b) { return (a + b - a * b) * (1.0 - a * b); }
};

#endif // __GFUZZYLOGIC_H__
