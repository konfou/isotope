/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GRAND_H__
#define __GRAND_H__

class GBigNumber;

// A pseudo random number generator for big numbers. It's supposed to be
// cryptographically strong, but I designed it myself so you shouldn't trust it.
class GRand
{
protected:
	int m_nSeedBytes;
	int m_nBytes;
	unsigned char* m_pSeed;
	unsigned char* m_pData;

public:
	// The seed should already be fully digested into random data when you call this
	// constructor.  The output random numbers will be the same size as the seed.
	GRand(const unsigned char* pSeedData, int nSeedBytes);
	virtual ~GRand();

	const unsigned char* GetRand();
	void AddEntropy(unsigned char* pData, int nSize);
	int GetRandByteCount() { return m_nSeedBytes; }

	static void Encrypt(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize);
	static void Decrypt(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize);

	// pEntropy is a pointer to some entropy bytes.
	// nEntropyBytes is the number of bytes of entropy.
	// nRatio is the ratio of entropy bits to random bits.  For example, if nRatio is 2, then 40 bytes of entropy will produce 20 bytes of randomness
	// pResults should be a buffer big enough to hold the results of digesting the entropy with the specified ratio.
	static void ShaDigestEntropy(unsigned char* pEntropy, int nEntropyBytes, int nRatio, unsigned int* pResults);

protected:
	static void Crypto(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize, bool bEncrypt);

};

#endif // __GRAND_H__
