/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MRealm.h"
#include "../GClasses/GHashTable.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GBillboardCamera.h"
#include "../GClasses/GArray.h"
#include "MObject.h"
#include "MStore.h"
#include "MGameClient.h"
#include "MGameImage.h"

MRealm::MRealm(Model* pModel)
{
	GAssert(m_pModel, "Model can't be NULL");
	m_pModel = pModel;
	m_pFirstObject = NULL;
	m_pNextObject = NULL;
	m_pObjectsByID = NULL;
	m_pClosestObject = NULL;
	m_fXMin = -100000;
	m_fXMax = 100000;
	m_fYMin = -100000;
	m_fYMax = 100000;
	m_pTerrain = new GImage();
}

MRealm::~MRealm()
{
	UnloadAllObjects();
	delete(m_pTerrain);
}

void MRealm::UnloadAllObjects()
{
	while(m_pFirstObject)
	{
		MObject* pOb = m_pFirstObject;
		UnlinkObject(pOb);
		delete(pOb);
	}
	delete(m_pObjectsByID);
	m_pObjectsByID = NULL;
}

void MRealm::LinkObject(MObject* pPrev, MObject* pOb)
{
	if(pPrev)
		pOb->m_pNext = pPrev->m_pNext;
	else
		pOb->m_pNext = m_pFirstObject;
	pOb->m_pPrev = pPrev;
	if(pPrev)
		pPrev->m_pNext = pOb;
	else
		m_pFirstObject = pOb;
	if(pOb->m_pNext)
		pOb->m_pNext->m_pPrev = pOb;
}

void MRealm::UnlinkObject(MObject* pOb)
{
	if(pOb->m_pPrev)
		pOb->m_pPrev->m_pNext = pOb->m_pNext;
	else
	{
		GAssert(m_pFirstObject = pOb, "That object is not in the list");
		m_pFirstObject = pOb->m_pNext;
	}
	if(pOb->m_pNext)
		pOb->m_pNext->m_pPrev = pOb->m_pPrev;
	pOb->m_pPrev = NULL;
	pOb->m_pNext = NULL;
}
/*
// todo: remove this method and only use "ReplaceObject"
void MRealm::AddObject(MObject* pOb)
{
	LinkObject(NULL, pOb);
	if(!m_pObjectsByID)
			m_pObjectsByID = new GHashTable(53);
	m_pObjectsByID->Add(pOb->GetUid(), pOb);
}
*/
/*static*/ int MRealm::CompareByCameraDistance(GBillboardCamera* pCamera, MObject* pThis, MObject* pThat)
{
	float x1, y1, x2, y2;
	pThis->GetPos(&x1, &y1);
	pThat->GetPos(&x2, &y2);
	if(pCamera->CalculateDistanceFromCamera(x1, y1) < pCamera->CalculateDistanceFromCamera(x2, y2))
		return -1;
	else
		return 1;
}

// Note: this is called only by the client model.  The server model never calls it.
void MRealm::Update(double time, GBillboardCamera* pCamera, MObject* pAvatarObject)
{
	// Update all the objects
	float fDistance;
	m_pClosestObject = NULL;
	float fClosestDistance = (float)1e20;
	MObject* pOb = m_pFirstObject;
	while(pOb)
	{
		// swap with neighbor if it's farther from the camera
		m_pNextObject = pOb->GetNext();
		if(m_pNextObject && CompareByCameraDistance(pCamera, pOb, m_pNextObject) < 0)
		{
			// Swap pOb with m_pNextObject
			UnlinkObject(pOb);
			LinkObject(m_pNextObject, pOb);
			pOb = m_pNextObject;
			m_pNextObject = pOb->GetNext();
		}

		// Check for closest object
		if(pOb->IsTangible() && pAvatarObject)
		{
			fDistance = pAvatarObject->GetDistanceSquared(pOb);
			if(fDistance < fClosestDistance)
			{
				fClosestDistance = fDistance;
				m_pClosestObject = pOb;
			}
		}

		// Update pOb
		pOb->Update(time);

		// Move to the next one
		pOb = m_pNextObject;
	}
}

void MRealm::GetObjectsWithinBox(GPointerArray* pObjects, float xMin, float yMin, float xMax, float yMax)
{
	GPosSize* pPosSize;
	MObject* pOb;
	for(pOb = m_pFirstObject; pOb; pOb = pOb->GetNext())
	{
		pPosSize = pOb->GetGhostPos();
		if(pPosSize->x + pPosSize->sx / 2 >= xMin && 
			pPosSize->y + pPosSize->sy >= yMin &&
			pPosSize->x - pPosSize->sx / 2 <= xMax &&
			pPosSize->y <= yMax)
		{
			if(pOb->IsTangible())
				pObjects->AddPointer(pOb);
		}
	}
}

MObject* MRealm::GetClosestObject(float x, float y)
{
	float fClosest = 1e6;
	float f;
	MObject* pClosest = NULL;
	MObject* pOb;
	for(pOb = m_pFirstObject; pOb; pOb = pOb->GetNext())
	{
		f = pOb->GetDistanceSquared(x, y);
		if(f < fClosest)
		{
			pClosest = pOb;
			fClosest = f;
		}
	}
	return pClosest;
}

