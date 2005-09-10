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
#include "GameEngine.h"

MRealm::MRealm(Model* pModel)
{
	GAssert(m_pModel, "Model can't be NULL");
	m_pModel = pModel;
	m_pObjectsByID = NULL;
	m_pClosestObject = NULL;
	m_fXMin = -100000;
	m_fXMax = 100000;
	m_fYMin = -100000;
	m_fYMax = 100000;
	m_pTerrain = new GImage();
	m_pObjects = new GPointerArray(64);
	m_pObjectsByID = new GHashTable(107);
}

MRealm::~MRealm()
{
	int n;
	for(n = m_pObjects->GetSize() - 1; n >= 0; n--)
	{
		MObject* pOb = (MObject*)m_pObjects->GetPointer(n);
		delete(pOb);
	}
	m_pObjects->Clear();
	delete(m_pObjectsByID);
	delete(m_pObjects);
	delete(m_pTerrain);
}

int MRealm::GetObjectCount()
{
	return m_pObjects->GetSize();
}

MObject* MRealm::GetObj(int n)
{
	return (MObject*)m_pObjects->GetPointer(n);
}

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

GBillboardCamera* g_pCamera = NULL;

int MObjectComparer(void* pA, void* pB)
{
	return MRealm::CompareByCameraDistance(g_pCamera, (MObject*)pA, (MObject*)pB);
}

void MRealm::SortObjects(GBillboardCamera* pCamera)
{
	// Sort the objects
	GAssert(g_pCamera == NULL, "Looks like there are there multiple threads sorting the objects--yikes!");
	g_pCamera = pCamera;
	m_pObjects->Sort(MObjectComparer);
	g_pCamera = NULL;
	RebuildIDTable();
}

void MRealm::RebuildIDTable() // todo: rebuild it lazily for perf
{
	// Rebuild the id table
	delete(m_pObjectsByID);
	int nSize = m_pObjects->GetSize();
	m_pObjectsByID = new GHashTable(MAX(107, nSize * 2));
	MObject* pOb;
	int n;
	for(n = nSize - 1; n >= 0; n--)
	{
		pOb = (MObject*)m_pObjects->GetPointer(n);
		m_pObjectsByID->Add(pOb->GetUid(), (void*)n);
	}
}

void MRealm::CheckObjects()
{
#ifdef _DEBUG
	int n, nIndex;
	for(n = m_pObjects->GetSize() - 1; n >= 0; n--)
	{
		MObject* pOb = (MObject*)m_pObjects->GetPointer(n);
		if(m_pObjectsByID->Get(pOb->GetUid(), (void**)&nIndex))
		{
			GAssert(nIndex == n, "ID wrong");
		}
		else
			GAssert(false, "ID not in table");
	}
#endif // _DEBUG
}

// Note: this is called only by the client model.  The server model never calls it.
void MRealm::Update(double time, GBillboardCamera* pCamera, MObject* pAvatarObject)
{
	// Update all the objects
	float fDistance;
	m_pClosestObject = NULL;
	float fClosestDistance = (float)1e20;
	MObject* pNext;
	MObject* pOb;
	int nOutOfOrderCount = 0;
	int nOutOfOrderMax = (int)(m_pObjects->GetSize() * .1);
	int n;
	for(n = m_pObjects->GetSize() - 1; n >= 0; n--)
	{
		pOb = (MObject*)m_pObjects->GetPointer(n);

		// swap with neighbor if it's farther from the camera
		if(nOutOfOrderCount < nOutOfOrderMax && n > 0)
		{
			pNext = (MObject*)m_pObjects->GetPointer(n - 1);
			if(pNext && CompareByCameraDistance(pCamera, pOb, pNext) < 0)
			{
				int nObUid = pOb->GetUid();
				int nNextUid = pNext->GetUid();
				m_pObjectsByID->Remove(nObUid);
				m_pObjectsByID->Remove(nNextUid);
				m_pObjects->SetPointer(n, pNext);
				m_pObjects->SetPointer(n - 1, pOb);
				m_pObjectsByID->Add(nObUid, (void*)(n - 1));
				m_pObjectsByID->Add(nNextUid, (void*)n);
				pOb = pNext;
				nOutOfOrderCount++;
			}
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

		// If several objects were removed in the call to Update, "n" might have a bogus value
		if(n > m_pObjects->GetSize())
			break;
	}

	// Sort objects if necessary
	if(nOutOfOrderCount >= nOutOfOrderMax)
		SortObjects(pCamera);
}

void MRealm::GetObjectsWithinBox(GPointerArray* pObjects, float xMin, float yMin, float xMax, float yMax)
{
	GPosSize* pPosSize;
	MObject* pOb;
	int n;
	for(n = m_pObjects->GetSize() - 1; n >= 0; n--)
	{
		pOb = (MObject*)m_pObjects->GetPointer(n);
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
	int n;
	for(n = m_pObjects->GetSize() - 1; n >= 0; n--)
	{
		pOb = (MObject*)m_pObjects->GetPointer(n);
		f = pOb->GetDistanceSquared(x, y);
		if(f < fClosest)
		{
			pClosest = pOb;
			fClosest = f;
		}
	}
	return pClosest;
}

int MRealm::IdToIndex(int nID)
{
	int nIndex;
	if(m_pObjectsByID->Get(nID, (void**)&nIndex))
	{
		GAssert(nIndex >= 0 && nIndex < m_pObjects->GetSize(), "out of range");
		return nIndex;
	}
	return -1;
}

MObject* MRealm::GetObjectByID(int nID)
{
	int nIndex = IdToIndex(nID);
	if(nIndex >= 0)
		return (MObject*)m_pObjects->GetPointer(nIndex);
	return NULL;
}

void MRealm::RemoveObject(int nConnection, int uid)
{
	int nIndex = IdToIndex(uid);
	if(nIndex >= 0)
	{
		MObject* pOldOb = (MObject*)m_pObjects->GetPointer(nIndex);
		if(!m_pModel->OnReplaceObject(nConnection, pOldOb, NULL))
			return;
		if(m_pClosestObject == pOldOb)
			m_pClosestObject = NULL;
		RemoveObject(nIndex, pOldOb, uid);
	}
}

void MRealm::RemoveObject(int nIndex, MObject* pOldOb, int uid)
{
	m_pObjects->DeleteCell(nIndex);
	delete(pOldOb);
	RebuildIDTable();
}

void MRealm::ReplaceObject(int nConnection, MObject* pNewOb)
{
	int uid = pNewOb->GetUid();
	int nIndex = IdToIndex(uid);
	MObject* pOldOb = NULL;
	if(nIndex >= 0)
		pOldOb = (MObject*)m_pObjects->GetPointer(nIndex);
	if(!m_pModel->OnReplaceObject(nConnection, pOldOb, pNewOb))
	{
		delete(pNewOb);
		return;
	}
	if(nIndex >= 0)
	{
		if(m_pClosestObject == pOldOb)
			m_pClosestObject = pNewOb;
		GPosSize* pPosSize = pOldOb->GetGhostPos();
		pNewOb->SetGhostPos(pPosSize->x, pPosSize->y);
		delete(pOldOb);
		m_pObjects->SetPointer(nIndex, pNewOb);
	}
	else
	{
		float x, y;
		pNewOb->GetPos(&x, &y);
		pNewOb->SetGhostPos(x, y);
		m_pObjectsByID->Add(uid, (void*)m_pObjects->GetSize());
		m_pObjects->AddPointer(pNewOb);
	}
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
		if(!pVH)
			GameEngine::ThrowError("The terrain is specified as \"%s\" but there is no image with that ID", pAttr->GetValue());
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

