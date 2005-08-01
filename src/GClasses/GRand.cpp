/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <string.h>
#include "GRand.h"
#include "GBigNumber.h"
#include <math.h>
#include "GMacros.h"
#include <stdlib.h>
#include "sha1.h"
#include "sha2.h"

#define HASH_BYTE_SIZE 64

GRand::GRand(const unsigned char* pSeedData, int nSeedBytes)
{
	GAssert(nSeedBytes > 0, "Not enough seed bytes");
	m_nSeedBytes = nSeedBytes;
	m_nBytes = nSeedBytes;
	if(m_nBytes < HASH_BYTE_SIZE)
		m_nBytes = HASH_BYTE_SIZE;
	m_pSeed = new unsigned char[m_nBytes];
	memset(m_pSeed, '\0', m_nBytes);
	memcpy(m_pSeed, pSeedData, nSeedBytes);	
	m_pData = new unsigned char[m_nBytes];
	memset(m_pData, '\0', m_nBytes);
}

GRand::~GRand()
{
	// Ensure that no sensitive info is left in deallocated memory
	memset(m_pData, '\0', m_nBytes);
	memset(m_pSeed, '\0', m_nBytes);

	// Free the memory
    delete(m_pData);
	delete(m_pSeed);
}

const unsigned char* GRand::GetRand()
{
	// Make the new random data
    sha512_ctx ctx;
	int n = 0;
	while(n < m_nBytes)
	{
		if(m_nBytes - n < HASH_BYTE_SIZE)
			n = m_nBytes - HASH_BYTE_SIZE;

        // Hash seed + data -> data
		sha512_begin(&ctx);
        sha512_hash(&m_pSeed[n], HASH_BYTE_SIZE, &ctx);
        sha512_hash(&m_pData[n], HASH_BYTE_SIZE, &ctx);
        sha512_end(&m_pData[n], &ctx);

        // Hash data + seed -> seed
		sha512_begin(&ctx);
        sha512_hash(&m_pData[n], HASH_BYTE_SIZE, &ctx);
        sha512_hash(&m_pSeed[n], HASH_BYTE_SIZE, &ctx);
        sha512_end(&m_pSeed[n], &ctx);

		n += HASH_BYTE_SIZE;
	}

	// Shuffle the seed
	unsigned char c;
	unsigned int nRandPos;
	for(n = m_nBytes - sizeof(unsigned int); n >= 0; n--)
	{
		nRandPos = (*(unsigned int*)&m_pSeed[n]) % m_nBytes;
		c = m_pSeed[n];
		m_pSeed[n] = m_pSeed[nRandPos];
		m_pSeed[nRandPos] = c;
	}

    return m_pData;
}

void GRand::AddEntropy(unsigned char* pData, int nSize)
{
	int n;
	int nRandPos;
	int nSeedPos = m_nBytes - sizeof(unsigned int);
	for(n = 0; n < nSize; n++)
	{
		nRandPos = (*(unsigned int*)&m_pSeed[nSeedPos]) % m_nBytes;
		m_pSeed[nRandPos] ^= pData[n];
		nSeedPos--;
		if(nSeedPos <= 0)
			nSeedPos = m_nBytes - sizeof(unsigned int);
	}
}

/*static*/ void GRand::Crypto(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize, bool bEncrypt)
{
	GRand rng(pPassphrase, nPassphraseSize);
	int nPos = 0;
	const unsigned char* pPad;
	int nBlockStart;
	int n;
	while(nPos < nDataSize)
	{
		nBlockStart = nPos;
		pPad = rng.GetRand();
		if(bEncrypt)
			rng.AddEntropy(&pData[nBlockStart], MIN(nPassphraseSize, nDataSize - nBlockStart));
		for(n = 0; n < nPassphraseSize && nPos < nDataSize; n++)
		{
			pData[nPos] ^= pPad[n];
			nPos++;
		}
		if(!bEncrypt)
			rng.AddEntropy(&pData[nBlockStart], MIN(nPassphraseSize, nDataSize - nBlockStart));
	}
}

/*static*/ void GRand::Encrypt(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize)
{
	Crypto(pData, nDataSize, pPassphrase, nPassphraseSize, true);
}

/*static*/ void GRand::Decrypt(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize)
{
	Crypto(pData, nDataSize, pPassphrase, nPassphraseSize, false);
}

/*static*/ void GRand::ShaDigestEntropy(unsigned char* pEntropy, int nEntropyBytes, int nRatio, unsigned int* pResults)
{
	// Scramble the entropy bytes (in case the entropy isn't evenly distributed)
	int n;
	for(n = nEntropyBytes - 1; n > 1; n--)
	{
		int nPos = rand() % n;
		unsigned char cTmp = pEntropy[n];
		pEntropy[n] = pEntropy[nPos];
		pEntropy[nPos] = cTmp;
	}

	// Digest all the 64-byte chunks with sha512
	unsigned char* pOutput = (unsigned char*)pResults;
	while(nEntropyBytes >= HASH_BYTE_SIZE * nRatio)
	{
		sha512_ctx ctx;
		memset(&ctx, '\0', sizeof(sha512_ctx));
		sha512_begin(&ctx);
		sha512_hash(pEntropy, 20 * nRatio, &ctx);
		sha512_end(pOutput, &ctx);
		pEntropy += (HASH_BYTE_SIZE * nRatio);
		nEntropyBytes -= (HASH_BYTE_SIZE * nRatio);
		pOutput += HASH_BYTE_SIZE;
	}

	// Digest remaining 20-byte chunks with sha1
	while(nEntropyBytes >= 20 * nRatio)
	{
		SHA_CTX ctx;
		memset(&ctx, '\0', sizeof(SHA_CTX));
		SHA1_Init(&ctx);
		SHA1_Update(&ctx, pEntropy, 20 * nRatio);
		SHA1_Final((sha1_byte*)pOutput, &ctx);
		pEntropy += (20 * nRatio);
		nEntropyBytes -= (20 * nRatio);
		pOutput += 20;
	}

	// Digest whatever's left with sha1
	if(nEntropyBytes > 0)
	{
		unsigned char szDigest[20];
		SHA_CTX ctx;
		memset(&ctx, '\0', sizeof(SHA_CTX));
		SHA1_Init(&ctx);
		SHA1_Update(&ctx, pEntropy, nEntropyBytes);
		SHA1_Final(szDigest, &ctx);
		memcpy(pOutput, szDigest, nEntropyBytes / nRatio);
	}
}

