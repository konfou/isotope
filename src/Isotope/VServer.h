/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VSERVER_H__
#define __VSERVER_H__

#include "ViewPort.h"

class MGameServer;
struct GRect;

// This shows a view of server status
class VServer : public ViewPort
{
protected:
	GImage* m_pImage;
	float* m_pHistory;
	int m_nHistoryPos;
	double m_time;
	MGameServer* m_pServer;

public:
	VServer(GRect* pRect, MGameServer* pServer);
	virtual ~VServer();

	virtual void Draw(SDL_Surface *pScreen);
	virtual void OnChar(char c) {}
	virtual void OnMouseDown(int x, int y) {}
	virtual void OnMouseUp(int x, int y) {}
	virtual void OnMousePos(int x, int y) {}
};

#endif // __VSERVER_H__
