/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "MLibrary.h"
#include "MachineObjects.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GXML.h"
#include "../Include/GashLib.h"

void RegisterMLibrary(GConstStringHashTable* pTable)
{
	pTable->Add("method !load(String)", new EMethodPointerHolder((MachineMethod1)&MLibrary::load));
	pTable->Add("method save(String)", new EMethodPointerHolder((MachineMethod1)&MLibrary::save));
}

void MLibrary::load(Engine* pEngine, EVar* pFilename)
{
	int nBufferSize;
	Holder<char*> hBuffer(pEngine->LoadFile(&pFilename->pStringObject->m_value, &nBufferSize));
	char* pBuffer = hBuffer.Get();
	Library* pLibrary = Library::LoadFromBuffer(pBuffer, nBufferSize);
	if(!pLibrary)
		pEngine->ThrowIOError(L"Error loading file: %s", pFilename->pStringObject->m_value.GetString());
	pEngine->SetThis(new MLibrary(pEngine, pLibrary));
}

void MLibrary::save(Engine* pEngine, EVar* pFilename)
{
	GXMLTag* pTag = m_pLibrary->GetLibraryTag();
	FileHolder fh(NULL);
	pEngine->OpenFile(&fh, &pFilename->pStringObject->m_value, "wb");
	FILE* pFile = fh.Get();
	if(!pTag->ToFile(pFile))
		pEngine->ThrowIOError(L"Error writing file: %s", pFilename->pStringObject->m_value.GetString());
}
