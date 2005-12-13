/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __MCONSOLE_H__
#define __MCONSOLE_H__

#include "../Include/GaspEngine.h"
#include "../../GClasses/GMacros.h"
#include "../BuiltIns/GaspString.h"
#include "../BuiltIns/GaspFloat.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

class MConsole : public WrapperObject
{
protected:
	MConsole(Engine* pEngine)
		: WrapperObject(pEngine, "Console")
	{
	}

public:
	virtual ~MConsole()
	{
	}

	void print(Engine* pEngine, EVar* pString)
	{
		wprintf(pString->pStringObject->m_value.GetString());
	}

	void printInt(Engine* pEngine, EVar* pString)
	{
		char szTmp[32];
		itoa(pString->pIntObject->m_value, szTmp, 10);
		printf(szTmp);
	}

	void printFloat(Engine* pEngine, EVar* pString)
	{
		char szTmp[64];
		sprintf(szTmp, "%lf", pString->pFloatObject->m_value);
		printf(szTmp);
	}

/*
	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		wcsncpy(pBuf, m_value.GetString(), nSize);
		pBuf[nSize - 1] = L'\0';
	}

	void allocate(Engine* pEngine)
	{
		pEngine->SetThis(new GaspString(pEngine));
	}

	void toInteger(Engine* pEngine, EVar* pOutValue)
	{
		ConvertUnicodeToAnsi(m_value.GetString(), szString);
		pOutValue->pIntObject->m_value = atoi(szString);
	}

	void fromInteger(Engine* pEngine, EVar* pValue)
	{
		char szTmp[32];
		itoa(pValue->pIntObject->m_value, szTmp, 10);
		ConvertAnsiToUnicode(szTmp, wszUnicode);
		m_value.Copy(wszUnicode);
	}

	void getLength(Engine* pEngine, EVar* pOutLength)
	{
		pOutLength->pIntObject->m_value = m_value.GetLength();
	}

	void getConstantString(Engine* pEngine, EVar* pIndex)
	{
		const char* szAnsi = pEngine->GetLibrary()->GetStringFromTable(pIndex->pIntObject->m_value);
		ConvertAnsiToUnicode(szAnsi, wszUnicode);
		GashString* pThis = new GashString(pEngine);
		pThis->m_value.Copy(wszUnicode);
		pEngine->SetThis(pThis);
	}

	void copy(Engine* pEngine, EVar* pOther)
	{
		m_value.Copy(&pOther->pStringObject->m_value);
	}

	void copySub(Engine* pEngine, EVar* pOther, EVar* pStart, EVar* pLength)
	{
		m_value.Copy(&pOther->pStringObject->m_value, pStart->pIntObject->m_value, pLength->pIntObject->m_value);
	}

	void add(Engine* pEngine, EVar* pOther)
	{
		m_value.Add(&pOther->pStringObject->m_value);
	}

	void compare(Engine* pEngine, EVar* pResult, EVar* pOther)
	{
		pResult->pIntObject->m_value = m_value.CompareTo(&pOther->pStringObject->m_value);
	}

	void compareIgnoringCase(Engine* pEngine, EVar* pResult, EVar* pOther)
	{
		pResult->pIntObject->m_value = m_value.CompareIgnoringCase(&pOther->pStringObject->m_value);
	}

	void clear(Engine* pEngine)
	{
		m_value.Copy(L"");
	}

	void find(Engine* pEngine, EVar* pOutIndex, EVar* pSubString)
	{
		pOutIndex->pIntObject->m_value = m_value.Find(&pSubString->pStringObject->m_value);
	}

	void findIgnoringCase(Engine* pEngine, EVar* pOutIndex, EVar* pSubString)
	{
		pOutIndex->pIntObject->m_value = m_value.FindIgnoringCase(&pSubString->pStringObject->m_value);
	}

	void toUpper(Engine* pEngine)
	{
		m_value.ToUpper();	
	}

	void toLower(Engine* pEngine)
	{
		m_value.ToLower();
	}*/
};


#endif // __MCONSOLE_H__
