/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __EINTERFACE_H__
#define __EINTERFACE_H__

#include "../Include/GashEngine.h"
#include "EType.h"

class EInterface : public EType
{
protected:
	int m_nMethodDeclCount;
	GXMLTag** m_pChildTags;

public:
	EInterface(GXMLTag* pTag, int nID);
	virtual ~EInterface();

	virtual TypeType GetTypeType() { return TT_INTERFACE; }
	int GetMethodDeclCount() { return m_nMethodDeclCount; }
	GXMLTag* GetChildTag(int nMethodDecl);
	int GetParamCount(int nMethodDecl);
};



class EMachineClass : public EInterface
{
protected:
	EMethodPointerHolder** m_pMachineMethods;

public:
	EMachineClass(GXMLTag* pTag, int nID)
	 : EInterface(pTag, nID)
	{
		m_pMachineMethods = NULL;
	}

	virtual ~EMachineClass()
	{
		delete [] m_pMachineMethods;
	}

	virtual TypeType GetTypeType() { return TT_MACHINE; }

	EMethodPointerHolder* GetMachineProc(int nMethodDeclIndex, GVM* pVM);
	const char* GetName();
};


#endif // __EINTERFACE_H__
