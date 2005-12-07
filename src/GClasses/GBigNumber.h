/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GBIGNUMBER_H__
#define __GBIGNUMBER_H__

class GKeyPair;
class GRand;

class GBigNumber
{
protected:
	enum
	{
		BITS_PER_INT = sizeof(unsigned int) * 8,
	};
	
	unsigned int m_nUInts;
	unsigned int* m_pBits;
	bool m_bSign;

public:
	GBigNumber();
	virtual ~GBigNumber();

	// Returns true if the number is positive and false if it is negative
	bool GetSign() { return m_bSign; }

	// Makes the number positive if bSign is true and negative if bSign is false
	void SetSign(bool bSign) { m_bSign = bSign; }

	// Returns the number of bits in the number
	unsigned int GetBitCount();

	// Returns the value of the nth bit where 0 represents the least significant bit (little endian)
	bool GetBit(unsigned int n)
	{
		return((n >= m_nUInts * BITS_PER_INT) ? false : ((m_pBits[n / BITS_PER_INT] & (1 << (n % BITS_PER_INT))) ? true : false));
	}

	// Sets the value of the nth bit where 0 represents the least significant bit (little endian)
	void SetBit(unsigned int nPos, bool bVal);

	// Returns the number of unsigned integers required to represent this number
	unsigned int GetUIntCount() { return m_nUInts; }

	// Returns the nth unsigned integer used to represent this number
	unsigned int GetUInt(unsigned int nPos) { return nPos >= m_nUInts ? 0 : m_pBits[nPos]; }

	// Sets the value of the nth unsigned integer used to represent this number
	void SetUInt(unsigned int nPos, unsigned int nVal);

	// Sets the number to zero
	void SetToZero();

	// Returns true if the number is zero
	bool IsZero();

	// Returns -1 if this is less than pBigNumber
	// Returns 0 if this is equal to pBigNumber
	// Returns 1 if this is greater than pBigNumber
	int CompareTo(GBigNumber* pBigNumber);

	// Copies the value of pBigNumber into this object
	void Copy(GBigNumber* pBigNumber);

	// Produces a big endian hexadecimal representation of this number
	bool ToHex(char* szBuff, int nBufferSize);
	
	// Extract a value from a big endian hexadecimal string
	bool FromHex(const char* szHexValue);

	// This gives you ownership of the buffer.  (You must delete it.)  It also sets the value to zero.
	unsigned int* ToBufferGiveOwnership();

	// Serializes the number. little-Endian (first bit in buffer will be LSB)
	bool ToBuffer(unsigned int* pBuffer, int nBufferSize);
	
	// Deserializes the number. little-Endian (first bit in buffer will be LSB)
	void FromBuffer(const unsigned int* pBuffer, int nBufferSize);

	// Deserializes the number.
	void FromByteBuffer(const unsigned char* pBuffer, int nBufferChars);

	// Multiplies the number by -1
	void Invert();

	// Adds one to the number
	void Increment();

	// Subtracts one from the number
	void Decrement();

	// Add another big number to this one
	void Add(GBigNumber* pBigNumber);

	// Subtract another big number from this one
	void Subtract(GBigNumber* pBigNumber);

	// Set this value to the product of another big number and an unsigned integer
	void Multiply(GBigNumber* pBigNumber, unsigned int nUInt);

	// Set this value to the product of two big numbers
	void Multiply(GBigNumber* pFirst, GBigNumber* pSecond);

	// Set this value to the ratio of two big numbers and return the remainder
	void Divide(GBigNumber* pInNominator, GBigNumber* pInDenominator, GBigNumber* pOutRemainder);

	// Shift left (multiply by 2)
	void ShiftLeft(unsigned int nBits);

	// Shift right (divide by 2 and round down)
	void ShiftRight(unsigned int nBits);

	// bitwise or
	void Or(GBigNumber* pBigNumber);

	// bitwise and
	void And(GBigNumber* pBigNumber);

	// bitwise xor
	void Xor(GBigNumber* pBigNumber);

	// Input:  integers a, b
	// Output: this will be set to the greatest common divisor of a,b.
	//         (If pOutX and pOutY are not NULL, they will be values such
	//          that "this" = ax + by.)
	void Euclid(GBigNumber* pA1, GBigNumber* pB1, GBigNumber* pOutX = NULL, GBigNumber* pOutY = NULL);
	
	// Input:  a, k>=0, n>=2
	// Output: this will be set to ((a raised to the power of k) modulus n)
	void PowerMod(GBigNumber* pA, GBigNumber* pK, GBigNumber* pN);
	
	// Input:  "this" must be >= 3, and 2 <= a < "this"
	// Output: "true" if this is either prime or a strong pseudoprime to base a,
	//         "false" otherwise
	bool MillerRabin(GBigNumber* pA);
	
	// Output: true = pretty darn sure (like 99.999%) it's prime
	//         false = definately (100%) not prime
	bool IsPrime();

	// Input:  pProd is the product of (p - 1) * (q - 1) where p and q are prime
	//         pRandomData is some random data that will be used to pick the key.
	// Output: It will return a key that has no common factors with pProd.  It starts
	//         with the random data you provide and increments it until it fits this
	//         criteria.
	void SelectPublicKey(const unsigned int* pRandomData, int nRandomDataUInts, GBigNumber* pProd);

	// Input:  pProd is the product of (p - 1) * (q - 1) where p and q are prime
	//         pPublicKey is a number that has no common factors with pProd
	// Output: this will become a private key to go with the public key
	void CalculatePrivateKey(GBigNumber* pPublicKey, GBigNumber* pProd);

	// DO NOT use for crypto--This is NOT a cryptographic random number generator
	void SetRandom(unsigned int nBits);

	// Cryptographic random number
	void SetToRand(GRand* pRand);

protected:
	void Resize(unsigned int nBits);
	void ShiftLeftBits(unsigned int nBits);
	void ShiftRightBits(unsigned int nBits);
	void ShiftLeftUInts(unsigned int nBits);
	void ShiftRightUInts(unsigned int nBits);
};

#endif // __GBIGNUMBER_H__
