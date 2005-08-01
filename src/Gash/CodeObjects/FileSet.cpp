/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "FileSet.h"
#include "../Engine/TagNames.h"
#include "Class.h"
#include "../Engine/ClassicSyntax.h"
#include "../Engine/Error.h"
#include "../../GClasses/GXML.h"
#include "Project.h"
#include "../../GClasses/GDirList.h"
#include "File.h"
#ifdef WIN32
#include <direct.h>
#else // WIN32
#include <unistd.h>
#endif // WIN32
COFileSet::COFileSet(const char* szFilename)
{
	m_szFilename = NULL;
	SetFilename(szFilename);
	m_bModified = false;
	m_pFiles = new GPointerArray(16);
}

COFileSet::~COFileSet()
{
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
		delete(GetFile(n));
	delete(m_pFiles);
}

int COFileSet::GetFileCount()
{
	return m_pFiles->GetSize();
}

COFile* COFileSet::GetFile(int n)
{
	return (COFile*)m_pFiles->GetPointer(n);
}

COFile* COFileSet::FindFile(const char* szFilename)
{
	int nCount = GetFileCount();
	COFile* pFile;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		if(stricmp(pFile->GetFilename(), szFilename) == 0)
			return pFile;
	}
	return NULL;
}

void COFileSet::AddFile(COFile* pFile)
{
	m_pFiles->AddPointer(pFile);
}

void COFileSet::MakeFilesFromRefs(const char** ppFiles, const char** pszFilenames, int nFileCount, GXMLTag* pXMLTags, ParseError* pError, ClassicSyntaxError* pClassicSyntaxError, COProject* pCOProject)
{
	int n;
	for(n = 0; n < nFileCount; n++)
	{
		// Determine the file type
		GXMLTag* pFileRoot;
		const char* szFile = ppFiles[n];
		const char* pStart;
		int nLine = 1;
		FILETYPES eFileType = IdentifyFileType(szFile, &pStart, &nLine);

		// Load it
		if(eFileType == FT_CLASSIC)
		{
			pFileRoot = ClassicSyntax::ConvertToXML(pszFilenames[n], pStart, strlen(pStart), nLine, pClassicSyntaxError);
			if(!pFileRoot)
			{
				GAssert(pClassicSyntaxError->HaveError(), "Error not set properly");
				break;
			}
		}
		else if(eFileType == FT_XML)
		{
			const char* szErrorMessage;
			int nErrorOffset;
			int nErrorLine;
			int nErrorColumn;
			pFileRoot = GXMLTag::FromString(szFile, strlen(szFile), &szErrorMessage, &nErrorOffset, &nErrorLine, &nErrorColumn);
			if(!pFileRoot)
			{
				pError->SetError(&Error::BAD_XML, NULL);
				break;
			}
		}
		else
		{
			pError->SetError(&Error::UNRECOGNIZED_SYNTAX_TYPE, NULL);
			break;
		}
		pXMLTags->AddChildTag(pFileRoot);
		if(stricmp(pFileRoot->GetName(), TAG_NAME_FILE) != 0)
		{
			pError->SetError(&Error::EXPECTED_FILE_TAG, pFileRoot);
			break;
		}
		COFile* pNewFile;
		if(eFileType == FT_CLASSIC)
			pNewFile = new COClassicFile(pszFilenames[n]);
		else
			pNewFile = new COXMLFile(pszFilenames[n]);
		m_pFiles->AddPointer(pNewFile);
	}
}

