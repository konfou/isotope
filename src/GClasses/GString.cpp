/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GString.h"
#include "GMacros.h"
#include "GQueue.h"
#include <string.h>
#include <stdlib.h>
#ifndef WIN32
#include <wchar.h>
#endif // WIN32
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

GString::GString(const wchar_t* wszString)
{
	m_pBuffer = NULL;
	m_nBufferSize = 0;
	m_nStringLength = 0;
	m_pQueue = NULL;
	if(wszString)
		Add(wszString);
}

GString::~GString()
{
	delete(m_pBuffer);
	delete(m_pQueue);
}

int GString::GetLength()
{
	int nQueueSize = m_pQueue ? m_pQueue->GetSize() / sizeof(wchar_t) : 0;
	return m_nStringLength + nQueueSize;
}

void GString::FlushQueue()
{
#ifdef _DEBUG
	int nOldLength = GetLength();
#endif // _DEBUG
	GAssert(m_pQueue, "no queue to flush");
	int nQueueSize = m_pQueue->GetSize() / sizeof(wchar_t);
	if(m_nBufferSize <= m_nStringLength + nQueueSize)
		ResizeBuffer(m_nStringLength + nQueueSize);
	int n;
	for(n = 0; n < nQueueSize; n++)
		m_pQueue->Pop(&m_pBuffer[m_nStringLength + n]);
	m_nStringLength += nQueueSize;
	m_pBuffer[m_nStringLength] = L'\0';
	delete(m_pQueue);
	m_pQueue = NULL;
	GAssert(GetLength() == nOldLength, "Problem flushing queue");
}

void GString::ResizeBuffer(int nLength)
{
	GAssert(nLength >= 0, "Out of range");
	GAssert(m_nStringLength == 0 || m_nStringLength < m_nBufferSize, "buffer size error");
	GAssert(m_nBufferSize <= nLength, "buffer is already big enough");
	int nNewSize = MAX(16, nLength + 1);
	wchar_t* pNewBuffer = new wchar_t[nNewSize];
	GAssert(pNewBuffer, "Failed to allocate buffer");
	memcpy(pNewBuffer, m_pBuffer, m_nStringLength * sizeof(wchar_t));
	pNewBuffer[m_nStringLength] = L'\0';
	delete(m_pBuffer);
	m_pBuffer = pNewBuffer;
	m_nBufferSize = nNewSize;
}

const wchar_t* GString::GetString()
{
	if(m_pQueue)
		FlushQueue();
	return m_pBuffer ? m_pBuffer : L"";
}

wchar_t GString::GetWChar(int nPos)
{
	if(m_pQueue)
		FlushQueue();
	GAssert(nPos >= 0 && nPos < m_nStringLength, "Out of range");
	return m_pBuffer[nPos];
}

void GString::SetChar(int nPos, wchar_t wc)
{
	GAssert(nPos >= 0, "Out of range");
	if(m_nBufferSize <= nPos)
		ResizeBuffer(nPos);
	if(m_pQueue)
		FlushQueue();
	if(nPos >= m_nStringLength)
	{
		m_nStringLength = nPos + 1;
		m_pBuffer[m_nStringLength] = L'\0';
	}
	m_pBuffer[nPos] = wc;
}

void GString::InsertChar(int nPos, wchar_t wc)
{
	if(m_pQueue)
		FlushQueue();
	if(m_nBufferSize >= m_nStringLength)
		ResizeBuffer(MAX(m_nStringLength * 2, 16));
	int n;
	for(n = m_nStringLength + 1; n >= nPos; n--)
		m_pBuffer[n + 1] = m_pBuffer[n];
	m_pBuffer[nPos] = wc;
}

void GString::RemoveLastChar()
{
	if(m_pQueue)
		FlushQueue();
	if(m_nStringLength > 0)
	{
		m_pBuffer[m_nStringLength - 1] = L'\0';
		m_nStringLength--;
	}
}

