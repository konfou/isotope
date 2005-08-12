/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MREALM_H__
#define __MREALM_H__

#include <stdio.h>
#include "../GClasses/GImage.h"

class GAVLTree;
class GHashTable;
class GBillboardCamera;
class MObject;
class GXMLTag;
class Model;
class MImageStore;
class GPointerArray;

// This class represents the model for a single realm (AKA world, AKA map).  Basically it just
// holds a linked list of objects.  If I ever get around to adding support for terrain maps, it
// will probably hold those too.
class MRealm
{
protected:
	GPointerArray* m_pObjects;
	GHashTable* m_pObjectsByID;
	MObject* m_pClosestObject;
	float m_fXMin, m_fXMax, m_fYMin, m_fYMax;
	GImage* m_pTerrain;
	Model* m_pModel;

public:
	MRealm(Model* pModel);
	~MRealm();

	int GetObjectCount();
	MObject* GetObj(int n);

	void Update(double time, GBillboardCamera* pCamera, MObject* pAvatar);

	// Remove the object, and return the object previous to it in the list
	void RemoveObject(int nIndex, MObject* pOldOb, int uid);

	// Remove the object with the specified ID (if one exists) and notify the model about the change
	void RemoveObject(int nConnection, int uid);

	// Replaces whatever object has the same UID with this one
	void ReplaceObject(int nConnection, MObject* pOb);

	// Returns the object closest to the avatar
	MObject* GetClosestObject() { return m_pClosestObject; }

	// Returns the object closest to the specified point
	MObject* GetClosestObject(float x, float y);

	// Fills pObjects with a list of all objects that lie partially or completely within the specified rectangle
	void GetObjectsWithinBox(GPointerArray* pObjects, float xMin, float yMin, float xMax, float yMax);

	// Finds an object by ID
	MObject* GetObjectByID(int nID);

	void SetMapRect(float fXMin, float fXMax, float fYMin, float fYMax)
	{
		m_fXMin = fXMin;
		m_fXMax = fXMax;
		m_fYMin = fYMin;
		m_fYMax = fYMax;
	}

	void GetMapRect(float* fXMin, float* fXMax, float* fYMin, float* fYMax)
	{
		*fXMin = m_fXMin;
		*fXMax = m_fXMax;
		*fYMin = m_fYMin;
		*fYMax = m_fYMax;
	}

	void FromXml(GXMLTag* pTag, MImageStore* pStore);

	GImage* GetTerrainMap() { return m_pTerrain; }

	void SortObjects(GBillboardCamera* pCamera);

	static int CompareByCameraDistance(GBillboardCamera* pCamera, MObject* pThis, MObject* pThat);

protected:
	int IdToIndex(int nID);
	void SetTerrainMap(GImage* pImage);
	void CheckObjects();
};

#endif // __MREALM_H__
