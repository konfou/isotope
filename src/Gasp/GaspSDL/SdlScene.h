/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __SDLSCENE_H__
#define __SDLSCENE_H__

#include "../Include/GaspEngine.h"
#include "../../GClasses/GRayTrace.h"
#include <SDL/SDL.h>
#include <wchar.h>

class GPreRendered3DScreen;
class SdlView;
struct TransformImagePair;





class SdlSceneSprite : public WrapperObject
{
protected:

	SdlSceneSprite(Engine* pEngine);

public:
	virtual ~SdlSceneSprite();

	void toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
	{
		GAssert(false, "todo: write me");
	}

	void fromStream(Engine* pEngine, EVar* pStream)
	{
		GAssert(false, "todo: write me");
	}

	void setRefs(Engine* pEngine, EVar* pRefs)
	{
		GAssert(false, "todo: write me");
	}

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		GAssert(nSize > 32, "Buffer too small");
		wcscpy(pBuf, L"SDL Scene Sprite");
	}

};



class SdlScene : public WrapperObject
{
protected:
	int m_nViews;
	SdlView* m_pViews;
	SdlView* m_pCurrentView;
	int m_nCounter;
//	Transform m_camera;

	SdlScene(Engine* pEngine);

public:
	virtual ~SdlScene();

	virtual void toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
	{
		GAssert(false, "todo: write me");
	}

	virtual void fromStream(Engine* pEngine, EVar* pStream)
	{
		GAssert(false, "todo: write me");
	}

	virtual void setRefs(Engine* pEngine, EVar* pRefs)
	{
		GAssert(false, "todo: write me");
	}

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		GAssert(nSize > 32, "Buffer too small");
		wcscpy(pBuf, L"SDL Scene");
	}

	void render(Engine* pEngine, EVar* pFilename);
	void save(Engine* pEngine, EVar* pFilename);
	void load(Engine* pEngine, EVar* pFilename);
	void draw(Engine* pEngine, EVar* pFrame, EVar* pFocusPoint, EVar* pFrameNumber);

protected:
	void GetBestView(Engine* pEngine, Point3D* pFocusPoint);
	void renderHelper(Engine* pEngine, EVar* pFilename);
};


#endif // __SDLSCENE_H__
