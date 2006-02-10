/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MGAMEIMAGE_H__
#define __MGAMEIMAGE_H__

#include "../Gasp/BuiltIns/GaspString.h"
#include "../Gasp/Include/GaspEngine.h"
#include "../GClasses/GImage.h"
#include "../GClasses/GMacros.h"

class MGameClient;


// This class is similar to MImage, but differs in how it is serialized.  The problem
// with images is that they are bulky when serialized, so this class represents an image
// that can be serialized into a very small blob by taking advantage of certain
// characteristics of the game.  Specifically, most images are pre-loaded so all we
// really need in the blob is the image ID.  For chat-bubbles which are dynamically
// created, this serializes to format that contains only the text of the chat bubble.
// Also, this class is careful not to bother creating the full image when deserializing
// on the server since it knows the server will never use the image anyway.
class MGameImage : public WrapperObject
{
public:
	enum Mode
	{
		Store,
		Text,
		RelativeUrl,
		GlobalId,
	};

	GImage m_value;
	Mode m_eMode;
	char* m_szText;

	MGameImage(Engine* pEngine, Mode eMode, const char* szText)
		: WrapperObject(pEngine, "GImage")
	{
		m_eMode = eMode;
		m_szText = new char[strlen(szText) + 1];
		strcpy(m_szText, szText);
	}

	virtual ~MGameImage()
	{
		delete(m_szText);
	}

	void newCopy(Engine* pEngine, EVar* pThat);

	void toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs);

	void fromStream(Engine* pEngine, EVar* pStream);

	void setRefs(Engine* pEngine, EVar* pRefs)
	{
	}

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		wcscpy(pBuf, L"<GImage>");
	}

	void makeTextImage(Engine* pEngine, EVar* pText);

	void setText(Engine* pEngine, EVar* pText);

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

	void invert(Engine* pEngine)
	{
		m_value.Invert();
	}

	void glow(Engine* pEngine)
	{
		m_value.MakeEdgesGlow((float)0.2, 5, 32, gARGB(0xff, 0x88, 0xbb, 0xff));
	}

	GImage* GetImage() { return &m_value; }

	void load(Engine* pEngine, EVar* pFilename);
	const char* GetID();
	void munge(Engine* pEngine, EVar* pStyle, EVar* pExtent);

protected:
	void makeTextImageHelper(Engine* pEngine, const char* szText);
	void drawTextImage(const char* szText);
	static MGameImage* DownloadImage(Engine* pEngine, MGameClient* pGameClient, const char* szUrl);
};

#endif // __MGAMEIMAGE_H__
