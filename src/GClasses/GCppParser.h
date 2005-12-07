/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GCPPPARSER_H__
#define __GCPPPARSER_H__

#include <stdio.h>

class GPointerArray;
class GPointerHashTable;
class GCppParser;
class GStringHeap;
class GConstStringHashTable;
class GQueue;


enum GCppAccess
{
	PUBLIC,
	PROTECTED,
	PRIVATE,
};

enum GCppMethodModifier
{
	MM_STATIC = 0x1,
	MM_VIRTUAL = 0x2,
	MM_CONST = 0x4,
	MM_ABSTRACT = 0x8,
	MM_CONSTRUCTOR = 0x10,
	MM_DESTRUCTOR = 0x20,
	MM_INLINE = 0x40,
	MM_VARARG = 0x80,
};

enum GCppVarModifier
{
	VM_CONST = 0x1,
	VM_UNSIGNED = 0x2,
	VM_REFERENCE = 0x4,
	VM_POINTER = 0x8,
	VM_POINTERPOINTER = 0x10,
	VM_ARRAY = 0x20,
};


// The base class of all CPP parsing objects
class GCppObject
{
protected:
	const char* m_szFilename;
	int m_nLineNumber;
	GQueue* m_pComment;

	GCppObject();
	virtual ~GCppObject();

public:
	const char* GetFilename() { return m_szFilename; }
	int GetLineNumber() { return m_nLineNumber; }
	
	void SetSource(const char* szFilename, int nLine)
	{
		m_szFilename = szFilename;
		m_nLineNumber = nLine;
	}
	GQueue* GetComment() { return m_pComment; }
};




// Represents a scope in CPP code
class GCppScope : public GCppObject
{
friend class GCppParser;
protected:
	GStringHeap* m_pStringHeap;
	GCppAccess m_eCurrentAccess;
	GConstStringHashTable* m_pTypes;
	GConstStringHashTable* m_pVariables;
	GConstStringHashTable* m_pMethods;

public:
	GCppScope();
	virtual ~GCppScope();

	GConstStringHashTable* GetTypes() { return m_pTypes; }
	GConstStringHashTable* GetVariables() { return m_pVariables; }
	GConstStringHashTable* GetMethods() { return m_pMethods; }
	virtual bool IsType() { return false; }
	virtual bool IsClass() { return false; }
};



// Represents a type in CPP code
class GCppType : public GCppScope
{
friend class GCppParser;
protected:
	const char* m_szName;
	bool m_bPrimitive;
	bool m_bDeclaredInProjectFile;

public:
	GCppType(const char* szName);
	virtual ~GCppType();

	const char* GetName() { return m_szName; }
	bool IsPrimitiveType() { return m_bPrimitive; }
	void SetDeclaredInProjectFile() { m_bDeclaredInProjectFile = true; }
	bool IsDeclaredInProjectFile() { return m_bDeclaredInProjectFile; }
	virtual bool IsType() { return true; }
};


// Represents a struct in CPP code
class GCppStruct : public GCppType
{
protected:

public:
	GCppStruct(const char* szName)
		: GCppType(szName)
	{
	}

	virtual ~GCppStruct()
	{
	}
};



// Represents a class in CPP code
class GCppClass : public GCppStruct
{
protected:
	GCppClass* m_pParent;

public:
	GCppClass(const char* szName)
		: GCppStruct(szName)
	{
		m_pParent = NULL;
		m_eCurrentAccess = PRIVATE;
	}

	virtual ~GCppClass()
	{
	}

	void SetParent(GCppClass* pParent)
	{
		m_pParent = pParent;
	}

	virtual bool IsClass() { return true; }
	GCppClass* GetParent() { return m_pParent; }
};



// Represents an enum in CPP code
class GCppEnum : public GCppType
{
protected:

public:
	GCppEnum(const char* szName)
		: GCppType(szName)
	{
	}

	virtual ~GCppEnum()
	{
	}
};


// Represents a function pointer in CPP code
class GCppFuncPtr : public GCppType
{
protected:

public:
	GCppFuncPtr(const char* szName)
		: GCppType(szName)
	{
	}

	virtual ~GCppFuncPtr()
	{
	}
};


