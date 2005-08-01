/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GCppParser.h"
#include <string.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include "GArray.h"
#include "GMacros.h"
#include "GFile.h"
#include "GHashTable.h"
#include "GQueue.h"

GCppObject::GCppObject()
{
	m_szFilename = NULL;
	m_nLineNumber = 0;
	m_pComment = NULL;
}

/*virtual*/ GCppObject::~GCppObject()
{
	delete(m_pComment);
}

// -------------------------------------------------------------------

GCppScope::GCppScope()
: GCppObject()
{
	m_pStringHeap = NULL;
	m_pTypes = new GConstStringHashTable(7, true);
	m_pVariables = new GConstStringHashTable(7, true);
	m_pMethods = new GConstStringHashTable(7, true);
	m_eCurrentAccess = PUBLIC;
}

#define DeleteHashTableContents(pht, typ) \
{\
	GHashTable htUnique(pht->GetCount() * 2);\
	void* pValue;\
	void* pTemp;\
	GHashTableEnumerator e(pht);\
	while(true)\
	{\
		const char* pKey = e.GetNextKey();\
		if(!pKey)\
			break;\
		if(!pht->Get(pKey, &pValue))\
			continue;\
		if(!htUnique.Get(pValue, &pTemp))\
		{\
			htUnique.Add(pValue, NULL);\
			delete((typ)pValue);\
		}\
	}\
}


GCppScope::~GCppScope()
{
	DeleteHashTableContents(m_pTypes, GCppType*);
	delete(m_pTypes);
	DeleteHashTableContents(m_pVariables, GCppVariable*);
	delete(m_pVariables);
	DeleteHashTableContents(m_pMethods, GCppMethod*);
	delete(m_pMethods);
	delete(m_pStringHeap);
}

// -------------------------------------------------------------------

GCppType::GCppType(const char* szName)
: GCppScope()
{
	m_szName = szName;
	m_bPrimitive = false;
	m_bDeclaredInProjectFile = false;
}

GCppType::~GCppType()
{
}

// -------------------------------------------------------------------

const char* g_pPrimitiveTypes[] =
{
	"void",
	"int",
	"bool",
	"short",
	"long",
	"float",
	"double",
	"char",
	"__int64",
	"PCONTEXT", // winnt.h
	"HGDIOBJ", // wingdi.h
	"HPALETTE", // wingdi.h
	"HDC", // wingdi.h
	"HBITMAP", // wingdi.h
	"HBRUSH", // wingdi.h
	"HFONT", // wingdi.h
	"HPEN", // wingdi.h
	"HMENU", // wingdi.h
	"HACCEL", // wingdi.h
};

#define PRIMITIVE_TYPE_COUNT (sizeof(g_pPrimitiveTypes) / sizeof(const char*))

// -------------------------------------------------------------------

GCppMethod::GCppMethod(GCppType* pReturnType, GCppScope* pScope, const char* szName, GCppMethodModifier eModifiers)
	: GCppDeclaration(pReturnType, szName)
{
	m_eModifiers = eModifiers;
	m_pParameters = new GPointerArray(4);
	m_pLocals = NULL;
	m_pCalls = NULL;
	m_pScope = pScope;
}

GCppMethod::~GCppMethod()
{
	int n;
	int nCount = m_pParameters->GetSize();
	for(n = 0; n < nCount; n++)
		delete((GCppVariable*)m_pParameters->GetPointer(n));
	delete(m_pParameters);

	if(m_pLocals)
	{
		DeleteHashTableContents(m_pLocals, GCppVariable*);
		delete(m_pLocals);
	}

	delete(m_pCalls);
}

void GCppMethod::AddParameter(GCppVariable* pVar)
{
	m_pParameters->AddPointer(pVar);
}

int GCppMethod::GetParameterCount()
{
	return m_pParameters->GetSize();
}

GCppVariable* GCppMethod::GetParameter(int n)
{
	return (GCppVariable*)m_pParameters->GetPointer(n);
}

int GCppMethod::GetCallCount()
{
	return m_pCalls->GetSize();
}

GCppMethod* GCppMethod::GetCall(int n)
{
	return (GCppMethod*)m_pCalls->GetPointer(n);
}

// -------------------------------------------------------------------

void GCppToken::Reset(GCppFile* pFile, int nStart, int nLength, int nLine, int nCol)
{
	if(pFile)
		pFile->AddRef();
	if(m_pFile)
		m_pFile->Release();
	m_pFile = pFile;
	m_nStart = nStart;
	m_nLength = nLength;
	m_nLine = nLine;
	m_nCol = nCol;
}

bool GCppToken::StartsWith(const char* szString)
{
	int n;
	for(n = 0; n < m_nLength && szString[n] != '\0'; n++)
	{
		if(m_pFile->m_pFile[m_nStart + n] != szString[n])
			return false;
	}
	return true;
}

bool GCppToken::Equals(const char* szString)
{
	int n;
	for(n = 0; n < m_nLength; n++)
	{
		if(m_pFile->m_pFile[m_nStart + n] != szString[n])
			return false;
	}
	if(szString[n] != '\0')
		return false;
	return true;
}

// -------------------------------------------------------------------

GCppFile::GCppFile(char* pFile, const char* szOldDir, bool bProjectFile, const char* szFilename)
{
	m_nFilePos = 0;
	m_nLine = 1;
	m_nLineStart = 0;
	m_pFile = pFile;
	m_nRefs = 0;
	m_bProjectFile = bProjectFile;
	m_szFilename = szFilename;
	m_pPrevDir = new char[strlen(szOldDir) + 1];
	strcpy(m_pPrevDir, szOldDir);
}

// -------------------------------------------------------------------

GCppParser::GCppParser()
{
	m_eMethodModifiers = 0;
	m_eVarModifiers = 0;
	m_nFileStackPos = 0;
	m_nTokenPos = 0;
	m_nActiveTokens = 0;
	m_nIfDefSkips = 0;
	m_nErrorCount = 0;
	m_nRecoveries = 0;
	m_pIncludePaths = new GPointerArray(8);
	m_pStringHeap = new GStringHeap(1024);
	m_pDefines = new GConstStringHashTable(31, true);
	m_pGlobalScope = new GCppScope();
	m_pScopeStack[0] = m_pGlobalScope;
	m_nScopeStackPointer = 1;
	m_bRetainComments = false;
	m_pComment = NULL;

	// Add primitive types to the global scope
	int n;
	for(n = 0; n < PRIMITIVE_TYPE_COUNT; n++)
	{
		GCppClass* pPrimitive = new GCppClass(g_pPrimitiveTypes[n]);
		pPrimitive->m_bPrimitive = true;
		m_pGlobalScope->m_pTypes->Add(g_pPrimitiveTypes[n], pPrimitive);
	}
}

GCppParser::~GCppParser()
{
	delete(m_pIncludePaths);
	delete(m_pDefines);
	delete(m_pStringHeap);
	delete(m_pGlobalScope);
	delete(m_pComment);
}

void GCppParser::AddIncludePath(const char* szPath)
{
	char* pPath = m_pStringHeap->Add(szPath);
	m_pIncludePaths->AddPointer(pPath);
}

void GCppParser::AddDefine(const char* szDefine, const char* szValue)
{
	GAssert(szDefine, "invalid parameter");
	char* pDefine = m_pStringHeap->Add(szDefine);
	char* pValue = NULL;
	if(szValue)
		pValue = m_pStringHeap->Add(szValue);
	m_pDefines->Add(pDefine, pValue);
}

