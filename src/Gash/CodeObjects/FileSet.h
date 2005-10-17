/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COFileSet_H__
#define __COFileSet_H__

#include "../../GClasses/GMacros.h"

class COProject;
class COType;
class COClass;
class COFile;
class GPointerArray;
class COInterface;
class COMachineClass;
class GXMLTag;
class ParseError;
class ErrorHandler;
class ClassicSyntaxError;

// A COFileSet is a collection of source-code files.
class COFileSet
{
friend class COProject;
protected:
	GPointerArray* m_pFiles;
	bool m_bModified; // (this means the project file was modified, not the files listed in the COFileSet)
	char* m_szFilename;

public:
	COFileSet(const char* szFilename);
	virtual ~COFileSet();

	GXMLTag* SaveToXML();
	bool Save(); // saves just the Source file

	COClass* FindClass(const char* szClassName);
	int FindClass(COClass* pClass);
	COInterface* FindInterface(const char* szName);
	COMachineClass* FindMachineClass(const char* szName);
	COType* FindType(int id);

	int GetFileCount();
	COFile* GetFile(int n);
	COFile* FindFile(const char* szFilename);
	void AddFile(COFile* pFile);

	virtual const char* GetFilename() { return m_szFilename; }
	void SetFilename(const char* szName) { GAssert(szName, "Must have a valid name"); delete(m_szFilename); m_szFilename = new char[strlen(szName) + 1]; strcpy(m_szFilename, szName); }

	bool GetModified() { return m_bModified; }
	void SetModified(bool bModified) { m_bModified = bModified; }

	void LoadAllLibraries(const char* szLibrariesFolder, COProject* pProject);
	int CountTypes();
	COType* GetType(int index);

	void ReplaceType(COType* pOld, COType* pNew);

protected:
	void LoadAllLibraries2(const char* szLibrariesFolder, COProject* pProject);
	bool LoadAllFileNames(GXMLTag* pSourceTag, GXMLTag* pXMLTags, ErrorHandler* pErrorHandler, COProject* pCOProject);
	void MakeFilesFromRefs(const char** ppFiles, const char** pszFilenames, int nFileCount, GXMLTag* pXMLTags, ParseError* pError, ClassicSyntaxError* pClassicSyntaxError, COProject* pCOProject);
	void LoadAllTypeNames(GXMLTag* pXMLTags, COProject* pCOProject, bool bXLib);
	void LoadAllClassDefinitions(GXMLTag* pXMLTags, COProject* pCOProject);
	void LoadMethodDeclarations(GXMLTag* pXMLTags, COProject* pCOProject);
	void LoadAllInstructions(GXMLTag* pXMLTags, COProject* pCOProject);
};

#endif // __COFileSet_H__
