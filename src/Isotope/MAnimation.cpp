/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MObject.h"
#include "MAnimation.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GHashTable.h"
#include "GameEngine.h"
#include "MGameClient.h"
#include "MStore.h"
#include "MScriptEngine.h"
#include "../Gasp/Include/GaspEngine.h"
#include "../Gasp/BuiltIns/GaspFloat.h"
#include "../Gasp/BuiltIns/GaspStream.h"
#include "MGameImage.h"

class MAnimationFrame
{
public:
	double m_endTime;
	GRect m_rect;

	MAnimationFrame();
	~MAnimationFrame();

	void CopyData(MAnimationFrame* pThat);
	void FromXml(GXMLTag* pTag, int width, int height);
	void Scale(float fac);
};

MAnimationFrame::MAnimationFrame()
{
}

MAnimationFrame::~MAnimationFrame()
{
}

void MAnimationFrame::Scale(float fac)
{
	m_rect.x = (int)(m_rect.x * fac);
	m_rect.y = (int)(m_rect.y * fac);
	m_rect.w = (int)(m_rect.w * fac);
	m_rect.h = (int)(m_rect.h * fac);
}

void MAnimationFrame::CopyData(MAnimationFrame* pThat)
{
	m_rect = pThat->m_rect;
	m_endTime = pThat->m_endTime;
}

void MAnimationFrame::FromXml(GXMLTag* pTag, int width, int height)
{
	GXMLAttribute* pAttrX = pTag->GetAttribute("x");
	GXMLAttribute* pAttrY = pTag->GetAttribute("y");
	GXMLAttribute* pAttrW = pTag->GetAttribute("w");
	GXMLAttribute* pAttrH = pTag->GetAttribute("h");
	GXMLAttribute* pAttrT = pTag->GetAttribute("t");
	if(!pAttrX || !pAttrY || !pAttrW || !pAttrH || !pAttrT)
		GameEngine::ThrowError("Missing expected XML attribute in animation frame");
	m_rect.x = atoi(pAttrX->GetValue());
	m_rect.y = atoi(pAttrY->GetValue());
	m_rect.w = atoi(pAttrW->GetValue());
	m_rect.h = atoi(pAttrH->GetValue());
	m_endTime = atof(pAttrT->GetValue());
	if(m_rect.x < 0 || m_rect.y < 0 || m_rect.x + m_rect.w > width || m_rect.y + m_rect.h > height)
		GameEngine::ThrowError("Frame out of the image range");
}


// -------------------------------------------------------------------------

void RegisterMAnimation(GConstStringHashTable* pTable)
{
	pTable->Add("method getFrame(!GImage, &Rect)", new EMethodPointerHolder((MachineMethod2)&MAnimation::getFrame));
	pTable->Add("method getColumnFrame(!GImage, &Rect, Float)", new EMethodPointerHolder((MachineMethod3)&MAnimation::getColumnFrame));
	pTable->Add("method !newCopy(Animation)", new EMethodPointerHolder((MachineMethod1)&MAnimation::newCopy));
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&MAnimation::toStream));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&MAnimation::fromStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&MAnimation::setRefs));
	pTable->Add("method &advanceTime(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&MAnimation::advanceTime));
	pTable->Add("method &setTime(Float)", new EMethodPointerHolder((MachineMethod1)&MAnimation::setTime));
}


GHashTable* MAnimation::s_pAllAnimations = new GHashTable(83);

MAnimation::MAnimation(Engine* pEngine)
 : WrapperObject(pEngine, "Animation")
{
	m_pImage = NULL;
	m_pFrames = NULL;
	m_nCurrentFrame = 0;
	m_nFrames = 0;
	m_time = 0;
	m_nColumns = 1;
	m_fDirection = 0;
	m_szID = NULL;
	m_nUID = GameEngine::GetUid();
	s_pAllAnimations->Add(m_nUID, this);
}

MAnimation::~MAnimation()
{
	s_pAllAnimations->Remove(m_nUID);
	delete [] m_pFrames;
	delete(m_szID);
}

void MAnimation::Scale(float fac)
{
	int n;
	for(n = 0; n < m_nFrames; n++)
		m_pFrames[n].Scale(fac);
}

void MAnimation::CopyGuts(MAnimation* pThat)
{
	m_pFrames = new MAnimationFrame[pThat->m_nFrames];
	int n;
	for(n = 0; n < pThat->m_nFrames; n++)
		m_pFrames[n].CopyData(&pThat->m_pFrames[n]);
	m_nFrames = pThat->m_nFrames;
	m_nCurrentFrame = 0;
	m_time = 0;
	m_nColumns = pThat->m_nColumns;
	m_fDirection = pThat->m_fDirection;
}

void MAnimation::CopyDataAcrossEngines(MAnimation* pThat, VarHolder* pImage, const char* szID)
{
	m_pImage = pImage;
	m_szID = new char[strlen(szID) + 1];
	strcpy(m_szID, szID);
	CopyGuts(pThat);
}