char* LoadFileToBuffer(const char* szFilename)
{
	FILE* pFile = fopen(szFilename, "rb");
	if(!pFile)
		return NULL;
	int nFileSize = filelength(fileno(pFile));
	Holder<char*> hBuffer(new char[nFileSize + 1]);
	if(!hBuffer.Get())
	{
		fclose(pFile);
		return NULL;
	}
	int nBytesRead = fread(hBuffer.Get(), sizeof(char), nFileSize, pFile);
	int err = ferror(pFile);
	if(err != 0 || nBytesRead != nFileSize)
	{
		fclose(pFile);
		return NULL;
	}
	hBuffer.Get()[nFileSize] = '\0';
	fclose(pFile);
	return hBuffer.Drop();
}

GCppScope* GCppParser::ParseCppFile(const char* szFilename)
{
	// Parse the file
	if(!PushFile(szFilename, true))
		return NULL;
	while(true)
	{
		GCppToken* pTok = GetToken(0);
		if(pTok->GetLength() <= 0)
			break;
		ParseScope();
	}

	// Flush all the tokens to make sure they aren't holding on to any files
	int n;
	for(n = 0; n < MAX_LOOK_AHEAD; n++)
		Advance();

	// Return the global scope
	GCppScope* pScope = m_pGlobalScope;
	m_pGlobalScope = NULL;
	pScope->m_pStringHeap = m_pStringHeap;
	m_pStringHeap = NULL;
	return pScope;
}

bool GCppParser::OnError(GCppToken* pTok, const char* szMessage)
{
	m_nErrorCount++;
	//GAssert(!GetCurrentFile()->IsProjectFile(), "break");
	return false;
}

bool GCppParser::ParseScope()
{
	int n = m_nScopeStackPointer;
	while(m_nScopeStackPointer >= n)
	{
		GCppToken* pTok = GetToken(0);
		if(pTok->GetLength() <= 0)
			break;
		if(pTok->m_nLine == 284)
		{
			do {} while(false);
			//GAssert(false, "Break");
		}
		if(!ParseScopeItem())
		{
			// Attempt to recover by skipping to the next ';' or '}' or '#"
			m_nRecoveries++;
			GAssert(m_nRecoveries == m_nErrorCount, "Someone didn't call OnError when an error was found!");
			Advance();
			int nScopeNests = 0;
			while(true)
			{
				pTok = GetToken(0);
				if(pTok->Equals("{"))
					nScopeNests++;
				else if(pTok->Equals("}"))
					nScopeNests--;
				if(nScopeNests <= 0)
				{
					if(pTok->GetLength() <= 0 || pTok->StartsWith("#"))
						break;
					if(pTok->Equals(";") || pTok->Equals("}"))
					{
						Advance();
						break;
					}
				}
				Advance();
			}
		}
	}
	return true;
}

bool GCppParser::PushFile(const char* szFilename, bool bProjectFile)
{
	// Separate the file and directory
	char szDrive[32];
	char szDir[256];
	char szFile[256];
	char szExt[256];
	_splitpath(szFilename, szDrive, szDir, szFile, szExt);
	char szNewDir[256];
	char szNewFile[256];
	_makepath(szNewDir, szDrive, szDir, NULL, NULL);
	_makepath(szNewFile, NULL, NULL, szFile, szExt);

	// Record the old directory and change to the new one
	char szOldDir[256];
	getcwd(szOldDir, 256);
	chdir(szNewDir);

	// Load the file
	Holder<char*> hFile(LoadFileToBuffer(szNewFile));
	if(!hFile.Get())
	{
		chdir(szOldDir);
		OnError(GetToken(0), "file not found");
		return false;
	}

	const char* szFN = m_pStringHeap->Add(szFilename);
	PushBuffer(hFile.Drop(), szOldDir, bProjectFile, szFN);

	return true;
}

// This takes ownership of szBuf
void GCppParser::PushBuffer(char* szBuf, const char* szOldDir, bool bProjectFile, const char* szFilename)
{
	GAssert(m_nFileStackPos < MAX_INCLUDE_NESTING - 10, "deep #include nesting");
	if(m_nFileStackPos >= MAX_INCLUDE_NESTING)
	{
		GAssert(false, "#include nesting too deep");
		return;
	}

	// Push the new file on the stack
	m_pFileStack[m_nFileStackPos] = new GCppFile(szBuf, szOldDir, bProjectFile, szFilename);
	m_pFileStack[m_nFileStackPos]->AddRef();
	m_nFileStackPos++;
}

bool GCppParser::PopFile()
{
	GAssert(m_nFileStackPos > 0, "Nothing to pop");
	m_nFileStackPos--;
	chdir(m_pFileStack[m_nFileStackPos]->GetOldDir());
	m_pFileStack[m_nFileStackPos]->Release();
	if(m_nFileStackPos <= 0)
		return false; // indicates that the file stack is now empty
	return true;
}

void GCppParser::PushScope(GCppScope* pNewScope)
{
	GAssert(m_nScopeStackPointer < MAX_SCOPE_NESTING, "nesting that deep not supported");
	m_pScopeStack[m_nScopeStackPointer] = pNewScope;
	m_nScopeStackPointer++;
	m_eMethodModifiers = 0;
	m_eVarModifiers = 0;
}

bool GCppParser::PopScope()
{
	if(m_nScopeStackPointer <= 0)
		return OnError(GetToken(0), "\"}\" without matching \"{\"");
	m_nScopeStackPointer--;
	m_eMethodModifiers = 0;
	m_eVarModifiers = 0;
	return true;
}

GCppToken* GCppParser::GetToken(int n)
{
	GAssert(n < MAX_LOOK_AHEAD, "can't look that far ahead");
	while(n >= m_nActiveTokens)
		GetNextToken();
	int nPos = m_nTokenPos + n;
	if(nPos >= MAX_LOOK_AHEAD)
		nPos -= MAX_LOOK_AHEAD;
	return &m_tokens[nPos];
}

void GCppParser::Advance()
{
	if(m_nActiveTokens == 0)
		GetNextToken();
	m_nTokenPos++;
	if(m_nTokenPos >= MAX_LOOK_AHEAD)
		m_nTokenPos = 0;
	m_nActiveTokens--;
}

void GCppParser::GetNextToken()
{
	GAssert(m_nActiveTokens < MAX_LOOK_AHEAD, "got all the tokens we can hold");
	if(m_nFileStackPos <= 0)
	{
		int nNextToken = m_nTokenPos + m_nActiveTokens;
		if(nNextToken >= MAX_LOOK_AHEAD)
			nNextToken -= MAX_LOOK_AHEAD;
		m_tokens[nNextToken].Reset(NULL, 0, 0, 0, 0);
		m_nActiveTokens++;
		return;
	}	
	int nStart, nLength, nLine, nCol;
	nLength = 0;
	while(true)
	{
		FindNextToken(&nStart, &nLength, &nLine, &nCol);

		// Check for macros
		GCppFile* pFile = GetCurrentFile();
		GAssert(pFile, "FindNextToken shouldn't pop files");

		// If we hit the end of the file, see if there's still one on the stack
		if(nLength <= 0)
		{
			if(PopFile())
				continue;
			else
				pFile = NULL;
		}

		// Set the token value
		int nNextToken = m_nTokenPos + m_nActiveTokens;
		if(nNextToken >= MAX_LOOK_AHEAD)
			nNextToken -= MAX_LOOK_AHEAD;
		m_tokens[nNextToken].Reset(pFile, nStart, nLength, nLine, nCol);
		m_nActiveTokens++;

		// Do any necessary macro substitutions
		const char* szMacro;
		if(pFile && GetMacro(&pFile->m_pFile[nStart], nLength, &szMacro))
		{
			if(!ParseMacro(szMacro))
				m_nRecoveries++;
			continue;
		}

		// It's just a normal token, so we're done
		break;
	}
}

void GCppParser::OnNewLine(int nPos)
{
	GCppFile* pFile = GetCurrentFile();
	pFile->m_nLine++;
	pFile->m_nLineStart = nPos + 1;
}

