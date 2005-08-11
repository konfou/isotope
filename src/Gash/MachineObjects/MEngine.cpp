/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "MEngine.h"
#include "MArray.h"
#include "MLibrary.h"
#include "MachineObjects.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GString.h"
#include "../Include/GashLib.h"
#include "../CodeObjects/Project.h"
#include "../Engine/Error.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN
#include "../BuiltIns/GashString.h"
#include "../Engine/GCompiler.h"

void RegisterMEngine(GConstStringHashTable* pTable)
{
	pTable->Add("method collectTheGarbage()", new EMethodPointerHolder((MachineMethod0)&GashEngine::collectTheGarbage));
	pTable->Add("method getClass(&Integer, Object)", new EMethodPointerHolder((MachineMethod2)&GashEngine::getClass));
	pTable->Add("method getObjectCount(&Integer)", new EMethodPointerHolder((MachineMethod1)&GashEngine::getObjectCount));
	pTable->Add("method getRandomNumber(&Integer)", new EMethodPointerHolder((MachineMethod1)&GashEngine::getRandomNumber));
	pTable->Add("method getRefs(&Integer, Object)", new EMethodPointerHolder((MachineMethod2)&GashEngine::getRefs));
	pTable->Add("method getGlobalObject(!Object)", new EMethodPointerHolder((MachineMethod1)&GashEngine::getGlobalObject));
	pTable->Add("method setGlobalObject(Object)", new EMethodPointerHolder((MachineMethod1)&GashEngine::setGlobalObject));
	pTable->Add("method buildProject(!Library, Project)", new EMethodPointerHolder((MachineMethod2)&GashEngine::buildProject));
}

class EngineErrorHandler : public ErrorHandler
{
protected:
	Engine* m_pEngine;

public:
	EngineErrorHandler(Engine* pEngine)
	{
		m_pEngine = pEngine;
	}

	virtual void OnError(ErrorHolder* pErrorHolder)
	{
		GString s;
		pErrorHolder->ToString(&s);
		m_pEngine->ThrowEngineError(s.GetString());
	}
};

char* LoadFileToBuffer(ErrorHandler* pErrorHandler, const char* szFilename); // todo: put this declaration in some header file

void GashEngine::buildProject(Engine* pEngine, EVar* pLibrary, EVar* pProject)
{
	// Create a list of filenames
	MArray* pFilenameList = (MArray*)pProject->pObjectObject->arrFields[0];
	GPointerArray* pArray = pFilenameList->GetArray();
	int nCount = pArray->GetSize();
	StringHolderArray filenames(nCount);
	int n;
	for(n = 0; n < nCount; n++)
	{
		GString* pString = &((GashString*)pArray->GetPointer(n))->m_value;
		filenames.m_pData[n] = new char[pString->GetLength() + 1];
		pString->GetAnsi(filenames.m_pData[n]);
	}

	// Load all the files
	EngineErrorHandler eh(pEngine);
	StringHolderArray files(nCount);
	int nFileSize;
	for(n = 0; n < nCount; n++)
		files.m_pData[n] = pEngine->LoadFile(filenames.m_pData[n], &nFileSize);

	// Create the project
	Holder<COProject*> hProj(new COProject("Untitled.proj"));
	if(!hProj.Get()->LoadSources((const char**)files.m_pData, (const char**)filenames.m_pData, nCount, &eh))
		pEngine->ThrowEngineError(L"Failed to create project"); // todo: change to a compile error

	// Build it
	CompileError errorHolder;
	Library* pLib = GCompiler::Compile(hProj.Drop(), true, &errorHolder);
	if(!pLib)
		pEngine->ThrowCompileError(&errorHolder);
	pEngine->SetVar(pLibrary, new MLibrary(pEngine, pLib));
}
