/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "../../GClasses/GQueue.h"
#include "../../GClasses/GXML.h"
#include "ClassicSyntax.h"
#include "Error.h"
#include "TagNames.h"
#include "../CodeObjects/Method.h"
#include "../CodeObjects/Instruction.h"
#include "../CodeObjects/Block.h"
#include "../CodeObjects/Expression.h"
#include "../CodeObjects/Class.h"
#include "../CodeObjects/Variable.h"
#include "../CodeObjects/Call.h"
#ifdef WIN32
#include <io.h>
#endif // WIN32


#ifdef NEWPARSER

void CSToken::Reset(const char* pFile, int nStart, int nLength, int nLine, int nCol)
{
	m_pFile = pFile;
	m_nStart = nStart;
	m_nLength = nLength;
	m_nLine = nLine;
	m_nCol = nCol;
}

bool CSToken::Advance(int n)
{
	GAssert(m_nLength >= n, "token not that big");
	m_nLength -= n;
	m_nStart += n;
	m_nCol += n;
	return true;
}

bool CSToken::StartsWith(const char* szString)
{
	int n;
	for(n = 0; n < m_nLength && szString[n] != '\0'; n++)
	{
		if((m_pFile[m_nStart + n] | 32) != (szString[n] | 32))
			return false;
	}
	return true;
}

bool CSToken::Equals(const char* szString)
{
	int n;
	for(n = 0; n < m_nLength; n++)
	{
		if(m_pFile[m_nStart + n] != szString[n])
			return false;
	}
	if(szString[n] != '\0')
		return false;
	return true;
}




#endif // NEWPARSER



// -------------------------------------------------------------------


// todo: unmagic all the hard-coded tokens in this file

ClassicSyntax::ClassicSyntax(const char* szFilename, const char* pFile, int nLength, int nStartLine, ClassicSyntaxError* pErrorHolder)
{
	m_szFilename = szFilename;
	m_pFile = pFile;
	m_nLength = nLength;
	m_nPos = 0;
	m_pErrorHolder = pErrorHolder;
	State m_eState = S_EXPECT_CLASS;
	m_nLineNumber = nStartLine;
	m_nLineStartPos = 0;
	m_bFoundBlankLine = false;
#ifdef NEWPARSER
	m_nTokenPos = 0;
	m_nActiveTokens = 0;
#else // NEWPARSER
	m_pTokenBuffer = new char[TOKEN_BUFFER_SIZE + 1];
	m_pTokenBuffer[0] = '\0';
#ifdef _DEBUG
	int n;
	for(n = 0; n < TOKEN_MEMORY_SIZE; n++)
	{
		m_pRecentlyEatenTokens[n] = new char[TOKEN_BUFFER_SIZE + 1];
		strcpy(m_pRecentlyEatenTokens[n], "");
	}
#endif // _DEBUG
#endif // NEWPARSER

	m_pCurrentFileTag = NULL;
	m_pCurrentClassTag = NULL;
	m_pCurrentMethodTag = NULL;
}

ClassicSyntax::~ClassicSyntax()
{
	GAssert(m_pCurrentFileTag == NULL, "Memory wasn't properly cleaned up");
#ifdef NEWPARSER
#else // NEWPARSER
	delete(m_pTokenBuffer);
#ifdef _DEBUG
	int n;
	for(n = 0; n < TOKEN_MEMORY_SIZE; n++)
		delete(m_pRecentlyEatenTokens[n]);
#endif // _DEBUG
#endif // NEWPARSER
}

#ifndef NEWPARSER

CSToken* ClassicSyntax::GetToken(int n)
{
	GAssert(n == 0, "old syntax doesn't support looking farther ahead");
	if(!PeekToken())
		strcpy(m_pTokenBuffer, "");
	m_token.szTok = m_pTokenBuffer;
	return &m_token;
}

#endif // !NEWPARSER


#ifdef NEWPARSER
CSToken* ClassicSyntax::GetToken(int n)
{
	GAssert(n < MAX_LOOK_AHEAD, "can't look that far ahead");
	while(n >= m_nActiveTokens)
		GetNextToken();
	int nPos = m_nTokenPos + n;
	if(nPos >= MAX_LOOK_AHEAD)
		nPos -= MAX_LOOK_AHEAD;
	return &m_tokens[nPos];
}

