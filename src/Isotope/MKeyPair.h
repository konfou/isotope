/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MKEYPAIR_H__
#define __MKEYPAIR_H__

#include "Model.h"

#define ENTROPY_BYTES 16384
#define KEY_BIT_SIZE 128

class GKeyPair;


class MKeyPair : public Model
{
protected:
	unsigned char* m_pEntropy;
	int m_nPos;
	const char* m_szFilename;

public:
	MKeyPair(Controller* pController, const char* szFilename);
	virtual ~MKeyPair();

	virtual void Update(double time)
	{
	}

	virtual bool OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew)
	{
		return true;
	}

	virtual ModelType GetType() { return KeyGenerator; }

	virtual void SendObject(GObject* pObj, int nConnection) {}

	void AddEntropy(int n);
	double GetPercent();
	void SaveKeyPair();
};

#endif // __MKEYPAIR_H__
