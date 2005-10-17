/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "File.h"
#include "../Engine/TagNames.h"
#include "../Engine/GCompiler.h"
#include "../Engine/ClassicSyntax.h"
#include "Class.h"
#include "Interface.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GArray.h"
#include "Project.h"
#include "Variable.h"

COFile::COFile(const char* szFilename)
: CodeObject(0, 0, 0)
{
	m_pClasses = new GPointerArray(8);
	m_pInterfaces = new GPointerArray(8);
	m_pMachineClasses = new GPointerArray(8);
	m_szFilename = new char[strlen(szFilename) + 1];
	strcpy(m_szFilename, szFilename);
	m_bModified = false;
#ifdef _DEBUG
	m_nPhase = 0;
#endif // _DEBUG
}

COFile::~COFile()
{
	int nCount = GetClassCount();
	int n;
	for(n = 0; n < nCount; n++)
		delete(GetClass(n));
	delete(m_pClasses);
	nCount = GetInterfaceCount();
	for(n = 0; n < nCount; n++)
		delete(GetInterface(n));
	delete(m_pInterfaces);
	nCount = GetMachineClassCount();
	for(n = 0; n < nCount; n++)
		delete(GetMachineClass(n));
	delete(m_pMachineClasses);
	delete(m_szFilename);
}

void COFile::GetFilenameNoPath(char* pBuffer)
{
	char szFile[256];
	char szExt[256];
	_splitpath(m_szFilename, NULL, NULL, szFile, szExt);
	_makepath(pBuffer, NULL, NULL, szFile, szExt);
}

void COFile::AddClass(COClass* pClass)
{
	m_pClasses->AddPointer(pClass);
}

int COFile::GetClassCount()
{
	return m_pClasses->GetSize();
}

COClass* COFile::GetClass(int n)
{
	return (COClass*)m_pClasses->GetPointer(n);
}

void COFile::AddInterface(COInterface* pInterface)
{
	m_pInterfaces->AddPointer(pInterface);
}

int COFile::GetInterfaceCount()
{
	return m_pInterfaces->GetSize();
}

COInterface* COFile::GetInterface(int n)
{
	return (COInterface*)m_pInterfaces->GetPointer(n);
}

void COFile::AddMachineClass(COMachineClass* pMachine)
{
	m_pMachineClasses->AddPointer(pMachine);
}

int COFile::GetMachineClassCount()
{
	return m_pMachineClasses->GetSize();
}

COMachineClass* COFile::GetMachineClass(int n)
{
	return (COMachineClass*)m_pMachineClasses->GetPointer(n);
}

void COFile::ReplaceType(COType* pOld, COType* pNew)
{
	GAssert(m_nPhase < 4, "Can't replace a type after the instructions have been loaded");
	int nCount = GetClassCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COClass* pClass = GetClass(n);
		pClass->ReplaceType(pOld, pNew);
	}
	nCount = GetInterfaceCount();
	for(n = 0; n < nCount; n++)
	{
		COInterface* pInterface = GetInterface(n);
		pInterface->ReplaceType(pOld, pNew);
	}
	nCount = GetMachineClassCount();
	for(n = 0; n < nCount; n++)
	{
		COMachineClass* pMachineClass = GetMachineClass(n);
		pMachineClass->ReplaceType(pOld, pNew);
	}
}

void COFile::AddType(COType* pNewType, COProject* pCOProject, GXMLTag* pFileTag)
{
	COType* pOldType = pCOProject->FindType(pNewType->GetName());
	if(pOldType)
	{
		if(pOldType->GetGeneration() <= pNewType->GetGeneration())
		{
			//if(pOldType->GetGeneration() == pNewType->GetGeneration())
			//	pCOProject->ThrowError(&Error::CONFLICTING_TYPES, pFileTag);
			delete(pNewType);
			return;
		}
	}

	// Add it to the file
	switch(pNewType->GetTypeType())
	{
		case COType::TT_CLASS:
			m_pClasses->AddPointer(pNewType);
			break;
		case COType::TT_INTERFACE:
			m_pInterfaces->AddPointer(pNewType);
			break;
		case COType::TT_MACHINE:
			m_pMachineClasses->AddPointer(pNewType);
			break;
		default:
			GAssert(false, "unexpected type type");
	}

	// Replace the old type with the new one
	if(pOldType)
		pCOProject->ReplaceType(pOldType, pNewType);
}

