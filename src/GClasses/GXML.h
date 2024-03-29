/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GXML_H__
#define __GXML_H__

#include <stdio.h>
#include "GLList.h"
#include "GMacros.h"

class GXMLTag;

// Represents an XML attribute
class GXMLAttribute : public GBucket
{
friend class GXMLTag;
protected:
	char* m_pName;
	char* m_pValue;

public:
	GXMLAttribute(const char* szName, const char* szValue);
	GXMLAttribute(const char* pName, int nNameLength, const char* pValue);
	GXMLAttribute(const char* pName, const char* pValue, int nValueLength);

	virtual ~GXMLAttribute();

	const char* GetName() { return m_pName; }
	const char* GetValue() { return m_pValue; }

	void SetName(const char* szName);
	void SetValue(const char* szValue);
	
	GXMLAttribute* Copy();
	virtual int Compare(GBucket* pBucket);

protected:
	int ToString(char* pBuffer, bool bEscapeQuotes);
};

// Represents an XML tag
class GXMLTag : public GBucket
{
protected:
	char* m_pName;
#ifdef _DEBUG
	GXMLAttribute* m_DEBUG_ONLY_attributes[4];
#endif
	GLList* m_pChildren;
	GLList* m_pAttributes;
	GXMLTag* m_pParent;
	int m_nLineNumber;
	unsigned int m_nColumnAndWidth;

public:
	GXMLTag(const char* szName);
	GXMLTag(const char* pName, int nLength);
	virtual ~GXMLTag();

	const char* GetName() { return m_pName; }
	GXMLTag* GetParentTag() { return m_pParent; }
	GXMLTag* GetFirstChildTag() { return (GXMLTag*)m_pChildren->GetFirst(); }
	GXMLTag* GetNextChildTag(GXMLTag* pCurrentTag) { return (GXMLTag*)m_pChildren->GetNext(pCurrentTag); }
	GXMLAttribute* GetFirstAttribute() { return (GXMLAttribute*)m_pAttributes->GetFirst(); }
	GXMLAttribute* GetNextAttribute(GXMLAttribute* pCurrentAttribute) { return (GXMLAttribute*)m_pAttributes->GetNext(pCurrentAttribute); }
	int GetChildTagCount() { return m_pChildren->GetCount(); }
	int GetAttributeCount() { return m_pAttributes->GetCount(); }
	GXMLTag* GetChildTag(const char* szName);
	GXMLAttribute* GetAttribute(const char* szName);
	int GetLineNumber() { return m_nLineNumber; }
	
	void SetColumnAndWidth(int nColumn, int nWidth)
	{
		m_nColumnAndWidth = ((nColumn & 0xffff)) << 16 | (nWidth & 0xffff);
	}

	void GetOffsetAndWidth(int* pnColumn, int* pnWidth)
	{
		*pnWidth = m_nColumnAndWidth & 0xffff;
		*pnColumn = ((m_nColumnAndWidth >> 16) & 0xffff);
	}

	GXMLTag* FindChildWithAttribute(const char* szAttrName, const char* szAttrValue);

	// Sets the name of the tag
	void SetName(const char* szName);

	// Add a child tag to this XML tag
	void AddChildTag(GXMLTag* pChild) { GAssert(pChild, "Null parameter"); if(!pChild) return; pChild->m_pParent = this; m_pChildren->Link(pChild); }
	
	// Add an attribute to this XML tag
	void AddAttribute(GXMLAttribute* pAttribute);

	// Set the line number for this XML tag
	void SetLineNumber(int nLineNumber) { m_nLineNumber = nLineNumber; }

	// Convert the entire XML tree to a string. You are responsible to delete the string this returns
	char* ToString();

	// You are responsible to delete the tag this returns
	static GXMLTag* FromString(const char* pBuffer, int nSize, const char** pszErrorMessage = NULL, int* pnErrorOffset = NULL, int* pnErrorLine = NULL, int* pnErrorColumn = NULL);

	// Save the XML tree to the specified file
	bool ToFile(const char* szFilename);

	// Save the XML tree to the specified stream
	bool ToFile(FILE* pFile);

	// Save the XML tree to a text file with C++ syntax so you can embed the XML in a .CPP file
	bool ToCppFile(const char* szFilename, const char* szVarName, const char* szHeader);

	// Parse an XML file and return the root tag. You are responsible to delete the tag this returns
	static GXMLTag* FromFile(const char* szFilename, const char** pszErrorMessage = NULL, int* pnErrorOffset = NULL, int* pnErrorLine = NULL, int* pnErrorColumn = NULL);

	// Make a deep copy of this tag
	GXMLTag* Copy();

	virtual int Compare(GBucket* pBucket);
	void DeleteChildTag(GXMLTag* pPreviousChildTag) { m_pChildren->DeleteBucket(pPreviousChildTag); }
	GXMLTag* UnlinkChildTag(GXMLTag* pPreviousChildTag) { return (GXMLTag*)m_pChildren->Unlink(pPreviousChildTag); }

protected:
	int ToString(char* pBuffer, int nTabs, const char* szLineStart, const char* szLineEnd, bool bEscapeQuotes);
	char* ToString(const char* szLineStart, const char* szLineEnd, bool bEscapeQuotes);
};

#endif // __GXML_H__