bool COFileSet::LoadAllFileNames(GXMLTag* pSourceTag, GXMLTag* pXMLTags, ErrorHandler* pErrorHandler, COProject* pCOProject)
{
	if(stricmp(pSourceTag->GetName(), TAG_NAME_SOURCE) != 0)
	{
		GlobalError err;
		err.SetError(&Error::EXPECTED_SOURCES_TAG);
		if(pErrorHandler)
			pErrorHandler->OnError(&err);
		return false;
	}

	// Add all the files to the Source
	const char* szFilename = NULL;
	GXMLTag* pChild;
	bool bOK = true;
	for(pChild = pSourceTag->GetFirstChildTag(); pChild; pChild = pSourceTag->GetNextChildTag(pChild))
	{
		if(stricmp(pChild->GetName(), TAG_NAME_FILE) != 0)
		{
			GlobalError err;
			err.SetError(&Error::EXPECTED_FILE_TAG);
			if(pErrorHandler)
				pErrorHandler->OnError(&err);
			break;
		}
		GXMLAttribute* pFileName = pChild->GetAttribute(ATTR_NAME);
		if(!pFileName)
		{
			GlobalError err;
			err.SetError(&Error::EXPECTED_NAME_ATTRIBUTE);
			if(pErrorHandler)
				pErrorHandler->OnError(&err);
			break;
		}
		FILETYPES eFileType;
		GXMLTag* pFileRoot = COFile::LoadAndConvertToXML(pFileName->GetValue(), pErrorHandler, &eFileType);
		if(!pFileRoot)
		{
			bOK = false;
			break;
		}
		pXMLTags->AddChildTag(pFileRoot);
		if(stricmp(pFileRoot->GetName(), TAG_NAME_FILE) != 0)
		{
			GlobalError err;
			err.SetError(&Error::EXPECTED_FILE_TAG);
			if(pErrorHandler)
				pErrorHandler->OnError(&err);
			break;
		}
		COFile* pNewFile;
		if(eFileType == FT_CLASSIC)
			pNewFile = new COClassicFile(pFileName->GetValue());
		else
			pNewFile = new COXMLFile(pFileName->GetValue());
		m_pFiles->AddPointer(pNewFile);
	}
	return bOK;
}

void COFileSet::LoadAllClassNames(GXMLTag* pXMLTags, COProject* pCOProject)
{
	int nCount = GetFileCount();
	GXMLTag* pFileTag = pXMLTags->GetFirstChildTag();
	COFile* pFile;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		if(pFile->GetFileType() == FT_XLIB)
			continue;
		pCOProject->m_pCurrentFile = pFile;
		pFile->LoadClassNames(pFileTag, pCOProject);
		pFileTag = pXMLTags->GetNextChildTag(pFileTag);
	}
}

void COFileSet::LoadAllClassDefinitions(GXMLTag* pXMLTags, COProject* pCOProject)
{
	int nCount = GetFileCount();
	GXMLTag* pFileTag = pXMLTags->GetFirstChildTag();
	COFile* pFile;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		if(pFile->GetFileType() == FT_XLIB)
			continue;
		pCOProject->m_pCurrentFile = pFile;
		pFile->LoadMembers(pFileTag, pCOProject, false);
		pFileTag = pXMLTags->GetNextChildTag(pFileTag);
	}
}

void COFileSet::LoadMethodDeclarations(GXMLTag* pXMLTags, COProject* pCOProject)
{
	int nCount = GetFileCount();
	GXMLTag* pFileTag = pXMLTags->GetFirstChildTag();
	COFile* pFile;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		if(pFile->GetFileType() == FT_XLIB)
			continue;
		pCOProject->m_pCurrentFile = pFile;
		pFile->LoadMethodDeclarations(pFileTag, pCOProject, false);
		pFileTag = pXMLTags->GetNextChildTag(pFileTag);
	}
}

void COFileSet::LoadAllInstructions(GXMLTag* pXMLTags, COProject* pCOProject)
{
	int nCount = GetFileCount();
	GXMLTag* pFileTag = pXMLTags->GetFirstChildTag();
	COFile* pFile;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		if(pFile->GetFileType() == FT_XLIB)
			continue;
		pCOProject->m_pCurrentFile = pFile;
		pFile->LoadInstructions(pFileTag, pCOProject, false);
		pFileTag = pXMLTags->GetNextChildTag(pFileTag);
	}
}

