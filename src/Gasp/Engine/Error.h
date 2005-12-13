/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __ERROR_H__
#define __ERROR_H__

#include <string.h>
#include "../Include/GaspEngine.h"

class GXMLTag;
class CodeObject;

class Error
{
public:
// This macro-trick will define external global pointers to error objects
#define ERROR_MACRO(a, b, c) static ErrorStruct a;
#include "ErrorMessages.h"
#undef ERROR_MACRO
};

class GlobalError : public ErrorHolder
{
public:
	GlobalError() : ErrorHolder() {}
	virtual ~GlobalError() {}

	virtual ERROR_TYPE GetErrorType() { return ET_GLOBAL; }
	void SetError(ErrorStruct* pError);
	virtual void ToString(GString* pString);
};



class ParseError : public ErrorHolder
{
public:
	GXMLTag* m_pTagWithError;
	char* m_szFilename;

	ParseError() : ErrorHolder()
	{
		m_pTagWithError = NULL;
		m_szFilename = NULL;
	}

	virtual ~ParseError()
	{
		delete(m_szFilename);
	}

	virtual ERROR_TYPE GetErrorType() { return ET_PARSE; }
	void SetError(ErrorStruct* pError, GXMLTag* pTagWithError);
	void Reset()
	{
		m_pError = &Error::ERROR_NONE;
		m_pTagWithError = NULL;
	}
	const char* GetFilename() { return m_szFilename; }
	void SetFilename(const char* szFilename)
	{
		delete(m_szFilename);
		m_szFilename = new char[strlen(szFilename) + 1];
		strcpy(m_szFilename, szFilename);
	}
	virtual void ToString(GString* pString);
};



class ClassicSyntaxError: public ErrorHolder
{
public:
	int m_nLineWithError;
	int m_nColumn;
	char* m_szFilename;

	ClassicSyntaxError() : ErrorHolder()
	{
		m_nLineWithError = -1;
		m_szFilename = NULL;
	}

	virtual ~ClassicSyntaxError()
	{
		delete(m_szFilename);
	}

	virtual ERROR_TYPE GetErrorType() { return ET_CLASSICSYNTAX; }
	void SetError(ErrorStruct* pError, int nLineNumber, int nColumn);

	void Reset()
	{
		m_pError = &Error::ERROR_NONE;
		m_nLineWithError = -1;
	}
	
	const char* GetFilename() { return m_szFilename; }
	
	void SetFilename(const char* szFilename)
	{
		delete(m_szFilename);
		m_szFilename = new char[strlen(szFilename) + 1];
		strcpy(m_szFilename, szFilename);
	}
	virtual void ToString(GString* pString);
};



class CompileError : public ErrorHolder
{
public:
	CodeObject* m_pCodeObjectWithError;
	char* m_szFilename;

	CompileError() : ErrorHolder()
	{
		m_pCodeObjectWithError = NULL;
		m_szFilename = NULL;
	}

	virtual ~CompileError()
	{
		delete(m_szFilename);
	}

	virtual ERROR_TYPE GetErrorType() { return ET_COMPILE; }
	void SetError(ErrorStruct* pError, CodeObject* pCodeObjectWithError);
	void Reset()
	{
		m_pError = &Error::ERROR_NONE;
		m_pCodeObjectWithError = NULL;
	}
	virtual void ToString(GString* pString);

	const char* GetFilename() { return m_szFilename; }

	void SetFilename(const char* szFilename)
	{
		delete(m_szFilename);
		m_szFilename = new char[strlen(szFilename) + 1];
		strcpy(m_szFilename, szFilename);
	}
};



class ExceptionError : public ErrorHolder
{
public:
	VarHolder* m_pException;

	ExceptionError() : ErrorHolder()
	{
		m_pException = NULL;
	}

	virtual ~ExceptionError() {}

	virtual ERROR_TYPE GetErrorType() { return ET_EXCEPTION; }
	void SetError(VarHolder* pException, bool bExpected);
	virtual void ToString(GString* pString);
};


void GotAnError();

#endif // __ERROR_H__
