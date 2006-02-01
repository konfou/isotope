/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MPUZSEARCHENGINE_H__
#define __MPUZSEARCHENGINE_H__

#include "Model.h"
#include <stdio.h>

class MPuzSearchEngineServer;

class MPuzSearchEngine : public Model
{
protected:
	Controller* m_pController;
	MPuzSearchEngineServer* m_pHttpServer;

public:
	MPuzSearchEngine(Controller* pController);
	virtual ~MPuzSearchEngine();

	virtual void Update(double time);
	virtual bool OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew) { return true; }
	virtual ModelType GetType() { return PuzSearchEngine; }
	virtual void SendObject(GObject* pObj, int nConnection) {}

protected:

};

#endif // __MPUZSEARCHENGINE_H__