void ClassicSyntax::GetNextToken()
{
	GAssert(m_nActiveTokens < MAX_LOOK_AHEAD, "got all the tokens we can hold");
/*	if(m_nFileStackPos <= 0)
	{
		int nNextToken = m_nTokenPos + m_nActiveTokens;
		if(nNextToken >= MAX_LOOK_AHEAD)
			nNextToken -= MAX_LOOK_AHEAD;
		m_tokens[nNextToken].Reset(NULL, 0, 0, 0, 0);
		m_nActiveTokens++;
		return;
	}*/	
	int nStart, nLength, nLine, nCol;
	nLength = 0;
	while(true)
	{
		FindNextToken(&nStart, &nLength, &nLine, &nCol);

		// Set the token value
		int nNextToken = m_nTokenPos + m_nActiveTokens;
		if(nNextToken >= MAX_LOOK_AHEAD)
			nNextToken -= MAX_LOOK_AHEAD;
		m_tokens[nNextToken].Reset(m_pFile, nStart, nLength, nLine, nCol);
		m_nActiveTokens++;
		
		// If we hit the end of the file, break
		if(nLength <= 0)
			break;
/*
		// Do any necessary macro substitutions
		const char* szMacro;
		if(GetMacro(&m_pFile[nStart], nLength, &szMacro))
		{
			if(!ParseMacro(szMacro))
				m_nRecoveries++;
			continue;
		}
*/
		// It's just a normal token, so we're done
		break;
	}
}

void ClassicSyntax::OnNewLine(int nPos)
{
	m_nLineNumber++;
	m_nLineStartPos = nPos + 1;
}

