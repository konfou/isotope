/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "Project.h"
#include "File.h"
#include "Method.h"
#include "Class.h"
#include "Variable.h"
#include "FileSet.h"
#include "Interface.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GFile.h"
#include "../Include/GaspEngine.h"
#ifdef WIN32
#	include <direct.h>
#else
#	include <unistd.h>
#endif // !WIN32

COProject::COProject(const char* szFilename)
{
	m_pString = NULL;
	m_pFloat = NULL;
	m_pSource = NULL;
	m_nMethodID = 0;
	m_nTypeID = 0;
	m_pCurrentFile = NULL;

	if(szFilename)
	{
		m_szFilename = new char[strlen(szFilename) + 1];
		strcpy(m_szFilename, szFilename);
	}
	else
		m_szFilename = NULL;

	// Built-in file
	m_pBuiltIn = new COXMLFile("<Built In>");

	// Built-in classes
	m_pObject = new COClass(0, 0, 0, CLASS_NAME_OBJECT, NULL, m_pBuiltIn, NULL, this);
	GAssert(m_pObject->GetID() == 0, "This should be the first class you create");
	m_pInteger = new COClass(0, 0, 0, CLASS_NAME_INTEGER, m_pObject, m_pBuiltIn, NULL, this);
	m_pStackLayer = new COClass(0, 0, 0, CLASS_NAME_STACKLAYER, m_pObject, m_pBuiltIn, NULL, this);
	m_pStackLayer->AddMember(new COVariable(0, 0, 0, "catcher", m_pObject, false, false, false));

	// Internal classes
	m_pInternalMember = new COClass(0, 0, 0, CLASS_NAME_MEMBER, m_pObject, m_pBuiltIn, NULL, this);
	m_pInternalComparator = new COClass(0, 0, 0, CLASS_NAME_COMPARATOR, m_pObject, m_pBuiltIn, NULL, this); // for comparators
	m_pInternalClass = new COClass(0, 0, 0, CLASS_NAME_CLASS, m_pObject, m_pBuiltIn, NULL, this); // for variables that refer to a class
	m_pInternalMethod = new COClass(0, 0, 0, CLASS_NAME_METHOD, m_pObject, m_pBuiltIn, NULL, this); // for variables that refer to a method
	COMethod* pNoop = new COMethod(-1, -1, -1, "Noop", m_pInternalMethod, true, this);
	m_pInternalMethod->AddProcedure(pNoop);

	// Internal variables
	m_pNull = new COVariable(0, 0, 0, VAL_NULL, m_pObject, true, true, false);

	CreateBuiltInMethods();
}

COProject::~COProject()
{
	delete(m_szFilename);
	delete(m_pSource);
	delete(m_pNull);
	delete(m_pInternalMethod);
	delete(m_pInternalComparator);
	delete(m_pInternalClass);
	delete(m_pInternalMember);
	delete(m_pStackLayer);
	delete(m_pInteger);
	delete(m_pObject);
	delete(m_pBuiltIn);
}

/*static*/ COProject* COProject::LoadProject(const char* szLibrariesPath, const char* szProjectFile, ErrorHandler* pErrorHandler)
{
	Holder<COProject*> hProject(new COProject(szProjectFile));
	//if(!hProject.Get()->LoadLibraries(szLibrariesPath, pErrorHandler))
	//	return NULL;
	if(!hProject.Get()->LoadSourcesInProjectFile(szProjectFile, pErrorHandler))
		return NULL;
	return hProject.Drop();
}

bool COProject::LoadLibraries(const char* szLibrariesPath, ErrorHandler* pErrorHandler)
{
	GAssert(szLibrariesPath, "No path specified");

	// Make some variables to hold error values
	bool bRet = true;
	try
	{
		GetSource()->LoadAllLibraries(szLibrariesPath, this);
	}
	catch(ParseError* pError)
	{
		bRet = false;
		if(pErrorHandler)
			pErrorHandler->OnError(pError);
		delete(m_pSource);
		m_pSource = NULL;
		delete(pError);
	}

	return bRet;
}

void COProject::ReplaceType(COType* pOld, COType* pNew)
{
	if(pOld->GetTypeType() != pNew->GetTypeType() || stricmp(pOld->GetName(), pNew->GetName()) != 0)
		ThrowError(&Error::CONFLICTING_TYPES, NULL);
	m_pSource->ReplaceType(pOld, pNew);
}