// Represents a declaration in CPP code
class GCppDeclaration : public GCppObject
{
public:
	enum DeclType
	{
		VAR,
		METHOD,
	};

protected:
	GCppType* m_pType;
	const char* m_szName;
	GCppAccess m_eAccess;
	bool m_bDeclaredInProjectFile;

public:
	GCppDeclaration(GCppType* pType, const char* szName, GCppAccess eAccess)
		: GCppObject()
	{
		m_pType = pType;
		m_szName = szName;
		m_eAccess = eAccess;
		m_bDeclaredInProjectFile = false;
	}

	virtual ~GCppDeclaration()
	{
	}

	const char* GetName() { return m_szName; }
	virtual DeclType GetDeclType() = 0;
	GCppType* GetType() { return m_pType; }
	void SetDeclaredInProjectFile() { m_bDeclaredInProjectFile = true; }
	bool IsDeclaredInProjectFile() { return m_bDeclaredInProjectFile; }
	GCppAccess GetAccess() { return m_eAccess; }
};



// Represents a variable in CPP code
class GCppVariable : public GCppDeclaration
{
friend class GCppParser;
protected:
	GCppVarModifier m_eModifiers;

public:
	GCppVariable(GCppType* pType, const char* szName, GCppAccess eAccess, GCppVarModifier eModifiers)
		: GCppDeclaration(pType, szName, eAccess)
	{
		m_eModifiers = eModifiers;
	}

	virtual ~GCppVariable()
	{
	}

	virtual DeclType GetDeclType()
	{
		return VAR;
	}

	GCppVarModifier GetModifiers() { return m_eModifiers; }
};




// Represents a method in CPP code
class GCppMethod : public GCppDeclaration
{
friend class GCppParser;
protected:
	unsigned int m_eModifiers;
	GPointerArray* m_pParameters;
	GConstStringHashTable* m_pLocals;
	GPointerArray* m_pCalls;
	GCppScope* m_pScope;

public:
	GCppMethod(GCppType* pReturnType, GCppScope* pScope, const char* szName, GCppAccess eAccess, GCppMethodModifier eModifiers);
	virtual ~GCppMethod();

	virtual DeclType GetDeclType()
	{
		return METHOD;
	}

	void AddParameter(GCppVariable* pVar);

	void AddModifier(GCppMethodModifier eModifier)
	{
		m_eModifiers |= eModifier;
	}

	GCppMethodModifier GetModifiers() { return (GCppMethodModifier)m_eModifiers; }
	int GetParameterCount();
	GCppVariable* GetParameter(int n);
	int GetCallCount();
	GCppMethod* GetCall(int n);
};



// Represents a file in CPP code
class GCppFile
{
friend class GCppParser;
friend class GCppToken;
protected:
	char* m_pFile;
	char* m_pPrevDir;
	const char* m_szFilename;
	int m_nFilePos;
	int m_nLine;
	int m_nLineStart;
	int m_nRefs;
	bool m_bProjectFile;

public:
	GCppFile(char* pFile, const char* szPrevDir, bool bProjectFile, const char* szFilename);

	virtual ~GCppFile()
	{
		delete(m_pPrevDir);
		delete(m_pFile);
	}

	void AddRef()
	{
		m_nRefs++;
	}

	void Release()
	{
		if(--m_nRefs <= 0)
			delete(this);
	}

	bool IsProjectFile() { return m_bProjectFile; }
	const char* GetOldDir() { return m_pPrevDir; }
	const char* GetFilename() { return m_szFilename; }
	int GetCurrentLine() { return m_nLine; }
};



// Represents a token in CPP code
class GCppToken
{
friend class GCppParser;
protected:
	int m_nStart;
	int m_nLength;
	GCppFile* m_pFile;
	int m_nLine;
	int m_nCol;

	GCppToken()
	{
		m_pFile = NULL;
	}

public:
	~GCppToken()
	{
		Reset(NULL, 0, 0, 0, 0);
	}

	void Reset(GCppFile* pFile, int nStart, int nLength, int nLine, int nCol);
	bool Equals(const char* szString);
	bool StartsWith(const char* szString);

	const char* GetValue() { return &m_pFile->m_pFile[m_nStart]; }
	int GetLength() { return m_nLength; }
};




#define MAX_LOOK_AHEAD 10
#define MAX_INCLUDE_NESTING 100
#define MAX_SCOPE_NESTING 15

// This class parses a C or C++ code and header files
class GCppParser
{
protected:
	// ParseFile stack
	GCppFile* m_pFileStack[MAX_INCLUDE_NESTING]; // stack
	int m_nFileStackPos;

