/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GASPLIB_H__
#define __GASPLIB_H__

#include "GaspEngine.h"

class GConstStringHashTable;

extern GConstStringHashTable* g_pGaspLibMachineObjects;

void RegisterGaspMachineClasses();


class GaspLibCallBackGetter : public CallBackGetter
{
public:
	GaspLibCallBackGetter() : CallBackGetter() {}
	virtual ~GaspLibCallBackGetter() {}

	virtual EMethodPointerHolder* GetCallBack(const char* szClassName, EMethodSignature* pMethodSignature)
	{
		if(!g_pGaspLibMachineObjects)
			RegisterGaspMachineClasses();
		EMethodPointerHolder* pMeth = WrapperObject::FindMachineMethod(g_pGaspLibMachineObjects, szClassName, pMethodSignature);
		if(pMeth)
			return pMeth;
		return CallBackGetter::GetCallBack(szClassName, pMethodSignature);
	}
};

#endif // __GASPLIB_H__
