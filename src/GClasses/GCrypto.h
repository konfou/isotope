/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <string.h>

class CEdit;
class CString;

#define DIGEST_BYTES 20
#define DATA_BYTES_TO_USE 128

#ifndef MIN
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#endif // MIN

#ifndef MAX
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#endif // MAX

inline void ShiftDataIn(unsigned char* pBuffer, int nBufferSize, const unsigned char* pNewData, int nDataSize)
{
	// Shift buffer to make room for new data at the end
	int nOldSize = nBufferSize - nDataSize;
	int n;
	for(n = 0; n < nOldSize; n++)
		pBuffer[n] = pBuffer[n + nDataSize];

	// Copy from pNewData into the end of the buffer
	memcpy(pBuffer + nOldSize, pNewData, nDataSize);
}

void WipeBuffer(unsigned char* pBuffer, int nSize);
void WipeBuffer(char* pBuffer, int nSize);
void SafeStrCpy(char* szDest, const char* szSrc, int nMaxSize);
void Xor(unsigned char* pDest, unsigned char* pSource, int nBytes);
void Digest(unsigned char* digest, unsigned char* pBlob, int nBlobSize); // Currently uses Sha1, but you can swap out any cryptographic digest algorithm you want

class GCrypto
{
private:
	bool m_bEncrypt;
	unsigned char m_digest[DIGEST_BYTES];
	unsigned char m_keyBuffer[DIGEST_BYTES];
	int m_nBlobSize;
	unsigned char* m_pBlob;
	unsigned char* m_pPassphrase;
	int m_nPassphraseSize;
	unsigned char* m_pPassphraseExtender;
	int m_nUnusedKeySize;

public:
	GCrypto(const unsigned char* pPassphrase, int nPassphraseSize, bool bEncrypt)
	{
		m_bEncrypt = bEncrypt;
		m_nBlobSize = DATA_BYTES_TO_USE + nPassphraseSize;
		m_pBlob = new unsigned char[m_nBlobSize];
		m_pPassphrase = new unsigned char[nPassphraseSize];
		memcpy(m_pPassphrase, pPassphrase, nPassphraseSize);
		m_nPassphraseSize = nPassphraseSize;
		m_pPassphraseExtender = new unsigned char[DATA_BYTES_TO_USE];
		memset(m_pPassphraseExtender, '\0', DATA_BYTES_TO_USE);
		m_nUnusedKeySize = 0;
	}

	~GCrypto()
	{
		WipeBuffer(m_pBlob, m_nBlobSize);
		delete(m_pBlob);
		WipeBuffer(m_pPassphrase, m_nPassphraseSize);
		delete(m_pPassphrase);
		m_nPassphraseSize = 0;
		WipeBuffer(m_pPassphraseExtender, DATA_BYTES_TO_USE);
		delete(m_pPassphraseExtender);
		WipeBuffer(m_digest, DIGEST_BYTES);
		WipeBuffer(m_keyBuffer, DIGEST_BYTES);
		m_bEncrypt = false;
		m_nUnusedKeySize = 0;
	}

	// Encrypts a buffer
	static void Encrypt(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize);

	// Decrypts a buffer
	static void Decrypt(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize);

	void ProcessChunk(unsigned char* pChunk, int nChunkSize)
	{
		// Finish the last block if we didn't yet
		if(m_nUnusedKeySize > 0)
		{
			int nTmp = m_nUnusedKeySize;
			if(!FinishBlock(pChunk, nChunkSize))
				return;
			pChunk += nTmp;
			nChunkSize -= nTmp;
			ManglePassword();
		}
		
		// Process all the full blocks
		while(nChunkSize >= DIGEST_BYTES)
		{
			ProcessBlock(pChunk, DIGEST_BYTES);
			pChunk += DIGEST_BYTES;
			nChunkSize -= DIGEST_BYTES;
			ManglePassword();
		}

		// If there's some left, do part of the next block
		if(nChunkSize > 0)
		{
			ProcessBlock(pChunk, nChunkSize);
			m_nUnusedKeySize = DIGEST_BYTES - nChunkSize;
		}
	}

private:
	bool FinishBlock(unsigned char* pChunk, int nChunkSize)
	{
		int nSize;
		bool bRet;
		if(m_nUnusedKeySize > nChunkSize)
		{
			// We don't have enough to finish the block
			nSize = nChunkSize;
			bRet = false;
		}
		else
		{
			// We do have enough to finish the block
			nSize = m_nUnusedKeySize;
			bRet = true;
		}
		if(m_bEncrypt)
			ShiftDataIn(m_pPassphraseExtender, DATA_BYTES_TO_USE, pChunk, nSize);
		Xor(pChunk, m_keyBuffer + DIGEST_BYTES - m_nUnusedKeySize, nSize);
		if(!m_bEncrypt)
			ShiftDataIn(m_pPassphraseExtender, DATA_BYTES_TO_USE, pChunk, nSize);
		m_nUnusedKeySize -= nSize;
		return bRet;
	}

	void ProcessBlock(unsigned char* pData, int nDataSize)
	{
		// Make a blob that contains the passphrase concatenated with some data
		memcpy(m_pBlob, m_pPassphrase, m_nPassphraseSize);
		memcpy(m_pBlob + m_nPassphraseSize, m_pPassphraseExtender, DATA_BYTES_TO_USE);

		// Digest the blob to make a key
		Digest(m_digest, m_pBlob, m_nBlobSize);

		// Use some data to extend the passphrase
		if(m_bEncrypt)
			ShiftDataIn(m_pPassphraseExtender, DATA_BYTES_TO_USE, pData, nDataSize);

		// XOR the block with the key
		Xor(pData, m_digest, nDataSize);

		// Use some data to extend the passphrase
		if(!m_bEncrypt)
			ShiftDataIn(m_pPassphraseExtender, DATA_BYTES_TO_USE, pData, nDataSize);

		// Preserve the unused portion of the encryption key for the next chunk
		if(nDataSize < DIGEST_BYTES)
			memcpy(m_keyBuffer + nDataSize, m_digest + nDataSize, DIGEST_BYTES - nDataSize);
	}

	void ManglePassword()
	{
		// Make a blob that contains some data concatenated with the passphrase
		memcpy(m_pBlob, m_pPassphraseExtender, DATA_BYTES_TO_USE);
		memcpy(m_pBlob + DATA_BYTES_TO_USE, m_pPassphrase, m_nPassphraseSize);

		// Digest the blob to make a key
		Digest(m_digest, m_pBlob, m_nBlobSize);

		// XOR the passphrase with the key (note that if the passphrase is bigger than the digest, not all of the password will be changed.  That doesn't matter because only a digest of the new password is ever used so the extra (unchanged) part of the password still provides additional encryption strength.)
		Xor(m_pPassphrase, m_digest, MIN(m_nPassphraseSize, DIGEST_BYTES));
	}
};

#endif // __CRYPTO_H__