void ClassicSyntax::FindNextToken(int* pnStart, int* pnLength, int* pnLine, int* pnCol)
{
	// Find the start of the token
	const char* pFile = m_pFile;
	int nPos = m_nPos;

	// Skip whitespace and comments, and process compiler directives
	while(pFile[nPos] != '\0')
	{
/*		if(pFile[nPos] == '#')
		{
			const char* pStart = &pFile[nPos];
			nPos++;
			while((pFile[nPos] != '\n' || pFile[nPos - 1] == '\\' || (pFile[nPos - 1] == '\r' && pFile[nPos - 2] == '\\')) && pFile[nPos] != '\0')
				nPos++;
			GetCurrentFile()->m_nFilePos = nPos;
			int nLen = &pFile[nPos] - pStart;
			if(!ParseCompilerDirective(pStart, nLen))
				m_nRecoveries++;
			pFile = GetCurrentFile()->m_pFile;
			nPos = GetCurrentFile()->m_nFilePos;
		}
		else if(m_nIfDefSkips > 0)
		{
			while(pFile[nPos] != '\n' && pFile[nPos] != '\0')
				nPos++;
			if(pFile[nPos] == '\n')
			{
				OnNewLine(nPos);
				nPos++;
			}
		}
		else */if(pFile[nPos] <= ' ')
		{
			while(pFile[nPos] <= ' ' && pFile[nPos] != '\0')
			{
				if(pFile[nPos] == '\n')
					OnNewLine(nPos);
				nPos++;
			}
		}
		else if(pFile[nPos] == '/' && pFile[nPos + 1] == '/')
		{
			nPos += 2;
			while(pFile[nPos] != '\n' && pFile[nPos] != '\0')
				nPos++;
		}
		else if(pFile[nPos] == '/' && pFile[nPos + 1] == '*')
		{
			nPos += 2;
			while(true)
			{
				if(pFile[nPos] == '\0')
					break;
				if(pFile[nPos] == '*' && pFile[nPos + 1] == '/')
					break;
				if(pFile[nPos] == '\n')
					OnNewLine(nPos);
				nPos++;
			}
			if(pFile[nPos] != '\0')
				nPos += 2;
		}
		else
			break;
	}

	// Record where the token starts
	*pnStart = nPos;
	*pnLine = m_nLineNumber;
	*pnCol = m_nLineStartPos - nPos;

	// Move past the token
	if(pFile[nPos] == '\0')
	{
	}
	else if(pFile[nPos] == ',')
		nPos++;
	else if(pFile[nPos] == '(')
		nPos++;
	else if(pFile[nPos] == ')')
		nPos++;
	else if(pFile[nPos] == '{')
		nPos++;
	else if(pFile[nPos] == '}')
		nPos++;
	else
	{
		while(pFile[nPos] != '\0' &&
			pFile[nPos] != '\n' &&
			pFile[nPos] != ',' &&
			pFile[nPos] != '(' &&
			pFile[nPos] != ')' &&
			pFile[nPos] != '{' &&
			pFile[nPos] != '}')
			nPos++;
	}

/*
	else if(pFile[nPos] == '"')
	{
		while(true)
		{
			int nTmpPos = nPos;
			while(pFile[nTmpPos] <= ' ' && pFile[nTmpPos] != '\0')
				nTmpPos++;
			if(pFile[nTmpPos] != '"')
				break;
			while(nPos <= nTmpPos)
			{
				if(pFile[nPos] == '\n')
					OnNewLine(nPos);
				nPos++;
			}
			while((pFile[nPos] != '"' || pFile[nPos - 1] == '\\') && pFile[nPos] != '\n' && pFile[nPos] != '\0')
				nPos++;
			if(pFile[nPos] == '"')
				nPos++;
		}
	}
	else if((pFile[nPos] >= '0' && pFile[nPos] <= '9') ||
		    (pFile[nPos] >= 'A' && pFile[nPos] <= 'Z') ||
			(pFile[nPos] >= 'a' && pFile[nPos] <= 'z') ||
			(pFile[nPos] == '_'))
	{
		nPos++;
		while((pFile[nPos] >= '0' && pFile[nPos] <= '9') ||
			  (pFile[nPos] >= 'A' && pFile[nPos] <= 'Z') ||
			  (pFile[nPos] >= 'a' && pFile[nPos] <= 'z') ||
			  (pFile[nPos] == '_'))
			nPos++;
	}
	else if(pFile[nPos + 1] == '=' && (
		pFile[nPos] == '=' ||
		pFile[nPos] == '!' ||
		pFile[nPos] == '<' ||
		pFile[nPos] == '>' ||
		pFile[nPos] == '~' ||
		pFile[nPos] == '&' ||
		pFile[nPos] == '|' ||
		pFile[nPos] == '^' ||
		pFile[nPos] == '+' ||
		pFile[nPos] == '-' ||
		pFile[nPos] == '*' ||
		pFile[nPos] == '/' ||
		pFile[nPos] == '%'))
		nPos += 2;
	else if(pFile[nPos + 1] == pFile[nPos] && (
		pFile[nPos] == ':' ||
		pFile[nPos] == '+' ||
		pFile[nPos] == '-' ||
		pFile[nPos] == '&' ||
		pFile[nPos] == '|' ||
		pFile[nPos] == '>' ||
		pFile[nPos] == '<'))
	{
		if(pFile[nPos + 2] == '=' && (
			pFile[nPos] == '>' ||
			pFile[nPos] == '<'))
			nPos += 3;
		else
			nPos += 2;
	}
	else
		nPos++;
*/
	*pnLength = nPos - *pnStart;
	m_nPos = nPos;
}


#endif // NEWPARSER






bool ClassicSyntax::AdvanceBytes(CSToken* pTok, int nBytes)
{
#ifdef NEWPARSER
	pTok->Advance(nBytes);
	return true;
#else // NEWPARSER
	while(nBytes > 0)
	{
		if(m_nPos >= m_nLength)
			return false;
		m_nPos++;
		nBytes--;
	}
	return true;
#endif // NEWPARSER
}


bool ClassicSyntax::Advance()
{
#ifdef NEWPARSER
	if(m_nActiveTokens == 0)
		GetNextToken();
	m_nTokenPos++;
	if(m_nTokenPos >= MAX_LOOK_AHEAD)
		m_nTokenPos = 0;
	m_nActiveTokens--;
	return true;
#else // NEWPARSER
	return EatToken(m_pTokenBuffer);
#endif // NEWPARSER
}


GXMLTag* ClassicSyntax::LoadAndParseToXML(const char* szFilename, ClassicSyntaxError* pErrorHolder)
{
	FILE* pFile = fopen(szFilename, "r");
	if(!pFile)
	{
		pErrorHolder->SetError(&Error::ERROR_OPENING_FILE_TO_READ, 0, 0);
		pErrorHolder->SetFilename(szFilename);
		return NULL;
	}
	unsigned int nFileSize = filelength(fileno(pFile));
	char* pBuff = new char[nFileSize];
	fread(pBuff, nFileSize, 1, pFile);
	GXMLTag* pTag = ConvertToXML(szFilename, pBuff, nFileSize, 1, pErrorHolder);
	if(!pTag)
		pErrorHolder->SetFilename(szFilename);
	return pTag;
}