void COFile::LoadTypeNames(GXMLTag* pFileTag, COProject* pCOProject, bool bXLib)
{
	GAssert(m_nPhase == 0, "expected to be in phase 0");
	if(stricmp(pFileTag->GetName(), TAG_NAME_FILE) != 0 && stricmp(pFileTag->GetName(), TAG_NAME_LIBRARY) != 0)
		pCOProject->ThrowError(&Error::EXPECTED_FILE_OR_LIBRARY_TAG, pFileTag);
	GXMLTag* pChildTag;
	GXMLAttribute* pName;
	for(pChildTag = pFileTag->GetFirstChildTag(); pChildTag; pChildTag = pFileTag->GetNextChildTag(pChildTag))
	{
		int nLine = pChildTag->GetLineNumber();
		int nCol, nWid;
		pChildTag->GetOffsetAndWidth(&nCol, &nWid);
		if(stricmp(pChildTag->GetName(), TAG_NAME_CLASS) == 0)
		{
			pName = pChildTag->GetAttribute(ATTR_NAME);
			if(!pName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChildTag);
			const char* szSource = NULL;
			if(GetFileType() == FT_XLIB)
			{
				GXMLAttribute* pSourceAttr = pChildTag->GetAttribute(ATTR_SOURCE);
				if(pSourceAttr)
					szSource = pSourceAttr->GetValue();
			}
			else
				szSource = GetFilename(); // todo: use a relative filename
			COClass* pNewClass = new COClass(nLine, nCol, nWid, pName->GetValue(), pCOProject->m_pObject, this, szSource, pCOProject);
			if(bXLib)
			{
				GXMLAttribute* pGenAttr = pChildTag->GetAttribute(ATTR_GEN);
				if(!pGenAttr)
					pCOProject->ThrowError(&Error::EXPECTED_GEN_ATTRIBUTE, pChildTag);
				pNewClass->SetGeneration(atoi(pGenAttr->GetValue()));
			}
			AddType(pNewClass, pCOProject, pChildTag);
		}
		else if(stricmp(pChildTag->GetName(), TAG_NAME_INTERFACE) == 0)
		{
			pName = pChildTag->GetAttribute(ATTR_NAME);
			if(!pName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChildTag);
			COInterface* pNewInterface = new COInterface(nLine, nCol, nWid, pName->GetValue(), this, pCOProject);
			if(bXLib)
			{
				GXMLAttribute* pGenAttr = pChildTag->GetAttribute(ATTR_GEN);
				if(!pGenAttr)
					pCOProject->ThrowError(&Error::EXPECTED_GEN_ATTRIBUTE, pChildTag);
				pNewInterface->SetGeneration(atoi(pGenAttr->GetValue()));
			}
			AddType(pNewInterface, pCOProject, pChildTag);
		}
		else if(stricmp(pChildTag->GetName(), TAG_NAME_MACHINE) == 0)
		{
			pName = pChildTag->GetAttribute(ATTR_NAME);
			if(!pName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChildTag);
			COMachineClass* pNewMachineClass = new COMachineClass(nLine, nCol, nWid, pName->GetValue(), this, pCOProject);
			if(bXLib)
			{
				GXMLAttribute* pGenAttr = pChildTag->GetAttribute(ATTR_GEN);
				if(!pGenAttr)
					pCOProject->ThrowError(&Error::EXPECTED_GEN_ATTRIBUTE, pChildTag);
				pNewMachineClass->SetGeneration(atoi(pGenAttr->GetValue()));
			}
			AddType(pNewMachineClass, pCOProject, pChildTag);
		}
	}
#ifdef _DEBUG
	m_nPhase = 1;
#endif // _DEBUG
}

void COFile::LoadMembers(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	GAssert(m_nPhase == 1, "Expected to be in phase 1");
	GXMLTag* pChildTag = pTag->GetFirstChildTag();
	GXMLAttribute* pAttrName;
	while(pChildTag)
	{
		if(stricmp(pChildTag->GetName(), TAG_NAME_CLASS) == 0)
		{
			pAttrName = pChildTag->GetAttribute(ATTR_NAME);
			if(!pAttrName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChildTag);
			COClass* pClass = FindClass(pAttrName->GetValue());
			if(pClass)
				pClass->LoadMembers(pChildTag, pCOProject, bPartial);
		}
		pChildTag = pTag->GetNextChildTag(pChildTag);
	}
#ifdef _DEBUG
	m_nPhase = 2;
#endif // _DEBUG
}

