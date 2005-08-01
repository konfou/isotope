/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "Project.h"
#include <direct.h>
#include "../../GClasses/GXML.h"
#include "../../GClasses/GDirList.h"
#include "../../GClasses/GArray.h"
#include <stdlib.h>

COHive::COHive(COProject* pCOProject) : CodeObject()
{
	m_pFiles = new GPointerArray(16);
	//m_pCOProject = pCOProject;
}

COHive::~COHive()
{
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
		delete(GetFile(n));
	delete(m_pFiles);
}

int COHive::GetFileCount()
{
	return m_pFiles->GetSize();
}

COFile* COHive::GetFile(int n)
{
	return (COFile*)m_pFiles->GetPointer(n);
}

void COHive::AddFile(COFile* pFile)
{
	GAssert(pFile->GetFileType() == FT_XLIB, "Not a library");
	m_pFiles->AddPointer(pFile);
}

bool COHive::LoadAllLibraries(const char* szLibrariesFolder, ParseError* pError, COProject* pProject)
{
	char szOldDir[512];
	_getcwd(szOldDir, 512);
	bool bRet = LoadAllLibraries2(szLibrariesFolder, pError, pProject);
	_chdir(szOldDir);
	return bRet;
}

bool COHive::LoadAllLibraries2(const char* szLibrariesFolder, ParseError* pError, COProject* pProject)
{
	if(_chdir(szLibrariesFolder) != 0)
	{
		pError->SetError(&Error::INVALID_LIBRARIES_FOLDER, NULL);
		return false;
	}
	GDirList dl(true, true, false, true);
	const char* szLibraryName;
	while(true)
	{
		szLibraryName = dl.GetNext();
		if(!szLibraryName)
			break;
		char szExt[256];
		_splitpath(szLibraryName, NULL, NULL, NULL, szExt);
		if(stricmp(szExt, EXT_XLIB) != 0)
			continue; // not a library
		const char* szErrorMessage;
		int nErrorOffset;
		int nErrorLine;
		int nErrorColumn;
		GXMLTag* pRoot = GXMLTag::FromFile(szLibraryName, &szErrorMessage, &nErrorOffset, &nErrorLine, &nErrorColumn);
		if(!pRoot)
		{
			pError->SetError(&Error::BAD_LIBRARY, NULL);
			return false;
		}
		COFile* pNewFile = new COFile(COFile::SYNTAX_LIBRARY, szLibraryName);
		m_pFiles->AddPointer(pNewFile);
		pNewFile->LoadAllClassNames(pRoot, pError, pProject);
		if(pError->HaveError())
		{
			pError->SetError(&Error::BAD_LIBRARY, NULL);
			return false;
		}
		pNewFile->LoadAllClassDefinitions(pRoot, pError, pProject);
		if(pError->HaveError())
		{
			pError->SetError(&Error::BAD_LIBRARY, NULL);
			return false;
		}
		pNewFile->LoadAllMethodDefinitions(pRoot, pError, pProject);
		if(pError->HaveError())
		{
			pError->SetError(&Error::BAD_LIBRARY, NULL);
			return false;
		}
	}
	return true;
}

COClass* COHive::FindClass(const char* szClassName)
{
	COClass* pClass;
	COFile* pFile;
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		pClass = pFile->FindClass(szClassName);
		if(pClass)
			return pClass;
	}
	return NULL;
}

COInterface* COHive::FindInterface(const char* szName)
{
	COInterface* pInterface;
	COFile* pFile;
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		pInterface = pFile->FindInterface(szName);
		if(pInterface)
			return pInterface;
	}
	return NULL;
}
