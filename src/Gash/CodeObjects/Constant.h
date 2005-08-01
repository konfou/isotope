/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __CONSTANT_H__
#define __CONSTANT_H__

#include "Instruction.h"
#include "../../GClasses/GMacros.h"

// A Constant is an integer or a string that can be identified by name
class COConstant : public CodeObject
{
protected:
	char* m_szName;
	char* m_szValue;
	int m_nValue;
	bool m_bIsString;

public:
	COConstant(int nLine, int nCol, int nWid, const char* szName, const char* szValue);
	COConstant(int nLine, int nCol, int nWid, const char* szName, int nValue);
	virtual ~COConstant();

	bool IsString() { return m_bIsString; }
	void SetName(const char* szName);
	const char* GetName() { return m_szName; }
	void SetValue(const char* szValue);
	void SetValue(int nValue);
	const char* GetStringValue() { GAssert(m_bIsString, "Not a string value"); return m_szValue; }
	int GetIntegerValue() { GAssert(!m_bIsString, "Not an integer value"); return m_nValue; }
	static COConstant* LoadFromXML(GXMLTag* pTag, COProject* pCOProject);
	GXMLTag* SaveToXML();
	void SaveToClassicSyntax(GQueue* pQ);
};

#endif // __CONSTANT_H__
