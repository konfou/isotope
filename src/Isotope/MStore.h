/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MSTORE_H__
#define __MSTORE_H__

// ------------------------------------
//
//  The classes in this file hold collections of media (images, animations, sound effects, etc)
//
// ------------------------------------


class GPointerArray;
class GImage;
class MAnimation;
class GXMLTag;
class MSound;
class GStringHeap;
class GConstStringHashTable;
class MScriptEngine;
class VarHolder;
class MSpot;
class Controller;


class MImageStore
{
protected:
	GStringHeap* m_pStringHeap;
	GConstStringHashTable* m_pHashTable;
	GPointerArray* m_pVarHolders;

public:
	MImageStore();
	~MImageStore();

	// Returns the index of the image with the specified ID
	int GetIndex(const char* szID);

	void FromXml(const char* szRemotePath, GXMLTag* pTag, MScriptEngine* pScriptEngine);

	// Gets the image with the specified ID
	VarHolder* GetVarHolder(int nIndex);

	void AddImage(const char* szRemotePath, MScriptEngine* pScriptEngine, const char* szID, const char* szFilename);
	void AddImage(MScriptEngine* pScriptEngine, const char* szGlobalID);

	int GetImageCount();
};




class MAnimationStore
{
protected:
	GPointerArray* m_pVarHolders;
	GStringHeap* m_pStringHeap;
	GConstStringHashTable* m_pHashTable;
	MScriptEngine* m_pScriptEngine;

public:
	MAnimationStore(MScriptEngine* pScriptEngine);
	~MAnimationStore();

	// Returns the index of the animation with the specified ID
	int GetIndex(const char* szID);

	// Gets the animation with the specified index
	VarHolder* GetVarHolder(int nIndex);

	void FromXml(GXMLTag* pTag, MImageStore* pImageStore);

	void AddAnimation(MScriptEngine* pScriptEngine, MImageStore* pImageStore, const char* szGlobalID);

	int GetAnimationCount();
};




class MSoundStore
{
protected:
	GPointerArray* m_pArray;
	GStringHeap* m_pStringHeap;
	GConstStringHashTable* m_pHashTable;

public:
	MSoundStore();
	~MSoundStore();

	// Returns the index of the sound with the specified ID
	int GetIndex(const char* szID);

	// Gets the sound with the specified index
	MSound* GetSound(int n);

	void FromXml(Controller* pController, const char* szRemotePath, GXMLTag* pTag);
};





class MSpotStore
{
protected:
	GPointerArray* m_pArray;
	GStringHeap* m_pStringHeap;
	GConstStringHashTable* m_pHashTable;

public:
	MSpotStore();
	~MSpotStore();

	// Returns the index of the spot with the specified ID
	int GetIndex(const char* szID);

	// Gets the spot with the specified index
	MSpot* GetSpot(int n);

	void FromXml(GXMLTag* pTag);
};

#endif // __MSTORE_H__
