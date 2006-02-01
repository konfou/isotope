/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MOBJECT_H__
#define __MOBJECT_H__

#include "../Gasp/Include/GaspEngine.h"
#include "../GClasses/GBillboardCamera.h"

// An object's "ghost" is the sprite that is actually drawn to the screen.
// Ideally it should always be located exactly where the object is, but when the
// network is being slow and a packet comes in late the object may jump to a
// new far away position.  In such cases we interpolate the position of the
// ghost to make it drift to where the object really is.  You can tune the
// values of INTERPOLATE_DISTANCE and INTERPOLATE_RATE to ajust this drifting.
#define INTERPOLATE_DISTANCE 100 // How big of a jump is required before interpolation kicks in
#define INTERPOLATE_RATE .75 // 0 = infinitely fast drift, 1 = infinitely slow drift


class MScriptEngine;
struct GRect;

#define MOB_FIELD_UID 0
#define MOB_FIELD_TIME 1
#define MOB_FIELD_X 2
#define MOB_FIELD_Y 3
#define MOB_FIELD_Z 4
#define MOB_FIELD_SX 5
#define MOB_FIELD_SY 6
#define MOB_FIELD_SZ 7
#define MOB_FIELD_COUNT 8

// This class is used for debugging to make sure there are no memory leaks
class AllocCounter
{
public:
	AllocCounter();
	virtual ~AllocCounter();

	static int s_allocs;
	static int s_deallocs;
};

inline float absfloat(float f)
{
	return f >= 0 ? f : -f;
}

// Flags
#define MOB_Tangible 0x1 // Only tangible objects can get focus and do their action
#define MOB_Panel 0x2 // Panels are billboards that face a constant direction instead of always facing the camera
#define MOB_Solid 0x4 // Solid objects prevent the avatar from walking through them


// The MObject class represents an object in the game.  It is basically a wrapper around a
// RealmObject.  RealmObject is a class in the Gasp language.  All scripted objects inherrit
// from RealmObject.  How can you wrap a Gasp object with a C++ object?  Well, all
// instantiations of classes defined in the Gasp language are actually objects of the
// ObjectObject class, which inherits from the GObject class.  GObject is the base class for
// all objects in the Gasp language.  To be more specific, MObject wraps VarHolder which
// wraps EVar which wraps GObject.  In this case the GObject will be an ObjectObject because
// it is an instantiation of some class defined in the script.  Why are there so many layers
// of wrapping?  Well, each layer has a specific purpose.  Here's a brief summary:
//
//   MObject - (C++ class) Contains next/prev pointers so you can link the objects together
//             in a list
//   VarHolder - (C++ class) Ensures that the GObject is pinned and unpinned properly so that
//               the Gasp garbage collector will know when to clean up the object
//   EVar - (C++ class) Represents a variable in the Gasp language, which references a GObject
//   RealmObject - (Gasp class) The base class of all objects in the game
//   ObjectObject - (C++ class) All Gasp objects are really instantiations of this C++ class,
//                  which inherrits from GObject.
//   GObject - (C++ class) The base class of all types in the Gasp language.  If you're
//             careful you can manipulate Gasp objects from within C++.
class MObject
#ifdef _DEBUG
	: public AllocCounter
#endif // _DEBUG
{
friend class MGameServer;
protected:
	MScriptEngine* m_pScriptEngine;
	VarHolder m_vh;
	GPosSize m_drawPos;
	int m_nPivotHeight;
	unsigned int m_flags; // todo: should these flags be moved into Gasp code so scripts can modify them?

public:
	MObject(MScriptEngine* pScriptEngine);
	virtual ~MObject();

	// Gets an image to blit onto the viewport to represent this object
	GImage* GetFrame(GRect* pRect, GBillboardCamera* pCamera);

	// Gets the objects position
	void GetPos(float* px, float* py);

	// Inits the position of the ghost to match the position of the object.
	// See the comment by "INTERPOLATE_DISTANCE" for an explanation of a "ghost".
	void SetGhostPos(float x, float y);

	// Returns the position and size of the object's ghost
	// See the comment by "INTERPOLATE_DISTANCE" for an explanation of a "ghost".
	GPosSize* GetGhostPos();

	// Sets the objects position
	void SetPos(float x, float y, float z);

	// Sets the objects size
	void SetSize(float sx, float sy, float sz);

	// Returns the Gasp object that this MObject wraps
	ObjectObject* GetGObject() { return (ObjectObject*)m_vh.GetGObject(); }

	// Sets the Gasp object to wrap
	void SetGObject(GObject* pOb) { m_vh.SetGObject(pOb); }

	// Tells the object to update itself.  (Calls into Gasp code to do the update)
	void Update(double time);

	// Returns the last time this object was updated
	double GetTime();

	// Sets the time when this object was last updated
	void SetTime(double time);

	// Returns a number that identifies this object
	int GetUid();

	// Sets the UID on this object
	void SetUid(int uid);

	// Returns a number that identifies this object
	static int GetUid(ObjectObject* pObj);

	MObject* NewAvatar();

	// Determines the square of the distance between two objects
	float GetDistanceSquared(MObject* pThat);

	// Determines the square of the distance to the specified coordinates
	float GetDistanceSquared(float x, float y);

	// Returns whether or not this object can be touched by the avatar.  Most objects
	// are tangible.  Intangible objects include the avatar, hint clouds, the goal marker flag
	bool IsTangible() { return (m_flags & MOB_Tangible) ? true : false; }

	// Specify whether or not this object can be touched by the avatar
	void SetTangible(bool b) { if(b) m_flags |= MOB_Tangible; else m_flags &= (~MOB_Tangible); }

	bool IsPanel() { return (m_flags & MOB_Panel) ? true : false; }
	void SetIsPanel(bool b) { if(b) m_flags |= MOB_Panel; else m_flags &= (~MOB_Panel); }

	bool IsSolid() { return (m_flags & MOB_Solid) ? true : false; }
	void SetIsSolid(bool b) { if(b) m_flags |= MOB_Solid; else m_flags &= (~MOB_Solid); }
	bool IsChatCloud();
	bool SanityCheck();
	const char* GetTypeName();
	inline int GetPivotHeight() { return m_nPivotHeight; }
	void SetPivotHeight(int n) { m_nPivotHeight = n; }
};

#endif // __MOBJECT_H__
