/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "GRiddle.h"
#include "sha2.h"
#include "GMacros.h"
#include "GRand.h"
#include "GXML.h"
#include "GKeyPair.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN
#include <stdlib.h>

#define DIGEST_SIZE 64

GRiddle::GRiddle()
{
	m_nTotalBytes = 0;
	m_nMysteryBits = 0;
	m_pRiddle = NULL;
	m_pDigest = new unsigned char[DIGEST_SIZE];
	m_pSignature = NULL;
}

GRiddle::~GRiddle()
{
	delete(m_pRiddle);
	delete(m_pDigest);
	delete(m_pSignature);
}

int GRiddle::GetDigestByteCount()
{
	return DIGEST_SIZE;
}

void GRiddle::DigestRiddle(unsigned char* pOutput)
{
	sha512_ctx ctx;
	sha512_begin(&ctx);
	sha512_hash(m_pRiddle, m_nTotalBytes, &ctx);
	sha512_end(pOutput, &ctx);
}

bool GRiddle::GenerateRiddle(GRand* pRand, int nTotalSize, int nMysteryBits)
{
	GAssert(nTotalSize * 8 > nMysteryBits, "out of range");
	GAssert(pRand->GetRandByteCount(), "pRand was not seeded properly");
	delete(m_pRiddle);
	m_pRiddle = new unsigned char[nTotalSize];
	m_nTotalBytes = nTotalSize;
	int nPos = 0;
	while(nPos < nTotalSize)
	{
		const unsigned char* pRandData = pRand->GetRand();
		int nBytes = MIN(pRand->GetRandByteCount(), m_nTotalBytes - nPos);
		memcpy(m_pRiddle + nPos, pRandData, nBytes);
		nPos += nBytes;
	}
	DigestRiddle(m_pDigest);
	m_nMysteryBits = nMysteryBits;
	return true;
}

unsigned char ClearFrontBits(unsigned char* pBuffer, int nBits)
{
	int nBytesToClear = nBits / 8;
	int nExtraBits = nBits % 8;
	unsigned char cExtraBitsMask = (0xff << nExtraBits);
	memset(pBuffer, '\0', nBytesToClear);
	pBuffer[nBytesToClear] &= cExtraBitsMask;
	return cExtraBitsMask;
}

bool GRiddle::Solve()
{
	if(CheckAnswer())
		return true;
	if(!m_pRiddle)
	{
		GAssert(false, "No riddle to solve");
		return false;
	}
	unsigned char cExtraBitsMask = ClearFrontBits(m_pRiddle, m_nMysteryBits);
	int nUnknownBytes = m_nMysteryBits / 8;
	unsigned char cCheck = m_pRiddle[nUnknownBytes];
	GAssert((m_pRiddle[nUnknownBytes] & cExtraBitsMask) == cCheck, "didn't clear bits properly");
	unsigned char digest[DIGEST_SIZE];
	while(true)
	{
		DigestRiddle(digest);
		if(memcmp((unsigned int*)digest, (unsigned int*)m_pDigest, DIGEST_SIZE) == 0)
			break;
		int i = 0;
		while(true)
		{
			if(++m_pRiddle[i] != 0)
				break;
			++i;
		}
		if((m_pRiddle[nUnknownBytes] & cExtraBitsMask) != cCheck)
			return false; // no solution
	}
	GAssert(CheckAnswer(), "didn't solve it right");
	return true;
}

GXMLTag* GRiddle::ToXML(bool IncludeAnswer)
{
	unsigned char* pBuf = (unsigned char*)alloca(m_nTotalBytes);
	memcpy(pBuf, m_pRiddle, m_nTotalBytes);
	if(!IncludeAnswer)
		ClearFrontBits(pBuf, m_nMysteryBits);
	char* pRiddleHex = (char*)alloca(m_nTotalBytes * 2 + 1);
	BufferToHex(pBuf, m_nTotalBytes, pRiddleHex);
	char szDigestHex[DIGEST_SIZE * 2 + 1];
	BufferToHex(m_pDigest, DIGEST_SIZE, szDigestHex);
	char szTmp[64];
	itoa(m_nMysteryBits, szTmp, 10);

	GXMLTag* pTag = new GXMLTag("GRiddle");
	pTag->AddAttribute(new GXMLAttribute("Riddle", pRiddleHex));
	pTag->AddAttribute(new GXMLAttribute("Digest", szDigestHex));
	pTag->AddAttribute(new GXMLAttribute("Bits", szTmp));
	if(m_pSignature)
		pTag->AddAttribute(new GXMLAttribute("Signature", m_pSignature));
	return pTag;
}

