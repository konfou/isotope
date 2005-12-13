/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COFILE_H__
#define __COFILE_H__

#include "CodeObject.h"
#include "../../GClasses/GMacros.h"
#include "../Include/GaspEngine.h"

class COFileSet;
class COClass;
class COInterface;
class COProject;
class GQueue;
class GPointerArray;
class GCompilerBase;
class DebugSourceManager;
class COMachineClass;

// A COFile is either a source-code file, or a library file
class COFile : public CodeObject
{
friend class COFileSet;
friend class DebugSourceManager;
protected:
	GPointerArray* m_pClasses;
	GPointerArray* m_pInterfaces;
	GPointerArray* m_pMachineClasses;
	char* m_szFilename;
	bool m_bModified;
#ifdef _DEBUG
	int m_nPhase; // 0 = not loaded, 1 = class names loaded, 2 = members loaded, 3 = method decls loaded, 4 = instructions loaded
#endif // _DEBUG

	COFile(const char* szFilename);
public:
	virtual ~COFile();

	static GXMLTag* LoadAndConvertToXML(const char* szFilename, ErrorHandler* pErrorHandler, FILETYPES* peFileType);

	virtual FILETYPES GetFileType() = 0;
	virtual Library* GetLibrary() = 0;
	virtual const char* GetFilename() { return m_szFilename; }
	void GetFilenameNoPath(char* pBuffer);
	void SetFilename(const char* szName) { GAssert(szName, "Must have a valid name"); delete(m_szFilename); m_szFilename = new char[strlen(szName) + 1]; strcpy(m_szFilename, szName); }
	void AddType(COType* pNewType, COProject* pCOProject, GXMLTag* pFileTag);

	// todo: remove these three methods.  Use AddType instead
	void AddClass(COClass* pClass);
	void AddInterface(COInterface* pInterface);
	void AddMachineClass(COMachineClass* pMachine);

	int GetClassCount();
	COClass* GetClass(int n);

	int GetInterfaceCount();
	COInterface* GetInterface(int n);

	int GetMachineClassCount();
	COMachineClass* GetMachineClass(int n);

	void LoadTypeNames(GXMLTag* pFileTag, COProject* pCOProject, bool bXLib);
	void LoadMembers(GXMLTag* pTag, COProject* pCOProject, bool bPartial);
	void LoadMethodDeclarations(GXMLTag* pTag, COProject* pCOProject, bool bPartial);
	void LoadInstructions(GXMLTag* pTag, COProject* pCOProject, bool bPartial);
	GXMLTag* SaveToXML();
	void SaveToClassicSyntax(GQueue* pQ);
	bool Save();

	COType* FindType(int id);
	COClass* FindClass(const char* szClassName);
	int FindClass(COClass* pFindMe);
	COInterface* FindInterface(const char* szName);
	COMachineClass* FindMachineClass(const char* szName);

	bool GetModified() { return m_bModified; }
	void SetModified(bool bModified) { m_bModified = bModified; }

	bool Compile(GCompiler* pCompiler );

	int CountTypes();
	COType* GetType(int index);
	void ReplaceType(COType* pOld, COType* pNew);
	bool RemoveUnlinkedType(COType* pOldType);

protected:
	static COFile* LoadPartialForSymbolCreation(const char* szFilename, ErrorHandler* pErrorHandler, COProject* pProject);
};


class COCodeFile : public COFile
{
protected:

	COCodeFile(const char* szFilename) : COFile(szFilename)
	{
	}

public:
	virtual ~COCodeFile()
	{
	}

	virtual Library* GetLibrary() { return NULL; }

};


class COClassicFile : public COCodeFile
{
public:
	COClassicFile(const char* szFilename) : COCodeFile(szFilename)
	{
	}

	virtual ~COClassicFile()
	{
	}

	virtual FILETYPES GetFileType() { return FT_CLASSIC; }

};

class COXMLFile : public COCodeFile
{
public:
	COXMLFile(const char* szFilename) : COCodeFile(szFilename)
	{
	}

	virtual ~COXMLFile()
	{
	}

	virtual FILETYPES GetFileType() { return FT_XML; }

};

class COXLibFile : public COFile
{
protected:
	Library* m_pLibrary;

public:
	COXLibFile(const char* szFilename) : COFile(szFilename)
	{
		m_pLibrary = NULL;
	}

	virtual ~COXLibFile()
	{
		delete(m_pLibrary);
	}

	virtual FILETYPES GetFileType() { return FT_XLIB; }

	virtual Library* GetLibrary();
};


#endif // __COFILE_H__
