/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __CLASSICSYNTAX_H__
#define __CLASSICSYNTAX_H__

#include "../../GClasses/GMacros.h"
#include "Error.h"

class ClassicSyntaxError;
class GXMLTag;

#define NEWPARSER

#ifdef NEWPARSER

class CSToken
{
friend class ClassicSyntax;
protected:
	int m_nStart;
	int m_nLength;
	int m_nLine;
	int m_nCol;
	const char* m_pFile;

	CSToken()
	{
		m_pFile = NULL;
	}

public:
	~CSToken()
	{
		Reset(NULL, 0, 0, 0, 0);
	}

	void Reset(const char* pFile, int nStart, int nLength, int nLine, int nCol);
	bool Equals(const char* szString);
	bool StartsWith(const char* szString);
	const char* GetValue() { return &m_pFile[m_nStart]; }
	int GetLength() { return m_nLength; }
	bool Advance(int n);
};


#define MAX_LOOK_AHEAD 10

#else // NEWPARSER

class CSToken
{
public:
	const char* szTok;

	bool Equals(const char* szString)
	{
		return stricmp(szTok, szString) == 0;
	}

	bool StartsWith(const char* szString)
	{
		return strnicmp(szTok, szString, strlen(szString)) == 0;
	}

	const char* GetValue() { return szTok; }
	int GetLength() { return strlen(szTok); }
	void Advance(int n) { szTok += n; }
};

#endif // NEWPARSER


#ifndef NEWPARSER
#define TOKEN_BUFFER_SIZE 4096
#endif // NEWPARSER


class ClassicSyntax
{
public:
	enum InstrType
	{
		IT_METH,
		IT_IF,
		IT_ELSE,
		IT_WHILE,
		IT_BLOCK,
	};

	enum State
	{
		S_EXPECT_CLASS,
		S_EXPECT_MEMBER_OR_METHOD,
	};

	const char* m_pFile;
	int m_nLength;

#ifdef NEWPARSER
	CSToken m_tokens[MAX_LOOK_AHEAD]; // wrap-around buffer
	int m_nTokenPos;
	int m_nActiveTokens;
#else // NEWPARSER
	CSToken m_token;
	char* m_pTokenBuffer;
#ifdef _DEBUG
#define TOKEN_MEMORY_SIZE 10
	char* m_pRecentlyEatenTokens[TOKEN_MEMORY_SIZE];
#endif // _DEBUG
#endif // NEWPARSER
	
	int m_nPos;
	State m_eState;
	ClassicSyntaxError* m_pErrorHolder;
	int m_nLineNumber;
	int m_nLineStartPos;
	bool m_bFoundBlankLine;
	const char* m_szFilename;

	GXMLTag* m_pCurrentFileTag;
	GXMLTag* m_pCurrentInterfaceTag;
	GXMLTag* m_pCurrentClassTag;
	GXMLTag* m_pCurrentMethodTag;

	CSToken* GetToken(int n);

#ifdef NEWPARSER
	void GetNextToken();
	void OnNewLine(int nPos);
	void FindNextToken(int* pnStart, int* pnLength, int* pnLine, int* pnCol);
#else // NEWPARSER
	bool PeekToken(const char* szDelimiters = NULL, bool bParensOK = false);
#endif // NEWPARSER

	// Helper methods
	bool Advance();
	void EatWhitespace();
	void SetError(ErrorStruct* pError);
	bool EatToken(const char* szToken);
	bool PeekCallBackToken();
	bool PeekCommentToken();
	bool IsDone();

	// Parsing methods
	GXMLTag* ParseFile();
	void ParseInterface(bool bMachine);
	void ParseInterfaceRef();
	void ParseConstant(GXMLTag* pParentTag);
	void ParseMethodDecl();
	void ParseCall(GXMLTag* pParentTag);
	void ParseCondition(GXMLTag* pParentTag, InstrType eType);
	void ParseElse(GXMLTag* pParentTag);
	void ParseSingleInstr(GXMLTag* pParentTag);

	ClassicSyntax(const char* szFilename, const char* pFile, int nLength, int nStartLine, ClassicSyntaxError* pErrorHolder);
	virtual ~ClassicSyntax();

	// Converts a file (already in memory) from Classic Syntax to XML
	static GXMLTag* ConvertToXML(const char* szFilename, const char* pFile, int nLength, int nStartLine, ClassicSyntaxError* pErrorHolder);
	
	// Loads a Classic Syntax file and converts it to XML
	static GXMLTag* LoadAndParseToXML(const char* szFilename, ClassicSyntaxError* pErrorHolder);

protected:
	bool AdvanceBytes(CSToken* pTok, int nBytes);
};

#endif // __CLASSICSYNTAX_H__
