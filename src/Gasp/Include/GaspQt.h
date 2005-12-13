/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GASPQT_H__
#define __GASPQT_H__

#include "GaspLib.h"

class ConsoleWindow;
class QApplication;
class QWidget;

QApplication* GetApp();
void ShowGaspGUI(const char* szAppPath, int argc, char** argv);
void CommandDebug(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv, CallBackGetter* pCBG);


class GConstStringHashTable;

extern GConstStringHashTable* g_pQtMachineObjects;

void RegisterQtMachineClasses();

class GaspQtCallBackGetter : public GaspLibCallBackGetter
{
public:
	GaspQtCallBackGetter() : GaspLibCallBackGetter() {}
	virtual ~GaspQtCallBackGetter() {}

	virtual EMethodPointerHolder* GetCallBack(const char* szClassName, EMethodSignature* pMethodSignature)
	{
		if(!g_pQtMachineObjects)
			RegisterQtMachineClasses();
		EMethodPointerHolder* pMeth = WrapperObject::FindMachineMethod(g_pQtMachineObjects, szClassName, pMethodSignature);
		if(pMeth)
			return pMeth;
		return GaspLibCallBackGetter::GetCallBack(szClassName, pMethodSignature);
	}
};


#endif // __GASPQT_H__