void MAnimation::CopyData(MAnimation* pThat)
{
	m_pImage = pThat->m_pImage;
	m_szID = new char[strlen(pThat->m_szID) + 1];
	strcpy(m_szID, pThat->m_szID);
	CopyGuts(pThat);
}

void MAnimation::newCopy(Engine* pEngine, EVar* pThat)
{
	MAnimation* pAnimation = new MAnimation(pEngine);
	pEngine->SetThis(pAnimation);
	pAnimation->CopyData((MAnimation*)pThat->pOb);
}

void MAnimation::toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
{
	GQueue* pQ = &pOutBlob->pStreamObject->m_value;
	pQ->Push(m_nUID);
	pQ->Push(m_fDirection);
	pQ->Push((int)strlen(m_szID));
	pQ->Push(m_szID);
}

void MAnimation::fromStream(Engine* pEngine, EVar* pStream)
{
	// Deserialize the data
	GQueue* pQ = &pStream->pStreamObject->m_value;
	int nUID;
	int len = 0;
	float fDirection;
	pQ->Pop(&nUID);
	pQ->Pop(&fDirection);
	pQ->Pop(&len);
	if(len <= 0 || len > 512)
		GameEngine::ThrowError("Bad Animation ID");
	if(pQ->GetSize() < len)
		GameEngine::ThrowError("Bad serialized animation");
	char* szID = (char*)alloca(len + 1);
	int n;
	for(n = 0; n < len; n++)
		pQ->Pop(&szID[n]);
	szID[len] = '\0';

	// Find or make the right object
	MAnimation* pNewAnim = NULL;
	if(!s_pAllAnimations->Get(nUID, (void**)&pNewAnim)) // Check the hash table for an existing object
	{
		MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
		if(pGameClient)
		{
			// Deserializing this animation on the client for the first time, so find it in
			// the store and make a copy
			MAnimationStore* pStore = pGameClient->GetAnimations();
			int index = pStore->GetIndex(szID);
			if(index < 0)
			{
				MAnimationStore* pGlobalStore = GameEngine::GetGlobalAnimationStore();
				if(pGlobalStore->GetIndex(szID) >= 0)
				{
					pStore->AddAnimation(((MVM*)pEngine)->m_pScriptEngine, pGameClient->GetImages(), szID);
					index = pStore->GetIndex(szID);
				}
				else
					GameEngine::ThrowError("No such animation: %s", szID);
			}
			MAnimation* pAnim = (MAnimation*)pStore->GetVarHolder(index)->GetGObject();
			GAssert(pAnim, "Failed to retrieve animation");
			pNewAnim = new MAnimation(pEngine);
			pNewAnim->CopyData(pAnim);
			pNewAnim->m_nUID = nUID;
		}
		else
		{
			// Deserializing on the server, so just make a place-holder object for the ID and UID
			pNewAnim = new MAnimation(pEngine);
			pNewAnim->m_nUID = nUID;
			pNewAnim->m_szID = new char[len + 1];
			strcpy(pNewAnim->m_szID, szID);
		}
	}
	pNewAnim->m_fDirection = fDirection;
	pEngine->SetThis(pNewAnim);
}

/*static*/ MAnimation* MAnimation::FromXml(GXMLTag* pTag, MScriptEngine* pScriptEngine, const char* szID, MImageStore* pImageStore)
{
	MAnimation* pAnim = new MAnimation(pScriptEngine->GetEngine());
	GXMLAttribute* pColumnsAttr = pTag->GetAttribute("Columns");
	if(pColumnsAttr)
		pAnim->SetColumnCount(atoi(pColumnsAttr->GetValue()));

	// Load the image
	GXMLAttribute* pAttr = pTag->GetAttribute("image");
	if(!pAttr)
		GameEngine::ThrowError("Animation missing expected \"image\" attribute");

	int nImageIndex = pImageStore->GetIndex(pAttr->GetValue());
	pAnim->m_pImage = pImageStore->GetVarHolder(nImageIndex);
	if(!pAnim->m_pImage)
		GameEngine::ThrowError("There is no image with the ID: %s", pAttr->GetValue());

	// Load the frames
	pAnim->m_nFrames = pTag->GetChildTagCount();
	pAnim->m_pFrames = new MAnimationFrame[pAnim->m_nFrames];
	GXMLTag* pChildTag;
	int n = 0;
	for(pChildTag = pTag->GetFirstChildTag(); pChildTag; pChildTag = pTag->GetNextChildTag(pChildTag))
	{
		GImage* pImage = &((MGameImage*)pAnim->m_pImage->GetGObject())->m_value;
		pAnim->m_pFrames[n].FromXml(pChildTag, pImage->GetWidth(), pImage->GetHeight());
		if(n > 0)
			pAnim->m_pFrames[n].m_endTime += pAnim->m_pFrames[n - 1].m_endTime;
		n++;
	}
	pAnim->m_szID = new char[strlen(szID) + 1];
	strcpy(pAnim->m_szID, szID);
	GAssert(pAnim->m_pImage && pAnim->m_pImage->GetGObject(), "Invalid image");
	return pAnim;
}

