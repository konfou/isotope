/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "GKeyPair.h"
#include "GBigNumber.h"
#include "GMacros.h"
#include "GXML.h"
#include "GRand.h"

GKeyPair::GKeyPair()
{
	m_pPrivateKey = NULL;
	m_pPublicKey = NULL;
	m_pN = NULL;
}

GKeyPair::~GKeyPair()
{
	if(m_pPrivateKey)
		m_pPrivateKey->SetToZero();
	if(m_pPublicKey)
		m_pPublicKey->SetToZero();
	if(m_pN)
		m_pN->SetToZero();
	delete(m_pPrivateKey);
	delete(m_pPublicKey);
	delete(m_pN);
}

void GKeyPair::SetPublicKey(GBigNumber* pPublicKey)
{
	delete(m_pPublicKey);
	m_pPublicKey = pPublicKey;
}

void GKeyPair::SetPrivateKey(GBigNumber* pPrivateKey)
{
	delete(m_pPrivateKey);
	m_pPrivateKey = pPrivateKey;
}

void GKeyPair::SetN(GBigNumber* pN)
{
	delete(m_pN);
	m_pN = pN;
}

void GKeyPair::CopyPublicKey(GBigNumber* pPublicKey)
{
	delete(m_pPublicKey);
	m_pPublicKey = new GBigNumber();
	m_pPublicKey->Copy(pPublicKey);
}

void GKeyPair::CopyPrivateKey(GBigNumber* pPrivateKey)
{
	delete(m_pPrivateKey);
	m_pPrivateKey = new GBigNumber();
	m_pPrivateKey->Copy(pPrivateKey);
}

void GKeyPair::CopyN(GBigNumber* pN)
{
	delete(m_pN);
	m_pN = new GBigNumber();
	m_pN->Copy(pN);
}

GBigNumber* GKeyPair::GetPublicKey()
{
	return m_pPublicKey;
}

GBigNumber* GKeyPair::GetPrivateKey()
{
	return m_pPrivateKey;
}

GBigNumber* GKeyPair::GetN()
{
	return m_pN;
}

void GKeyPair::GenerateKeyPair(GRand* pRand)
{
	// Make places to put the data
	GBigNumber* pOutPublicKey = new GBigNumber();
	GBigNumber* pOutPrivateKey = new GBigNumber();
	GBigNumber* pOutN = new GBigNumber();

	// Find two primes
	GBigNumber p;
	GBigNumber q;
	int n;
	int nSize = pRand->GetRandByteCount() / sizeof(unsigned int);
	GAssert(nSize > 0, "size too small");
	const unsigned int* pRandomData = (const unsigned int*)pRand->GetRand();
	for(n = nSize - 1; n >= 0; n--)
		p.SetUInt(n, pRandomData[n]);
	pRandomData = (const unsigned int*)pRand->GetRand();
	for(n = nSize - 1; n >= 0; n--)
		q.SetUInt(n, pRandomData[n]);
	p.SetBit(0, true);
	q.SetBit(0, true);
	int nTries = 0;
	while(!p.IsPrime())
	{
		p.Increment();
		p.Increment();
		nTries++;
	}
	nTries = 0;
	while(!q.IsPrime())
	{
		q.Increment();
		q.Increment();
		nTries++;
	}

	// Calculate N (the product of the two primes)
	pOutN->Multiply(&p, &q);

	// Calculate prod ((p - 1) * (q - 1))
	p.Decrement();
	q.Decrement();
	GBigNumber prod;
	prod.Multiply(&p, &q);

	// Calculate public and private keys
	pOutPublicKey->SelectPublicKey((const unsigned int*)pRand->GetRand(), nSize, &prod);
	pOutPrivateKey->CalculatePrivateKey(pOutPublicKey, &prod);

	// Fill in "this" GKeyPair object
	SetPublicKey(pOutPublicKey);
	SetPrivateKey(pOutPrivateKey);
	SetN(pOutN);
}

GXMLTag* GKeyPair::ToXML(bool bIncludePrivateKey)
{
	GXMLTag* pTag = new GXMLTag("KeyPair");
	char pBuf[4096];
	if(GetN())
	{
		if(!GetN()->ToHex(pBuf, 4096))
		{
			delete(pTag);
			return NULL;
		}
		pTag->AddAttribute(new GXMLAttribute("N", pBuf));
	}
	else
		return pTag;
	if(GetPublicKey())
	{
		if(!GetPublicKey()->ToHex(pBuf, 4096))
		{
			delete(pTag);
			return NULL;
		}
		pTag->AddAttribute(new GXMLAttribute("Public", pBuf));
	}
	if(bIncludePrivateKey && GetPrivateKey())
	{
		if(!GetPrivateKey()->ToHex(pBuf, 4096))
		{
			delete(pTag);
			return NULL;
		}
		pTag->AddAttribute(new GXMLAttribute("Private", pBuf));
	}
	return pTag;
}

bool GKeyPair::FromXML(GXMLTag* pTag)
{
	if(stricmp(pTag->GetName(), "KeyPair") != 0)
		return false;
	GXMLAttribute* pAttr;
	pAttr = pTag->GetAttribute("N");
	if(pAttr)
	{
		GBigNumber* pBN = new GBigNumber();
		if(!pBN->FromHex(pAttr->GetValue()))
			return false;
		SetN(pBN);
	}
	pAttr = pTag->GetAttribute("Public");
	if(pAttr)
	{
		GBigNumber* pBN = new GBigNumber();
		if(!pBN->FromHex(pAttr->GetValue()))
			return false;
		SetPublicKey(pBN);
	}
	pAttr = pTag->GetAttribute("Private");
	if(pAttr)
	{
		GBigNumber* pBN = new GBigNumber();
		if(!pBN->FromHex(pAttr->GetValue()))
			return false;
		SetPrivateKey(pBN);
	}
	return true;
}

bool GKeyPair::FromXML(const char* pBuf, int nBufSize)
{
	GXMLTag* pTag = GXMLTag::FromString(pBuf, nBufSize);
	if(!pTag)
		return false;
	bool bRet = FromXML(pTag);
	delete(pTag);
	return bRet;
}

int GKeyPair::GetMaxBlockSize()
{
	return (m_pN->GetBitCount() - 1) / 8;
}

unsigned char* GKeyPair::PowerMod(const unsigned char* pInput, int nInputSize, bool bPublicKey, int* pnOutputSize)
{
	GBigNumber input;
	input.FromByteBuffer(pInput, nInputSize);
	GBigNumber results;
	results.PowerMod(&input, bPublicKey ? GetPublicKey() : GetPrivateKey(), GetN());
	*pnOutputSize = results.GetUIntCount() * sizeof(unsigned int);
	unsigned char* pOutput = (unsigned char*)results.ToBufferGiveOwnership();
	while(pOutput[(*pnOutputSize) - 1] == 0)
		(*pnOutputSize)--;
	return pOutput;
}
