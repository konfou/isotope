/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __POPUPS_H__
#define __POPUPS_H__

#include "../Include/GashEngine.h"

class PopUps : public WrapperObject
{
protected:
	PopUps(Engine* pEngine)
		: WrapperObject(pEngine, "PopUps")
	{
	}

public:
	virtual ~PopUps()
	{
	}

	void messageBox(Engine* pEngine, EVar* pTitle, EVar* pMessage);
	void getStringBox(Engine* pEngine, EVar* pOutString, EVar* pTitle);
};

#endif // __POPUPS_H__
