/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GBigNumber.h"
#include "GMacros.h"
#include "GKeyPair.h"
#include "GRand.h"
#ifndef WIN32
#include <sys/types.h>
typedef int64_t __int64;
#endif // not WIN32

GBigNumber::GBigNumber()
{
	m_pBits = NULL;
	m_nUInts = 0;
	m_bSign = true;
}

GBigNumber::~GBigNumber()
{
	delete [] m_pBits;
}

unsigned int GBigNumber::GetBitCount()
{
	if(m_nUInts < 1)
		return 0;
	unsigned int nEnd = m_nUInts - 1;
	unsigned int nBitCount = m_nUInts * BITS_PER_INT;
	while(m_pBits[nEnd] == 0)
	{
		nBitCount -= BITS_PER_INT;
		if(nEnd == 0)
			return 0;
		nEnd--;
	}
	unsigned int nVal = m_pBits[nEnd];
	unsigned int nMask = 1 << (BITS_PER_INT - 1);
	while((nVal & nMask) == 0)
	{
		nBitCount--;
		nMask >>= 1;
	}
	return nBitCount;
}

void GBigNumber::Resize(unsigned int nBits)
{
	if(nBits < 1)
	{
		delete [] m_pBits;
		m_pBits = NULL;
		m_nUInts = 0;
		return;
	}
	unsigned int nNewUInts = ((nBits - 1) / BITS_PER_INT) + 1;
	if(nNewUInts <= m_nUInts && nNewUInts + 2 > m_nUInts / 2) // magic heuristic
	{
		unsigned int i;
		for(i = nNewUInts; i < m_nUInts; i++)
			m_pBits[i] = 0;
		return;
	}
	unsigned int* pNewBits = new unsigned int[nNewUInts];
	unsigned int nTop = m_nUInts;
	if(nNewUInts < nTop)
		nTop = nNewUInts;
	unsigned int n;
	for(n = 0; n < nTop; n++)
		pNewBits[n] = m_pBits[n];
	for( ; n < nNewUInts; n++)
		pNewBits[n] = 0;
	delete [] m_pBits;
	m_pBits = pNewBits;
	m_nUInts = nNewUInts;
}

void GBigNumber::SetBit(unsigned int nPos, bool bVal)
{
	if(nPos >= m_nUInts * BITS_PER_INT)
		Resize(nPos + 1);
	if(bVal)
		m_pBits[nPos / BITS_PER_INT] |= (1 << (nPos % BITS_PER_INT));
	else
		m_pBits[nPos / BITS_PER_INT] &= ~(1 << (nPos % BITS_PER_INT));
}

void GBigNumber::SetUInt(unsigned int nPos, unsigned int nVal)
{
	if(nPos >= m_nUInts)
		Resize((nPos + 1) * BITS_PER_INT);
	m_pBits[nPos] = nVal;
}

void GBigNumber::Copy(GBigNumber* pBigNumber)
{
	if(m_nUInts < pBigNumber->m_nUInts)
		Resize(pBigNumber->m_nUInts * BITS_PER_INT);
	if(m_nUInts < 1)
		return;
	unsigned int n;
	for(n = 0; n < pBigNumber->m_nUInts; n++)
		m_pBits[n] = pBigNumber->m_pBits[n];
	for(;n < m_nUInts; n++)
		m_pBits[n] = 0;
	m_bSign = pBigNumber->m_bSign;
}

void GBigNumber::SetToZero()
{
	memset(m_pBits, '\0', m_nUInts * sizeof(unsigned int));
	m_bSign = true;
}

bool GBigNumber::IsZero()
{
	if(m_nUInts < 1)
		return true;
	unsigned int n;
	for(n = 0; n < m_nUInts; n++)
	{
		if(m_pBits[n] != 0)
			return false;
	}
	return true;
}

unsigned int* GBigNumber::ToBufferGiveOwnership()
{
	unsigned int* pBuffer = m_pBits;
	m_bSign = true;
	m_nUInts = 0;
	m_pBits = NULL;
	return pBuffer;
}

bool GBigNumber::ToBuffer(unsigned int* pBuffer, int nBufferSize)
{
	int nSize = GetUIntCount();
	if(nBufferSize < nSize)
		return false;
	int n;
	for(n = nSize - 1; n >= 0; n--)
		pBuffer[n] = m_pBits[n];
	return true;
}