void GString::Add(const wchar_t* wszString, int nLen)
{
	if(m_pQueue || m_nBufferSize <= m_nStringLength + nLen)
	{
		// Put in a queue.  We'll add it into the buffer later.
		if(!m_pQueue)
			m_pQueue = new GQueue();
		int n;
		for(n = 0; n < nLen; n++)
			m_pQueue->Push(wszString[n]);
	}
	else
	{
		// The buffer is already big enough, so add it now.
		memcpy(m_pBuffer + m_nStringLength, wszString, nLen * sizeof(wchar_t));
		m_nStringLength += nLen;
		m_pBuffer[m_nStringLength] = L'\0';
	}
}

void GString::Add(GString* pString)
{
	const wchar_t* pStr = pString->GetString();
	Add(pStr, pString->m_nStringLength);
}

void GString::Add(GString* pString, int nStartPos, int nLen)
{
	Add(pString->GetString() + nStartPos, nLen);
}

void GString::Add(const char* szString)
{
	ConvertAnsiToUnicode(szString, usString);
	Add(usString);
}

void GString::Add(const wchar_t* wszString)
{
	Add(wszString, wcslen(wszString));
}

void GString::Add(const wchar_t wChar)
{
	wchar_t buf[2];
	buf[1] = L'\0';
	buf[0] = wChar;
	Add(buf, 1);
}

void GString::Add(const char c)
{
	wchar_t wc = c;
	Add(wc);
}

void GString::Add(int n)
{
	wchar_t wszBuf[64];
	swprintf(wszBuf, 64, L"%d", n);
	Add(wszBuf);
}

void GString::Add(double d)
{
	wchar_t wszBuf[64];
	swprintf(wszBuf, 64, L"%lf", d);
	Add(wszBuf);
}

void GString::Add(float f)
{
	wchar_t wszBuf[64];
	swprintf(wszBuf, 64, L"%f", f);
	Add(wszBuf);
}

void GString::Clear()
{
	m_nStringLength = 0;
	delete(m_pQueue);
	m_pQueue = NULL;
}

void GString::Copy(GString* pString)
{
	Clear();
	const wchar_t* pStr = pString->GetString();
	Add(pStr, pString->m_nStringLength);
}

void GString::Copy(const char* szString)
{
	ConvertAnsiToUnicode(szString, usString);
	Clear();
	Add(usString);
}

void GString::Copy(const wchar_t* wszString)
{
	Clear();
	Add(wszString);
}

void GString::Copy(const wchar_t* wszString, int nLen)
{
	Clear();
	Add(wszString, nLen);
}

void GString::Copy(GString* pString, int nStartPos, int nLen)
{
	Clear();
	Add(pString, nStartPos, nLen);
}

void GString::Copy(int n)
{
	Clear();
	Add(n);
}

void GString::Copy(double d)
{
	Clear();
	Add(d);
}

void GString::Copy(float f)
{
	Clear();
	Add(f);
}

void GString::SetBufferLengthAtLeast(int nNewLength)
{
	if(m_nBufferSize <= nNewLength)
		ResizeBuffer(nNewLength);
}

void GString::GetAnsi(char* pBuf)
{
	if(m_pQueue)
		FlushQueue();
	int n;
	for(n = 0; n < m_nStringLength; n++)
		pBuf[n] = (char)m_pBuffer[n];
	pBuf[n] = '\0';
}

int GString::CompareTo(GString* pString)
{
	return CompareTo(pString->GetString());
}

int GString::CompareTo(const wchar_t* wszString)
{
	return wcscmp(GetString(), wszString);
}

int GString::CompareIgnoringCase(GString* pString)
{
	return CompareIgnoringCase(pString->GetString());
}

inline wchar_t WCToUpper(wchar_t wc)
{
	if(wc >= L'a' && wc <= L'z')
		return wc + L'A'- L'a';
	return wc;
}

inline wchar_t WCToLower(wchar_t wc)
{
	if(wc >= L'A' && wc <= L'Z')
		return wc + L'a'- L'A';
	return wc;
}

#ifndef WIN32
int wcsicmp(const wchar_t* pA, const wchar_t* pB)
{
	while(true)
	{
		if(*pA < *pB)
			return -1;
		else if(*pA > *pB)
			return 1;
		else if(*pA == L'\0')
			return 0;
		pA++;
		pB++;
	}
}
#endif // !WIN32