MObject* MRealm::GetObjectByID(int nID)
{
	MObject* pOb;
	if(!m_pObjectsByID)
		m_pObjectsByID = new GHashTable(53);
	if(m_pObjectsByID->Get(nID, (void**)&pOb))
		return pOb;
	return NULL;
}

void MRealm::RemoveObject(int nConnection, int uid)
{
	MObject* pOldOb = GetObjectByID(uid);
	if(pOldOb)
	{
		if(!m_pModel->OnReplaceObject(nConnection, pOldOb, NULL))
			return;
		if(m_pClosestObject == pOldOb)
			m_pClosestObject = NULL;
		if(m_pNextObject == pOldOb)
			m_pNextObject = NULL;
		RemoveObject(pOldOb, uid);
	}
}

MObject* MRealm::RemoveObject(MObject* pOldOb, int uid)
{
	m_pObjectsByID->Remove(uid);
	MObject* pPrev = pOldOb->GetPrev();
	UnlinkObject(pOldOb);
	delete(pOldOb);
	return pPrev;
}

void MRealm::ReplaceObject(int nConnection, MObject* pNewOb)
{
	int uid = pNewOb->GetUid();
	MObject* pOldOb = GetObjectByID(uid);
	if(!m_pModel->OnReplaceObject(nConnection, pOldOb, pNewOb))
	{
		delete(pNewOb);
		return;
	}
	MObject* pPrev = NULL;
	if(pOldOb)
	{
		if(m_pClosestObject == pOldOb)
			m_pClosestObject = pNewOb;
		if(m_pNextObject == pOldOb)
			m_pNextObject = NULL;
		GPosSize* pPosSize = pOldOb->GetGhostPos();
		pNewOb->SetGhostPos(pPosSize->x, pPosSize->y);
		pPrev = RemoveObject(pOldOb, uid);
	}
	else
	{
		float x, y;
		pNewOb->GetPos(&x, &y);
		pNewOb->SetGhostPos(x, y);
	}
	m_pObjectsByID->Add(uid, pNewOb);
	LinkObject(pPrev, pNewOb);
}

void MRealm::SetTerrainMap(GImage* pImage)
{
	// Calculate grayscale value for the terrain height
	GColor c, c2;
	int x, y, dx, dy;
	int h = pImage->GetHeight();
	int w = pImage->GetWidth();
	m_pTerrain->SetSize(pImage->GetWidth(), pImage->GetHeight());
	for(y = 0; y < h; y++)
	{
		for(x = 0; x < w; x++)
		{
			c = pImage->GetPixel(x, y);
			m_pTerrain->SetPixel(x, y, (77 * gRed(c) + 150 * gGreen(c) + 29 * gBlue(c)) | 0xffff0000);
		}
	}

	// Determine angle with neighbors for terrain shading
	unsigned int diff;
	unsigned int shadeHoriz;
	unsigned int shadeVert;
	unsigned int mask;
	w--;
	h--;
	for(y = 1; y < h; y++)
	{
		for(x = 1; x < w; x++)
		{
			shadeHoriz = 0;
			shadeVert = 0;
			c = m_pTerrain->GetPixel(x, y) & 0xffff;
			for(dy = -1; dy <= 1; dy++)
			{
				for(dx = (dy == 0 ? -1 : 0); dx <= (dy == 0 ? 1 : 0); dx += 2)
				{
					c2 = m_pTerrain->GetPixel(x + dx, y + dy) & 0xffff;
					if(c2 > c)
						diff = c2 - c;
					else
						diff = c - c2;
					if(dy == 0)
						shadeHoriz += diff;
					else
						shadeVert += diff;
				}
			}
			shadeHoriz /= 2;
			shadeVert /= 2;
			shadeHoriz >>= 4;
			if(shadeHoriz > 0xff)
				shadeHoriz = 0xff;
			shadeVert >>= 5;
			if(shadeVert > 0xff)
				shadeVert = 0xff;
			mask = ((0xff - shadeHoriz) << 8) | (0xff - shadeVert);
			m_pTerrain->SetPixel(x, y, (mask << 16) | c);
		}
	}
}

void MRealm::FromXml(GXMLTag* pTag, MImageStore* pStore)
{
	// Load the map boundaries
	GXMLAttribute* pAttr;
	pAttr = pTag->GetAttribute("xmin");
	if(pAttr)
		m_fXMin = (float)atof(pAttr->GetValue());
	pAttr = pTag->GetAttribute("xmax");
	if(pAttr)
		m_fXMax = (float)atof(pAttr->GetValue());
	pAttr = pTag->GetAttribute("ymin");
	if(pAttr)
		m_fYMin = (float)atof(pAttr->GetValue());
	pAttr = pTag->GetAttribute("ymax");
	if(pAttr)
		m_fYMax = (float)atof(pAttr->GetValue());

	// Load the terrain map
	pAttr = pTag->GetAttribute("terrain");
	if(pAttr && pStore)
	{
		VarHolder* pVH = pStore->GetVarHolder(pAttr->GetValue());
		SetTerrainMap(&((MGameImage*)pVH->GetGObject())->m_value);
	}
	else
	{
		GImage tmp;
		tmp.SetSize(1, 1);
		tmp.SetPixel(0, 0, 0);
		SetTerrainMap(&tmp);
	}
}