void GCppParser::FindNextToken(int* pnStart, int* pnLength, int* pnLine, int* pnCol)
{
	// Find the start of the token
	GAssert(m_nFileStackPos > 0, "No files on stack");
	GCppFile* pCppFile = GetCurrentFile();
	const char* pFile = pCppFile->m_pFile;
	int nPos = pCppFile->m_nFilePos;

	// Skip whitespace and comments, and process compiler directives
	while(pFile[nPos] != '\0')
	{
		if(pFile[nPos] == '#')
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
		else if(pFile[nPos] <= ' ')
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
			if(m_bRetainComments)
			{
				if(!m_pComment)
					m_pComment = new GQueue(64);
				nPos += 2;
				while(pFile[nPos] != '\n' && pFile[nPos] != '\0')
				{
					m_pComment->Push(pFile[nPos]);
					nPos++;
				}
			}
			else
			{
				nPos += 2;
				while(pFile[nPos] != '\n' && pFile[nPos] != '\0')
					nPos++;
			}
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
	GAssert(pFile[nPos] != '#', "How'd that happen?");

	// Record where the token starts
	*pnStart = nPos;
	*pnLine = GetCurrentFile()->m_nLine;
	*pnCol = GetCurrentFile()->m_nLineStart - nPos;

	// Move past the token
	if(pFile[nPos] == '\0')
	{
	}
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
	else if(pFile[nPos] == '-' && pFile[nPos + 1] == '>')
		nPos += 2;
	else
		nPos++;

	*pnLength = nPos - *pnStart;
	GetCurrentFile()->m_nFilePos = nPos;
}

bool GCppParser::ParseScopeItem()
{
	GCppToken* pTok = GetToken(0);
	if(pTok->Equals(";"))
	{
		Advance();
		return true;
	}
	else if(pTok->Equals("extern"))
	{
		Advance();
		return true;
	}
	else if(pTok->Equals("struct"))
		return ParseStruct(false);
	else if(pTok->Equals("class"))
		return ParseStruct(true);
	else if(pTok->Equals("enum"))
		return ParseEnum();
	else if(pTok->Equals("union"))
		return ParseUnion();
	else if(pTok->Equals("typedef"))
		return ParseTypeDef();
	else if(pTok->Equals("}"))
	{
		Advance();
		return PopScope();
	}
	else if(pTok->Equals("friend"))
		return ParseFriend();
	else if(pTok->Equals("public"))
		return ParseSetAccess(PUBLIC);
	else if(pTok->Equals("protected"))
		return ParseSetAccess(PROTECTED);
	else if(pTok->Equals("private"))
		return ParseSetAccess(PRIVATE);
	else if(pTok->Equals("static"))
		return ParseMethodModifier(MM_STATIC);
	else if(pTok->Equals("virtual"))
		return ParseMethodModifier(MM_VIRTUAL);
	else if(pTok->Equals("inline"))
		return ParseMethodModifier(MM_INLINE);
/*	else if(pTok->Equals("_CRTIMP")) // hack for stdio.h
	{
		Advance();
		return true;
	}
	else if(pTok->Equals("NTSYSAPI")) // hack for winnt.h
	{
		Advance();
		return true;
	}*/
	return ParseDeclaration();
}

bool GCppParser::GetMacro(const char* pTok, int nLength, const char** pszValue)
{
	char* szTok = (char*)_alloca(nLength + 1);
	memcpy(szTok, pTok, nLength);
	szTok[nLength] = '\0';
	return m_pDefines->Get(szTok, (void**)pszValue);
}

bool startsWith(const char* pStart, int nLength, const char* szString)
{
	int n;
	for(n = 0; n < nLength && szString[n] != '\0'; n++)
	{
		if(pStart[n] != szString[n])
			return false;
	}
	return true;
}

bool GCppParser::ParseCompilerDirective(const char* pStart, int nLength)
{
	if(startsWith(pStart, nLength, "#else"))
		return ParseElse(pStart, nLength);
	else if(startsWith(pStart, nLength, "#endif"))
		return ParseEndIf(pStart, nLength);
	else if(startsWith(pStart, nLength, "#elif"))
		return ParseElIf(pStart, nLength);
	else if(startsWith(pStart, nLength, "#ifdef"))
		return ParseIfDef(pStart, nLength, true);
	else if(startsWith(pStart, nLength, "#ifndef"))
		return ParseIfDef(pStart, nLength, false);
	else if(startsWith(pStart, nLength, "#if"))
		return ParseIfDef(pStart, nLength, true);
	else if(m_nIfDefSkips > 0)
		return true;
	else if(startsWith(pStart, nLength, "#include"))
		return ParseInclude(pStart, nLength);
	else if(startsWith(pStart, nLength, "#define"))
		return ParseDefine(pStart, nLength);
	else if(startsWith(pStart, nLength, "#undef"))
		return ParseUnDef(pStart, nLength);
	else if(startsWith(pStart, nLength, "#pragma"))
		return true;
	else if(startsWith(pStart, nLength, "#error"))
		return true;
	return OnError(NULL, "unrecognized compiler directive");
}

bool GCppParser::ParseInclude(const char* pFile, int nEnd)
{
	// Parse out the filename
	int nPos = 8; // 8 = strlen("#include")
	while(pFile[nPos] <= ' ' && pFile[nPos] != '\0' && nPos < nEnd)
		nPos++;
	bool bUseIncludePaths = false;
	char cClose = '>';
	if(pFile[nPos] == '"')
		cClose = '"';
	else if(pFile[nPos] == '<')
		bUseIncludePaths = true;
	else
		return OnError(NULL, "Expected a '\"' or '<'");
	nPos++;
	int nStart = nPos;
	while(pFile[nPos] != cClose && pFile[nPos] != '\0' && nPos < nEnd)
		nPos++;
	if(pFile[nPos] != cClose)
		return OnError(NULL, "unterminated #include");
	int nLength = nPos - nStart;
	
	// Find the corresponding file
	char szFilename[260];
	if(bUseIncludePaths)
	{
		int nCount = m_pIncludePaths->GetSize();
		int n;
		for(n = 0; n < nCount; n++)
		{
			strcpy(szFilename, (const char*)m_pIncludePaths->GetPointer(n));
			int nLen = strlen(szFilename);
			if(szFilename[nLen - 1] == '/' || szFilename[nLen - 1] == '\\')
			{
			}
			else
			{
				szFilename[nLen] = '/';
				nLen++;
			}
			memcpy(szFilename + nLen, &pFile[nStart], nLength);
			szFilename[nLen + nLength] = '\0';
			if(GFile::DoesFileExist(szFilename))
				break;
		}
	}
	else
	{
		memcpy(szFilename, &pFile[nStart], nLength);
		szFilename[nLength] = '\0';
	}

	// Put the file on the stack
	bool bProjectFile = !bUseIncludePaths && GetCurrentFile()->IsProjectFile();
	return PushFile(szFilename, bProjectFile);
}

bool GCppParser::ParseDefine(const char* pFile, int nEnd)
{
	// Parse the statement (not including macro parameters)
	int nPos = 8; // 8 = strlen("#define ")
	while(pFile[nPos] <= ' ' && pFile[nPos] != '\0' && nPos < nEnd)
		nPos++;
	int nStart = nPos;
	while(pFile[nPos] > ' ' && pFile[nPos] != '\0' && pFile[nPos] != '(' && nPos < nEnd)
		nPos++;
	int nLength = nPos - nStart;
	if(nLength < 1)
		return OnError(NULL, "Expected a statement after #define");
//if(strnicmp(&pFile[nStart], "WINBASEAPI", 10) == 0) GAssert(false, "break");

	// Parse the macro parameters and value
	int nValueStart = nPos;
	while(pFile[nPos] != '\0' && nPos < nEnd &&
		!(pFile[nPos] == '/' && (pFile[nPos + 1] == '/' || pFile[nPos + 1] == '*'))
		)
		nPos++;
	int nValueLength = nPos - nValueStart;
	while(nValueLength > 0 &&
		(pFile[nValueStart + nValueLength - 1] == '\r' || pFile[nValueStart + nValueLength - 1] == '\n'))
		nValueLength--;

	// Add the define
	char* pStatement = m_pStringHeap->Add(&pFile[nStart], nLength);
	char* pValue = NULL;
	if(nValueLength > 0)
		pValue = m_pStringHeap->Add(&pFile[nValueStart], nValueLength);
	m_pDefines->Add(pStatement, pValue);
	
	return true;
}

bool GCppParser::ParseIfDef(const char* pFile, int nEnd, bool bPositive)
{
	if(m_nIfDefSkips > 0)
	{
		m_nIfDefSkips++;
		return true;
	}

	// Parse the statement
	int nPos = 6 + (bPositive ? 0 : 1); // 6 = strlen("#ifdef") + 1 for the 'n' in #ifndef
	nPos++; // for the space after it
	while(pFile[nPos] <= ' ' && pFile[nPos] != '\0' && nPos < nEnd)
		nPos++;
	int nStart = nPos;
	while(pFile[nPos] > ' ' && pFile[nPos] != '\0' && nPos < nEnd)
		nPos++;
	int nLength = nPos - nStart;
	if(nLength < 1)
		return OnError(NULL, "Missing statement");

	// See if it's defined
	char* szDefine = (char*)_alloca(nLength + 1);
	memcpy(szDefine, &pFile[nStart], nLength);
	szDefine[nLength] = '\0';
	char* szValue;
	bool bDefined = m_pDefines->Get(szDefine, (void**)&szValue);
	if(bPositive != bDefined)
		m_nIfDefSkips++;

	return true;
}

bool GCppParser::ParseElse(const char* pFile, int nEnd)
{
	if(m_nIfDefSkips == 0)
		m_nIfDefSkips++;
	else if(m_nIfDefSkips == 1)
		m_nIfDefSkips--;
	return true;
}

bool GCppParser::ParseEndIf(const char* pFile, int nEnd)
{
	if(m_nIfDefSkips > 0)
		m_nIfDefSkips--;
	return true;
}

bool GCppParser::ParseElIf(const char* pFile, int nEnd)
{
	// todo: fix this incorrect behavior
	if(m_nIfDefSkips == 0)
		m_nIfDefSkips++;
	return true;
}

bool GCppParser::ParseUnDef(const char* pFile, int nEnd)
{
	// Parse the statement
	int nPos = 7; // 7 = strlen("#undef ")
	while(pFile[nPos] <= ' ' && pFile[nPos] != '\0' && nPos < nEnd)
		nPos++;
	int nStart = nPos;
	while(pFile[nPos] > ' ' && pFile[nPos] != '\0' && nPos < nEnd)
		nPos++;
	int nLength = nPos - nStart;
	if(nLength < 1)
		return OnError(NULL, "Expected a statement after #undef");

	// Remove it from the defines
	char* pDefine = (char*)_alloca(nLength + 1);
	memcpy(pDefine, &pFile[nPos], nLength);
	pDefine[nLength] = '\0';
	m_pDefines->Remove(pDefine);

	return true;
}

GCppType* GCppParser::FindType(const char* pName, int nLength)
{
	char* szName = (char*)_alloca(nLength + 1);
	memcpy(szName, pName, nLength);
	szName[nLength] = '\0';
	int n;
	for(n = m_nScopeStackPointer - 1; n >= 0; n--)
	{
		GCppType* pType;
		if(m_pScopeStack[n]->m_pTypes->Get(szName, (void**)&pType))
			return pType;
	}
	return NULL;
}

GCppVariable* GCppParser::FindVariable(const char* szName, GCppScope* pScope)
{
	GCppVariable* pVar;
	while(pScope)
	{
		if(pScope->m_pVariables->Get(szName, (void**)&pVar))
			return pVar;
		if(!pScope->IsClass())
			break;
		GCppClass* pClass = (GCppClass*)pScope;
		pScope = pClass->GetParent();
	}
	return NULL;
}

GCppVariable* GCppParser::FindVariable(const char* pName, int nLength, GCppScope* pScope)
{
	char* szName = (char*)_alloca(nLength + 1);
	memcpy(szName, pName, nLength);
	szName[nLength] = '\0';
	return FindVariable(szName, pScope);
}

GCppVariable* GCppParser::FindVariable(const char* pName, int nLength, GCppMethod* pMethod)
{
	char* szName = (char*)_alloca(nLength + 1);
	memcpy(szName, pName, nLength);
	szName[nLength] = '\0';
	GCppVariable* pVar;
	if(pMethod->m_pLocals && pMethod->m_pLocals->Get(szName, (void**)&pVar))
		return pVar;
	int n;
	for(n = m_nScopeStackPointer - 1; n >= 0; n--)
	{
		GCppScope* pScope = m_pScopeStack[n];
		pVar = FindVariable(szName, pScope);
		if(pVar)
			return pVar;
	}
	return NULL;
}

GCppMethod* GCppParser::FindMethod(const char* szName, GCppScope* pScope)
{
	GCppMethod* pMethod;
	while(pScope)
	{
		if(pScope->m_pMethods->Get(szName, (void**)&pMethod))
			return pMethod;
		if(!pScope->IsClass())
			break;
		GCppClass* pClass = (GCppClass*)pScope;
		pScope = pClass->GetParent();
	}
	return NULL;
}

GCppMethod* GCppParser::FindMethod(const char* pName, int nLength, GCppScope* pScope)
{
	char* szName = (char*)_alloca(nLength + 1);
	memcpy(szName, pName, nLength);
	szName[nLength] = '\0';
	return FindMethod(szName, pScope);
}

GCppMethod* GCppParser::FindMethod(const char* pName, int nLength)
{
	char* szName = (char*)_alloca(nLength + 1);
	memcpy(szName, pName, nLength);
	szName[nLength] = '\0';
	GCppMethod* pMethod;
	int n;
	for(n = m_nScopeStackPointer - 1; n >= 0; n--)
	{
		GCppScope* pScope = m_pScopeStack[n];
		pMethod = FindMethod(szName, pScope);
		if(pMethod)
			return pMethod;
	}
	return NULL;
}

bool GCppParser::ParseStruct(bool bClass)
{
	GCppType* pType = ParseStruct2(bClass, false);
	if(pType)
		return true;
	else
		return false;
}

bool GCppParser::ParseUnion()
{
	GCppType* pType = ParseUnion2(false);
	if(pType)
		return true;
	else
		return false;
}

GCppType* GCppParser::ParseUnion2(bool bTypeDef)
{
	return ParseStruct2(false, bTypeDef);
}

void GetRandomName(char* pBuf)
{
	int n;
	for(n = 0; n < 8; n++)
		pBuf[n] = (char)((rand() % 96) + 32);
}

GCppType* GCppParser::ParseStruct2(bool bClass, bool bTypeDef)
{
	// Find or make the struct
	Advance(); // move past the "class" or "struct"
	GCppToken* pTok = GetToken(0);
	char szTmp[8];
	const char* pName;
	int nNameLen;
	bool bNoname;
	if(pTok->Equals("{"))
	{
		// no name is specified, so make a random name
		GetRandomName(szTmp);
		pName = szTmp;
		nNameLen = 8;
		bNoname = true;
	}
	else
	{
		pName = pTok->GetValue();
		nNameLen = pTok->GetLength();
		Advance();
		pTok = GetToken(0);
		bNoname = false;
	}

	// See if the type already exists
	GCppType* pType = FindType(pName, nNameLen);
	GCppStruct* pStruct;
	if(pType)
	{
		// todo: make sure it's a struct or class as appropriate
		pStruct = (GCppStruct*)pType;
	}
	else
	{
		// Make a new struct or class
		char* pStructName = m_pStringHeap->Add(pName, nNameLen);
		if(bClass)
			pStruct = new GCppClass(pStructName);
		else
			pStruct = new GCppStruct(pStructName);

		// Add the type to the current scope
		GetCurrentScope()->m_pTypes->Add(pStructName, pStruct);

		// Set the comment
		if(m_pComment)
		{
			pStruct->m_pComment = m_pComment;
			m_pComment = NULL;
		}
	}

	// Parse inheritance and initializers
	if(bClass)
	{
		// Get the parent -- todo: support multiple inherritance
		if(pTok->Equals(":"))	
		{
			Advance();
			pTok = GetToken(0);
			if(pTok->Equals("public"))
			{
				Advance();
				pTok = GetToken(0);
			}
			GCppType* pParent = FindType(pTok->GetValue(), pTok->GetLength());
			if(!pParent)
			{
				//OnError(pTok, "Parent class not found");
			}
			// todo: special-case the "Object" class
			GCppClass* pClass = (GCppClass*)pStruct;
			pClass->SetParent((GCppClass*)pParent); // todo: make sure it's a class
			Advance();
			pTok = GetToken(0);
		}

		// Skip interfaces and member initializers
		while(!pTok->Equals(";") && !pTok->Equals("{"))
		{
			Advance();
			pTok = GetToken(0);
		}
	}

	// If it's just a type declaration, we're done
	if(pTok->Equals(";"))
		return pStruct;

	// if we're doing a typedef and it's not a '{', we're done
	if(bTypeDef)
	{
		if(!pTok->Equals("{"))
			return pStruct;
	}

	// Parse the scope
	if(pTok->Equals("{"))
	{
		Advance(); // Move past the "{"
		GCppFile* pFile = GetCurrentFile();
		if(pFile->IsProjectFile() && !bNoname)
			pStruct->SetDeclaredInProjectFile();
		pStruct->SetSource(pFile->GetFilename(), pFile->GetCurrentLine());
		PushScope(pStruct);
		if(!ParseScope())
			return NULL;
	}

	// See if it's also a declaration
	pTok = GetToken(0);
	if(!bTypeDef && !pTok->Equals(";"))
	{
		GCppDeclaration* pDecl = parseDecl2(pStruct, false, false);
		if(!pDecl)
			return NULL;
		if(pDecl->GetDeclType() == GCppDeclaration::VAR)
			GetCurrentScope()->m_pVariables->Add(pDecl->GetName(), pDecl);
		else if(pDecl->GetDeclType() == GCppDeclaration::METHOD)
			GetCurrentScope()->m_pMethods->Add(pDecl->GetName(), pDecl);
		else
		{
			delete(pDecl);
			OnError(GetToken(0), "unexpected declaration type");
			return NULL;
		}
	}

	return pStruct;
}

bool GCppParser::ParseEnum()
{
	GCppEnum* pEnum = ParseEnum(false);
	if(pEnum)
		return true;
	else
		return false;
}

GCppEnum* GCppParser::ParseEnum(bool bTypeDef)
{
	// Find or make the enum
	Advance(); // move past the "enum"
	GCppToken* pTok = GetToken(0);
	GCppType* pType = NULL;
	if(!pTok->Equals("{"))
		pType = FindType(pTok->GetValue(), pTok->GetLength());
	GCppEnum* pEnum;
	bool bNoname = false;
	if(pType)
	{
		// todo: make sure it's an enum
		pEnum = (GCppEnum*)pType;
		bNoname = !pEnum->IsDeclaredInProjectFile();
	}
	else
	{
		// Get a name for the enum
		char szTmp[8];
		const char* pName;
		int nNameLen;
		if(pTok->Equals("{"))
		{
			// no name is specified, so make a random name
			GetRandomName(szTmp);
			pName = szTmp;
			nNameLen = 8;
			bNoname = true;
		}
		else
		{
			// use the specified name
			pName = pTok->GetValue();
			nNameLen = pTok->GetLength();
			Advance();
			bNoname = false;
		}

		// make the enum
		char* pEnumName = m_pStringHeap->Add(pName, nNameLen);
		pEnum = new GCppEnum(pEnumName);

		// Add the type to the current scope
		GetCurrentScope()->m_pTypes->Add(pEnumName, pEnum);

		// Set the comment
		if(m_pComment)
		{
			pEnum->m_pComment = m_pComment;
			m_pComment = NULL;
		}
	}

	// If it's just a declaration, we're done
	pTok = GetToken(0);
	if(pTok->Equals(";"))
		return pEnum;

	// if we're doing a typedef and it's not a '{', we're done
	if(bTypeDef)
	{
		if(!pTok->Equals("{"))
			return pEnum;
	}

	// Just skip the scope--todo: don't skip it
	if(pTok->Equals("{"))
	{
		if(GetCurrentFile()->IsProjectFile() && !bNoname)
			pEnum->SetDeclaredInProjectFile();
		Advance(); // Move past the "{"
		while(true)
		{
			pTok = GetToken(0);
			if(pTok->GetLength() == 0)
				break;
			if(pTok->Equals("}"))
				break;
			GAssert(!pTok->Equals("{"), "unexpected '{' inside enum");
			Advance();
		}
		Advance(); // Move past the "}:
	}

	// See if it's also a declaration
	pTok = GetToken(0);
	if(!bTypeDef && !pTok->Equals(";"))
	{
		GCppDeclaration* pDecl = parseDecl2(pEnum, false, false);
		if(!pDecl)
			return NULL;
		if(pDecl->GetDeclType() == GCppDeclaration::VAR)
			GetCurrentScope()->m_pVariables->Add(pDecl->GetName(), pDecl);
		else if(pDecl->GetDeclType() == GCppDeclaration::METHOD)
			GetCurrentScope()->m_pMethods->Add(pDecl->GetName(), pDecl);
		else
		{
			delete(pDecl);
			OnError(GetToken(0), "unexpected declaration type");
			return NULL;
		}
	}

	return pEnum;
}

bool GCppParser::ParseFriend()
{
	Advance(); // Move past the "friend" token
	while(true)
	{
		GCppToken* pTok = GetToken(0);
		if(pTok->GetLength() < 1 || pTok->Equals(";"))
			break;
		Advance();
	}
	return true;
}

bool GCppParser::ParseSetAccess(GCppAccess eAccess)
{
	Advance();
	GCppToken* pTok = GetToken(0);
	if(!pTok->Equals(":"))
		return OnError(pTok, "Expected a ':'");
	Advance(); // Move past the ':'
	GetCurrentScope()->m_eCurrentAccess = eAccess;
	return true;
}

bool GCppParser::ParseMethodModifier(GCppMethodModifier eModifier)
{
	Advance();
	m_eMethodModifiers |= eModifier;
	return true;
}

bool GCppParser::ParseVarModifier(GCppVarModifier eModifier)
{
	Advance();
	if(eModifier == VM_POINTER && ((m_eVarModifiers & VM_POINTER) != 0))
	{
		m_eVarModifiers &= (~VM_POINTER);
		eModifier = VM_POINTERPOINTER;
	}
	m_eVarModifiers |= eModifier;
	return true;
}

bool GCppParser::ParseDeclaration()
{
	GCppDeclaration* pDecl = ParseDecl(false);
	if(!pDecl)
		return false;
	if(pDecl->GetDeclType() == GCppDeclaration::VAR)
	{
		GetCurrentScope()->m_pVariables->Add(pDecl->GetName(), pDecl);
		return true;
	}
	else if(pDecl->GetDeclType() == GCppDeclaration::METHOD)
	{
		GetCurrentScope()->m_pMethods->Add(pDecl->GetName(), pDecl);
		return true;
	}

	delete(pDecl);
	OnError(GetToken(0), "unexpected declaration type");
	return false;
}

GCppType* GCppParser::ParseTypeRef(bool bParam, bool bTypeDef)
{
	// Skip any type specifiers
	GCppToken* pTok = GetToken(0);
	if(pTok->Equals("class") ||
		pTok->Equals("struct") ||
		pTok->Equals("enum"))
		Advance();

	// Parse any prefix modifiers
	while(true)
	{
		pTok = GetToken(0);
		if(pTok->Equals("const"))
			ParseVarModifier(VM_CONST);
		else if(pTok->Equals("unsigned"))
			ParseVarModifier(VM_UNSIGNED);
		else
			break;
	}

	// Find the type
	pTok = GetToken(0);
	GCppType* pType = FindType(pTok->GetValue(), pTok->GetLength());
	if(!pType)
	{
		OnError(pTok, "undeclared identifier");
		return NULL;
	}
	Advance();

	if(!bTypeDef)
		ParseSuffixModifiers(bParam);

	return pType;
}

void GCppParser::ParseSuffixModifiers(bool bParam)
{
	// Parse any suffix modifiers
	while(true)
	{
		GCppToken* pTok = GetToken(0);
		if(bParam && pTok->Equals("&"))
			ParseVarModifier(VM_REFERENCE);
		else if(pTok->Equals("*"))
			ParseVarModifier(VM_POINTER);
		else if(pTok->Equals("__cdecl"))
			Advance();
//		else if(pTok->Equals("near"))
//			Advance();
//		else if(pTok->Equals("far"))
//			Advance();
//		else if(pTok->Equals("NEAR"))
//			Advance();
//		else if(pTok->Equals("FAR"))
//			Advance();
//		else if(pTok->Equals("POINTER_64"))
//			Advance();
//		else if(pTok->Equals("APIENTRY"))
//			Advance();
//		else if(pTok->Equals("NTAPI"))
//			Advance();
//		else if(pTok->Equals("WINAPI"))
//			Advance();
//		else if(pTok->Equals("RESTRICTED_POINTER"))
//			Advance();
//		else if(pTok->Equals("UNALIGNED"))
//			Advance();
		else
			break;
	}
}

GCppDeclaration* GCppParser::ParseDecl(bool bParam)
{
	// See if it's a destructor
	bool bDestructor = false;
	GCppToken* pTok = GetToken(0);
	if(pTok->Equals("~"))
	{
		bDestructor = true;
		Advance();
	}

	// Parse the type
	GCppType* pType = ParseTypeRef(bParam, false);
	if(!pType)
		return NULL;

	return parseDecl2(pType, bParam, bDestructor);
}

GCppDeclaration* GCppParser::parseDecl2(GCppType* pType, bool bParam, bool bDestructor)
{
	ParseSuffixModifiers(bParam);

	// Parse scope specifiers
	bool bScopeSpecified = false;
	if(GetToken(1)->Equals(":") && GetToken(2)->Equals(":"))
	{
		GCppToken* pTok = GetToken(0);
		GCppType* pType = FindType(pTok->GetValue(), pTok->GetLength());
		if(!pType)
		{
			OnError(pTok, "Unrecognized type");
			return NULL;
		}
		bScopeSpecified = true;
		PushScope(pType);
	}

	// Parse the declaration
	GCppDeclaration* pDecl = parseDecl3(pType, bParam, bDestructor);

	// Undo any specified scope
	if(bScopeSpecified)
	{
		if(!PopScope())
			return NULL;
	}
	return pDecl;
}

GCppDeclaration* GCppParser::parseDecl3(GCppType* pType, bool bParam, bool bDestructor)
{
	// See if it's a constructor
	bool bConstructor = false;
	const char* szName;
	GCppToken* pTok = GetToken(0);
	if(pTok->Equals("("))
	{
		if(pType != GetCurrentScope())
		{
			OnError(pTok, "Constructor name doesn't match type");
			return NULL;
		}
		if(bDestructor)
		{
			char* pTmp = (char*)_alloca(strlen(pType->m_szName) + 2);
			strcpy(pTmp, "~");
			strcat(pTmp, pType->m_szName);
			szName = m_pStringHeap->Add(pTmp);
			m_eMethodModifiers |= MM_DESTRUCTOR;
		}
		else
		{
			szName = pType->m_szName;
			bConstructor = true;
			m_eMethodModifiers |= MM_CONSTRUCTOR;
		}
	}
	else if(bParam && (pTok->Equals(")") || pTok->Equals(",")) || pTok->Equals(":"))
	{
		szName = "<unnamed>";
	}
	else
	{
		szName = m_pStringHeap->Add(pTok->GetValue(), pTok->GetLength());
		Advance();
	}
	pTok = GetToken(0);

	// See if it's an array declaration
	while(pTok->Equals("["))
	{
		m_eVarModifiers |= VM_ARRAY;
		Advance(); // move past the "["

		// Skip the count of elements
		while(true)
		{
			pTok = GetToken(0);
			if(pTok->GetLength() <= 0 || pTok->Equals("]"))
				break;
			Advance();
		}
		Advance(); // move past the "]"
		pTok = GetToken(0);
	}

	// Swallow any bit specifiers
	if(pTok->Equals(":"))
	{
		Advance(); // Move past the ':'
		Advance(); // Move past the bit count
		pTok = GetToken(0);
	}

	// if it's a variable declaration, make it now
	if(pTok->Equals(";") ||
		pTok->Equals("=") ||
		pTok->Equals(",") ||
		(bParam && pTok->Equals(")")))
	{
		// Skip default parameter values
		if(bParam && pTok->Equals("="))
		{
			Advance();
			while(true)
			{
				pTok = GetToken(0);
				if(pTok->GetLength() <= 0 || pTok->Equals(",") || pTok->Equals(")"))
					break;
				Advance();
			}
		}

		// Create the variable declaration
		unsigned int eVarModifiers = m_eVarModifiers;
		m_eVarModifiers = 0;
		GCppVariable* pNewVar = new GCppVariable(pType, szName, (GCppVarModifier)eVarModifiers);
		GCppFile* pFile = GetCurrentFile();
		if(pFile->IsProjectFile())
			pNewVar->SetDeclaredInProjectFile();
		pNewVar->SetSource(pFile->GetFilename(), pFile->GetCurrentLine());
		if(m_pComment)
		{
			pNewVar->m_pComment = m_pComment;
			m_pComment = NULL;
		}
		return pNewVar;
	}

	// Move past the "("
	if(bParam || !pTok->Equals("("))
	{
		OnError(pTok, "unexpected token in declaration");
		return NULL;
	}
	Advance(); // Move past the "(" token

	// Parse the parameters
	unsigned int eMethodModifiers = m_eMethodModifiers;
	Holder<GCppMethod*> hMethod(new GCppMethod(pType, GetCurrentScope(), szName, (GCppMethodModifier)eMethodModifiers));
	GCppFile* pFile = GetCurrentFile();
	if(m_pComment)
	{
		hMethod.Get()->m_pComment = m_pComment;
		m_pComment = NULL;
	}
	if(pFile->IsProjectFile())
		hMethod.Get()->SetDeclaredInProjectFile();
	hMethod.Get()->SetSource(pFile->GetFilename(), pFile->GetCurrentLine());
	m_eMethodModifiers = 0;
	while(true)
	{
		pTok = GetToken(0);
		if(pTok->Equals(")"))
			break;
		if(pTok->Equals(".") && GetToken(1)->Equals(".") && GetToken(2)->Equals("."))
		{
			Advance();
			Advance();
			Advance();
			hMethod.Get()->AddModifier(MM_VARARG);
		}
		else
		{
			GCppDeclaration* pVar = ParseDecl(true);
			if(!pVar)
				return NULL;
			if(pVar->GetDeclType() != GCppDeclaration::VAR)
			{
				OnError(GetToken(0), "Expected a variable declaration");
				return NULL;
			}
			hMethod.Get()->AddParameter((GCppVariable*)pVar);
		}
		pTok = GetToken(0);
		if(pTok->Equals(","))
			Advance();
		else if(!pTok->Equals(")"))
		{
			OnError(pTok, "Expected a ',' or ')'");
			return NULL;
		}
	}
	if(!pTok->Equals(")"))
	{
		OnError(pTok, "expected a ')'");
		return NULL;
	}
	Advance();
	pTok = GetToken(0);

	// Check for abstract "=0" suffix
	if(pTok->Equals("="))
	{
		Advance(); // skip the '='
		Advance(); // skip the '0'
		hMethod.Get()->AddModifier(MM_ABSTRACT);
		pTok = GetToken(0);
	}

	// Skip member initializers and base constructor calls
	if(bConstructor)
	{
		while(true)
		{
			pTok = GetToken(0);
			if(pTok->GetLength() <= 0 || pTok->Equals(";") || pTok->Equals("{"))
				break;
			Advance();
		}
	}

	// Skip the body
	if(pTok->Equals("{"))
	{
		if(!ParseMethodBody(hMethod.Get()))
			return NULL;
	}

	return hMethod.Drop();
}

GCppType* GCppParser::ParseFunctionPointer(const char** ppName)
{
	// Parse the return type
	GCppType* pRetType = ParseTypeRef(false, false);
	if(!pRetType)
		return NULL;

	GCppToken* pTok = GetToken(0);
	if(!pTok->Equals("("))
	{
		OnError(pTok, "This doesn't look like the definition of a function pointer");
		return NULL;
	}
	Advance(); // move past the "("

	// Skip any modifiers
	while(true)
	{
		pTok = GetToken(0);
		if(pTok->Equals("*"))
			break;
		else if(pTok->GetLength() < 0 || pTok->Equals(")") || pTok->Equals(";"))
		{
			OnError(pTok, "This doesn't look like the definition of a function pointer");
			return NULL;
		}
		Advance();
	}
	Advance(); // Move past the '*'

	// Parse the name
	pTok = GetToken(0);
	char szTmp[8];
	const char* pName;
	int nNameLen;
	if(pTok->Equals("("))
	{
		// no name is specified, so make a random name
		GetRandomName(szTmp);
		pName = szTmp;
		nNameLen = 8;
	}
	else
	{
		pName = pTok->GetValue();
		nNameLen = pTok->GetLength();
		Advance();
		pTok = GetToken(0);
	}
	*ppName = m_pStringHeap->Add(pName, nNameLen);

	// Move past the ")"
	if(!pTok->Equals(")"))
	{
		*ppName = NULL;
		OnError(pTok, "Expected a ')'");
		return NULL;
	}
	Advance(); // Move past the ")"

	// Eat the parameters
	pTok = GetToken(0);
	if(!pTok->Equals("("))
	{
		*ppName = NULL;
		OnError(pTok, "Expected a '('");
		return NULL;
	}
	Advance(); // Move past the "("
	int nParams = 1;
	while(nParams > 0)
	{
		pTok = GetToken(0);
		if(pTok->Equals("("))
			nParams++;
		else if(pTok->Equals(")"))
			nParams--;
		Advance();
	}

	// Make a random name for the type
	char szTmp2[8];
	GetRandomName(szTmp2);
	char* szTypeName = m_pStringHeap->Add(szTmp2, 8);

	// Make the type and add it to the current scope
	GCppType* pNewType = new GCppFuncPtr(szTypeName);
	GetCurrentScope()->m_pTypes->Add(szTypeName, pNewType);

	//if(GetCurrentFile()->IsProjectFile())
	//	pNewType->SetDeclaredInProjectFile();

	return pNewType;
}

bool GCppParser::ParseTypeDef()
{
	Advance(); // move past the "typedef" token

	// Parse the type
	const char* szTypeName = NULL;
	GCppToken* pTok = GetToken(0);
	GCppType* pType;
	if(pTok->Equals("struct"))
		pType = ParseStruct2(false, true);
	else if(pTok->Equals("class"))
		pType = ParseStruct2(true, true);
	else if(pTok->Equals("enum"))
		pType = ParseEnum(true);
	else if(pTok->Equals("union"))
		pType = ParseUnion2(true);
	else if(GetToken(1)->Equals("("))
		pType = ParseFunctionPointer(&szTypeName);
	else
		pType = ParseTypeRef(false, true);
	if(!pType)
		return false;

	// Add the type to the current scope
	pTok = GetToken(0);
	while(true)
	{
		// Ignore any modifiers
		ParseSuffixModifiers(false);
		// todo: consume the modifiers

		// Make the new type
		pTok = GetToken(0);
		if(szTypeName == NULL)
		{
			szTypeName = m_pStringHeap->Add(pTok->GetValue(), pTok->GetLength());
			Advance();
			pTok = GetToken(0);
		}
		GetCurrentScope()->m_pTypes->Add(szTypeName, pType);
		szTypeName = NULL;

		// hack for winnt.h
		if(pTok->Equals("["))
		{
			Advance();
			Advance();
			Advance();
			pTok = GetToken(0);
		}

		// See if there's another identifier for this type
		if(!pTok->Equals(","))
			break;
		Advance(); // Move past the ","
	}

	return true;
}

GCppMethod* GCppParser::GetVarReferencedMethod(GCppType* pType)
{
	while(true)
	{
		GCppToken* pTok = GetToken(0);
		if(pTok->Equals(".") || pTok->Equals("->"))
			Advance();
		else
			break;
		pTok = GetToken(0);
		GCppMethod* pTargetMethod = FindMethod(pTok->GetValue(), pTok->GetLength(), pType);
		if(pTargetMethod)
			return pTargetMethod;
		GCppVariable* pVar = FindVariable(pTok->GetValue(), pTok->GetLength(), pType);
		if(!pVar)
			break;
		pType = pVar->GetType();
		Advance(); // Move past the var name
	}
	return NULL;
}

bool GCppParser::ParseMethodBody(GCppMethod* pMethod)
{
	Advance(); // move past the "{" token
	int nLevel = 1;
	bool bLineStart = true;
	while(nLevel > 0)
	{
		GCppToken* pTok = GetToken(0);
		if(pTok->GetLength() == 0)
			return OnError(pTok, "Unexpected end of file");
		if(pTok->Equals("{"))
		{
			nLevel++;
			bLineStart = true;
		}
		else if(pTok->Equals("}"))
		{
			nLevel--;
			bLineStart = true;
		}
		else
		{
			// See if it's a local variable declaration
			if(bLineStart)
			{
				GCppType* pType = FindType(pTok->GetValue(), pTok->GetLength());
				if(pType)
				{
					Advance(); // Move past the type
					while(true)
					{
						ParseSuffixModifiers(false);
						pTok = GetToken(0);
						if(pTok->GetLength() <= 0 || pTok->m_pFile->m_pFile[pTok->m_nStart] < 'A' || pTok->m_pFile->m_pFile[pTok->m_nStart] > 'z')
							break;

						// Make the local variable
						char* szName = m_pStringHeap->Add(pTok->GetValue(), pTok->GetLength());
						GCppVariable* pNewVar = new GCppVariable(pType, szName, (GCppVarModifier)m_eVarModifiers);
						m_eVarModifiers = 0;
						GCppFile* pFile = GetCurrentFile();
						if(pFile->IsProjectFile())
							pNewVar->SetDeclaredInProjectFile();
						pNewVar->SetSource(pFile->GetFilename(), pFile->GetCurrentLine());
						if(!pMethod->m_pLocals)
							pMethod->m_pLocals = new GConstStringHashTable(7, true);
						pMethod->m_pLocals->Add(szName, pNewVar);

						// See if there's more variables in a comma-list
						Advance();
						pTok = GetToken(0);
						if(pTok->Equals(","))
							Advance();
						else
							break;
					}
					m_eVarModifiers = 0;
				}				
			}

			// Look for direct method reference
			GCppMethod* pTargetMethod = FindMethod(pTok->GetValue(), pTok->GetLength());
			
			// Look for variable-referenced method
			if(!pTargetMethod)
			{
				GCppVariable* pVar = FindVariable(pTok->GetValue(), pTok->GetLength(), pMethod);
				if(pVar)
				{
					Advance(); // move past the var name
					pTargetMethod = GetVarReferencedMethod(pVar->GetType());
				}
			}

			// Look for type-referenced method
			if(!pTargetMethod)
			{
				GCppType* pType = FindType(pTok->GetValue(), pTok->GetLength());
				if(pType)
				{
					Advance();
					pTok = GetToken(0);
					if(pTok->Equals(":"))
					{
						Advance();
						pTok = GetToken(0);
						if(pTok->Equals(":"))
						{
							Advance();
							pTok = GetToken(0);
							pTargetMethod = FindMethod(pTok->GetValue(), pTok->GetLength(), pType);
						}
					}
				}
			}

			// Consume the method call(s)
			if(pTargetMethod)
			{
				while(true)
				{
					// Add the call
					if(!pMethod->m_pCalls)
						pMethod->m_pCalls = new GPointerArray(16);
					pMethod->m_pCalls->AddPointer(pTargetMethod);

					// Check for method calls on the return type
					Advance(); // Move past the method name
					pTok = GetToken(0);
					if(pTok->Equals("("))
					{
						// Move past the parameters
						int nLevel = 1;
						while(nLevel > 0)
						{
							Advance();
							pTok = GetToken(0);
							if(pTok->GetLength() <= 0 ||
								pTok->Equals("{") ||
								pTok->Equals("}"))
								return OnError(pTok, "Expected a ')'");
							if(pTok->Equals("("))
								nLevel++;
							else if(pTok->Equals(")"))
								nLevel--;
						}
						Advance();
						pTok = GetToken(0);

						// Check for another call
						pTargetMethod = GetVarReferencedMethod(pTargetMethod->GetType());
						if(!pTargetMethod)
							break;
					}
				}
			}

			bLineStart = false;
			if(pTok->Equals(";"))
				bLineStart = true;
		}
		Advance();
	}
	return true;
}

bool MacroCompare(const char* szBody, const char* szParam, int* nSize)
{
	*nSize = 0;
	while(true)
	{
		if(*szParam <= ' ' || *szParam == ',' || *szParam == ')')
			break;
		if(*szBody != *szParam)
			return false;
		szBody++;
		szParam++;
		(*nSize)++;
	}
	if((*szBody >= 'A' && *szBody <= 'Z') ||
		(*szBody >= 'a' && *szBody <= 'z') ||
		(*szBody >= '0' && *szBody <= '9') ||
		*szBody == '_')
		return false;
	return true;
}

#define MAX_MACRO_PARAMS 25

bool GCppParser::ParseMacro(const char* szDef)
{
	// Parse the macro definition params
	int nDefPos = 0;
	int nParamCount = 0;
	const char* pParamDefs[MAX_MACRO_PARAMS];
	if(szDef && szDef[nDefPos] == '(')
	{
		bool bNewParam = true;
		while(true)
		{
			nDefPos++;
			if(szDef[nDefPos] == ')' || szDef[nDefPos] == '\0')
				break;
			if(bNewParam && szDef[nDefPos] > ' ')
			{
				pParamDefs[nParamCount++] = &szDef[nDefPos];
				bNewParam = false;
			}
			else if(szDef[nDefPos] == ',')
				bNewParam = true;			
			if(nParamCount >= MAX_MACRO_PARAMS)
				break;
		}
		if(szDef[nDefPos] == ')')
			nDefPos++;
	}

	// Parse the macro implementation params
	GAssert(m_nActiveTokens > 0, "expected a macro token");
	m_nActiveTokens--; // undo the macro name token
	if(!szDef)
		return true;
	
	const char* pParamImpls[MAX_MACRO_PARAMS];
	if(nParamCount > 0)
	{
		GetNextToken(); // Read the next token
		GCppToken* pTok = GetToken(m_nActiveTokens - 1);
		if(!pTok->Equals("("))
			return OnError(pTok, "Expected a '('");
		m_nActiveTokens--; // undo the "(" token
		int nImplParams = 0;
		GetNextToken();
		pTok = GetToken(m_nActiveTokens - 1);
		while(true)
		{
			if(pTok->Equals(")"))
				break;
			if(nImplParams >= MAX_MACRO_PARAMS)
				break;
			pParamImpls[nImplParams++] = pTok->GetValue();
			m_nActiveTokens--; // undo the token
			GetNextToken();
			pTok = GetToken(m_nActiveTokens - 1);
			if(pTok->Equals(")"))
				break;
			if(!pTok->Equals(","))
				return OnError(pTok, "Expected a ',' or ')'");
			m_nActiveTokens--; // undo the ',' token
			GetNextToken();
			pTok = GetToken(m_nActiveTokens - 1);
		}
		if(nImplParams < nParamCount)
			return OnError(pTok, "Not enough macro parameters");
		m_nActiveTokens--; // undo the ')' token
	}

	// Make an implementation of the macro (in two passes)
	char* szBuf = NULL;
	int nBufPos;
	int nPass;
	for(nPass = 0; nPass < 2; nPass++)
	{
		nBufPos = 0;
		const char* szDefBody = &szDef[nDefPos];
		bool bTokenStart = true;
		int nParamSize;
		while(*szDefBody != '\0')
		{
			if(*szDefBody == '#')
			{
				szDefBody++;
				continue;
			}

			// See if we've got a parameter
			const char* szParam = NULL;
			if(bTokenStart && *szDefBody >= 'A')
			{
				bTokenStart = false;
				int i;
				for(i = 0; i < nParamCount; i++)
				{
					if(MacroCompare(szDefBody, pParamDefs[i], &nParamSize))
					{
						szParam = pParamImpls[i];
						break;
					}
				}
			}

			// See if we're on a token delimeter
			if(*szDefBody < 'A' || *szDefBody > 'z' || (*szDefBody > 'Z' && *szDefBody < 'a' && *szDefBody != '_'))
				bTokenStart = true;

			// Update the buffer
			if(szParam)
			{
				// Do the parameter replacement
				szDefBody += nParamSize;
				while(true)
				{
					if(*szParam <= ' ' || *szParam == ',' || *szParam == ')')
						break;
					if(nPass > 0)
						szBuf[nBufPos] = *szParam;
					nBufPos++;
					szParam++;
				}
			}
			else
			{
				if(nPass > 0)
					szBuf[nBufPos] = *szDefBody;
				nBufPos++;
				szDefBody++;
			}
		}
		if(nPass == 0)
			szBuf = new char[nBufPos + 10];
		else
			szBuf[nBufPos] = '\0';
	}

	// Parse the buffer like a #include file
	char szOldDir[256];
	getcwd(szOldDir, 256);
	bool bProjectFile = GetCurrentFile()->IsProjectFile();
	PushBuffer(szBuf, szOldDir, bProjectFile, GetCurrentFile()->GetFilename());

	return true;
}
