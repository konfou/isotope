/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MStore.h"
#include "GameEngine.h"
#include "MAnimation.h"
#include "VWave.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GImage.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GHashTable.h"
#include "MGameImage.h"
#include "MScriptEngine.h"
#include "MSpot.h"
#include "Controller.h"


MImageStore::MImageStore()
{
	m_pStringHeap = new GStringHeap(1024);
	m_pHashTable = new GConstStringHashTable(37, false);
	m_pVarHolders = new GPointerArray(64);
}

MImageStore::~MImageStore()
{
	delete(m_pHashTable);
	delete(m_pStringHeap);
	int nCount = m_pVarHolders->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((VarHolder*)m_pVarHolders->GetPointer(n));
	delete(m_pVarHolders);
}

VarHolder* MImageStore::GetVarHolder(int nIndex)
{
	return (VarHolder*)m_pVarHolders->GetPointer(nIndex);
}

int MImageStore::GetIndex(const char* szID)
{
	int index;
	if(!m_pHashTable->Get(szID, (void**)&index))
		return -1;
	return index;
}

void MImageStore::FromXml(const char* szRemotePath, GXMLTag* pTag, MScriptEngine* pScriptEngine)
{
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		GXMLAttribute* pAttrId = pChild->GetAttribute("id");
		if(!pAttrId)
			GameEngine::ThrowError("Expected an \"id\" attribute");
		GXMLAttribute* pAttrFile = pChild->GetAttribute("File");
		if(!pAttrFile)
			GameEngine::ThrowError("Expected a \"file\" attribute");
		AddImage(szRemotePath, pScriptEngine, pAttrId->GetValue(), pAttrFile->GetValue());
	}
}

void MImageStore::AddImage(const char* szRemotePath, MScriptEngine* pScriptEngine, const char* szID, const char* szFilename)
{
	char* szIDString = m_pStringHeap->Add(szID);
	m_pHashTable->Add(szIDString, (void*)(m_pVarHolders->GetSize()));
	VarHolder* pVarHolder = pScriptEngine->LoadPNGImage(szRemotePath, szFilename, szIDString);
	m_pVarHolders->AddPointer(pVarHolder);
}

void MImageStore::AddImage(MScriptEngine* pScriptEngine, const char* szGlobalID)
{
	char* szIDString = m_pStringHeap->Add(szGlobalID);
	m_pHashTable->Add(szIDString, (void*)(m_pVarHolders->GetSize()));
	VarHolder* pVarHolder = pScriptEngine->CopyGlobalImage(szGlobalID);
	m_pVarHolders->AddPointer(pVarHolder);
}

int MImageStore::GetImageCount()
{
	return m_pVarHolders->GetSize();
}

// -------------------------------------------------------------------------------

MAnimationStore::MAnimationStore(MScriptEngine* pScriptEngine)
{
	m_pVarHolders = new GPointerArray(64);
	m_pScriptEngine = pScriptEngine;
	m_pStringHeap = new GStringHeap(1024);
	m_pHashTable = new GConstStringHashTable(37, false);
}

MAnimationStore::~MAnimationStore()
{
	delete(m_pHashTable);
	delete(m_pStringHeap);
	int nCount = m_pVarHolders->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((VarHolder*)m_pVarHolders->GetPointer(n));
	delete(m_pVarHolders);
}

int MAnimationStore::GetIndex(const char* szID)
{
	int index;
	if(!m_pHashTable->Get(szID, (void**)&index))
		return -1;
	return index;
}

VarHolder* MAnimationStore::GetVarHolder(int nIndex)
{
	return (VarHolder*)m_pVarHolders->GetPointer(nIndex);
}

void MAnimationStore::FromXml(GXMLTag* pTag, MImageStore* pImageStore)
{
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		GXMLAttribute* pAttrId = pChild->GetAttribute("id");
		if(!pAttrId)
			GameEngine::ThrowError("Expected an \"id\" attribute.");
		Holder<VarHolder*> hVarHolder(new VarHolder(m_pScriptEngine->GetEngine()));
		GAssert(hVarHolder.Get()->m_szID = "Animation", "");
		hVarHolder.Get()->SetGObject(MAnimation::FromXml(pChild, m_pScriptEngine, pAttrId->GetValue(), pImageStore));
		char* szIDString = m_pStringHeap->Add(pAttrId->GetValue());
		m_pHashTable->Add(szIDString, (void*)m_pVarHolders->GetSize());
		m_pVarHolders->AddPointer(hVarHolder.Drop());
	}
}

