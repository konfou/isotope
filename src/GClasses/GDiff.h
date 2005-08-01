/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GDIFF_H__
#define __GDIFF_H__


struct DiffLine
{
	const char* pLine;
	int nLength;
	int nLineNumber1;
	int nLineNumber2;
};


// This class is for finding the differences between two text files
class GDiff
{
public:
	bool m_bIgnoreWhiteSpace;
	
	GDiff(const char* szFile1, const char* szFile2);
	virtual ~GDiff();

	bool GetNextLine(struct DiffLine* pDiffLine);

protected:
	const char* m_szFile1;
	const char* m_szFile2;
	const char* m_pPos1;
	const char* m_pPos2;
	int m_nLineNumber1;
	int m_nLineNumber2;
	bool m_bRememberUseFile1;
	const char* m_pRememberMatchPos;

	const char* FindFirstMatch(const char* szNeedle, const char* szHaystack);
	bool DoesLineMatch(const char* pLine1, const char* pLine2);
	bool GetOneFileOnlyLine(struct DiffLine* pDiffLine, bool bNewStuffInFirstFile);
	bool GetMatchingLine(struct DiffLine* pDiffLine);
};


#endif // __GDIFF_H__