GXMLTag* ClassicSyntax::ConvertToXML(const char* szFilename, const char* pFile, int nLength, int nStartLine, ClassicSyntaxError* pErrorHolder)
{
	ClassicSyntax parser(szFilename, pFile, nLength, nStartLine, pErrorHolder);
	Holder<GXMLTag*> hTag(parser.ParseFile());
	if(pErrorHolder->HaveError())
		return NULL;
	return hTag.Drop();
}

inline char toUpper(char c)
{
	return c & (~32);
}

inline bool frontcmp(const char* szA, const char* szB)
{
	while(*szA != '\0' && *szB != '\0')
	{
		if(toUpper(*szA) != toUpper(*szB))
			return false;
		szA++;
		szB++;
	}
	if(*szB == '\0')
		return true;
	else
		return false;
}

inline bool IsWhitespace(char c)
{
	return((c <= ' ') ? true : false);
}

void ClassicSyntax::EatWhitespace()
{
	int nStartLineNumber = m_nLineNumber;
	while(m_nPos < m_nLength && IsWhitespace(m_pFile[m_nPos]))
	{
		if(m_pFile[m_nPos] == '\n')
		{
			m_nLineNumber++;
			m_nLineStartPos = m_nPos + 1;
		}
		m_nPos++;
	}
	if(m_nLineNumber - 1 > nStartLineNumber)
		m_bFoundBlankLine = true;
}

const char* g_stdDel = "\n,(){}";

inline bool IsDelimiter(char c, const char* szDelimiters)
{
	while(*szDelimiters != '\0')
	{
		if(c == *szDelimiters)
			return true;
		szDelimiters++;
	}
	return false;
}

#ifndef NEWPARSER
bool ClassicSyntax::PeekToken(const char* szDelimiters/*=NULL*/, bool bParensOK/*=false*/)
{
	if(!szDelimiters)
		szDelimiters = g_stdDel;
	EatWhitespace();
	if(m_nPos >= m_nLength)
		return false;
	int n;
	int nParenDepth = 0;
	bool bFirstDel;
	bool bIsDel;
	for(n = 0; true; n++)
	{
		if(m_nPos + n >= m_nLength)
			break;
		bIsDel = IsDelimiter(m_pFile[m_nPos + n], szDelimiters);
		if(bIsDel && bParensOK)
		{
			if(m_pFile[m_nPos + n] == '(')
			{
				nParenDepth++;
				bIsDel = false;
			}
			else if(m_pFile[m_nPos + n] == ')' && nParenDepth > 0)
			{
				nParenDepth--;
				bIsDel = false;
			}
		}
		if(n == 0)
			bFirstDel = bIsDel;
		else if(bIsDel != bFirstDel)
			break;
		if(n >= TOKEN_BUFFER_SIZE - 1)
			break;
		m_pTokenBuffer[n] = m_pFile[m_nPos + n];
	}
	m_pTokenBuffer[n] = '\0';
	while(n > 0 && IsWhitespace(m_pTokenBuffer[n - 1]))
		m_pTokenBuffer[--n] = '\0';
	if(n < 1)
		return false;
	else
		return true;
}
#endif // NEWPARSER


// todo: remove this whole method"
bool ClassicSyntax::EatToken(const char* szToken)
{
#ifdef NEWPARSER
	CSToken* pTok = GetToken(0);
	while(pTok->StartsWith(" "))
		pTok->Advance(1);
	if(!pTok->StartsWith(szToken))
		return false;
	int nLen = strlen(szToken);
	while(nLen > 0 && nLen >= pTok->GetLength())
	{
		szToken += pTok->GetLength();
		nLen -= pTok->GetLength();
		Advance();
		pTok = GetToken(0);
		if(nLen > 0 && pTok->GetLength() <= 0)
			return false;
	}
	if(nLen > 0)
	{
		pTok->Advance(nLen);
	}
	return true;
#else // NEWPARSER
	const char* pTok = szToken;
	EatWhitespace();
	while(*pTok != '\0')
	{
		if(m_nPos >= m_nLength)
			return false;
		if(toUpper(*pTok) != toUpper(m_pFile[m_nPos]))
			return false;
		m_nPos++;
		pTok++;
	}
#ifdef _DEBUG
	int n;
	for(n = TOKEN_MEMORY_SIZE - 1; n > 0; n--)
		strcpy(m_pRecentlyEatenTokens[n], m_pRecentlyEatenTokens[n - 1]);
	strcpy(m_pRecentlyEatenTokens[0], szToken);
#endif // _DEBUG
	return true;
#endif // NEWPARSER
}

