/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GASHQT_H__
#define __GASHQT_H__

#include "GashLib.h"

class ConsoleWindow;
class QApplication;
class QWidget;

QApplication* GetApp();
void ShowGashGUI(const char* szAppPath, int argc, char** argv);
void CommandDebug(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv, CallBackGetter* pCBG);


class GConstStringHashTable;

extern GConstStringHashTable* g_pQtMachineObjects;

void RegisterQtMachineClasses();

class GashQtCallBackGetter : public GashLibCallBackGetter
{
public:
	GashQtCallBackGetter() : GashLibCallBackGetter() {}
	virtual ~GashQtCallBackGetter() {}

	virtual EMethodPointerHolder* GetCallBack(const char* szClassName, EMethodSignature* pMethodSignature)
	{
		if(!g_pQtMachineObjects)
			RegisterQtMachineClasses();
		EMethodPointerHolder* pMeth = WrapperObject::FindMachineMethod(g_pQtMachineObjects, szClassName, pMethodSignature);
		if(pMeth)
			return pMeth;
		return GashLibCallBackGetter::GetCallBack(szClassName, pMethodSignature);
	}
};


#endif // __GASHQT_H__
