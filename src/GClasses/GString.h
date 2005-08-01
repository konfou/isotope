/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GSTRING_H__
#define __GSTRING_H__

#include <stdlib.h>

class GQueue;

class GString
{
protected:
	wchar_t* m_pBuffer;
	GQueue* m_pQueue;
	int m_nBufferSize;
	int m_nStringLength;

public:
	GString(const wchar_t* wszString = NULL);
	virtual ~GString();

	// Get
	int GetLength();
	const wchar_t* GetString();
	void GetAnsi(char* pBuf); // pBuf should be at least GetLength() + 1 bytes in size
	wchar_t GetWChar(int nPos);

	// Add
	void Add(GString* pString);
	void Add(const wchar_t wChar);
	void Add(const wchar_t* wszString);
	void Add(const char* szString);
	void Add(const wchar_t* wszString, int nLen);
	void Add(GString* pString, int nStartPos, int nLen);
	void Add(int n);

	// Copy
	void Copy(GString* pString);
	void Copy(const char* szString);
	void Copy(const wchar_t* wszString);
	void Copy(const wchar_t* wszString, int nLen);
	void Copy(GString* pString, int nStartPos, int nLen);

	// Compare
	int CompareTo(GString* pString);
	int CompareTo(const wchar_t* wszString);
	int CompareIgnoringCase(GString* pString);
	int CompareIgnoringCase(const wchar_t* wszString);

	// Find
	int Find(GString* pSubString);
	int Find(const wchar_t* wszSubstring);
	int FindIgnoringCase(const wchar_t* wszString);
	int FindIgnoringCase(GString* pSubString);

	// Misc
	void Clear();
	void SetChar(int nPos, wchar_t wc);
	void InsertChar(int nPos, wchar_t wc);
	void ToUpper();
	void ToLower();
	void SetBufferLengthAtLeast(int nNewLength);
	//void Format(const wchar_t* wszFormat, ...);






	// --------------
	// ANSI functions
	// --------------

	// This is similar to strncpy, but it always makes sure that
	// there is a null-terminating '\0' at the end of the new string.
	static void StrCpy(char* szDest, const char* szSrc, int nDestBufferSize);

	// This cuts out part of a string
	static void StrCut(char* szBuff, int n);

	// This inserts szInsertMe at the beginning of szString
	static void StrIns(char* szString, const char* szInsertMe);

	// This inserts Line-feeds after each newline character
	// (Warning: your buffer must be big enough (2x) )
	static void StrInsLF(char* szString);

	// This just chops spaces off the end of a string   
	static void RemoveTrailingSpaces(char* szBuff);

	// This returns true if the strings match until a null-terminator is found
	static bool DoesStrMatch(const char* szA, const char* szB);

protected:
	void ResizeBuffer(int nLength);
	void FlushQueue();
};

#endif // __GSTRING_H__