COType* COFileSet::FindType(int id)
{
	COType* pType;
	COFile* pFile;
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		pType = pFile->FindType(id);
		if(pType)
			return pType;
	}
	return NULL;
}

int COFileSet::CountTypes()
{
	int nTotal = 0;
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
		nTotal += GetFile(n)->CountTypes();
	return nTotal;
}

COType* COFileSet::GetType(int index)
{
	COFile* pFile;
	int nTypesInFile;
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(nCount - 1 - n);
		nTypesInFile = pFile->CountTypes();
		if(index < nTypesInFile)
			return pFile->GetType(index);
		index -= nTypesInFile;
	}
	GAssert(false, "index out of range");
	return NULL;
}

COClass* COFileSet::FindClass(const char* szClassName)
{
	// First search non-XLib files (so code types get precidence over imported types)
	COClass* pClass;
	COFile* pFile;
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		if(pFile->GetFileType() == FT_XLIB)
			continue;
		pClass = pFile->FindClass(szClassName);
		if(pClass)
			return pClass;
	}

	// Now search XLib files
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		if(pFile->GetFileType() != FT_XLIB)
			continue;
		pClass = pFile->FindClass(szClassName);
		if(pClass)
			return pClass;
	}
	return NULL;
}

int COFileSet::FindClass(COClass* pClass)
{
	int nCount = 0;
	COFile* pFile;
	int nFiles = GetFileCount();
	int n;
	for(n = 0; n < nFiles; n++)
	{
		pFile = GetFile(n);
		n = pFile->FindClass(pClass);
		if(n < 0)
			nCount += pFile->GetClassCount();
		else
			return(nCount + n);
	}
	return -1;
}

COInterface* COFileSet::FindInterface(const char* szName)
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

COMachineClass* COFileSet::FindMachineClass(const char* szName)
{
	COMachineClass* pMachineClass;
	COFile* pFile;
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		pMachineClass = pFile->FindMachineClass(szName);
		if(pMachineClass)
			return pMachineClass;
	}
	return NULL;
}

GXMLTag* COFileSet::SaveToXML()
{
	//Create the Source in XML
	GXMLTag* pSource = new GXMLTag(TAG_NAME_SOURCE);

	//create and save all the Files in the Source in XML format
	COFile* pFile;
	char szBuff[256];
	int nCount = GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = GetFile(n);
		GXMLTag* pFileTag = new GXMLTag(TAG_NAME_FILE);
		pFile->GetFilenameNoPath(szBuff);
		pFileTag->AddAttribute(new GXMLAttribute(ATTR_NAME, szBuff));
		pFileTag->AddAttribute(new GXMLAttribute(ATTR_SYNTAX, pFile->GetFileType() == FT_CLASSIC ? VAL_CLASSIC : VAL_XML));
		pSource->AddChildTag(pFileTag);
	}
	return pSource;
}

bool COFileSet::Save()
{
	GXMLTag* pXMLTag = SaveToXML();
	bool bRet = pXMLTag->ToFile(m_szFilename);
	if(bRet)
		m_bModified = false;
	return bRet;
}

bool COFileSet::LoadAllLibraries(const char* szLibrariesFolder, ParseError* pError, COProject* pProject)
{
	char szOldDir[512];
	getcwd(szOldDir, 512);
	bool bRet = LoadAllLibraries2(szLibrariesFolder, pError, pProject);
	chdir(szOldDir);
	return bRet;
}

bool COFileSet::LoadAllLibraries2(const char* szLibrariesFolder, ParseError* pError, COProject* pProject)
{
	if(chdir(szLibrariesFolder) != 0)
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
		COFile* pNewFile = new COXLibFile(szLibraryName);
		pProject->m_pCurrentFile = pNewFile;
		m_pFiles->AddPointer(pNewFile);
		pNewFile->LoadClassNames(pRoot, pProject);
		pNewFile->LoadMembers(pRoot, pProject, false);
		pNewFile->LoadMethodDeclarations(pRoot, pProject, false);
	}
	return true;
}

