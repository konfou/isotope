/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GASPFLOAT_H__
#define __GASPFLOAT_H__

#include "../Include/GaspEngine.h"
#include "GaspString.h"
#include <math.h>

class GaspFloat : public WrapperObject
{
friend class VarHolder;
public:
	double m_value;

	GaspFloat(Engine* pEngine)
		: WrapperObject(pEngine, "Float")
	{
		m_value = 0;
	}

	virtual ~GaspFloat()
	{
	}

	void toStream(Engine* pEngine, EVar* pStream, EVar* pRefs);
	void fromStream(Engine* pEngine, EVar* pStream);
	void setRefs(Engine* pEngine, EVar* pRefs);

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		GAssert(nSize > 32, "Buffer too small");
		swprintf(pBuf, 32, L"%f", m_value);
	}

	void allocate(Engine* pEngine)
	{
		pEngine->SetThis(new GaspFloat(pEngine));
	}

	void newcopy(Engine* pEngine, EVar* pThat)
	{
		GaspFloat* pNewThis = new GaspFloat(pEngine);
		pNewThis->m_value = pThat->pFloatObject->m_value;
		pEngine->SetThis(pNewThis);
	}

	void copy(Engine* pEngine, EVar* pThat)
	{
		m_value = pThat->pFloatObject->m_value;
	}

	void copyInt(Engine* pEngine, EVar* pThat)
	{
		m_value = pThat->pIntObject->m_value;
	}

	void isZero(Engine* pEngine, EVar* pOutBool)
	{
		pOutBool->pIntObject->m_value = (m_value == 0 ? 1 : 0);
	}

	void isPositive(Engine* pEngine, EVar* pOutBool)
	{
		pOutBool->pIntObject->m_value = (m_value >= 0 ? 1 : 0);
	}

	void isNonZero(Engine* pEngine, EVar* pOutBool)
	{
		pOutBool->pIntObject->m_value = (m_value != 0 ? 1 : 0);
	}

	void toInteger(Engine* pEngine, EVar* pOutValue)
	{
		pOutValue->pIntObject->m_value = (int)m_value;
	}

	void fromInteger(Engine* pEngine, EVar* pValue)
	{
		m_value = pValue->pIntObject->m_value;
	}

	void toString(Engine* pEngine, EVar* pOutString)
	{
		wchar_t tmp[64];
		swprintf(tmp, 64, L"%f", m_value);
		pOutString->pStringObject->m_value.Copy(tmp);
	}

	void fromString(Engine* pEngine, EVar* pString)
	{
		wchar_t* tmp;
		m_value = wcstod(pString->pStringObject->m_value.GetString(), &tmp);
	}

	void add(Engine* pEngine, EVar* pOther)
	{
		m_value += pOther->pFloatObject->m_value;
	}

	void addTwo(Engine* pEngine, EVar* pResult, EVar* pOther)
	{
		GaspFloat* pNewFloat = new GaspFloat(pEngine);
		pEngine->SetVar(pResult, pNewFloat);
		pNewFloat->m_value = m_value + pOther->pFloatObject->m_value;
	}

	void addInt(Engine* pEngine, EVar* pInt)
	{
		m_value += pInt->pIntObject->m_value;
	}

	void subtract(Engine* pEngine, EVar* pOther)
	{
		m_value -= pOther->pFloatObject->m_value;
	}

	void subtractTwo(Engine* pEngine, EVar* pResult, EVar* pOther)
	{
		GaspFloat* pNewFloat = new GaspFloat(pEngine);
		pEngine->SetVar(pResult, pNewFloat);
		pNewFloat->m_value = m_value - pOther->pFloatObject->m_value;
	}

	void subtractInt(Engine* pEngine, EVar* pOther)
	{
		m_value -= pOther->pIntObject->m_value;
	}

	void multiply(Engine* pEngine, EVar* pOther)
	{
		m_value *= pOther->pFloatObject->m_value;
	}

	void multiplyTwo(Engine* pEngine, EVar* pResult, EVar* pOther)
	{
		GaspFloat* pNewFloat = new GaspFloat(pEngine);
		pEngine->SetVar(pResult, pNewFloat);
		pNewFloat->m_value = m_value * pOther->pFloatObject->m_value;
	}

	void multiplyInt(Engine* pEngine, EVar* pOther)
	{
		m_value *= pOther->pIntObject->m_value;
	}

	void divide(Engine* pEngine, EVar* pOther)
	{
		m_value /= pOther->pFloatObject->m_value;
	}

	void divideTwo(Engine* pEngine, EVar* pResult, EVar* pOther)
	{
		GaspFloat* pNewFloat = new GaspFloat(pEngine);
		pEngine->SetVar(pResult, pNewFloat);
		pNewFloat->m_value = m_value / pOther->pFloatObject->m_value;
	}

	void maxTwo(Engine* pEngine, EVar* pResult, EVar* pOther)
	{
		GaspFloat* pNewFloat = new GaspFloat(pEngine);
		pEngine->SetVar(pResult, pNewFloat);
		if(m_value > pOther->pFloatObject->m_value)
            pNewFloat->m_value = m_value;
		else
			pNewFloat->m_value = pOther->pFloatObject->m_value;
	}

	void minTwo(Engine* pEngine, EVar* pResult, EVar* pOther)
	{
		GaspFloat* pNewFloat = new GaspFloat(pEngine);
		pEngine->SetVar(pResult, pNewFloat);
		if(m_value < pOther->pFloatObject->m_value)
            pNewFloat->m_value = m_value;
		else
			pNewFloat->m_value = pOther->pFloatObject->m_value;
	}

	void divideInt(Engine* pEngine, EVar* pOther)
	{
		m_value /= pOther->pIntObject->m_value;
	}

	void sine(Engine* pEngine, EVar* pOther)
	{
		m_value = sin(pOther->pFloatObject->m_value);
	}

	void cosine(Engine* pEngine, EVar* pOther)
	{
		m_value = cos(pOther->pFloatObject->m_value);
	}

	void abs(Engine* pEngine)
	{
		if(m_value < 0)
			m_value = -m_value;
	}

	void compareTo(Engine* pEngine, EVar* pOutResult, EVar* pOther)
	{
		double dThat = pOther->pFloatObject->m_value;
		if(m_value < dThat)
			pOutResult->pIntObject->m_value = -1;
		else if(m_value > dThat)
			pOutResult->pIntObject->m_value = 1;
		else
			pOutResult->pIntObject->m_value = 0;
	}

	void isEqual(Engine* pEngine, EVar* pOutResult, EVar* pOther)
	{
		double dThat = pOther->pFloatObject->m_value;
		pOutResult->pIntObject->m_value = (m_value == dThat);
	}

	void isNotEqual(Engine* pEngine, EVar* pOutResult, EVar* pOther)
	{
		double dThat = pOther->pFloatObject->m_value;
		pOutResult->pIntObject->m_value = (m_value != dThat);
	}

	void isLessThan(Engine* pEngine, EVar* pOutResult, EVar* pOther)
	{
		double dThat = pOther->pFloatObject->m_value;
		pOutResult->pIntObject->m_value = (m_value < dThat);
	}

	void isGreaterThan(Engine* pEngine, EVar* pOutResult, EVar* pOther)
	{
		double dThat = pOther->pFloatObject->m_value;
		pOutResult->pIntObject->m_value = (m_value > dThat);
	}

/*
	void compareTo(Engine* pEngine, EVar* pOutResult, EVar* pOther, EVar* pTolerance)
	{
		double dThat = pOther->pFloatObject->m_value;
		double dTol = pTolerance->pFloatObject->m_value;
		if(m_value < dThat - dTol)
			pOutResult->pIntObject->m_value = -1;
		else if(m_value > dThat + dTol)
			pOutResult->pIntObject->m_value = 1;
		else
			pOutResult->pIntObject->m_value = 0;
	}
*/
};

#endif // __GASPFLOAT_H__