bool ClassicSyntax::IsDone()
{
	return(m_nPos >= m_nLength ? true : m_pErrorHolder->HaveError());
}

void ClassicSyntax::SetError(ErrorStruct* pError)
{
	m_pErrorHolder->SetError(pError, m_nLineNumber, m_nPos - m_nLineStartPos + 1);
	m_pErrorHolder->SetFilename(m_szFilename);
}

GXMLTag* ClassicSyntax::ParseFile()
{
	m_pCurrentFileTag = new GXMLTag(TAG_NAME_FILE);
	m_pCurrentFileTag->SetLineNumber(m_nLineNumber);
	m_pCurrentFileTag->SetColumnAndWidth(m_nPos - m_nLineStartPos + 1, 5);
	while(true)
	{
		EatWhitespace();
		if(IsDone())
			break;
		CSToken* pTok = GetToken(0);
		if(pTok->GetLength() <= 0)
		{
			SetError(&Error::UNEXPECTED_EOF);
			break;
		}
		if(pTok->StartsWith("interface"))
			ParseInterface(false);
		else if(pTok->StartsWith("machine"))
			ParseInterface(true);
		else
			COClass::FromClassicSyntax(this);
		if(IsDone())
			break;
	}
	Holder<GXMLTag*> hFileTag(m_pCurrentFileTag);
	m_pCurrentFileTag = NULL;
	if(m_pErrorHolder->HaveError())
		return NULL;
	//GAssert(hFileTag->ToFile("c:\\mike\\debugme.xml"), "Failed to dump to file");
	return hFileTag.Drop();
}

void ClassicSyntax::ParseInterface(bool bMachine)
{
	const char* szTok = bMachine ? "machine" : "interface";
	if(!EatToken(szTok))
	{
		GAssert(false, "names inconsistent");
		SetError(&Error::EXPECTED_CLASS_INTERFACE_OR_MACHINE);
		return;
	}
	m_pCurrentInterfaceTag = new GXMLTag(bMachine ? TAG_NAME_MACHINE : TAG_NAME_INTERFACE);
	m_pCurrentInterfaceTag->SetLineNumber(m_nLineNumber);
	m_pCurrentInterfaceTag->SetColumnAndWidth(m_nPos - m_nLineStartPos + 1, 5);
	m_pCurrentFileTag->AddChildTag(m_pCurrentInterfaceTag);
	CSToken* pTok = GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	m_pCurrentInterfaceTag->AddAttribute(new GXMLAttribute(ATTR_NAME, pTok->GetValue(), pTok->GetLength()));
	bool b = Advance();
	GAssert(b, "unexpected state");
	if(!EatToken("{"))
	{
		SetError(&Error::EXPECTED_OPEN_SQUIGGLY_BRACE);
		return;
	}
	while(true)
	{
		pTok = GetToken(0);
		if(pTok->GetLength() <= 0)
		{
			SetError(&Error::EXPECTED_CLOSE_SQUIGGLY_BRACE);
			return;
		}
		if(pTok->StartsWith("}"))
			break;
		ParseMethodDecl();
		if(m_pErrorHolder->HaveError())
			return;
	}
	b = EatToken("}");
	GAssert(b, "unexpected state");
}

void ClassicSyntax::ParseInterfaceRef()
{
	GXMLTag* pInterfaceTag = new GXMLTag(TAG_NAME_INTERFACE);
	pInterfaceTag->SetLineNumber(m_nLineNumber);
	int nStartCol = m_nPos - m_nLineStartPos + 1;
	m_pCurrentClassTag->AddChildTag(pInterfaceTag);
	bool b = EatToken("interface");
	GAssert(b, "unexpected state");
	CSToken* pTok = GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	pInterfaceTag->AddAttribute(new GXMLAttribute(ATTR_NAME, pTok->GetValue(), pTok->GetLength()));
	b = Advance();
	GAssert(b, "unexpected state");
	int nEndCol = m_nPos - m_nLineStartPos + 1;
	pInterfaceTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);
}

