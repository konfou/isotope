/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __MIMAGE_H__
#define __MIMAGE_H__

#include "../BuiltIns/GashString.h"
#include "../Include/GashEngine.h"
#include "../../GClasses/GImage.h"
#include "../../GClasses/GMacros.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN
#include "../../GClasses/GFile.h"

class MImage : public WrapperObject
{
friend class SdlFrame;
public:
	GImage m_value;

	MImage(Engine* pEngine)
		: WrapperObject(pEngine, "Image")
	{
	}

	virtual ~MImage()
	{
	}

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
		wcscpy(pBuf, L"<Image>"); // todo: do something better
	}

	void allocate(Engine* pEngine)
	{
		pEngine->SetThis(new MImage(pEngine));
	}

	void setSize(Engine* pEngine, EVar* pWidth, EVar* pHeight)
	{
		m_value.SetSize(pWidth->pIntObject->m_value, pHeight->pIntObject->m_value);
	}

	void getPixel(Engine* pEngine, EVar* pColor, EVar* pX, EVar* pY)
	{
		pColor->pIntObject->m_value = m_value.GetPixel(pX->pIntObject->m_value, pY->pIntObject->m_value);
	}

	void getSize(Engine* pEngine, EVar* pWidth, EVar* pHeight)
	{
		pWidth->pIntObject->m_value = m_value.GetWidth();
		pHeight->pIntObject->m_value = m_value.GetHeight();
	}

	void getRect(Engine* pEngine, EVar* pOutRect)
	{
		((IntObject*)pOutRect->pObjectObject->arrFields[0])->m_value = 0;
		((IntObject*)pOutRect->pObjectObject->arrFields[1])->m_value = 0;
		((IntObject*)pOutRect->pObjectObject->arrFields[2])->m_value = m_value.GetWidth();
		((IntObject*)pOutRect->pObjectObject->arrFields[3])->m_value = m_value.GetHeight();
	}

	void setPixel(Engine* pEngine, EVar* pX, EVar* pY, EVar* pColor)
	{
		m_value.SetPixel(pX->pIntObject->m_value, pY->pIntObject->m_value, pColor->pIntObject->m_value);
	}

	GImage* GetImage() { return &m_value; }

	void load(Engine* pEngine, EVar* pFilename)
	{
		char szBuf[512];
		char szExt[256];
		pFilename->pStringObject->m_value.GetAnsi(szBuf);
		if(!GFile::DoesFileExist(szBuf))
			pEngine->ThrowFileNotFoundError(pFilename->pStringObject->m_value.GetString());
		_splitpath(szBuf, NULL, NULL, NULL, szExt);
		bool bOK;
		if(stricmp(szExt, ".bmp") == 0)
			bOK = m_value.LoadBMPFile(szBuf);
		else if(stricmp(szExt, ".ppm") == 0)
			bOK = m_value.LoadPPMFile(szBuf);
		else if(stricmp(szExt, ".pgm") == 0)
			bOK = m_value.LoadPGMFile(szBuf);
		else
		{
			GString s;
			s.Add(szExt);
			pEngine->ThrowIOError(L"Unrecognized image extension: %ls", s.GetString());
		}
		if(!bOK)
			pEngine->ThrowIOError(L"Failed to load the image file: %ls", pFilename->pStringObject->m_value.GetString());
	}

	void save(Engine* pEngine, EVar* pFilename)
	{
		char szBuf[512];
		char szExt[256];
		pFilename->pStringObject->m_value.GetAnsi(szBuf);
		_splitpath(szBuf, NULL, NULL, NULL, szExt);
		bool bOK;
		if(stricmp(szExt, ".bmp") == 0)
			bOK = m_value.SaveBMPFile(szBuf);
		else if(stricmp(szExt, ".ppm") == 0)
			bOK = m_value.SavePPMFile(szBuf);
		else if(stricmp(szExt, ".pgm") == 0)
			bOK = m_value.SavePGMFile(szBuf);
		else
		{
			GString s;
			s.Add(szExt);
			pEngine->ThrowIOError(L"Unrecognized image extension: %ls", s.GetString());
		}
		if(!bOK)
			pEngine->ThrowIOError(L"Failed to save the file: %ls", pFilename->pStringObject->m_value.GetString());
	}
};

#endif // __MIMAGE_H__