void GBigNumber::FromBuffer(const unsigned int* pBuffer, int nBufferSize)
{
	SetToZero();
	int n;
	for(n = nBufferSize - 1; n >= 0; n--)
		SetUInt(n, pBuffer[n]);
}

void GBigNumber::FromByteBuffer(const unsigned char* pBuffer, int nBufferChars)
{
	// Make sure input is alligned to sizeof(unsigned int)
#ifdef WIN32
	unsigned char* pBuf = (unsigned char*)_alloca(nBufferChars + sizeof(unsigned int) - 1);
#else
	unsigned char* pBuf = (unsigned char*)alloca(nBufferChars + sizeof(unsigned int) - 1);
#endif
	if((nBufferChars % sizeof(unsigned int)) != 0)
	{
		memset(pBuf, '\0', nBufferChars + sizeof(unsigned int) - 1);
		memcpy(pBuf, pBuffer, nBufferChars);
		pBuffer = pBuf;
		nBufferChars += sizeof(unsigned int) - 1;
	}
	nBufferChars /= sizeof(unsigned int);
	FromBuffer((unsigned int*)pBuffer, nBufferChars);
}

bool GBigNumber::ToHex(char* szBuff, int nBufferSize)
{
	bool bStarted = false;
	int nUInts = GetUIntCount();
	int n, i;
	unsigned char byte;
	char c;
	int nPos = 0;
	for(n = nUInts - 1; n >= 0; n--)
	{
		for(i = sizeof(int) * 2 - 1; i >= 0; i--)
		{
			byte = (m_pBits[n] >> (4 * i)) & 15;
			if(byte == 0 && !bStarted)
				continue;
			bStarted = true;
			c = byte < 10 ? '0' + byte : 'a' - 10 + byte;
			szBuff[nPos] = c;
			nPos++;
			if(nPos >= nBufferSize)
				return false;
		}
	}
	szBuff[nPos] = '\0';
	return true;
}

bool GBigNumber::FromHex(const char* szHexValue)
{
	unsigned int nLength = strlen(szHexValue);
	Resize(nLength * 4);
	SetToZero();
	unsigned int nUIntPos = 0;
	unsigned int nHexCount = 0;
	unsigned int n;
	for(n = 0; n < nLength; n++)
	{
		unsigned int nTmp;
		char cTmp = szHexValue[nLength - n - 1];
		if(cTmp >= '0' && cTmp <= '9')
			nTmp = cTmp - '0';
		else if(cTmp >= 'A' && cTmp <= 'F')
			nTmp = cTmp - 'A' + 10;
		else if(cTmp >= 'a' && cTmp <= 'f')
			nTmp = cTmp - 'a' + 10;
		else
			return false;
		m_pBits[nUIntPos] |= (nTmp << (4 * nHexCount));
		nHexCount++;
		if(nHexCount >= sizeof(unsigned int) * 2)
		{
			nHexCount = 0;
			nUIntPos++;
		}
	}
	return true;
}

void GBigNumber::Invert()
{
	m_bSign = !m_bSign;
}

int GBigNumber::CompareTo(GBigNumber* pOperand)
{
	if(m_bSign != pOperand->m_bSign)
	{
		if(IsZero() && pOperand->IsZero())
			return 0;
		return m_bSign ? 1 : -1;
	}
	int nCmp = 0;
	unsigned int nA;
	unsigned int nB;
	unsigned int n = m_nUInts;
	if(pOperand->m_nUInts > n)
		n = pOperand->m_nUInts;
	n--;
	while(true)
	{
		nA = GetUInt(n);
		nB = pOperand->GetUInt(n);
		if(nA != nB)
		{
			nCmp = nA > nB ? 1 : -1;
			break;
		}
		if(n == 0)
			break;
		n--;
	}
	return m_bSign ? nCmp : -nCmp;
}

void GBigNumber::Increment()
{
	if(!m_bSign)
	{
		Invert();
		Decrement();
		Invert();
		return;
	}
	unsigned int n;
	for(n = 0; true; n++)
	{
		if(n == m_nUInts)
			Resize(m_nUInts * BITS_PER_INT + 1);
		m_pBits[n]++;
		if(m_pBits[n] != 0)
			return;
	}
}