void COProject::ThrowError(ErrorStruct* pError, GXMLTag* pTagWithError, const char* szParam1)
{
	ParseError* pErrorHolder = new ParseError();
	pErrorHolder->SetError(pError, pTagWithError);
	if(szParam1)
		pErrorHolder->SetParam1(szParam1);
	if(m_pCurrentFile)
		pErrorHolder->SetFilename(m_pCurrentFile->GetFilename());
	throw pErrorHolder;
}

void COProject::ThrowError(ErrorStruct* pError, GXMLTag* pTagWithError)
{
	ThrowError(pError, pTagWithError, NULL);
}

bool COProject::LoadSourcesInProjectFile(const char* szFilename, ErrorHandler* pErrorHandler)
{
	char szOldDir[512];
	getcwd(szOldDir, 512);
	
	GXMLTag xmlFiles("XMLFiles");
	bool bRet = true;

	try
	{
		if(m_pSource)
			ThrowError(&Error::SOURCE_ALREADY_LOADED, NULL);

		// Parse the XML file
		char szDrive[256];
		char szDir[256];
		char szPath[512];
		char szName[256];
		char szExt[256];
		char szFN[256];
		_splitpath(szFilename, szDrive, szDir, szName, szExt);
		_makepath(szPath, szDrive, szDir, NULL, NULL);
		_makepath(szFN, NULL, NULL, szName, szExt);
		chdir(szPath);

		const char* szErrorMessage;
		int nErrorOffset;
		int nErrorLine;
		int nErrorColumn;
		Holder<GXMLTag*> hTag(GXMLTag::FromFile(szFN, &szErrorMessage, &nErrorOffset, &nErrorLine, &nErrorColumn));
		if(!hTag.Get())
		{
			if(GFile::DoesFileExist(szFilename))
				ThrowError(&Error::BAD_XML, NULL);
			else
				ThrowError(&Error::FILE_NOT_FOUND, NULL, szFN);
		}

		// Make sure this tag has the right name
		if(stricmp(hTag.Get()->GetName(), TAG_NAME_SOURCE) != 0)
			ThrowError(&Error::EXPECTED_SOURCE_TAG, hTag.Get());

		// Load all the source files named in the project file
		if(!GetSource()->LoadAllFileNames(hTag.Get(), &xmlFiles, pErrorHandler, this))
			bRet = false;
		else
			LoadSources(&xmlFiles);
	}
	catch(ParseError* pError)
	{
		bRet = false;
		if(pErrorHandler)
			pErrorHandler->OnError((ErrorHolder*)pError);
		delete(m_pSource);
		m_pSource = NULL;
		delete(pError);
	}
	m_pCurrentFile = NULL;

	chdir(szOldDir);

	return bRet;
}

void COProject::LoadSources(GXMLTag* pXMLFiles)
{
	m_pSource->LoadAllTypeNames(pXMLFiles, this, false);
	m_pSource->LoadAllClassDefinitions(pXMLFiles, this);
	m_pSource->LoadMethodDeclarations(pXMLFiles, this);
	m_pSource->LoadAllInstructions(pXMLFiles, this);
}

bool COProject::LoadSources(const char** ppFiles, const char** pszFilenames, int nFileCount, ErrorHandler* pErrorHandler)
{
	// Create the sources
	GXMLTag xmlFiles("XMLFiles");
	ParseError errorHolder;
	ClassicSyntaxError errorHolder2;
	GetSource()->MakeFilesFromRefs(ppFiles, pszFilenames, nFileCount, &xmlFiles, &errorHolder, &errorHolder2, this);

	// Call the error-handler callback
	bool bRet = true;
	if(errorHolder.HaveError() || errorHolder2.HaveError())
	{
		bRet = false;
		if(pErrorHandler)
			pErrorHandler->OnError(errorHolder.HaveError() ? (ErrorHolder*)&errorHolder : (ErrorHolder*)&errorHolder2);
		delete(m_pSource);
		m_pSource = NULL;
	}
	else
	{
		try
		{
			LoadSources(&xmlFiles);
		}
		catch(ParseError* pError)
		{
			bRet = false;
			if(pErrorHandler)
				pErrorHandler->OnError((ErrorHolder*)pError);
			delete(m_pSource);
			m_pSource = NULL;
			delete(pError);
		}
		m_pCurrentFile = NULL;
	}

	return bRet;
}

