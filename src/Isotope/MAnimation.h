/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MANIMATION_H__
#define __MANIMATION_H__

#include "../Gash/Include/GashSdl.h"
#include <wchar.h>

class MAnimationFrame;
class GXMLTag;
class MImageStore;
class MGameImage;
struct GRect;
class MScriptEngine;

// This class represents an animation (which consists of an image and a collection of frames, where
// a frame is a rectangular region of the image).  The toStream and fromStream methods only
// serialize/deserialize the ID of the MAnimation object, not all the data, so you can't dynamically
// generate an MAnimation object in script and expect to be able to pass it back and forth between
// the client and the server.  I don't think this limitation will ever be an issue.
class MAnimation : public WrapperObject
{
friend class MAnimationStore;
protected:
	static GHashTable* s_pAllAnimations;

	VarHolder* m_pImage;
	float m_fDirection;
	int m_nColumns;
	double m_time;
	int m_nFrames;
	MAnimationFrame* m_pFrames;
	int m_nCurrentFrame;
	char* m_szID;
	int m_nUID;

public:
	MAnimation(Engine* pEngine);
	virtual ~MAnimation();

	static MAnimation* FromXml(GXMLTag* pTag, MScriptEngine* pScriptEngine, const char* szID, MImageStore* pImageStore);

	// Advance the animation
	bool AdvanceTime(double dt);

	// Returns the image and rect for the current animation frame
	void getFrame(Engine* pEngine, EVar* pOutImage, EVar* pOutRect);

	// This method is a special case of "getFrame" in which the frames are arranged in a
	// 2D grid.  Each column of frames represents the same animation from a different
	// camera angle.  This is used for objects that can be viewed from any angle.
	void getColumnFrame(Engine* pEngine, EVar* pOutImage, EVar* pOutRect, EVar* pCameraDirection);

	// This method is called from within Gash scripts.  It's just a wrapper around AdvanceTime
	void advanceTime(Engine* pEngine, EVar* pOutLooped, EVar* pTime);

	// This method is called from within Gash scripts.  It makes a copy of the animation
	void newCopy(Engine* pEngine, EVar* pThat);

	// This method is called from Gash when the object needs to be serialized
	void toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs);

	// This method is called from Gash as part of the deserialization process
	void fromStream(Engine* pEngine, EVar* pStream);

	// This method is called from Gash as part of the deserialization process
	void setRefs(Engine* pEngine, EVar* pRefs)
	{
	}

	// This method is called from Gash when debugging
	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		GAssert(nSize > 32, "Buffer too small");
		wcscpy(pBuf, L"Animation");
	}

	void SetDirection(float radians) { m_fDirection = radians; }
	int GetColumnCount() { return m_nColumns; }
	void SetColumnCount(int n) { m_nColumns = n; }
	GImage* GetFrame(GRect* pRect);
	GImage* GetColumnFrame(GRect* pRect, float fCameraDirection);
	void CopyDataAcrossEngines(MAnimation* pThat, VarHolder* pImage, const char* szID);
	VarHolder* GetImage() { return m_pImage; }

protected:
	// Copies just the primitive data parts of pThat into this animation.  (This
	// method does the work that is common between CopyData and CopyDataAcrossEngines)
	void CopyGuts(MAnimation* pThat);

	// Copies pThat into this animation (including the ID and image)
	void CopyData(MAnimation* pThat);
};

#endif // __MANIMATION_H__
