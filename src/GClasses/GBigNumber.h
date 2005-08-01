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

	// Basic Manipulation
    bool GetSign() { return m_bSign; }
    void SetSign(bool bSign) { m_bSign = bSign; }
    unsigned int GetBitCount();
    bool GetBit(unsigned int n) // 0 = LSB (Little Endian)
    {   return((n >= m_nUInts * BITS_PER_INT) ? false : ((m_pBits[n / BITS_PER_INT] & (1 << (n % BITS_PER_INT))) ? true : false));  }
    void SetBit(unsigned int nPos, bool bVal);
    unsigned int GetUIntCount() { return m_nUInts; }
    unsigned int GetUInt(unsigned int nPos) { return nPos >= m_nUInts ? 0 : m_pBits[nPos]; }
    void SetUInt(unsigned int nPos, unsigned int nVal);
	void SetToZero();
    bool IsZero();
	int CompareTo(GBigNumber* pBigNumber); // -1 = this is smaller than pBigNumber, 0 = equal, 1 = this is bigger
	void Copy(GBigNumber* pBigNumber);

	// String and buffer conversion
	bool ToHex(char* szBuff, int nBufferSize); // big-endian 
	bool FromHex(const char* szHexValue); // big-endian
	unsigned int* ToBufferGiveOwnership(); // This gives you ownership of the buffer.  (You must delete it.)  It also sets the value to zero.
	bool ToBuffer(unsigned int* pBuffer, int nBufferSize); // little-Endian (first bit in buffer will be LSB)
	void FromBuffer(const unsigned int* pBuffer, int nBufferSize); // little-Endian (first bit in buffer will be LSB)
	void FromByteBuffer(const unsigned char* pBuffer, int nBufferChars); // little-Endian (first bit in buffer will be LSB)

	// Arithmetic
	void Invert();
	void Increment();
	void Decrement();
	void Add(GBigNumber* pBigNumber);
	void Subtract(GBigNumber* pBigNumber);
    void Multiply(GBigNumber* pBigNumber, unsigned int nUInt);
	void Multiply(GBigNumber* pFirst, GBigNumber* pSecond);
	void Divide(GBigNumber* pInNominator, GBigNumber* pInDenominator, GBigNumber* pOutRemainder); // Order n operation

	// Shift
	void ShiftLeft(unsigned int nBits);
	void ShiftRight(unsigned int nBits);

	// Logic
	void Or(GBigNumber* pBigNumber);
	void And(GBigNumber* pBigNumber);
	void Xor(GBigNumber* pBigNumber);

	// Crypto

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
