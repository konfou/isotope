/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COProject_H__
#define __COProject_H__

#include "../Engine/TagNames.h"
#include "../Engine/Error.h"

class COFile;
class COClass;
class COVariable;
class COFileSet;
class COMethod;
class ParseError;
class ClassicSyntaxError;
class ErrorHolder;
class COInterface;
class GXMLTag;
class GPointerArray;
class COType;
class ErrorHandler;
class COMachineClass;

class COProject
{
protected:
	int m_nMethodID;
	int m_nTypeID;
	COFileSet* m_pSource;
	char* m_szFilename;

	// Lazy-loaded redundant pointers
	COMachineClass* m_pString;
	COMachineClass* m_pFloat;

public:
	COFile* m_pBuiltIn;
	COClass* m_pObject;
	COClass* m_pInteger;
	COClass* m_pStackLayer;
	COClass* m_pInternalClass;
	COClass* m_pInternalMember;
	COClass* m_pInternalComparator;
	COClass* m_pInternalMethod;
	COVariable* m_pNull;

	// Built in methods
	COMethod* m_pObject_return;
	COMethod* m_pObject_if;
	COMethod* m_pObject_while;
	COMethod* m_pObject_else;
	COMethod* m_pObject_allocate;
	COMethod* m_pObject_set;
	COMethod* m_pObject_copy;
	COMethod* m_pObject_throw;
	COMethod* m_pInteger_new;
	COMethod* m_pInteger_increment;
	COMethod* m_pInteger_decrement;
	COMethod* m_pInteger_add;
	COMethod* m_pInteger_subtract;
	COMethod* m_pInteger_multiply;
	COMethod* m_pInteger_divide;
	COMethod* m_pInteger_modulus;
	COMethod* m_pInteger_and;
	COMethod* m_pInteger_or;
	COMethod* m_pInteger_xor;
	COMethod* m_pInteger_invert;
	COMethod* m_pInteger_shiftLeft;
	COMethod* m_pInteger_shiftRight;

	COFile* m_pCurrentFile;

public:
	COProject(const char* szFilename);
	virtual ~COProject();

	// Load a project file
	static COProject* LoadProject(const char* szLibrariesPath, const char* szProjectFile, ErrorHandler* pErrorHandler);

	bool IsSourceLoaded() { return (m_pSource ? true : false); }
	COClass* FindClass(const char* szName);
	COClass* FindClass(const char* pName, int nLen);
	COInterface* FindInterface(const char* szName);
	COMachineClass* FindMachineClass(const char* szName);
	int CountTypes();
	COType* GetType(int index);
	COType* FindType(int id);
	COType* FindType(const char* szName);
	COType* FindType(const char* pName, int nLen);
	COFileSet* GetSource();
	int GetUniqueMethodID() { return m_nMethodID++; }
	int GetUniqueTypeID() { return m_nTypeID++; }
	bool HasChanges();
	bool SaveChanges();
	COMachineClass* GetStringClass();
	COMachineClass* GetFloatClass();
	COMethod* GetNoop();
	bool LoadSources(const char** ppFiles, const char** pszFilenames, int nFileCount, ErrorHandler* pErrorHandler);
	bool LoadLibraries(const char* szLibrariesPath, ErrorHandler* pErrorHandler);

	// note: this may return NULL
	const char* GetFilename() { return m_szFilename; }

	void ThrowError(ErrorStruct* pError, GXMLTag* pTagWithError);
	void ThrowError(ErrorStruct* pError, GXMLTag* pTagWithError, const char* szParam1);

protected:
	bool LoadSourcesInProjectFile(const char* szFilename, ErrorHandler* pErrorHandler);
	void LoadSources(GXMLTag* pXMLFiles);
	void CreateBuiltInMethods();
};

#endif // __COProject_H__
