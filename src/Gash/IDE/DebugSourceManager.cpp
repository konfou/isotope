#include <stdio.h>
#include <string.h>
#include "DebugSourceManager.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GAVLTree.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GFile.h"
#include "../Include/GashQt.h"
#include "../CodeObjects/Project.h"
#include "../CodeObjects/Class.h"
#include "../CodeObjects/FileSet.h"
#include "../CodeObjects/File.h"
#include "../Engine/GCompiler.h"
#include "../Engine/EMethod.h"
#include "../Engine/EClass.h"
#include "../Engine/EInstrArray.h"
#include "../Engine/Disassembler.h"

char* LoadFileToBuffer(ErrorHandler* pErrorHandler, const char* szFilename);


class CacheEntry : public GAVLNode
{
protected:
	char* m_szName;
	void* m_pValue;

public:
	CacheEntry(const char* szName, void* pValue)
	{
		m_szName = new char[strlen(szName) + 1];
		strcpy(m_szName, szName);
		m_pValue = pValue;
	}

	virtual int Compare(GAVLNode* pThat)
	{
		return strcmp(m_szName, ((CacheEntry*)pThat)->m_szName);
	}

	const char* GetName() { return m_szName; }
	void* GetValue() { return m_pValue; }
};


// --------------------------------------------------------------

class DebugSourceManagerAssertingErrorHandler : public ErrorHandler
{
public:
	virtual void OnError(ErrorHolder* pErrorHolder)
	{
		GAssert(false, pErrorHolder->m_pError->message);
	}
};


// --------------------------------------------------------------


DebugSourceManager::DebugSourceManager(Library* pLibrary)
{
	m_pSourceSearchPaths = new GPointerArray(8);
	m_pSourceCacheSpecified = new GAVLTree();
	m_pSourceCacheFound = new GAVLTree();
	strcpy(m_szCurrentSource, "");
	m_pCurrentTypeTag = NULL;
	m_pPartialProject = new COProject("*Partial*");
	
	// todo: remove these two lines
	DebugSourceManagerAssertingErrorHandler errorHandler;
	m_pPartialProject->LoadLibraries("c:\\mike\\cpp\\source\\gash\\xlib", &errorHandler);
	
	int nDisassemblyLength;
	m_szDisassembly = Disassembler::DisassembleLibraryToText(pLibrary, &nDisassemblyLength);
}

DebugSourceManager::~DebugSourceManager()
{
	// Delete source search paths
	int n;
	int nCount = m_pSourceSearchPaths->GetSize();
	for(n = 0; n < nCount; n++)
		delete((char*)m_pSourceSearchPaths->GetPointer(n));
	delete(m_pSourceSearchPaths);

	// Delete source caches
	delete(m_pSourceCacheSpecified);
	while(m_pSourceCacheFound->GetSize() > 0)
	{
		CacheEntry* pNode = (CacheEntry*)m_pSourceCacheFound->GetNode(0);
		delete((char*)pNode->GetValue());
		m_pSourceCacheFound->Delete(0);
	}
	delete(m_pSourceCacheFound);
	delete(m_pPartialProject);
	delete(m_szDisassembly);
}

void DebugSourceManager::AddSourceSearchPath(const char* szPath)
{
	char* szPathCopy = new char[strlen(szPath) + 1];
	strcpy(szPathCopy, szPath);
	m_pSourceSearchPaths->AddPointer(szPathCopy);
}