void ClassicSyntax::ParseConstant(GXMLTag* pParentTag)
{
	GXMLTag* pConstTag = new GXMLTag(TAG_NAME_CONSTANT);
	pConstTag->SetLineNumber(m_nLineNumber);
	int nStartCol = m_nPos - m_nLineStartPos + 1;

	pParentTag->AddChildTag(pConstTag);
	bool b = EatToken("const");
	GAssert(b, "unexpected state");
	CSToken* pTok = GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	pConstTag->AddAttribute(new GXMLAttribute(ATTR_NAME, pTok->GetValue(), pTok->GetLength()));
	b = Advance();
	GAssert(b, "unexpected state");
	if(!EatToken("("))
	{
		SetError(&Error::EXPECTED_OPEN_PAREN);
		return;
	}
	pTok = GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	pConstTag->AddAttribute(new GXMLAttribute(ATTR_VAL, pTok->GetValue(), pTok->GetLength()));
	b = Advance();
	GAssert(b, "unexpected state");
	if(!EatToken(")"))
	{
		SetError(&Error::EXPECTED_CLOSE_PAREN);
		return;
	}
	int nEndCol = m_nPos - m_nLineStartPos + 1;
	pConstTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);
}

void ClassicSyntax::ParseMethodDecl()
{
	if(!EatToken("method"))
	{
		SetError(&Error::EXPECTED_METHOD_TOKEN);
		return;
	}
	GXMLTag* pMethodSignatureTag = new GXMLTag(TAG_NAME_METHOD);
	pMethodSignatureTag->SetLineNumber(m_nLineNumber);
	int nStartCol = m_nPos - m_nLineStartPos + 1;
	m_pCurrentInterfaceTag->AddChildTag(pMethodSignatureTag);
	CSToken* pTok = GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	pMethodSignatureTag->AddAttribute(new GXMLAttribute(ATTR_NAME, pTok->GetValue(), pTok->GetLength()));
	bool b = Advance();
	GAssert(b, "unexpected state");
	if(!EatToken("("))
	{
		SetError(&Error::EXPECTED_OPEN_PAREN);
		return;
	}
	bool bFirst = true;
	while(true)
	{
		CSToken* pTok = GetToken(0);
		if(pTok->GetLength() <= 0)
		{
			SetError(&Error::UNEXPECTED_EOF);
			return;
		}
		if(pTok->StartsWith(")"))
			break;
		if(bFirst)
			bFirst = false;
		else
		{
			if(!EatToken(","))
			{
				SetError(&Error::EXPECTED_COMMA_TOKEN);
				return;
			}
		}
		COVariable::FromClassicSyntax(this, pMethodSignatureTag);
		if(m_pErrorHolder->HaveError())
			return;
	}
	b = EatToken(")");
	GAssert(b, "unexpected state");
	int nEndCol = m_nPos - m_nLineStartPos + 1;
	pMethodSignatureTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);
}

void ClassicSyntax::ParseSingleInstr(GXMLTag* pParentTag)
{
	CSToken* pTok = GetToken(0);
	if(pTok->StartsWith("/"))
		COBlock::FromClassicSyntax(this, pParentTag);
	else if(pTok->StartsWith("if"))
		ParseCondition(pParentTag, IT_IF);
	else if(pTok->StartsWith("while"))
		ParseCondition(pParentTag, IT_WHILE);
	else
		ParseCall(pParentTag);
}

void ClassicSyntax::ParseElse(GXMLTag* pParentTag)
{
	bool b = EatToken("Else");
	GAssert(b, "unexpected state");
	GXMLTag* pElse = new GXMLTag(TAG_NAME_CALL);
	pElse->SetLineNumber(m_nLineNumber);
	pElse->SetColumnAndWidth(m_nPos - m_nLineStartPos + 1, 4);
	pParentTag->AddChildTag(pElse);
	//pElse->AddAttribute(new GXMLAttribute(ATTR_THIS, CLASS_NAME_ASM));
	pElse->AddAttribute(new GXMLAttribute(ATTR_NAME, METHOD_NAME_ELSE));
	COInstruction::FromClassicSyntax(this, pParentTag, IT_ELSE);
}

