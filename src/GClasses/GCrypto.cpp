/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GCrypto.h"
#include "sha1.h"

void WipeBuffer(unsigned char* pBuffer, int nSize)
{
	// don't use zeros, because a wiped-buffer can be used to
	// wipe out a CString, and it will stop overwriting data when
	// it hits a null-terminator thus leaving the CString unwiped.
	memset(pBuffer, 'X', nSize);
}

void WipeBuffer(char* pBuffer, int nSize)
{
	WipeBuffer((unsigned char*)pBuffer, nSize);
}

/*static*/ void GCrypto::Encrypt(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize)
{
	GCrypto crypto(pPassphrase, nPassphraseSize, true);
	crypto.ProcessChunk(pData, nDataSize);
}

/*static*/ void GCrypto::Decrypt(unsigned char* pData, int nDataSize, const unsigned char* pPassphrase, int nPassphraseSize)
{
	GCrypto crypto(pPassphrase, nPassphraseSize, false);
	crypto.ProcessChunk(pData, nDataSize);
}

// Currently uses Sha1, but you can swap out any cryptographic digest algorithm you want
void Digest(unsigned char* digest, unsigned char* pBlob, int nBlobSize)
{
	if(DIGEST_BYTES != 20)
		throw "Sha1 requires the digest size be set to 20 bytes!";
	SHA_CTX ctx;
	memset(&ctx, '\0', sizeof(SHA_CTX));
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, pBlob, nBlobSize);
	SHA1_Final(digest, &ctx); // note: this wipes the ctx structure, so we don't need to
}

void SafeStrCpy(char* szDest, const char* szSrc, int nMaxSize)
{
   nMaxSize--;
   if(nMaxSize < 0)
      return;
   int n;
   for(n = 0; szSrc[n] != '\0' && n < nMaxSize; n++)
      szDest[n] = szSrc[n];
   szDest[n] = '\0';
}

void Xor(unsigned char* pDest, unsigned char* pSource, int nBytes)
{
	while(nBytes)
	{
		*pDest++ ^= *pSource++;
		nBytes--;
	}
}