void GBigNumber::Decrement()
{
	if(!m_bSign)
	{
		Invert();
		Increment();
		Invert();
		return;
	}
	if(IsZero())
	{
		Increment();
		Invert();
		return;
	}
	unsigned int n;
	for(n = 0; true; n++)
	{
		if(m_pBits[n] == 0)
			m_pBits[n]--;
		else
		{
			m_pBits[n]--;
			return;
		}
	}
}

void GBigNumber::Add(GBigNumber* pBigNumber)
{
	// Check signs
	if(!m_bSign)
	{
		Invert();
		Subtract(pBigNumber);
		Invert();
		return;
	}
	if(!pBigNumber->m_bSign)
	{
		pBigNumber->Invert();
		Subtract(pBigNumber);
		pBigNumber->Invert();
		return;
	}

	// See if we need a bigger buffer
	unsigned int nBits = GetBitCount();
	unsigned int nTmp = pBigNumber->GetBitCount();
	if(nTmp > nBits)
		nBits = nTmp;
	nBits++;
	if(nBits > m_nUInts * BITS_PER_INT)
		Resize(nBits);

	// Add it up
	unsigned int nSum;
	unsigned int n;
	unsigned int nOperand;
	bool bNextCarry;
	bool bCarry = false;
	for(n = 0; n < m_nUInts; n++)
	{
		nOperand = pBigNumber->GetUInt(n);
		nSum = m_pBits[n] + nOperand;
		if(nSum < m_pBits[n] && nSum < nOperand)
			bNextCarry = true;
		else
			bNextCarry = false;
		if(bCarry)
		{
			if(++nSum == 0)
				bNextCarry = true;
		}
		bCarry = bNextCarry;
		m_pBits[n] = nSum;
	}
}

void GBigNumber::Subtract(GBigNumber* pBigNumber)
{
	// Check signs
	if(!m_bSign)
	{
		Invert();
		Add(pBigNumber);
		Invert();
		return;
	}
	if(!pBigNumber->m_bSign)
	{
		pBigNumber->Invert();
		Add(pBigNumber);
		pBigNumber->Invert();
		return;
	}

	// Check sizes
	GBigNumber tmp;
	GBigNumber* pA = this;
	GBigNumber* pB = pBigNumber;
	int nCmp = CompareTo(pBigNumber);
	if(nCmp < 0)
	{
		tmp.Copy(pBigNumber);
		pA = &tmp;
		pB = this;
	}

	// Subtract
	unsigned int n;
	unsigned int nA;
	unsigned int nB;
	bool bNextBorrow;
	bool bBorrow = false;
	for(n = 0; n < pA->m_nUInts; n++)
	{
		nA = pA->m_pBits[n];
		nB = pB->GetUInt(n);
		bNextBorrow = false;
		if(bBorrow)
		{
			if(nA == 0)
				bNextBorrow = true;
			nA--;
		}
		if(nB > nA)
			bNextBorrow = true;
		pA->m_pBits[n] = nA - nB;
		bBorrow = bNextBorrow;
	}

	// Invert again if we swapped A and B
	if(nCmp < 0)
	{
		tmp.Invert();
		Copy(&tmp);
	}
}

void GBigNumber::ShiftLeft(unsigned int nBits)
{
	ShiftLeftBits(nBits % BITS_PER_INT);
	ShiftLeftUInts(nBits / BITS_PER_INT);
}

void GBigNumber::ShiftLeftBits(unsigned int nBits)
{
	if(m_nUInts == 0)
		return;
	if(nBits == 0)
		return;
	if(m_pBits[m_nUInts - 1] != 0)
		Resize(GetBitCount() + nBits);
	unsigned int n;
	unsigned int nCarry = 0;
	unsigned int nNextCarry;
	for(n = 0; n < m_nUInts; n++)
	{
		nNextCarry = m_pBits[n] >> (BITS_PER_INT - nBits);
		m_pBits[n] <<= nBits;
		m_pBits[n] |= nCarry;
		nCarry = nNextCarry;
	}
}

void GBigNumber::ShiftLeftUInts(unsigned int nUInts)
{
	if(m_nUInts == 0)
		return;
	if(nUInts == 0)
		return;
	if(!(nUInts == 1 && m_pBits[m_nUInts - 1] == 0)) // optimization to make Multiply faster
		Resize((m_nUInts + nUInts) * BITS_PER_INT);
	unsigned int n = m_nUInts - 1;
	if(n >= nUInts)
	{
		while(true)
		{
			m_pBits[n] = m_pBits[n - nUInts];
			if(n - nUInts == 0)
			{
				n--;
				break;
			}
			n--;
		}
	}
	while(true)
	{
		m_pBits[n] = 0;
		if(n == 0)
			break;
		n--;
	}
	return;
}

