/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "GDiff.h"

GDiff::GDiff(const char* szFile1, const char* szFile2)
{
	m_szFile1 = szFile1;
	m_szFile2 = szFile2;
	m_pPos1 = m_szFile1;
	m_pPos2 = m_szFile2;
	m_nLineNumber1 = 1;
	m_nLineNumber2 = 1;
	m_bIgnoreWhiteSpace = true;
	m_pRememberMatchPos = NULL;
}

GDiff::~GDiff()
{

}

bool GDiff::DoesLineMatch(const char* pLine1, const char* pLine2)
{
	// See if this line matches (ignoring whitespace)
	while(true)
	{
		if(m_bIgnoreWhiteSpace)
		{
			while(*pLine1 < ' ' && *pLine1 != '\n' && *pLine1 != '\0')
				pLine1++;
			while(*pLine2 < ' ' && *pLine2 != '\n' && *pLine2 != '\0')
				pLine2++;
		}
		if(*pLine1 == '\0' || *pLine1 == '\n' || *pLine1 == '\r')
		{
			if(*pLine2 == '\0' || *pLine2 == '\n' || *pLine2 == '\r')
				return true;
			else
				return false;
		}
		if(*pLine1 != *pLine2)
			return false;
		pLine1++;
		pLine2++;
	}
}

const char* GDiff::FindFirstMatch(const char* szNeedle, const char* szHaystack)
{
	if(*szNeedle == '\0')
		return NULL;
	while(*szHaystack != '\0')
	{
		if(DoesLineMatch(szNeedle, szHaystack))
			return szHaystack;
		while(*szHaystack != '\n' && *szHaystack != '\0')
			szHaystack++;
		if(*szHaystack == '\n')
			szHaystack++;
	}
	return NULL;
}

bool GDiff::GetNextLine(struct DiffLine* pDiffLine)
{
	if(*m_pPos1 == '\0' && *m_pPos2 == '\0') // if we're done
		return false;
	if(m_pRememberMatchPos)
		return GetOneFileOnlyLine(pDiffLine, m_bRememberUseFile1);
	if(DoesLineMatch(m_pPos1, m_pPos2))
		return GetMatchingLine(pDiffLine);
	const char* pFirstMatchInFile2 = FindFirstMatch(m_pPos1, m_pPos2);
	const char* pFirstMatchInFile1 = FindFirstMatch(m_pPos2, m_pPos1);
	bool bNoMatches = false;
	bool bUseFile1;
	if(pFirstMatchInFile2 && pFirstMatchInFile1)
	{
		if(pFirstMatchInFile1 - m_pPos1 < pFirstMatchInFile2 - m_pPos2)
			bUseFile1 = false;
		else
			bUseFile1 = true;
	}
	else
	{
		if(pFirstMatchInFile2)
			bUseFile1 = false;
		else if(pFirstMatchInFile1)
			bUseFile1 = true;
		else
			bNoMatches = true;
	}
	if(bNoMatches)
	{
		if(*m_pPos1 != '\0')
			return GetOneFileOnlyLine(pDiffLine, true);
		else
			return GetOneFileOnlyLine(pDiffLine, false);
	}
	else
	{
		m_pRememberMatchPos = bUseFile1 ? pFirstMatchInFile1 : pFirstMatchInFile2;
		m_bRememberUseFile1 = bUseFile1;
		return GetOneFileOnlyLine(pDiffLine, bUseFile1);
	}
}

bool GDiff::GetMatchingLine(struct DiffLine* pDiffLine)
{
	// Set the line
	pDiffLine->pLine = m_pPos1;

	// Set the line numbers
	pDiffLine->nLineNumber1 = m_nLineNumber1;
	pDiffLine->nLineNumber2 = m_nLineNumber2;

	// Move both file positions to the next line
	while(*m_pPos1 != '\n' && *m_pPos1 != '\0')
		m_pPos1++;
	if(*m_pPos1 == '\n')
		m_pPos1++;
	m_nLineNumber1++;
	while(*m_pPos2 != '\n' && *m_pPos2 != '\0')
		m_pPos2++;
	if(*m_pPos2 == '\n')
		m_pPos2++;
	m_nLineNumber2++;

	// Set the Length
	pDiffLine->nLength = m_pPos1 - pDiffLine->pLine;
	return true;
}

bool GDiff::GetOneFileOnlyLine(struct DiffLine* pDiffLine, bool bUseFile1)
{
	if(bUseFile1)
	{
		// Set the line
		pDiffLine->pLine = m_pPos1;

		// Set the line numbers
		pDiffLine->nLineNumber1 = m_nLineNumber1;
		pDiffLine->nLineNumber2 = 0; // This line doesn't exist in file2

		// Move the pointer to the next line
		while(*m_pPos1 != '\n' && *m_pPos1 != '\0')
			m_pPos1++;
		if(*m_pPos1 == '\n')
			m_pPos1++;
		m_nLineNumber1++;

		// Set the Length
		pDiffLine->nLength = m_pPos1 - pDiffLine->pLine;

		// Clear the remembry if we can
		if(m_pRememberMatchPos)
		{
			if(m_pPos1 >= m_pRememberMatchPos)
				m_pRememberMatchPos = NULL;
		}
	}
	else
	{
		// Set the line
		pDiffLine->pLine = m_pPos2;

		// Set the line numbers
		pDiffLine->nLineNumber1 = 0; // This line doesn't exist in file1
		pDiffLine->nLineNumber2 = m_nLineNumber2;

		// Move the pointer to the next line
		while(*m_pPos2 != '\n' && *m_pPos2 != '\0')
			m_pPos2++;
		if(*m_pPos2 == '\n')
			m_pPos2++;
		m_nLineNumber2++;

		// Set the Length
		pDiffLine->nLength = m_pPos2 - pDiffLine->pLine;

		// Clear the remembry if we can
		if(m_pRememberMatchPos)
		{
			if(m_pPos2 >= m_pRememberMatchPos)
				m_pRememberMatchPos = NULL;
		}
	}
	return true;
}