void MAnimationStore::AddAnimation(MScriptEngine* pScriptEngine, MImageStore* pImageStore, const char* szGlobalID)
{
	char* szIDString = m_pStringHeap->Add(szGlobalID);
	m_pHashTable->Add(szIDString, (void*)(m_pVarHolders->GetSize()));
	VarHolder* pVarHolder = pScriptEngine->CopyGlobalAnimation(pImageStore, szGlobalID);
	m_pVarHolders->AddPointer(pVarHolder);
}

int MAnimationStore::GetAnimationCount()
{
	return m_pVarHolders->GetSize();
}

// -------------------------------------------------------------------------------

MSoundStore::MSoundStore()
{
	m_pArray = new GPointerArray(64);
	m_pStringHeap = new GStringHeap(1024);
	m_pHashTable = new GConstStringHashTable(37, false);
}

MSoundStore::~MSoundStore()
{
	int n;
	int nCount = m_pArray->GetSize();
	for(n = 0; n < nCount; n++)
		delete((MSound*)m_pArray->GetPointer(n));
	delete(m_pArray);
	delete(m_pHashTable);
	delete(m_pStringHeap);
}

int MSoundStore::GetIndex(const char* szID)
{
	int index;
	if(!m_pHashTable->Get(szID, (void**)&index))
		return -1;
	return index;
}

MSound* MSoundStore::GetSound(int n)
{
	return (MSound*)m_pArray->GetPointer(n);
}

void MSoundStore::FromXml(Controller* pController, const char* szRemotePath, GXMLTag* pTag)
{
int nFootprint = 0;
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		GXMLAttribute* pAttrId = pChild->GetAttribute("id");
		if(!pAttrId)
			GameEngine::ThrowError("Expected an \"id\" attribute.");
		GXMLAttribute* pAttrFile = pChild->GetAttribute("File");
		if(!pAttrFile)
			GameEngine::ThrowError("Expected a \"file\" attribute.");
		Holder<char*> hFilename(pController->LoadFileFromUrl(szRemotePath, pAttrFile->GetValue(), NULL));
		char* szFilename = hFilename.Get();
		MSound* pSound = new MSound(szFilename);
nFootprint += pSound->GetSize();
		char* szIDString = m_pStringHeap->Add(pAttrId->GetValue());
		m_pHashTable->Add(szIDString, (void*)m_pArray->GetSize());
		m_pArray->AddPointer(pSound);
	}
printf("Total SFX footprint: %d\n", nFootprint);
}

// -------------------------------------------------------------------------------

MSpotStore::MSpotStore()
{
	m_pArray = new GPointerArray(32);
	m_pStringHeap = new GStringHeap(256);
	m_pHashTable = new GConstStringHashTable(17, false);
}

MSpotStore::~MSpotStore()
{
	int n;
	int nCount = m_pArray->GetSize();
	for(n = 0; n < nCount; n++)
		delete((MSpot*)m_pArray->GetPointer(n));
	delete(m_pArray);
	delete(m_pHashTable);
	delete(m_pStringHeap);
}

int MSpotStore::GetIndex(const char* szID)
{
	int index;
	if(!m_pHashTable->Get(szID, (void**)&index))
		return -1;
	return index;
}

MSpot* MSpotStore::GetSpot(int n)
{
	return (MSpot*)m_pArray->GetPointer(n);
}

void MSpotStore::FromXml(GXMLTag* pTag)
{
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		GXMLAttribute* pAttrId = pChild->GetAttribute("id");
		if(!pAttrId)
			GameEngine::ThrowError("Expected an \"id\" attribute.");
		float x = 0;
		float y = 0;
		GXMLAttribute* pAttrX = pChild->GetAttribute("x");
		if(pAttrX)
			x = (float)atof(pAttrX->GetValue());
		GXMLAttribute* pAttrY = pChild->GetAttribute("Y");
		if(pAttrY)
			y = (float)atof(pAttrY->GetValue());
		MSpot* pSpot = new MSpot(x, y);
		char* szIDString = m_pStringHeap->Add(pAttrId->GetValue());
		m_pHashTable->Add(szIDString, (void*)m_pArray->GetSize());
		m_pArray->AddPointer(pSpot);
	}
}