void GBigNumber::ShiftRight(unsigned int nBits)
{
	ShiftRightBits(nBits % BITS_PER_INT);
	ShiftRightUInts(nBits / BITS_PER_INT);
}

void GBigNumber::ShiftRightBits(unsigned int nBits)
{
	if(m_nUInts == 0)
		return;
	if(nBits == 0)
		return;
	unsigned int n = m_nUInts - 1;
	unsigned int nCarry = 0;
	unsigned int nNextCarry;
	while(true)
	{
		nNextCarry = m_pBits[n] << (BITS_PER_INT - nBits);
		m_pBits[n] >>= nBits;
		m_pBits[n] |= nCarry;
		nCarry = nNextCarry;
		if(n == 0)
			break;
		n--;
	}
}

void GBigNumber::ShiftRightUInts(unsigned int nUInts)
{
	if(m_nUInts == 0)
		return;
	if(nUInts == 0)
		return;
	unsigned int n;
	for(n = 0; n + nUInts < m_nUInts; n++)
		m_pBits[n] = m_pBits[n + nUInts];
	for(;n < m_nUInts; n++)
		m_pBits[n] = 0;
}

void GBigNumber::Or(GBigNumber* pBigNumber)
{
	if(m_nUInts < pBigNumber->m_nUInts)
		Resize(pBigNumber->m_nUInts * BITS_PER_INT);
	unsigned int n;
	for(n = 0; n < m_nUInts; n++)
		m_pBits[n] |= pBigNumber->GetUInt(n);
}

void GBigNumber::And(GBigNumber* pBigNumber)
{
	unsigned int n;
	for(n = 0; n < m_nUInts; n++)
		m_pBits[n] &= pBigNumber->GetUInt(n);
}

void GBigNumber::Xor(GBigNumber* pBigNumber)
{
	if(m_nUInts < pBigNumber->m_nUInts)
		Resize(pBigNumber->m_nUInts * BITS_PER_INT);
	unsigned int n;
	for(n = 0; n < m_nUInts; n++)
		m_pBits[n] ^= pBigNumber->GetUInt(n);
}

void GBigNumber::Multiply(GBigNumber* pBigNumber, unsigned int nUInt)
{
	if(nUInt == 0)
	{
		SetToZero();
		return;
	}
	SetToZero();
	Resize((pBigNumber->m_nUInts + 1) * BITS_PER_INT);
	unsigned int n;
	for(n = 0; n < pBigNumber->m_nUInts; n++)
	{
#ifdef BIG_ENDIAN
		__int64 prod = (__int64)pBigNumber->m_pBits[n] * (__int64)nUInt;
		__int64 rev;
		((unsigned int*)&rev)[0] = ((unsigned int*)&prod)[1];
		((unsigned int*)&rev)[1] = ((unsigned int*)&prod)[0];
		*((__int64*)&m_pBits[n]) += rev;
#else // BIG_ENDIAN
		*((__int64*)&m_pBits[n]) += (__int64)pBigNumber->m_pBits[n] * (__int64)nUInt;
#endif // !BIG_ENDIAN
	}
}

void GBigNumber::Multiply(GBigNumber* pFirst, GBigNumber* pSecond)
{
	SetToZero();
	Resize(pFirst->GetBitCount() + pSecond->GetBitCount());
	GBigNumber tmp;
	unsigned int nUInts = pFirst->m_nUInts;
	if(pSecond->m_nUInts > nUInts)
		nUInts = pSecond->m_nUInts;
	unsigned int n;
	for(n = 0; n < nUInts; n++)
	{
		ShiftLeftUInts(1);
		tmp.Multiply(pFirst, pSecond->GetUInt(nUInts - 1 - n));
		Add(&tmp);
	}
	m_bSign = ((pFirst->m_bSign == pSecond->m_bSign) ? true : false);
}

