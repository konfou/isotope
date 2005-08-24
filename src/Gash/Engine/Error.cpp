/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "../../GClasses/GMacros.h"
#include "../../GClasses/GString.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GFile.h"
#include "../CodeObjects/CodeObject.h"
#include "Error.h"
#include "EType.h"
#include "EClass.h"
#include "TagNames.h"
#include "../BuiltIns/GashString.h"

// Macro trick to make global variables that point to each error object
#define ERROR_MACRO(a, b, c) ErrorStruct Error::a = {c, b};
#include "ErrorMessages.h"
#undef ERROR_MACRO

void GotAnError()
{
#ifdef _DEBUG
	// *** THIS IS A REALLY GOOD PLACE TO PUT A BREAKPOINT ***
	do {} while(false);
#endif
}

// -------------------------------------------------------------------

ErrorHolder::ErrorHolder()
{
	m_pError = &Error::ERROR_NONE;
	m_szParam1 = NULL;
}

ErrorHolder::~ErrorHolder()
{
	delete(m_szParam1);
}

bool ErrorHolder::HaveError()
{
	return ((m_pError == &Error::ERROR_NONE) ? false : true);
}

void ErrorHolder::CheckError()
{
	GAssert(m_pError != &Error::ERROR_NONE, "Error not set");
}

void ErrorHolder::SetParam1(const char* szParam)
{
	m_szParam1 = new char[strlen(szParam) + 1];
	strcpy(m_szParam1, szParam);
}

void ErrorHolder::AddMessage(GString* pString)
{
	if(m_szParam1)
	{
		GAssert(m_pError->nParams == 1, "Wrong number of parameters for this error message");
		ConvertAnsiToUnicode(m_szParam1, wszParam1);		
		int nWChars = wcslen(m_pError->message) + wcslen(wszParam1) + 2;
		wchar_t* pBuf = (wchar_t*)alloca(nWChars * sizeof(wchar_t));
		swprintf(pBuf, nWChars, m_pError->message, wszParam1);
		pString->Add(pBuf);
	}
	else
	{
		GAssert(m_pError->nParams == 0, "Not enough parameters for this error message");
		pString->Add(m_pError->message);
	}
}

// -------------------------------------------------------------------

void GlobalError::SetError(ErrorStruct* pError)
{
	GAssert(m_pError == &Error::ERROR_NONE, "An error has already been set");
	m_pError = pError;
	GotAnError();
}

void GlobalError::ToString(GString* pString)
{
	pString->Add(L"Global error: ");
	AddMessage(pString);
}

// -------------------------------------------------------------------

class AssertingErrorHandler : public ErrorHandler
{
public:
	virtual void OnError(ErrorHolder* pErrorHolder)
	{
		ConvertUnicodeToAnsi(pErrorHolder->m_pError->message, szMessage);
		GAssert(false, szMessage);
	}
};

char* LoadFileToBuffer(ErrorHandler* pErrorHandler, const char* szFilename);

void AddSourceLines(GString* pString, const char* szFilename, int nErrorLine, int nHeight, int nCol, int nWid)
{
	if(!GFile::DoesFileExist(szFilename))
		return;

	// Load the file
	AssertingErrorHandler eh;
	Holder<char*> hBuf(LoadFileToBuffer(&eh, szFilename));
	if(!hBuf.Get())
		return;
	char* szFile = hBuf.Get();
	int nLine = 1;

	// Calculate the starting/ending line
	int nStart = nErrorLine - nHeight;
	if(nStart < 1)
		nStart = 1;
	int nEnd = nErrorLine + nHeight;

	// Skip to the start line
	while(*szFile != '\0' && nLine < nStart)
	{
		if(*szFile == '\n')
			nLine++;
		szFile++;
	}

	// Dump until we get to the end line
	while(*szFile != '\0' && nLine < nEnd)
	{
		if(*szFile == '\n')
		{
			nLine++;
			if(nLine == nErrorLine || nLine == nErrorLine + 1)
			{
				// Print "~~~~~~~~" around the error
				pString->Add(L'\n');
#ifdef _DEBUG
				int nOldLen = pString->GetLength();
#endif // _DEBUG
				int i;
				for(i = 0; i < nCol; i++)
					pString->Add(L'~');
				for(i = 0; i < nWid; i++)
					pString->Add(L'~');
				GAssert(pString->GetLength() - nOldLen == nCol + nWid, "Something's wrong printing tildas");
			}
		}
		pString->Add((wchar_t)*szFile);
		szFile++;
	}
}

void ParseError::SetError(ErrorStruct* pError, GXMLTag* pTagWithError)
{
	GAssert(m_pTagWithError == NULL && m_pError == &Error::ERROR_NONE, "An error has already been set");
	m_pError = pError;
	m_pTagWithError = pTagWithError;
	GotAnError();
}

