/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __MBIGINT_H__
#define __MBIGINT_H__

#include "../BuiltIns/GaspString.h"
#include "../Include/GaspEngine.h"
#include "../../GClasses/GBigNumber.h"
#include "../../GClasses/GMacros.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

class MBigInt : public WrapperObject
{
protected:
	GBigNumber m_value;

	MBigInt(Engine* pEngine)
		: WrapperObject(pEngine, "BigInt")
	{
	}

public:
	virtual ~MBigInt()
	{
	}

	void toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
	{
		GAssert(false, "todo: write me");
	}

	void fromStream(Engine* pEngine, EVar* pStream)
	{
		GAssert(false, "todo: write me");
	}

	void setRefs(Engine* pEngine, EVar* pRefs)
	{
		GAssert(false, "todo: write me");
	}

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		wcscpy(pBuf, L"<BigInt>"); // todo: do something better
	}

	void allocate(Engine* pEngine)
	{
		pEngine->SetThis(new MBigInt(pEngine));
	}

	void getSign(Engine* pEngine, EVar* pOutBool)
	{
		pOutBool->pIntObject->m_value = m_value.GetSign() ? 1 : 0;
	}

	void setSign(Engine* pEngine, EVar* pBool)
	{
		m_value.SetSign(pBool->pIntObject->m_value ? true : false);
	}

	void getBitCount(Engine* pEngine, EVar* pOutInt)
	{
		pOutInt->pIntObject->m_value = m_value.GetBitCount();
	}

	void getBit(Engine* pEngine, EVar* pOutBool, EVar* pIndex)
	{
		pOutBool->pIntObject->m_value = m_value.GetBit(pIndex->pIntObject->m_value) ? 1 : 0;
	}

	void setBit(Engine* pEngine, EVar* pIndex, EVar* pBool)
	{
		m_value.SetBit(pIndex->pIntObject->m_value, pBool->pIntObject->m_value ? true : false);
	}

	void getUIntCount(Engine* pEngine, EVar* pOutInt)
	{
		pOutInt->pIntObject->m_value = m_value.GetUIntCount();
	}

	void getUInt(Engine* pEngine, EVar* pOutInt, EVar* pIndex)
	{
		pOutInt->pIntObject->m_value = m_value.GetUInt(pIndex->pIntObject->m_value);
	}

	void setUInt(Engine* pEngine, EVar* pIndex, EVar* pInt)
	{
		m_value.SetUInt(pIndex->pIntObject->m_value, pInt->pIntObject->m_value);
	}

	void setToZero(Engine* pEngine)
	{
		m_value.SetToZero();
	}

	void isZero(Engine* pEngine, EVar* pOutBool)
	{
		pOutBool->pIntObject->m_value = m_value.IsZero() ? 1 : 0;
	}

	void compareTo(Engine* pEngine, EVar* pOutResult, EVar* pOther)
	{
		pOutResult->pIntObject->m_value = m_value.CompareTo(&pOther->pBigInt->m_value);
	}

	void copy(Engine* pEngine, EVar* pOther)
	{
		m_value.Copy(&pOther->pBigInt->m_value);
	}

	void toHex(Engine* pEngine, EVar* pOutString)
	{
		int nBufSize = m_value.GetUIntCount() * sizeof(unsigned int) * 2 + 64; // the 64 is just a padding of random size
		char* pBuf = (char*)alloca(nBufSize);
		m_value.ToHex(pBuf, nBufSize);
		ConvertAnsiToUnicode(pBuf, wszUnicode);
		pOutString->pStringObject->m_value.Copy(wszUnicode);
	}

	void fromHex(Engine* pEngine, EVar* pString)
	{
		ConvertUnicodeToAnsi(pString->pStringObject->m_value.GetString(), szString);
		m_value.FromHex(szString);
	}

	void invert(Engine* pEngine)
	{
		m_value.Invert();
	}

	void increment(Engine* pEngine)
	{
		m_value.Increment();
	}

	void decrement(Engine* pEngine)
	{
		m_value.Decrement();
	}

	void add(Engine* pEngine, EVar* pOther)
	{
		m_value.Add(&pOther->pBigInt->m_value);
	}

	void subtract(Engine* pEngine, EVar* pOther)
	{
		m_value.Subtract(&pOther->pBigInt->m_value);
	}

	void multiply(Engine* pEngine, EVar* pInt)
	{
		GBigNumber tmp;
		tmp.Copy(&m_value);
		m_value.Multiply(&tmp, pInt->pIntObject->m_value);
	}

	void multiply2(Engine* pEngine, EVar* pA, EVar* pB)
	{
		m_value.Multiply(&pA->pBigInt->m_value, &pB->pBigInt->m_value);
	}

	void divide(Engine* pEngine, EVar* pNominator, EVar* pDenominator, EVar* pRemainder)
	{
		m_value.Divide(&pNominator->pBigInt->m_value, &pDenominator->pBigInt->m_value, &pRemainder->pBigInt->m_value);
	}

	void shiftLeft(Engine* pEngine, EVar* pInt)
	{
		m_value.ShiftLeft(pInt->pIntObject->m_value);
	}

	void shiftRight(Engine* pEngine, EVar* pInt)
	{
		m_value.ShiftRight(pInt->pIntObject->m_value);
	}

	void Or(Engine* pEngine, EVar* pOther)
	{
		m_value.Or(&pOther->pBigInt->m_value);
	}

	void And(Engine* pEngine, EVar* pOther)
	{
		m_value.And(&pOther->pBigInt->m_value);
	}

	void Xor(Engine* pEngine, EVar* pOther)
	{
		m_value.Xor(&pOther->pBigInt->m_value);
	}

	void isPrime(Engine* pEngine, EVar* pOutBool)
	{
		pOutBool->pIntObject->m_value = m_value.IsPrime() ? 1 : 0;
	}

	void euclid(Engine* pEngine, EVar* pA, EVar* pB, EVar* pOutX, EVar* pOutY)
	{
		m_value.Euclid(&pA->pBigInt->m_value, &pB->pBigInt->m_value, pOutX->pBigInt ? &pOutX->pBigInt->m_value : NULL, pOutY->pBigInt ? &pOutY->pBigInt->m_value : NULL);
	}

	void powerMod(Engine* pEngine, EVar* pA, EVar* pK, EVar* pN)
	{
		m_value.PowerMod(&pA->pBigInt->m_value, &pK->pBigInt->m_value, &pN->pBigInt->m_value);
	}

	void millerRabin(Engine* pEngine, EVar* pOutBool, EVar* pSeed)
	{
		pOutBool->pIntObject->m_value = m_value.MillerRabin(&pSeed->pBigInt->m_value) ? 1 : 0;
	}

};

#endif // __MBIGINT_H__