void GBigNumber::Divide(GBigNumber* pInNominator, GBigNumber* pInDenominator, GBigNumber* pOutRemainder)
{
	SetToZero();
	Resize(pInNominator->GetBitCount());
	pOutRemainder->SetToZero();
	pOutRemainder->Resize(pInDenominator->GetBitCount());
	unsigned int nBits = pInNominator->GetBitCount();
	unsigned int n;
	for(n = 0; n < nBits; n++)
	{
		pOutRemainder->ShiftLeftBits(1);
		pOutRemainder->SetBit(0, pInNominator->GetBit(nBits - 1 - n));
		ShiftLeftBits(1);
		if(pOutRemainder->CompareTo(pInDenominator) >= 0)
		{
			SetBit(0, true);
			pOutRemainder->Subtract(pInDenominator);
		}
		else
		{
			SetBit(0, false);
		}
	}
	m_bSign = ((pInNominator->m_bSign == pInDenominator->m_bSign) ? true : false);
	pOutRemainder->m_bSign = pInNominator->m_bSign;
}

// DO NOT use for crypto
// This is NOT a cryptographic random number generator
void GBigNumber::SetRandom(unsigned int nBits)
{
	Resize(nBits);
	unsigned int nBytes = nBits / 8;
	unsigned int nExtraBits = nBits % 8;
	unsigned int n;
	for(n = 0; n < nBytes; n++)
		((unsigned char*)m_pBits)[n] = (unsigned char)rand();
	if(nExtraBits > 0)
	{
		unsigned char c = (unsigned char)rand();
		c <<= (8 - nExtraBits);
		c >>= (8 - nExtraBits);
		((unsigned char*)m_pBits)[n] = c;
	}
}

// Input:  integers a, b
// Output: [this,x,y] where "this" is the greatest common divisor of a,b and where g=xa+by (x or y can be negative)
void GBigNumber::Euclid(GBigNumber* pA1, GBigNumber* pB1, GBigNumber* pOutX/*=NULL*/, GBigNumber* pOutY/*=NULL*/)
{
	GBigNumber q;
	GBigNumber r;
	GBigNumber x;
	GBigNumber y;

	GBigNumber a;
	a.Copy(pA1);
	a.SetSign(true);

	GBigNumber b;
	b.Copy(pB1);
	b.SetSign(true);

	GBigNumber x0;
	x0.Increment();
	x0.SetSign(pA1->GetSign());
	GBigNumber x1;
	GBigNumber y0;
	GBigNumber y1;
	y1.Increment();
	y1.SetSign(pB1->GetSign());

	while(!b.IsZero())
	{
		q.Divide(&a, &b, &r);
		a.Copy(&b);
		b.Copy(&r);
		
		x.Multiply(&x1, &q);
		x.Invert();
		x.Add(&x0);
		
		x0.Copy(&x1);
		x1.Copy(&x);

		y.Multiply(&y1, &q);
		y.Invert();
		y.Add(&y0);

		y0.Copy(&y1);
		y1.Copy(&y);
	}
	Copy(&a);
	if(pOutX)
		pOutX->Copy(&x0);
	if(pOutY)
		pOutY->Copy(&y0);
}


// Input:  pProd is the product of (p - 1) * (q - 1) where p and q are prime
//         pRandomData is some random data that will be used to pick the key.
// Output: It will return a key that has no common factors with pProd.  It starts
//         with the random data you provide and increments it until it fits this
//         criteria.
void GBigNumber::SelectPublicKey(const unsigned int* pRandomData, int nRandomDataUInts, GBigNumber* pProd)
{
	// copy random data
	SetToZero();
	int n;
	for(n = nRandomDataUInts - 1; n >= 0; n--)
		SetUInt(n, pRandomData[n]);

	// increment until this number has no common factor with pProd
	GBigNumber tmp;
	while(true)
	{
		tmp.Euclid(this, pProd);
		tmp.Decrement();
		if(tmp.IsZero())
			break;
		Increment();
	}
}

// (Used by CalculatePrivateKey)
void EuclidSwap(GBigNumber* a, GBigNumber* b, GBigNumber* c)
{
	GBigNumber t1;
	t1.Copy(a);
	a->Copy(b);
	GBigNumber tmp;
	tmp.Multiply(b, c);
	b->Copy(&t1);
	b->Subtract(&tmp);
}

