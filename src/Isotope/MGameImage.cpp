/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MGameImage.h"
#include "../GClasses/GHashTable.h"
#include "../Gash/Include/GashLib.h"
#include "../Gash/BuiltIns/GashStream.h"
#include "GameEngine.h"
#include "MScriptEngine.h"
#include "MStore.h"
#include "MGameClient.h"
#include "../GClasses/GFile.h"

void RegisterMGameImage(GConstStringHashTable* pTable)
{
	pTable->Add("method !makeTextImage(String)", new EMethodPointerHolder((MachineMethod1)&MGameImage::makeTextImage));
	pTable->Add("method &setSize(Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&MGameImage::setSize));
	pTable->Add("method &setPixel(Integer, Integer, Color)", new EMethodPointerHolder((MachineMethod3)&MGameImage::setPixel));
	pTable->Add("method getPixel(&Color, Integer, Integer)", new EMethodPointerHolder((MachineMethod3)&MGameImage::getPixel));
	pTable->Add("method getRect(&Rect)", new EMethodPointerHolder((MachineMethod1)&MGameImage::getRect));
	pTable->Add("method getSize(&Integer, &Integer)", new EMethodPointerHolder((MachineMethod2)&MGameImage::getSize));
	pTable->Add("method !load(String)", new EMethodPointerHolder((MachineMethod1)&MGameImage::load));
	pTable->Add("method &setText(String)", new EMethodPointerHolder((MachineMethod1)&MGameImage::setText));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&MGameImage::fromStream));
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&MGameImage::toStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&MGameImage::setRefs));
}



void MGameImage::makeTextImage(Engine* pEngine, EVar* pText)
{
	GString* pString = &pText->pStringObject->m_value;
	char* szText = (char*)alloca(pString->GetLength() + 1);
	pString->GetAnsi(szText);
	makeTextImageHelper(pEngine, szText);
}

void MGameImage::makeTextImageHelper(Engine* pEngine, const char* szText)
{
	MGameImage* pNewImage = new MGameImage(pEngine, Text, szText);
	pEngine->SetThis(pNewImage);
	pNewImage->drawTextImage(szText);
}

void MGameImage::drawTextImage(const char* szText)
{
	int hgt = 33;
	int wid = m_value.MeasureHardTextWidth(hgt, szText, 1);
	m_value.SetSize(wid + 4, hgt + 4);
	m_value.Clear(0xd0ffffff);
	m_value.DrawBox(0, 0, wid + 3, hgt + 3, 0xff0000ff, false);
	m_value.DrawBox(0, 0, wid + 2, hgt + 2, 0xff0000ff, false);
	GRect r;
	r.x = 3;
	r.y = 2;
	r.w = wid;
	r.h = hgt;
	m_value.DrawHardText(&r, szText, 0xff000000, 1);
}

void MGameImage::setText(Engine* pEngine, EVar* pText)
{
	GString* pString = &pText->pStringObject->m_value;
	char* szText = (char*)alloca(pString->GetLength() + 1);
	pString->GetAnsi(szText);
	drawTextImage(szText);
}

void MGameImage::toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
{
	GQueue* pQ = &pOutBlob->pStreamObject->m_value;
	pQ->Push((char)m_eMode);
	pQ->Push((int)strlen(m_szText));
	pQ->Push(m_szText);
}

void MGameImage::fromStream(Engine* pEngine, EVar* pStream)
{
	// Parse the blob
	GQueue* pQ = &pStream->pStreamObject->m_value;
	char cMode;
	int nLen = 0;
	pQ->Pop(&cMode);
	if(cMode < Store || cMode > GlobalId)
		GameEngine::ThrowError("Bad Image mode");
	pQ->Pop(&nLen);
	if(nLen <= 0 || nLen > 512)
		GameEngine::ThrowError("Bad Image ID");
	char* szText = (char*)alloca(nLen + 1);
	if(pQ->GetSize() < nLen)
		GameEngine::ThrowError("Bad serialized Image");
	char* szID = (char*)alloca(nLen + 1);
	int n;
	for(n = 0; n < nLen; n++)
		pQ->Pop(&szID[n]);
	szID[nLen] = '\0';

	// Get or instantiate the image
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(pGameClient)
	{
		// Client
		MImageStore* pStore = pGameClient->GetImages();
		if(cMode == Store)
		{
			// Find the image in the store
			VarHolder* pVH = pStore->GetVarHolder(szID);
			if(!pVH)
				GameEngine::ThrowError("There is no image with the ID: %s", szID);
			pEngine->SetThis((MGameImage*)pVH->GetGObject());
		}
		else if(cMode == Text)
		{
			// Create a new text image
			GAssert(cMode == Text, "unexptected mode");
			makeTextImageHelper(pEngine, szID);
		}
		else if(cMode == Url)
		{
			pEngine->SetThis(DownloadImage(pEngine, szID));
		}
		else if(cMode == GlobalId)
		{
			MImageStore* pGlobalStore = GameEngine::GetGlobalImageStore();
			VarHolder* pVH = pGlobalStore->GetVarHolder(szID);
			MGameImage* pGameImage = (MGameImage*)pVH->GetGObject();
			MGameImage* pNewImage = new MGameImage(pEngine, GlobalId, szID);
			pNewImage->m_value.CopyImage(&pGameImage->m_value);
			pEngine->SetThis(pNewImage);
		}
	}
	else
	{
		// Server (Make an empty wrapper.  Don't bother creating the actual
		//         image because the server will never use it)
		MGameImage* pNewImage = new MGameImage(pEngine, (Mode)cMode, szID);
		pEngine->SetThis(pNewImage);
	}
}

/*static*/ MGameImage* MGameImage::DownloadImage(Engine* pEngine, const char* szUrl)
{
	// Download the file
	int nSize;
	Holder<unsigned char*> hImageFile((unsigned char*)GameEngine::LoadFileFromUrl("", szUrl, &nSize));
	unsigned char* pImageFile = hImageFile.Get();
	if(!pImageFile)
	{
		ConvertAnsiToUnicode(szUrl, wszUrl);
		pEngine->ThrowIOError(L"Failed to download image file: %s", wszUrl);
	}

	// Create the image
	MGameImage* pNewImage = new MGameImage(pEngine, Url, szUrl);
	if(!pNewImage->m_value.LoadPNGFile(pImageFile, nSize))
	{
		ConvertAnsiToUnicode(szUrl, wszUrl);
		pEngine->ThrowIOError(L"Bad PNG file: %s", wszUrl);
	}
	return pNewImage;
}

void MGameImage::load(Engine* pEngine, EVar* pFilename)
{
	// Determine the full sandboxed URL
	MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
	if(!pGameClient)
		pEngine->ThrowEngineError(L"The server shouldn't call this method");
	const char* szRemoteFolder = pGameClient->GetRemoteFolder();
	int nRemoteFolderLen = strlen(szRemoteFolder);
	GString* pString = &pFilename->pStringObject->m_value;
	char* szFullUrl = (char*)alloca(nRemoteFolderLen + pString->GetLength() + 10);
	strcpy(szFullUrl, szRemoteFolder);
	pString->GetAnsi(szFullUrl + nRemoteFolderLen);

	// Create the image
	pEngine->SetThis(DownloadImage(pEngine, szFullUrl));
}

const char* MGameImage::GetID()
{
	GAssert(m_eMode == GlobalId || m_eMode == Store, "This image has no ID");
	return m_szText;
}