void COFile::LoadMethodDeclarations(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	GAssert(m_nPhase == 2, "expected to be in phase 2");
	GXMLTag* pChildTag = pTag->GetFirstChildTag();
	GXMLAttribute* pAttrName;
	while(pChildTag)
	{
		if(stricmp(pChildTag->GetName(), TAG_NAME_CLASS) == 0)
		{
			pAttrName = pChildTag->GetAttribute(ATTR_NAME);
			if(!pAttrName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChildTag);
			COClass* pClass = FindClass(pAttrName->GetValue());
			if(pClass)
				pClass->LoadAllMethodDefinitions(pChildTag, pCOProject, bPartial);
		}
		else if(stricmp(pChildTag->GetName(), TAG_NAME_INTERFACE) == 0)
		{
			pAttrName = pChildTag->GetAttribute(ATTR_NAME);
			if(!pAttrName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChildTag);
			COInterface* pInterface = FindInterface(pAttrName->GetValue());
			if(pInterface)
				pInterface->LoadMethodDecls(pChildTag, pCOProject, bPartial);
		}
		else if(stricmp(pChildTag->GetName(), TAG_NAME_MACHINE) == 0)
		{
			pAttrName = pChildTag->GetAttribute(ATTR_NAME);
			if(!pAttrName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChildTag);
			COMachineClass* pMachineClass = FindMachineClass(pAttrName->GetValue());
			if(pMachineClass)
				pMachineClass->LoadMethodDecls(pChildTag, pCOProject, bPartial);
		}
		pChildTag = pTag->GetNextChildTag(pChildTag);
	}
#ifdef _DEBUG
	m_nPhase = 3;
#endif // _DEBUG
}

void COFile::LoadInstructions(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	GAssert(m_nPhase == 3, "expected to be in phase 3");
	GXMLTag* pChildTag = pTag->GetFirstChildTag();
	GXMLAttribute* pAttrName;
	while(pChildTag)
	{
		if(stricmp(pChildTag->GetName(), TAG_NAME_CLASS) == 0)
		{
			pAttrName = pChildTag->GetAttribute(ATTR_NAME);
			if(!pAttrName)
				pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pChildTag);
			COClass* pClass = FindClass(pAttrName->GetValue());
			if(pClass)
				pClass->LoadAllInstructions(pChildTag, pCOProject, bPartial);
		}
		pChildTag = pTag->GetNextChildTag(pChildTag);
	}
#ifdef _DEBUG
	m_nPhase = 4;
#endif // _DEBUG
}

GXMLTag* COFile::SaveToXML()
{
	GXMLTag* pFileTag = new GXMLTag(TAG_NAME_FILE);
	int nCount = GetInterfaceCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COInterface* pInterface = GetInterface(n);
		pFileTag->AddChildTag(pInterface->SaveToXML());
	}
	nCount = GetClassCount();
	for(n = 0; n < nCount; n++)
	{
		COClass* pClass = GetClass(n);
		pFileTag->AddChildTag(pClass->SaveToXML());
	}
	return pFileTag;
}

void COFile::SaveToClassicSyntax(GQueue* pQ)
{
	int nCount = GetInterfaceCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COInterface* pInterface = GetInterface(n);
		pInterface->SaveToClassicSyntax(pQ);
	}
	nCount = GetClassCount();
	for(n = 0; n < nCount; n++)
	{
		COClass* pClass = GetClass(n);
		pClass->SaveToClassicSyntax(pQ);
	}
}