// Input:  pProd is the product of (p - 1) * (q - 1) where p and q are prime
//         pPublicKey is a number that has no common factors with pProd
// Output: this will become a private key to go with the public key
void GBigNumber::CalculatePrivateKey(GBigNumber* pPublicKey, GBigNumber* pProd)
{
	GBigNumber d;
	GBigNumber q;
	GBigNumber d2;
	GBigNumber u;
	GBigNumber u2;
	GBigNumber v;
	GBigNumber v2;
	GBigNumber r; // holds a remainder (unused)

	d.Copy(pPublicKey);
	d2.Copy(pProd);

	u.Increment();
	v2.Increment();

	while(true)
	{
		if(d2.IsZero())
			break;

		q.Divide(&d, &d2, &r);

		EuclidSwap(&d, &d2, &q);                
		EuclidSwap(&u, &u2, &q);                
		EuclidSwap(&v, &v2, &q);       
	}        

	if(!u.GetSign())
		u.Add(pProd);        
	Copy(&u);
}

// Input:  a, k>=0, n>=2
// Output: "this" where "this" = (a^k)%n   (^ = exponent operator, not xor operatore)
void GBigNumber::PowerMod(GBigNumber* pA, GBigNumber* pK, GBigNumber* pN)
{
	GBigNumber k;
	k.Copy(pK);
	GBigNumber c;
	c.Copy(pA);
	GBigNumber b;
	b.Increment();
	GBigNumber p;
	GBigNumber q;
	while(!k.IsZero())
	{
		if(k.GetBit(0))
		{
			k.Decrement();
			p.Multiply(&b, &c);
			q.Divide(&p, pN, &b);
		}
		p.Multiply(&c, &c);
		q.Divide(&p, pN, &c);
		k.ShiftRightBits(1);
	}
	Copy(&b);
}

// Input:  n>=3, a where 2<=a<n
// Output: "true" if this is either prime or a strong pseudoprime to base a,
//		   "false" otherwise
bool GBigNumber::MillerRabin(GBigNumber* pA)
{
	if(!GetBit(0))
		return false;
	GBigNumber g;
	g.Euclid(pA, this);
	GBigNumber one;
	one.Increment();
	if(g.CompareTo(&one) > 0)
		return false;
	GBigNumber m;
	m.Copy(this);
	m.Decrement();
	GBigNumber s;
	while(!m.GetBit(0))
	{
		m.ShiftRightBits(1);
		s.Increment();
	}
	GBigNumber b;
	b.PowerMod(pA, &m, this);
	if(b.CompareTo(&one) == 0)
		return true;
	Decrement();
	int nCmp = b.CompareTo(this);
	Increment();
	if(nCmp == 0)
		return true;
	GBigNumber i;
	GBigNumber b1;
	i.Increment();
	while(i.CompareTo(&s) < 0)
	{
		m.Multiply(&b, &b);
		g.Divide(&m, this, &b1);
		Decrement();
		nCmp = b1.CompareTo(this);
		Increment();
		if(nCmp == 0)
			return true;

		if(b1.CompareTo(&one) == 0)
			return false;
		b.Copy(&b1);
		i.Increment();
	}
	return false;
}

// Output: true = pretty darn sure (99.99%) it's prime
//		   false = definately (100%) not prime
bool GBigNumber::IsPrime()
{
	// Two is prime.  All values less than Two are not prime
	GBigNumber two;
	two.Increment();
	two.Increment();
	int nCmp = CompareTo(&two);
	if(nCmp < 1)
	{
		if(nCmp == 0)
			return true;
		else
			return false;
	}

	// 9 and 15 are the only 4-bit odd primes
	unsigned int nBits = GetBitCount();
	if(nBits <= 4)
	{
		unsigned int nValue = GetUInt(0);
		if((nValue & 1) == 0)
			return false;
		if(nValue == 9 || nValue == 15)
			return false;
		return true;
	}

	// Do 25 itterations of Miller-Rabin
	nBits--;
	unsigned int i;
	GBigNumber a;
	for(i = 1; i <= 25; i++)
	{
		a.SetRandom(nBits);
		if(a.CompareTo(&two) < 0)
		{
			i--;
			continue;
		}
		if(!MillerRabin(&a))
			return false;
	}
	return true;
}

void GBigNumber::SetToRand(GRand* pRand)
{
	SetToZero();
	const unsigned int* pData = (const unsigned int*)pRand->GetRand();
	int nUInts = pRand->GetRandByteCount() / sizeof(unsigned int);
	int n;
	for(n = nUInts - 1; n >= 0; n--)
		SetUInt(n, pData[n]);
}