void ParseError::ToString(GString* pString)
{
	pString->Add(L"Parse error");
	if(GetFilename())
	{
		pString->Add(" in file: ");
		pString->Add(GetFilename());
	}
	int nCol = -1;
	int nWid = -1;
	if(m_pTagWithError)
	{
		pString->Add("\nat line: ");
		pString->Add(m_pTagWithError->GetLineNumber());
		m_pTagWithError->GetOffsetAndWidth(&nCol, &nWid);
		if(nWid > 80)
		{
			GAssert(nWid < 8192, "That's one big line");
			nWid = 80;
		}
		pString->Add(" column: ");
		pString->Add(nCol);
		pString->Add(" length: ");
		pString->Add(nWid);
		if(nWid == 0)
			nWid = 40;
	}
	pString->Add("\n");
	AddMessage(pString);
	pString->Add(L"\n");
	if(GetFilename() && m_pTagWithError)
		AddSourceLines(pString, GetFilename(), m_pTagWithError->GetLineNumber(), 15, nCol, nWid);
}

// -------------------------------------------------------------------

void CompileError::SetError(ErrorStruct* pError, CodeObject* pCodeObjectWithError)
{
	GAssert(m_pCodeObjectWithError == NULL && m_pError == &Error::ERROR_NONE, "An error has already been set");
	m_pError = pError;
	m_pCodeObjectWithError = pCodeObjectWithError;
	GotAnError();
}

void CompileError::ToString(GString* pString)
{
	pString->Add(L"Compile error");
	if(GetFilename())
	{
		pString->Add(" in file: ");
		pString->Add(GetFilename());
		pString->Add("\n");
	}
	else
		pString->Add(":\n");
	if(m_pCodeObjectWithError)
	{
		pString->Add("at line: ");
		pString->Add(m_pCodeObjectWithError->GetLineNumber());
		int nCol, nWid;
		m_pCodeObjectWithError->GetColumnAndWidth(&nCol, &nWid);
		pString->Add(" column: ");
		pString->Add(nCol);
		pString->Add(" length: ");
		pString->Add(nWid);
		pString->Add("\n");
		AddMessage(pString);
		pString->Add("\n");
		if(GetFilename())
			AddSourceLines(pString, GetFilename(), m_pCodeObjectWithError->GetLineNumber(), 15, nCol, nWid);
	}
}

// -------------------------------------------------------------------

void ClassicSyntaxError::SetError(ErrorStruct* pError, int nLineNumber, int nColumn)
{
	GAssert(m_nLineWithError == -1 && m_pError == &Error::ERROR_NONE, "An error has already been set");
	m_pError = pError;
	m_nLineWithError = nLineNumber;
	m_nColumn = nColumn;
	GotAnError();
}

void ClassicSyntaxError::ToString(GString* pString)
{
	pString->Add(L"Syntax error");
	if(GetFilename())
	{
		pString->Add(" in file: ");
		pString->Add(GetFilename());
	}
	pString->Add("\nat line: ");
	pString->Add(m_nLineWithError);
	pString->Add(" column: ");
	pString->Add(m_nColumn);
	pString->Add("\n");
	AddMessage(pString);
	pString->Add("\n\n");
	if(GetFilename())
		AddSourceLines(pString, GetFilename(), m_nLineWithError, 15, m_nColumn, 15);
}


// -------------------------------------------------------------------

void ExceptionError::SetError(VarHolder* pException, bool bExpected)
{
	GAssert(pException != NULL, "no exception?");
	GAssert(m_pException == NULL, "error already set");
	m_pException = pException;
	m_pError = &Error::UNHANDLED_EXCEPTION;
	if(!bExpected)
		GotAnError();
}

void ExceptionError::ToString(GString* pString)
{
	AddMessage(pString);
	pString->Add("\n");

	// Print all string members of the exception object
	GObject* pOb = m_pException->GetGObject();
	EType* pType = pOb->GetType();
	if(pType->GetTypeType() == EType::TT_CLASS)
	{
		EClass* pClass = (EClass*)pType;
		if(!pClass->IsIntegerType())
		{
			ObjectObject* pObj = (ObjectObject*)pOb;
			int nMembers = pClass->GetTotalMemberCount();
			int n;
			for(n = 0; n < nMembers; n++)
			{
				GObject* pFieldObj = pObj->arrFields[n];
				EType* pFieldType = pFieldObj->GetType();
				if(stricmp(pFieldType->GetName(), CLASS_NAME_STRING) == 0)
				{
					GashString* pStr = (GashString*)pFieldObj;
					pString->Add(&pStr->m_value);
					pString->Add(L"\n");
				}
			}
		}
	}
}