bool COFile::Save()
{
	GAssert(GetFileType() == FT_CLASSIC || GetFileType() == FT_XML, "Invalid file type for this operation");
	bool bRet;
	if(GetFileType() == FT_CLASSIC)
	{
		GQueue q(4096);
		SaveToClassicSyntax(&q);
		int nSize = q.GetSize();
		Holder<char*> hBuf(new char[nSize + 1]);
		char* pBuffer = hBuf.Get();
		if(pBuffer)
		{
			bRet = true;
			int n;
			for(n = 0; n < nSize; n++)
			{
				if(!q.Pop(&pBuffer[n]))
				{
					GAssert(false, "problem with the queue");
					bRet = false;
					break;
				}
			}
			pBuffer[n] = '\0';
			if(bRet)
			{
				FILE* pFile = fopen(m_szFilename, "w");
				if(pFile)
				{
					if(fwrite(pBuffer, nSize + 1, 1, pFile) != 1)
					{
						GAssert(false, "error writing to file");
						bRet = false;
					}
					fclose(pFile);
				}
				else
				{
					GAssert(false, "failed to open file for writing");
					bRet = false;
				}
			}
		}
		else
		{
			GAssert(false, "failed to allocate enough memory for the file");
			bRet = false;
		}
	}
	else
	{
		GXMLTag* pXMLTag = SaveToXML();
		bRet = pXMLTag->ToFile(m_szFilename);
	}
	if(bRet)
		m_bModified = false;
	return bRet;
}

int COFile::CountTypes()
{
	return GetClassCount() + GetInterfaceCount() + GetMachineClassCount();
}

COType* COFile::GetType(int index)
{
	int nCount = GetClassCount();
	if(index < nCount)
		return GetClass(index);
	index -= nCount;
	nCount = GetInterfaceCount();
	if(index < nCount)
		return GetInterface(index);
	index -= nCount;
	nCount = GetMachineClassCount();
	if(index < nCount)
		return GetMachineClass(index);
	GAssert(false, "index out of range");
	return NULL;
}

bool COFile::RemoveUnlinkedType(COType* pOldType)
{
	int n;
	if(pOldType->GetTypeType() == COType::TT_CLASS)
	{
		for(n = 0; n < GetClassCount(); n++)
		{
			if(GetClass(n) == pOldType)
			{
				delete(GetClass(n));
				m_pClasses->DeleteCell(n);
				return true;
			}
		}
	}
	else if(pOldType->GetTypeType() == COType::TT_INTERFACE)
	{
		for(n = 0; n < GetInterfaceCount(); n++)
		{
			if(GetInterface(n) == pOldType)
			{
				delete(GetInterface(n));
				m_pInterfaces->DeleteCell(n);
				return true;
			}
		}
	}
	else if(pOldType->GetTypeType() == COType::TT_MACHINE)
	{
		for(n = 0; n < GetMachineClassCount(); n++)
		{
			if(GetMachineClass(n) == pOldType)
			{
				delete(GetMachineClass(n));
				m_pMachineClasses->DeleteCell(n);
				return true;
			}
		}
	}
	else
	{
		GAssert(false, "unexpected type type");
	}
	return false;
}

COType* COFile::FindType(int id)
{
	int nCount = GetClassCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COClass* pClass = GetClass(n);
		if(pClass->GetID() == id)
			return pClass;
	}
	nCount = GetInterfaceCount();
	for(n = 0; n < nCount; n++)
	{
		COInterface* pInterface = GetInterface(n);
		if(pInterface->GetID() == id)
			return pInterface;
	}
	nCount = GetMachineClassCount();
	for(n = 0; n < nCount; n++)
	{
		COMachineClass* pMachineClass = GetMachineClass(n);
		if(pMachineClass->GetID() == id)
			return pMachineClass;
	}
	return NULL;
}

COClass* COFile::FindClass(const char* szClass)
{
	int nCount = GetClassCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COClass* pClass = GetClass(n);
		if(stricmp(pClass->GetName(), szClass) == 0)
			return pClass;
	}
	return NULL;
}

int COFile::FindClass(COClass* pFindMe)
{
	int nCount = GetClassCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COClass* pClass = GetClass(n);
		if(pClass == pFindMe)
			return n;
	}
	return -1;
}

COInterface* COFile::FindInterface(const char* szName)
{
	int nCount = GetInterfaceCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COInterface* pInterface = GetInterface(n);
		if(stricmp(pInterface->GetName(), szName) == 0)
			return pInterface;
	}
	return NULL;
}

COMachineClass* COFile::FindMachineClass(const char* szName)
{
	int nCount = GetMachineClassCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COMachineClass* pMachineClass = GetMachineClass(n);
		if(stricmp(pMachineClass->GetName(), szName) == 0)
			return pMachineClass;
	}
	return NULL;
}