void ClassicSyntax::ParseCondition(GXMLTag* pParentTag, InstrType eType)
{
	GAssert(eType == IT_IF || eType == IT_WHILE, "not a condition type");
	GXMLTag* pTag = new GXMLTag(TAG_NAME_CALL);
	pTag->SetLineNumber(m_nLineNumber);
	int nStartCol = m_nPos - m_nLineStartPos + 1;
	pParentTag->AddChildTag(pTag);
	//pTag->AddAttribute(new GXMLAttribute(ATTR_THIS, CLASS_NAME_ASM));
	if(eType == IT_IF)
	{
		bool b = EatToken("If");
		GAssert(b, "unexpected state");
		pTag->AddAttribute(new GXMLAttribute(ATTR_NAME, METHOD_NAME_IF));
	}
	else
	{
		bool b = EatToken("While");
		GAssert(b, "unexpected state");
		pTag->AddAttribute(new GXMLAttribute(ATTR_NAME, METHOD_NAME_WHILE));
	}

	// Parse the parameters
	if(!EatToken("("))
	{
		SetError(&Error::EXPECTED_OPEN_PAREN);
		return;
	}
	COExpression::FromClassicSyntax(this, pTag);
	if(m_pErrorHolder->HaveError())
		return;
	if(!EatToken(")"))
	{
		SetError(&Error::EXPECTED_CLOSE_PAREN);
		return;
	}

	int nEndCol = m_nPos - m_nLineStartPos + 1;
	pTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);
	
	// Instructions	
	GXMLTag* pInstrTag = new GXMLTag(TAG_NAME_INSTRUCTIONS);
	pInstrTag->SetLineNumber(m_nLineNumber);
	pTag->AddChildTag(pInstrTag);
	COInstruction::FromClassicSyntax(this, pInstrTag, eType);
}

void ClassicSyntax::ParseCall(GXMLTag* pParentTag)
{
	GXMLTag* pCallTag = new GXMLTag(TAG_NAME_CALL);
	pCallTag->SetLineNumber(m_nLineNumber);
	int nStartCol = m_nPos - m_nLineStartPos + 1;
	pParentTag->AddChildTag(pCallTag);
	bool b;

	// Parse the catcher (if there is one)
	CSToken* pTok = GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	const char* szVal = pTok->GetValue();
	bool bFoundEquals = false;
	int n = 0;
	int nLen = pTok->GetLength();
	while(n < nLen && szVal[n] != '=')
		n++;
	if(n < nLen && szVal[n] == '=')
	{
		while(n > 0 && szVal[n - 1] <= ' ')
			n--;
		pCallTag->AddAttribute(new GXMLAttribute(ATTR_CATCH, szVal, n));
		b = AdvanceBytes(pTok, n);
		GAssert(b, "unexpected state");
		b = EatToken("=");
		GAssert(b, "unexpected state");
	}

	// Parse the object param ("This" var) if there is one
	pTok = GetToken(0);
	int nTokLen = pTok->GetLength();
	if(nTokLen <= 0)
	{
		SetError(&Error::UNEXPECTED_EOF);
		return;
	}
	const char* szTok = pTok->GetValue();
	bool bFoundPeriod = false;
	n = 0;
	while(n < nTokLen)
	{
		if(szTok[n] == '.')
			bFoundPeriod = true;
		n++;
	}
	if(bFoundPeriod)
	{
		n--;
		while(n > 0 && szTok[n] != '.')
			n--;
		if(n > 0 && szTok[n] == '.')
		{
			pCallTag->AddAttribute(new GXMLAttribute(ATTR_THIS, szTok, n));
			AdvanceBytes(pTok, n);
			if(pTok->GetLength() <= 0)
				Advance();
			b = EatToken(".");
			GAssert(b, "unexpected state");
		}
	}

	// Parse the method name
	pTok = GetToken(0);
	pCallTag->AddAttribute(new GXMLAttribute(ATTR_NAME, pTok->GetValue(), pTok->GetLength()));
	Advance();

	// Parse the parameters
	if(!EatToken("("))
	{
		SetError(&Error::EXPECTED_OPEN_PAREN);
		return;
	}
	COExpression::FromClassicSyntax(this, pCallTag);
	if(m_pErrorHolder->HaveError())
		return;
	if(!EatToken(")"))
	{
		SetError(&Error::EXPECTED_CLOSE_PAREN);
		return;
	}
	int nEndCol = m_nPos - m_nLineStartPos + 1;
	pCallTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);
}

