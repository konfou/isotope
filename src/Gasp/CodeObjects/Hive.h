/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __COHIVE_H__
#define __COHIVE_H__

#include "CodeObject.h"
#include "File.h"

class COFile;
class COClass;
class GPointerArray;

// A COHive is a collection of library (compiled app) files.
class COHive : public CodeObject
{
protected:
	GPointerArray* m_pFiles;
	//COProject* m_pCOProject;

	void LoadAllClassDefinitions(GXMLTag* pLibrary);

public:
	COHive(COProject* pCOProject);
	virtual ~COHive();

	bool LoadAllLibraries(const char* szStartPath, ParseError* pError, COProject* pProject);
	bool LoadAllLibraries2(const char* szStartPath, ParseError* pError, COProject* pProject);
	void AddFile(COFile* pFile);
	int GetFileCount();
	COFile* GetFile(int n);
	COClass* FindClass(const char* szClassName);
	COInterface* FindInterface(const char* szName);
};

#endif // __COHIVE_H__
