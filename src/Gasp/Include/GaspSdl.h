/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GASPSDL_H__
#define __GASPSDL_H__

#include "GaspLib.h"

class ConsoleWindow;
class QApplication;
class QWidget;

class GConstStringHashTable;

extern GConstStringHashTable* g_pSdlMachineObjects;

void RegisterSdlMachineClasses();

class GaspSdlCallBackGetter : public GaspLibCallBackGetter
{
public:
	GaspSdlCallBackGetter() : GaspLibCallBackGetter() {}
	virtual ~GaspSdlCallBackGetter() {}

	virtual EMethodPointerHolder* GetCallBack(const char* szClassName, EMethodSignature* pMethodSignature)
	{
		if(!g_pSdlMachineObjects)
			RegisterSdlMachineClasses();
		EMethodPointerHolder* pMeth = WrapperObject::FindMachineMethod(g_pSdlMachineObjects, szClassName, pMethodSignature);
		if(pMeth)
			return pMeth;
		return GaspLibCallBackGetter::GetCallBack(szClassName, pMethodSignature);
	}
};


#endif // __GASPSDL_H__