void GRiddle::SetSignature(const char* szSig)
{
	delete(m_pSignature);
	if(!szSig)
	{
		m_pSignature = NULL;
		return;
	}
	m_pSignature = new char[strlen(szSig) + 1];
	strcpy(m_pSignature, szSig);
}

void GRiddle::Sign(GKeyPair* pKeyPair)
{
	GAssert(pKeyPair->GetPrivateKey(), "Can't sign with a keypair that doesn't have a private key");
	int nSignatureSize;
	unsigned char* pSignature = pKeyPair->PowerMod(m_pDigest, DIGEST_SIZE, false, &nSignatureSize);
	char* pSigHex = new char[nSignatureSize * 2 + 1];
	BufferToHex(pSignature, nSignatureSize, pSigHex);
	SetSignature(pSigHex);
	GAssert(CheckSignature(pKeyPair), "Signing error");
	delete(pSigHex);
	delete(pSignature);
}

bool GRiddle::CheckAnswer()
{
	unsigned char tmp[DIGEST_SIZE];
	DigestRiddle(tmp);
	return(memcmp(tmp, m_pDigest, DIGEST_SIZE) == 0);
}

bool GRiddle::CheckSignature(GKeyPair* pKeyPair)
{
	if(!m_pSignature)
		return false;
	GAssert(pKeyPair->GetPrivateKey(), "sorry, checking with public key not implemented yet");
	int nSignatureSize;
	unsigned char* pSignature =	pKeyPair->PowerMod(m_pDigest, DIGEST_SIZE, false, &nSignatureSize);
	char* pSigHex = new char[nSignatureSize * 2 + 1];
	BufferToHex(pSignature, nSignatureSize, pSigHex);
	bool bRet = (stricmp(pSigHex, m_pSignature) == 0);
	SetSignature(pSigHex);
	delete(pSigHex);
	delete(pSignature);
	return bRet;
/*	if(!m_pSignature)
		return false;
	int nLen = strlen(m_pSignature);
	unsigned char* pSig = (unsigned char*)_alloca(nLen / 2 + 1);
	HexToBuffer(m_pSignature, nLen, pSig);
	int nSignedDigestSize;
	unsigned char* pSignedDigest = pKeyPair->PowerMod(pSig, nLen / 2, true, &nSignedDigestSize);
	bool bOK = true;
	if(nSignedDigestSize != DIGEST_SIZE)
		bOK = false;
	if(memcmp(pSignedDigest, m_pDigest, DIGEST_SIZE) != 0)
		bOK = false;
	delete(pSignedDigest);
	return bOK;*/
}

bool GRiddle::FromXML(GXMLTag* pTag)
{
	if(stricmp(pTag->GetName(), "GRiddle") != 0)
	{
		GAssert(false, "not a GRiddle XML serialization");
		return false;
	}

	// Decode the riddle
	GXMLAttribute* pAttr;
	pAttr = pTag->GetAttribute("Riddle");
	if(!pAttr)
		return false;
	int nLen = strlen(pAttr->GetValue());
	if(nLen % 2 != 0)
	{
		GAssert(false, "hex length not multiple of two");
		return false;
	}
	m_nTotalBytes = nLen / 2;
	delete(m_pRiddle);
	m_pRiddle = new unsigned char[m_nTotalBytes];
	HexToBuffer(pAttr->GetValue(), nLen, m_pRiddle);

	// Decode the digest
	pAttr = pTag->GetAttribute("Digest");
	if(!pAttr)
		return false;
	nLen = strlen(pAttr->GetValue());
	if(nLen != 2 * DIGEST_SIZE)
	{
		GAssert(false, "digest wrong size");
		return false;
	}
	HexToBuffer(pAttr->GetValue(), nLen, m_pDigest);

	// Decode the mystery bit count
	pAttr = pTag->GetAttribute("Bits");
	if(!pAttr)
		return false;
	m_nMysteryBits = atoi(pAttr->GetValue());

	// Handle the signature
	pAttr = pTag->GetAttribute("Signature");
	if(pAttr)
		SetSignature(pAttr->GetValue());
	else
		SetSignature(NULL);

	return true;
}