void COProject::CreateBuiltInMethods()
{
	// --------------
	// Object Methods
	// --------------

	m_pObject_return = new COMethod(0, 0, 0, "return", m_pObject, true, this);
	m_pObject->AddProcedure(m_pObject_return);

	m_pObject_if = new COMethod(0, 0, 0, "if", m_pObject, true, this);
	m_pObject_if->AddParameter(new COVariable(0, 0, 0, "condition", m_pObject, true, true, false));
	m_pObject->AddProcedure(m_pObject_if);

	m_pObject_while = new COMethod(0, 0, 0, "while", m_pObject, true, this);
	m_pObject_while->AddParameter(new COVariable(0, 0, 0, "condition", m_pObject, true, true, false));
	m_pObject->AddProcedure(m_pObject_while);

	m_pObject_else = new COMethod(0, 0, 0, "else", m_pObject, true, this);
	m_pObject->AddProcedure(m_pObject_else);

	m_pObject_allocate = new COMethod(0, 0, 0, "allocate", m_pObject, true, this);
	m_pObject_allocate->AddParameter(new COVariable(0, 0, 0, "dest", m_pObject, false, false, false));
	m_pObject->AddProcedure(m_pObject_allocate);

	m_pObject_set = new COMethod(0, 0, 0, "set", m_pObject, true, this);
	m_pObject_set->AddParameter(new COVariable(0, 0, 0, "dest", m_pObject, false, false, false));
	m_pObject_set->AddParameter(new COVariable(0, 0, 0, "source", m_pObject, true, true, false));
	m_pObject->AddProcedure(m_pObject_set);

	m_pObject_copy = new COMethod(0, 0, 0, "copy", m_pObject, true, this);
	m_pObject_copy->AddParameter(new COVariable(0, 0, 0, "dest", m_pObject, true, false, false));
	m_pObject_copy->AddParameter(new COVariable(0, 0, 0, "source", m_pObject, true, true, false));
	m_pObject->AddProcedure(m_pObject_copy);

	m_pObject_throw = new COMethod(0, 0, 0, "throw", m_pObject, true, this);
	m_pObject_throw->AddParameter(new COVariable(0, 0, 0, "exception", m_pObject, true, true, false));
	m_pObject->AddProcedure(m_pObject_throw);


	// --------------
	// Integer Methods
	// --------------

	m_pInteger_new = new COMethod(0, 0, 0, "new", m_pInteger, false, this);
	m_pInteger_new->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, false, false, false));
	m_pInteger->AddMethod(m_pInteger_new);

	m_pInteger_increment = new COMethod(0, 0, 0, "increment", m_pInteger, true, this);
	m_pInteger_increment->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger->AddProcedure(m_pInteger_increment);

	m_pInteger_decrement = new COMethod(0, 0, 0, "decrement", m_pInteger, true, this);
	m_pInteger_decrement->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger->AddProcedure(m_pInteger_decrement);

	m_pInteger_add = new COMethod(0, 0, 0, "add", m_pInteger, true, this);
	m_pInteger_add->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_add->AddParameter(new COVariable(0, 0, 0, "source", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_add);

	m_pInteger_subtract = new COMethod(0, 0, 0, "subtract", m_pInteger, true, this);
	m_pInteger_subtract->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_subtract->AddParameter(new COVariable(0, 0, 0, "source", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_subtract);

	m_pInteger_multiply = new COMethod(0, 0, 0, "multiply", m_pInteger, true, this);
	m_pInteger_multiply->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_multiply->AddParameter(new COVariable(0, 0, 0, "source", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_multiply);

	m_pInteger_divide = new COMethod(0, 0, 0, "divide", m_pInteger, true, this);
	m_pInteger_divide->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_divide->AddParameter(new COVariable(0, 0, 0, "source", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_divide);

	m_pInteger_modulus = new COMethod(0, 0, 0, "modulus", m_pInteger, true, this);
	m_pInteger_modulus->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_modulus->AddParameter(new COVariable(0, 0, 0, "source", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_modulus);

	m_pInteger_and = new COMethod(0, 0, 0, "and", m_pInteger, true, this);
	m_pInteger_and->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_and->AddParameter(new COVariable(0, 0, 0, "source", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_and);

	m_pInteger_or = new COMethod(0, 0, 0, "or", m_pInteger, true, this);
	m_pInteger_or->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_or->AddParameter(new COVariable(0, 0, 0, "source", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_or);

	m_pInteger_xor = new COMethod(0, 0, 0, "xor", m_pInteger, true, this);
	m_pInteger_xor->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_xor->AddParameter(new COVariable(0, 0, 0, "source", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_xor);

	m_pInteger_invert = new COMethod(0, 0, 0, "invert", m_pInteger, true, this);
	m_pInteger_invert->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger->AddProcedure(m_pInteger_invert);

	m_pInteger_shiftLeft = new COMethod(0, 0, 0, "shiftLeft", m_pInteger, true, this);
	m_pInteger_shiftLeft->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_shiftLeft->AddParameter(new COVariable(0, 0, 0, "bits", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_shiftLeft);

	m_pInteger_shiftRight = new COMethod(0, 0, 0, "shiftRight", m_pInteger, true, this);
	m_pInteger_shiftRight->AddParameter(new COVariable(0, 0, 0, "dest", m_pInteger, true, false, false));
	m_pInteger_shiftRight->AddParameter(new COVariable(0, 0, 0, "bits", m_pInteger, true, true, false));
	m_pInteger->AddProcedure(m_pInteger_shiftRight);
}

COMachineClass* COProject::GetStringClass()
{
	if(m_pString)
		return m_pString;
	m_pString = FindMachineClass(TAG_NAME_STRING);
	GAssert(m_pString, "Failed to get string class"); // todo: throw an error here
	return m_pString;
}

COMachineClass* COProject::GetFloatClass()
{
	if(m_pFloat)
		return m_pFloat;
	m_pFloat = FindMachineClass(TAG_NAME_FLOAT);
	GAssert(m_pFloat, "Failed to get float class"); // todo: throw an error here
	return m_pFloat;
}

COClass* COProject::FindClass(const char* pName, int nLen)
{
	char* szTmp = (char*)alloca(nLen + 1);
	memcpy(szTmp, pName, nLen);
	szTmp[nLen] = '\0';
	return FindClass(szTmp);
}

COClass* COProject::FindClass(const char* szName)
{
	// Check built in types
	if(stricmp(szName, CLASS_NAME_OBJECT) == 0)
		return m_pObject;
	if(stricmp(szName, CLASS_NAME_INTEGER) == 0)
		return m_pInteger;

	// Search the Source if Source was loaded
	COClass* pClass;
	if(m_pSource)
	{
		pClass = m_pSource->FindClass(szName);
		if(pClass)
			return pClass;
	}

	return NULL;
}

COInterface* COProject::FindInterface(const char* szName)
{
	// Search the Source if Source was loaded
	COInterface* pInterface;
	if(m_pSource)
	{
		pInterface = m_pSource->FindInterface(szName);
		if(pInterface)
			return pInterface;
	}

	return NULL;
}

COMachineClass* COProject::FindMachineClass(const char* szName)
{
	// Search the Source if Source was loaded
	COMachineClass* pMachineClass;
	if(m_pSource)
	{
		pMachineClass = m_pSource->FindMachineClass(szName);
		if(pMachineClass)
			return pMachineClass;
	}

	return NULL;
}

int COProject::CountTypes()
{
	return m_pSource->CountTypes();
}

COType* COProject::GetType(int index)
{
	return m_pSource->GetType(index);
}

COType* COProject::FindType(int id)
{
	if(m_pSource)
		return m_pSource->FindType(id);
	else
		return NULL;
}

COType* COProject::FindType(const char* szName)
{
	COType* pType = FindClass(szName);
	if(pType)
		return pType;
	pType = FindInterface(szName);
	if(pType)
		return pType;
	return FindMachineClass(szName);
}

COType* COProject::FindType(const char* pName, int nLen)
{
	char* szTmp = (char*)alloca(nLen + 1);
	memcpy(szTmp, pName, nLen);
	szTmp[nLen] = '\0';
	return FindType(szTmp);
}

bool COProject::SaveChanges()
{
	GAssert(m_pSource, "No source loaded");
	COFile* pFile;
	bool bRet = true;
	int nCount = m_pSource->GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = m_pSource->GetFile(n);
		if(pFile->GetModified())
		{
			if(!pFile->Save())
				bRet = false;
		}
	}
	if(m_pSource->GetModified())
	{
		if(!m_pSource->Save())
			bRet = false;
	}
	return bRet;
}

COFileSet* COProject::GetSource()
{
	if(!m_pSource)
		m_pSource = new COFileSet("*Files*");
	return m_pSource;
}

bool COProject::HasChanges()
{
	GAssert(m_pSource, "No Source loaded");
	if(m_pSource->GetModified())
		return true;
	COFile* pFile;
	int nCount = m_pSource->GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = m_pSource->GetFile(n);
		if(pFile->GetModified())
			return true;
	}
	return false;
}

COMethod* COProject::GetNoop()
{
	return m_pInternalMethod->GetProcedure(0);
}