int GString::CompareIgnoringCase(const wchar_t* wszString)
{
	return wcsicmp(GetString(), wszString);
}

int GString::Find(const wchar_t* wszString)
{
	if(m_pQueue)
		FlushQueue();
	int n, i;
	for(n = 0; m_pBuffer[n] != L'\0'; n++)
	{
		for(i = 0; wszString[i] != L'\0'; i++)
		{
			if(m_pBuffer[n + i] != wszString[i])
				break;
		}
		if(wszString[i] == L'\0')
			return n;
	}
	return -1;
}

int GString::Find(GString* pSubString)
{
	return Find(pSubString->GetString());
}

void GString::ToUpper()
{
	if(m_pQueue)
		FlushQueue();
	int n;
	for(n = 0; n < m_nStringLength; n++)
		m_pBuffer[n] = WCToUpper(m_pBuffer[n]);
	GAssert(m_pBuffer[n] == '\0', "Length not correct");
}

void GString::ToLower()
{
	if(m_pQueue)
		FlushQueue();
	int n;
	for(n = 0; n < m_nStringLength; n++)
		m_pBuffer[n] = WCToUpper(m_pBuffer[n]);
	GAssert(m_pBuffer[n] == '\0', "Length not correct");
}

int GString::FindIgnoringCase(const wchar_t* wszString)
{
	if(m_pQueue)
		FlushQueue();
	int n, i;
	for(n = 0; m_pBuffer[n] != L'\0'; n++)
	{
		for(i = 0; wszString[i] != L'\0'; i++)
		{
			if(WCToUpper(m_pBuffer[n + i]) != WCToUpper(wszString[i]))
				break;
		}
		if(wszString[i] == L'\0')
			return n;
	}
	return -1;
}

int GString::FindIgnoringCase(GString* pSubString)
{
	return FindIgnoringCase(pSubString->GetString());
}
/*
void GString::Format(const wchar_t* wszFormat, ...)
{
	va_list args;
	va_start(args, dwResourceID);
	int nSize = vscwprintf(wszFormat, args);
	wchar_t* wszTemp = (wchar_t*)alloca(nSize * sizeof(wchar_t));
	vswprintf(wszTemp, wszFormat, args);
	va_end(args);
	Copy(wszTemp);
}
*/






// ----------------------------------
// ANSI functions
// ----------------------------------

/*static*/ void GString::RemoveTrailingSpaces(char* szBuff)
{
   int n;
   for(n = strlen(szBuff) - 1; n > 0 && szBuff[n] == ' '; n--)
   {
      szBuff[n] = '\0';
   }
}

/*static*/ void GString::StrCpy(char* szDest, const char* szSrc, int nMaxSize)
{
   nMaxSize--;
   if(nMaxSize < 0)
      return;
   int n;
   for(n = 0; szSrc[n] != '\0' && n < nMaxSize; n++)
      szDest[n] = szSrc[n];
   szDest[n] = '\0';
}

/*static*/ void GString::StrIns(char* szString, const char* szInsertMe)
{
	int l = strlen(szInsertMe);
	int n;
	for(n = 0; szString[n] != '\0'; n++);
	for( ; n >= 0; n--)
		szString[n + l] = szString[n];
	for(n = 0; n < l; n++)
		szString[n] = szInsertMe[n];
}

/*static*/ void GString::StrInsLF(char* szString)
{
	int n;
	for(n = 0; szString[n] != '\0'; n++)
	{
		if(szString[n] == '\r')
		{
			StrIns(szString + n + 1, "\n");
			n++;
		}
	}
}

/*static*/ void GString::StrCut(char* szBuff, int n)
{
   char* pFront;
   char* pEnd;
   pFront = szBuff;
   pEnd = pFront + n;
   while(*pEnd != '\0')
   {
      *pFront = *pEnd;
      pFront++;
      pEnd++;
   }
   *pFront = '\0';
}

/*static*/ bool GString::DoesStrMatch(const char* szA, const char* szB)
{
	int n;
	for(n = 0; szA[n] != '\0' && szB[n] != '\0'; n++)
	{
		if(szA[n] != szB[n])
			return(false);
	}
	return(true);
}


