/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "../Include/GaspEngine.h"
#include "../CodeObjects/Project.h"
#include "../../GClasses/GXML.h"
#include "EClass.h"

bool EType::CanCastTo(EType* pTargetType, Library* pLibrary)
{
	if(this == pTargetType)
		return true;
	if(pTargetType->GetTypeType() == EType::TT_CLASS)
	{
		if(((EClass*)pTargetType)->IsIntegerType())
		{
			if(!GetTypeType() == EType::TT_CLASS)
				return false;
			if(!((EClass*)this)->IsIntegerType())
				return false;
			return true;
		}
		EClass* pThisClass = (EClass*)this;
		while(pThisClass->m_nParentClassID != 0)
		{
			pThisClass = (EClass*)pLibrary->GetEType(pThisClass->m_nParentClassID);
			if(pThisClass == pTargetType)
				return true;
		}
		return false;
	}
	else
	{
		if(!(GetTypeType() == EType::TT_CLASS))
			return false; // we already checked that they're not the same interface
		EClass* pThisClass = (EClass*)this;
		if(pThisClass->DoesImplement((EInterface*)pTargetType))
			return true;
		while(pThisClass->m_nParentClassID != 0)
		{
			pThisClass = (EClass*)pLibrary->GetEType(pThisClass->m_nParentClassID);
			if(pThisClass->DoesImplement((EInterface*)pTargetType))
				return true;
		}
		return false;
	}
}

COType* EType::GetCOType(COProject* pProject)
{
	GXMLAttribute* pNameAttr = m_pTag->GetAttribute(ATTR_NAME);
	GAssert(pNameAttr, "no name attribute");
	return pProject->FindType(pNameAttr->GetValue());
}

const char* EType::GetName()
{
	GXMLAttribute* pAttr = m_pTag->GetAttribute(ATTR_NAME);
	if(!pAttr)
	{
		GAssert(false, "todo: handle this case");
		return "";
	}
	return(pAttr->GetValue());
}