	// Tokens
	GCppToken m_tokens[MAX_LOOK_AHEAD]; // wrap-around buffer
	int m_nTokenPos;
	int m_nActiveTokens;

	// Scope
	GCppScope* m_pGlobalScope;
	GCppScope* m_pScopeStack[MAX_SCOPE_NESTING];
	int m_nScopeStackPointer;

	// Data
	GPointerArray* m_pIncludePaths;
	GConstStringHashTable* m_pDefines;
	GStringHeap* m_pStringHeap;
	int m_nIfDefSkips;
	unsigned int m_eMethodModifiers;
	unsigned int m_eVarModifiers;

	// Errors
	int m_nErrorCount;
	int m_nRecoveries;

	// Comments
	bool m_bRetainComments;
	GQueue* m_pComment;

public:
	GCppParser();
	virtual ~GCppParser();

	// add a path to search for include files in angle brackets
	void AddIncludePath(const char* szPath);

	// szValue can be NULL, but szDefine can't
	void AddDefine(const char* szDefine, const char* szValue);

	// Parse the CPP file
	GCppScope* ParseCppFile(const char* szFilename);

	// Set whether or not to store comments that precede each object with the object
	void SetRetainComments(bool b) { m_bRetainComments = b; }

protected:
	GCppFile* GetCurrentFile() { return m_pFileStack[m_nFileStackPos - 1]; }
	GCppScope* GetCurrentScope() { return m_pScopeStack[m_nScopeStackPointer - 1]; }
	GCppToken* GetToken(int n);
	void Advance();
	void OnNewLine(int nPos);
	void FindNextToken(int* pnStart, int* pnLength, int* pnLine, int* pnCol);
	void GetNextToken();
	bool PushFile(const char* szFilename, bool bProjectFile);
	void PushBuffer(char* szBuf, const char* szOldDir, bool bProjectFile, const char* szFilename);
	bool PopFile();
	void PushScope(GCppScope* pNewScope);
	bool PopScope();
	bool OnError(GCppToken* pTok, const char* szMessage);
	GCppType* FindType(const char* pName, int nLength);
	GCppVariable* FindVariable(const char* szName, GCppScope* pScope);
	GCppVariable* FindVariable(const char* pName, int nLength, GCppScope* pScope);
	GCppVariable* FindVariable(const char* pName, int nLength, GCppMethod* pMethod);
	GCppMethod* FindMethod(const char* szName, GCppScope* pScope);
	GCppMethod* FindMethod(const char* pName, int nLength, GCppScope* pScope);
	GCppMethod* FindMethod(const char* pName, int nLength);
	bool ParseScope();
	bool ParseScopeItem();
	bool GetMacro(const char* pTok, int nLength, const char** pszValue);
	bool ParseCompilerDirective(const char* pStart, int nLength);
	bool ParseInclude(const char* pFile, int nEnd);
	bool ParseDefine(const char* pFile, int nEnd);
	bool ParseIfDef(const char* pFile, int nEnd, bool bPositive);
	bool ParseElse(const char* pFile, int nEnd);
	bool ParseEndIf(const char* pFile, int nEnd);
	bool ParseElIf(const char* pFile, int nEnd);
	bool ParseUnDef(const char* pFile, int nEnd);
	bool ParseUnion();
	GCppType* ParseUnion2(bool bTypeDef);
	bool ParseStruct(bool bClass);
	GCppType* ParseStruct2(bool bClass, bool bTypeDef);
	bool ParseEnum();
	GCppEnum* ParseEnum(bool bTypeDef);
	bool ParseFriend();
	bool ParseSetAccess(GCppAccess eAccess);
	bool ParseMethodModifier(GCppMethodModifier eModifier);
	bool ParseVarModifier(GCppVarModifier eModifier);
	bool ParseDeclaration();
	GCppType* ParseTypeRef(bool bParam, bool bTypeDef);
	void ParseSuffixModifiers(bool bParam);
	GCppDeclaration* ParseDecl(bool bParam);
	GCppDeclaration* parseDecl2(GCppType* pType, bool bParam, bool bDestructor);
	GCppDeclaration* parseDecl3(GCppType* pType, bool bParam, bool bDestructor);
	GCppType* ParseFunctionPointer(const char** ppName);
	bool ParseTypeDef();
	GCppMethod* GetVarReferencedMethod(GCppType* pType);
	bool ParseMethodBody(GCppMethod* pMethod);
	bool ParseMacro(const char* szDef);
};

#endif // __GCPPPARSER_H__