/*static*/ GXMLTag* COFile::LoadAndConvertToXML(const char* szFilename, ErrorHandler* pErrorHandler, FILETYPES* peFileType)
{
	// Load the file
	const char* pStart;
	int nStartLine = 1;
	FILETYPES eFileType;
	Holder<char*> hBuf(LoadFile(pErrorHandler, szFilename, &eFileType, &pStart, &nStartLine));
	if(!hBuf.Get())
		return NULL;
	GXMLTag* pRoot = NULL;

	// Convert to XML
	if(eFileType == FT_CLASSIC)
	{
		ClassicSyntaxError err;
		pRoot = ClassicSyntax::ConvertToXML(szFilename, pStart, strlen(pStart), nStartLine, &err);
		if(!pRoot)
		{
			GAssert(err.HaveError(), "Error not set properly");
			if(pErrorHandler)
				pErrorHandler->OnError(&err);
			return NULL;
		}
	}
	else if(eFileType == FT_XML)
	{
		const char* szErrorMessage;
		int nErrorOffset;
		int nErrorLine;
		int nErrorColumn;
		pRoot = GXMLTag::FromString(pStart, strlen(pStart), &szErrorMessage, &nErrorOffset, &nErrorLine, &nErrorColumn);
		if(!pRoot)
		{
			GlobalError err;
			err.SetError(&Error::BAD_XML);
			pErrorHandler->OnError(&err);
			return NULL;
		}
	}
	else
	{
		GlobalError err;
		err.SetError(&Error::INVALID_SYNTAX_TYPE);
		if(pErrorHandler)
			pErrorHandler->OnError(&err);
		return NULL;
	}		

	*peFileType = eFileType;
	return pRoot;
}

COFile* COFile::LoadPartialForSymbolCreation(const char* szFilename, ErrorHandler* pErrorHandler, COProject* pProject)
{
	FILETYPES eFileType;
	GXMLTag* pTag = LoadAndConvertToXML(szFilename, pErrorHandler, &eFileType);
	if(!pTag)
		return NULL;
	Holder<COFile*> hFile(NULL);
	if(eFileType == FT_CLASSIC)
		hFile.Set(new COClassicFile(szFilename));
	else
	{
		GAssert(eFileType == FT_XML, "unexpected file type");
		hFile.Set(new COXMLFile(szFilename));
	}
	COFile* pF = hFile.Get();
	bool bOK = true;
	try
	{
		pF->LoadTypeNames(pTag, pProject, false);
		pF->LoadMembers(pTag, pProject, true);
		pF->LoadMethodDeclarations(pTag, pProject, true);
		pF->LoadInstructions(pTag, pProject, true);
	}
	catch(ParseError* pError)
	{
		pErrorHandler->OnError(pError);
		delete(pError);
		bOK = false;
	}
	if(!bOK)
		return NULL;
	return hFile.Drop();
}

bool COFile::Compile(GCompiler* pCompiler)
{
	pCompiler->SetCurrentFile(this);

	// Compile Machine Classes
	int nCount = GetMachineClassCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		COMachineClass* pMachineClass = GetMachineClass(n);
		if(!pCompiler->CompileInterface(pMachineClass))
		{
			pCompiler->CheckError();
			return false;
		}
	}

	// Compile Interfaces
	nCount = GetInterfaceCount();
	n;
	for(n = 0; n < nCount; n++)
	{
		COInterface* pInterface = GetInterface(n);
		if(!pCompiler->CompileInterface(pInterface))
		{
			pCompiler->CheckError();
			return false;
		}
	}

	// Compile Classes
	nCount = GetClassCount();
	for(n = 0; n < nCount; n++)
	{
		COClass* pClass = GetClass(n);
		if(!pClass->Compile(pCompiler))
		{
			pCompiler->CheckError();
			return false;
		}
	}

	pCompiler->SetCurrentFile(NULL);
	return true;
}


// ----------------------------------------------------------------------

Library* COXLibFile::GetLibrary()
{
	if(!m_pLibrary)
		m_pLibrary = Library::LoadFromFile(m_szFilename, NULL, false);
	return m_pLibrary;
}


