/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Type.h"
#include "Class.h"
#include "Project.h"
#include "File.h"

COType::COType(int nLine, int nCol, int nWid, const char* szName, COFile* pFile, COProject* pCOProject)
: CodeObject(nLine, nCol, nWid)
{
	GAssert(szName, "Must have a valid name");
	GAssert(pFile, "Must have a valid file");
	m_szName = new char[strlen(szName) + 1];
	strcpy(m_szName, szName);
	m_pFile = pFile;
	m_nID = pCOProject->GetUniqueTypeID();
}

COType::~COType()
{
	delete(m_szName);
}

bool COType::CanCastTo(COType* pDestType, bool* pbNeedCast, ErrorStruct** ppErrorStruct)
{
	bool bNeedCast = true;
	if(GetTypeType() == COType::TT_CLASS)
	{
		if(pDestType->GetTypeType() == COType::TT_CLASS)
		{
			if(((COClass*)this)->DoesInherritFrom(((COClass*)pDestType)))
				bNeedCast = false;
			else if(!((COClass*)pDestType)->DoesInherritFrom((COClass*)this))
			{
				if(ppErrorStruct)
					*ppErrorStruct = &Error::BAD_CAST;
				return false;
			}
		}
		else if(pDestType->GetTypeType() == COType::TT_INTERFACE)
		{
			if(((COClass*)this)->DoesImplement((COInterface*)pDestType))
				bNeedCast = false;
			else
			{
				if(ppErrorStruct)
					*ppErrorStruct = &Error::CLASS_DOESNT_IMPLEMENT_INTERFACE;
				return false;
			}
		}
		else
		{
			GAssert(pDestType->GetTypeType() == COType::TT_MACHINE, "unexpected type");
			if(((COClass*)this)->GetParent() != NULL)
			{
				if(ppErrorStruct)
					*ppErrorStruct = &Error::BAD_CAST;
				return false;
			}
		}
	}
	else if(GetTypeType() == COType::TT_MACHINE)
	{
		if(pDestType->GetTypeType() == COType::TT_CLASS && ((COClass*)pDestType)->GetParent() == NULL)
			bNeedCast = false;
		else if(pDestType != this)
		{
			if(ppErrorStruct)
				*ppErrorStruct = &Error::BAD_CAST;
			return false;
		}
	}
	else
	{
		GAssert(GetTypeType() == COType::TT_INTERFACE, "unexpected type");
		if(pDestType->GetTypeType() == COType::TT_CLASS)
		{
			if(((COClass*)pDestType)->GetParent() == NULL)
				bNeedCast = false;
			if(!((COClass*)pDestType)->DoesImplement((COInterface*)this))
			{
				if(ppErrorStruct)
					*ppErrorStruct = &Error::BAD_CAST;
				return false;
			}
		}
		else if(pDestType->GetTypeType() == COType::TT_INTERFACE)
		{
			if(this == pDestType)
				bNeedCast = false;
			else
			{
				if(ppErrorStruct)
					*ppErrorStruct = &Error::CANT_SET_DIFFERENT_INTERFACES;
				return false;
			}
		}
		else
		{
			GAssert(pDestType->GetTypeType() == COType::TT_MACHINE, "unexpected type");
			if(ppErrorStruct)
				*ppErrorStruct = &Error::BAD_CAST;
			return false;
		}
	}
	if(pbNeedCast)
		*pbNeedCast = bNeedCast;
	return true;
}

Library* COType::GetLibrary()
{
	return m_pFile->GetLibrary();
}