bool MAnimation::AdvanceTime(double dt)
{
	GAssert(m_time >= 0 && m_time <= m_pFrames[m_nFrames - 1].m_endTime, "m_time is out of range");
	bool bRet = true;
	m_time += dt;
	if(dt >= 0)
	{
		if(m_time >= m_pFrames[m_nFrames - 1].m_endTime)
		{
			m_time -= m_pFrames[m_nFrames - 1].m_endTime * (int)(m_time / m_pFrames[m_nFrames - 1].m_endTime);
			GAssert(m_time >= 0 && m_time <= m_pFrames[m_nFrames - 1].m_endTime, "m_time is out of range");
			m_nCurrentFrame = 0;
			bRet = false;
		}
		while(m_nCurrentFrame < m_nFrames - 1 && m_time >= m_pFrames[m_nCurrentFrame].m_endTime)
			m_nCurrentFrame++;
	}
	else
	{
		if(m_time < 0)
		{
			m_time += m_pFrames[m_nFrames - 1].m_endTime * (int)((-m_time / m_pFrames[m_nFrames - 1].m_endTime) + 1);
			if(m_time >= m_pFrames[m_nFrames - 1].m_endTime)
				m_time -= m_pFrames[m_nFrames - 1].m_endTime; // some floating point rounding issue makes this necessary
			GAssert(m_time >= 0 && m_time <= m_pFrames[m_nFrames - 1].m_endTime, "m_time is out of range");
			m_nCurrentFrame = m_nFrames - 1;
			bRet = false;
		}
		while(m_nCurrentFrame > 0 && m_time < m_pFrames[m_nCurrentFrame - 1].m_endTime)
			m_nCurrentFrame--;
	}
	GAssert(m_nCurrentFrame >= 0 && m_nCurrentFrame < m_nFrames, "out of range");
	return bRet;
}

GImage* MAnimation::GetFrame(GRect* pRect)
{
	*pRect = m_pFrames[m_nCurrentFrame].m_rect;
	return &((MGameImage*)m_pImage->GetGObject())->m_value;
}

GImage* MAnimation::GetColumnFrame(GRect* pRect, float fCameraDirection)
{
	GImage* pImage = GetFrame(pRect);
	int col = (int)((fCameraDirection - m_fDirection) * m_nColumns / (float)6.28318);
	while(col < 0) // todo: this could be a perf issue
		col += m_nColumns;
	while(col >= m_nColumns) // todo: this could be a perf issue
		col -= m_nColumns;
	pRect->x += pRect->w * col;
	return pImage;
}

void MAnimation::getFrame(Engine* pEngine, EVar* pOutImage, EVar* pOutRect)
{
	if(!m_pImage)
	{
		// This animation was deserialized from a blob that the server sent
		// We now need to lazily find the image and frame information based on the ID string
		MGameClient* pGameClient = ((MVM*)pEngine)->m_pGameClient;
		if(!pGameClient)
			GameEngine::ThrowError("The server should not call this method");
		MAnimationStore* pStore = pGameClient->GetAnimations();
		int index = pStore->GetIndex(m_szID);
		if(index < 0)
			GameEngine::ThrowError("Invalid animation ID: %s", m_szID);
		VarHolder* pVH = pStore->GetVarHolder(index);
		MAnimation* pThatAnim = (MAnimation*)pVH->GetGObject();
		CopyData(pThatAnim);
	}
	pEngine->SetVar(pOutImage, m_pImage->GetGObject());
	GAssert(m_nCurrentFrame >= 0 && m_nCurrentFrame < m_nFrames, "out of range");
	GRect* pGRect = &m_pFrames[m_nCurrentFrame].m_rect;
	GAssert(pGRect->x >= 0 && pGRect->y >= 0 && pGRect->w >= 0 && pGRect->h >= 0, "looks like a bad range");
	MScriptEngine::GRectToMRect(pGRect, pOutRect->pObjectObject);
}

void MAnimation::getColumnFrame(Engine* pEngine, EVar* pOutImage, EVar* pOutRect, EVar* pCameraDirection)
{
	getFrame(pEngine, pOutImage, pOutRect);
	int col = (int)((pCameraDirection->pFloatObject->m_value - m_fDirection) * m_nColumns / (float)6.28318);
	while(col < 0) // todo: this could be a perf issue
		col += m_nColumns;
	while(col >= m_nColumns) // todo: this could be a perf issue
		col -= m_nColumns;
	((IntObject*)pOutRect->pObjectObject->arrFields[0])->m_value +=
		((IntObject*)pOutRect->pObjectObject->arrFields[2])->m_value * col;
}

void MAnimation::advanceTime(Engine* pEngine, EVar* pOutLooped, EVar* pTime)
{
	bool bLooped = AdvanceTime(pTime->pFloatObject->m_value) ? 1 : 0;
	if(pOutLooped->pIntObject)
		pOutLooped->pIntObject->m_value = bLooped;
}

void MAnimation::setTime(Engine* pEngine, EVar* pTime)
{
	m_time = 0;
	AdvanceTime(pTime->pFloatObject->m_value);
}