const char* DebugSourceManager::GetSourceFile(const char* szFilename, const char** szOutActualFilename)
{
	// Check the cache
	int nIndex;
	CacheEntry likeMe(szFilename, NULL);
	CacheEntry* pEntry = (CacheEntry*)m_pSourceCacheSpecified->GetNode(&likeMe, &nIndex);
	if(pEntry)
	{
		CacheEntry* pFound = (CacheEntry*)pEntry->GetValue();
		*szOutActualFilename = pFound->GetName();
		return (const char*)pFound->GetValue();
	}

	// Find the full path to an actual file
	char szFullPath[512];
	int n;
	int nCount = m_pSourceSearchPaths->GetSize();
	bool bFoundIt = false;
	for(n = 0; n < nCount; n++)
	{
		// Copy the path to the buffer
		const char* szPath = (const char*)m_pSourceSearchPaths->GetPointer(n);
		strcpy(szFullPath, szPath);
		int nPathLen = strlen(szFullPath);
		if(nPathLen > 0 && (szFullPath[nPathLen - 1] != '/' && szFullPath[nPathLen - 1] != '\\'))
		{
#ifdef WIN32
			strcpy(szFullPath + nPathLen, "\\");
#else // WIN32
			strcpy(szFullPath + nPathLen, "/");
#endif // !WIN32
			nPathLen++;
		}

		// Append successively shrinking parts of the filename until we find it
		int i = 0;
		while(true)
		{
			strcpy(szFullPath + nPathLen, szFilename + i);
			if(GFile::DoesFileExist(szFullPath))
			{
				bFoundIt = true;
				break;
			}
			while(szFilename[i] != '/' && szFilename[i] != '\\' && szFilename[i] != '\0')
				i++;
			if(szFilename[i] == '\0')
				break;
			i++;
		}
		if(bFoundIt)
			break;
	}
	if(!bFoundIt)
		return NULL;

	// Check the cache
	CacheEntry likeMe2(szFullPath, NULL);
	pEntry = (CacheEntry*)m_pSourceCacheFound->GetNode(&likeMe2, &nIndex);
	if(pEntry)
	{
		// Add a specified cache entry
		CacheEntry* pSpecifiedEntry = new CacheEntry(szFilename, pEntry);
		m_pSourceCacheSpecified->Insert(pSpecifiedEntry);
		*szOutActualFilename = pEntry->GetName();
		return (const char*)pEntry->GetValue();
	}

	// Load the file
	DebugSourceManagerAssertingErrorHandler errorHandler;
	char* pFile = LoadFileToBuffer(&errorHandler, szFullPath);
	if(!pFile)
		return NULL;

	// Cache the results
	CacheEntry* pFoundEntry = new CacheEntry(szFullPath, pFile);
	m_pSourceCacheFound->Insert(pFoundEntry);
	CacheEntry* pSpecifiedEntry = new CacheEntry(szFilename, pFoundEntry);
	m_pSourceCacheSpecified->Insert(pSpecifiedEntry);
	*szOutActualFilename = (char*)pFoundEntry->GetName();
	return pFile;
}

bool DebugSourceManager::MakeSymbols(EInstrArray* pEInstrArray, EMethod* pEMethod)
{
	COFileSet* pFiles = m_pPartialProject->GetSource();
	COFile* pFile = pFiles->FindFile(m_szCurrentSource);
	if(!pFile)
	{
		DebugSourceManagerAssertingErrorHandler errorHandler;
		pFile = COFile::LoadPartialForSymbolCreation(m_szCurrentSource, &errorHandler, m_pPartialProject);
		pFiles->AddFile(pFile);
	}
	GXMLTag* pTypeTag = pEMethod->GetClass()->GetTag();
	if(!pTypeTag)
	{
		GAssert(false, "Couldn't get type tag");
		return false;
	}
	GXMLAttribute* pNameAttr = pTypeTag->GetAttribute("Name");
	if(!pNameAttr)
	{
		GAssert(false, "Type has no Name attribute");
		return false;
	}
	COClass* pClass = pFile->FindClass(pNameAttr->GetValue());
	if(!pClass)
	{
		GAssert(false, "Couldn't find that class in this file");
		return false;
	}
	EMethodSignature* pSig = pEMethod->GetSignature();
	COMethod* pMethod = pClass->FindMethod(pSig);
	if(!pMethod)
	{
		GAssert(false, "Couldn't find a method with that signature");
		return false;
	}
	EInstrArray* pEInstrArrayWithSymbols = GCompiler::PartialCompileMethod(pMethod, m_pPartialProject);
	if(!pEInstrArrayWithSymbols)
	{
		GAssert(false, "Failed to compile the partially-loaded method");
		return false;
	}
	pEInstrArray->AcquireSymbols(pEInstrArrayWithSymbols);
	delete(pEInstrArrayWithSymbols);
	return true;
}

bool DebugSourceManager::IsCurrentSource(const char* szSource)
{
	if(stricmp(m_szCurrentSource, szSource) == 0)
		return true;
	return false;
}

void DebugSourceManager::SetCurrentSource(const char* szSource)
{
	strcpy(m_szCurrentSource, szSource);
}

