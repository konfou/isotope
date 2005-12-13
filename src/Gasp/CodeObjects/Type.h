/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __TYPE_H__
#define __TYPE_H__

#include "CodeObject.h"
#include "../../GClasses/GMacros.h"

class COFile;

class COType : public CodeObject
{
friend class COProject; // So it can set special ID's for Integer, Wrapper, and Object
public:
	enum TypeType
	{
		TT_CLASS,
		TT_INTERFACE,
		TT_MACHINE,
	};

protected:
	COFile* m_pFile;
	char* m_szName;
	int m_nID;
	int m_nGeneration;

public:
	COType(int nLine, int nCol, int nWid, const char* szName, COFile* pFile, COProject* pCOProject);
	virtual ~COType();

	int GetID() { return m_nID; }
	const char* GetName() { return m_szName; }
	void SetName(const char* szName) { GAssert(szName, "Must have a valid name"); delete(m_szName); m_szName = new char[strlen(szName) + 1]; strcpy(m_szName, szName); }
	virtual TypeType GetTypeType() = 0;
	COFile* GetFile() { return m_pFile; }
	bool CanCastTo(COType* pDestType, bool* pbNeedCast = NULL, ErrorStruct** ppErrorStruct = NULL);
	Library* GetLibrary();
	virtual GXMLTag* ToXMLForLibrary(GCompiler* pCompiler, bool bImport) = 0;
	void SetGeneration(int n) { m_nGeneration = n; }
	int GetGeneration() { return m_nGeneration; }

protected:
	void SetID(int nID) { m_nID = nID; }

};

#endif // __TYPE_H__
