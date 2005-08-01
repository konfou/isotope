/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __ETYPE_H__
#define __ETYPE_H__

class COType;


class EType
{
protected:
	GXMLTag* m_pTag;
	int m_nID;
#ifdef _DEBUG
	const char* m_szName;
#endif // _DEBUG

public:
	enum TypeType
	{
		TT_CLASS,
		TT_INTERFACE,
		TT_MACHINE,
	};

	EType(GXMLTag* pTag, int nID)
		: m_pTag(pTag),
		m_nID(nID)
	{
#ifdef _DEBUG
		m_szName = GetName();
#endif // _DEBUG
	}

	virtual ~EType()
	{
	}

	virtual TypeType GetTypeType() = 0;

	bool CanCastTo(EType* pTargetType, Library* pLibrary);
	GXMLTag* GetTag() { return m_pTag; }
	COType* GetCOType(COProject* pProject);
	const char* GetName();
	int GetID() { return m_nID; }
};


#endif // __ETYPE_H__
