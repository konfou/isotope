/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GXML.h"
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#else // WIN32
#endif // !WIN32

// This is a helper class used by GXMLTag::ParseXMLFile
class GXMLParser
{
protected:
	GXMLTag* m_pRootTag;
	GXMLTag* m_pCurrentTag;
	int m_nPos;
	const char* m_pFile;
	int m_nLength;
	int m_nLine;
	int m_nLineStart;
	const char* m_szErrorMessage;
	int m_nErrorOffset;
	int m_nErrorLine;
	int m_nErrorColumn;

public:
	GXMLParser(const char* pFile, int nSize);
	virtual ~GXMLParser();

	GXMLTag* Parse(const char** pszErrorMessage, int* pnErrorOffset, int* pnErrorLine, int* pnErrorColumn);

protected:
	void SetError(const char* szMessage);
	GXMLTag* ParseTag();
	bool ParseCloser(int nNameStart, int nNameLength);
	int ParseName();
	bool IsTheNextOneACloser();
	void EatWhitespace();
	GXMLAttribute* ParseAttribute();
	bool UnescapeAttrValue(const char* pValue, int nLength, char* pBuffer);
};

GXMLParser::GXMLParser(const char* pFile, int nSize)
{
	m_pRootTag = NULL;
	m_pCurrentTag = NULL;
	m_nPos = 0;
	m_nLineStart = 0;
	m_pFile = pFile;
	m_nLength = nSize;
	m_nLine = 1;
	m_szErrorMessage = NULL;
	m_nErrorOffset = 0;
	m_nErrorLine = 0;
	m_nErrorColumn = 0;
}

GXMLParser::~GXMLParser()
{

}

inline bool isWhitespace(char c)
{
	return c <= ' ' ? true : false;
}

void GXMLParser::SetError(const char* szMessage)
{
	GAssert(!m_szErrorMessage, "Error message already set");
	m_szErrorMessage = szMessage;
	m_nErrorOffset = m_nPos;
	m_nErrorLine = m_nLine;
	m_nErrorColumn = m_nPos - m_nLineStart;
}

void GXMLParser::EatWhitespace()
{
	char c;
	while(true)
	{
		if(m_nPos >= m_nLength)
			break;
		c = m_pFile[m_nPos];
		if(!isWhitespace(c))
			break;
		if(c == '\n')
		{
			m_nLine++;
			m_nLineStart = m_nPos + 1;
		}
		m_nPos++;
	}
}

GXMLTag* GXMLParser::Parse(const char** pszErrorMessage, int* pnErrorOffset, int* pnErrorLine, int* pnErrorColumn)
{
	GXMLTag* pRoot = ParseTag();
	if(pszErrorMessage)
		*pszErrorMessage = m_szErrorMessage;
	if(pnErrorOffset)
		*pnErrorOffset = m_nErrorOffset;
	if(pnErrorLine)
		*pnErrorLine = m_nErrorLine;
	if(pnErrorColumn)
		*pnErrorColumn = m_nErrorColumn;
	return pRoot;
}

GXMLTag* GXMLParser::ParseTag()
{
	// Parse the name
	EatWhitespace();
	if(m_nPos >= m_nLength || m_pFile[m_nPos] != '<')
	{
		SetError("Expected a '<'");
		return NULL;
	}
	m_nPos++;
	int nNameStart = m_nPos;
	int nNameLength = ParseName();
	if(nNameLength < 1)
	{
		SetError("Expected a tag name");
		return NULL;
	}
	GXMLTag* pNewTag = new GXMLTag(&m_pFile[nNameStart], nNameLength);
	pNewTag->SetLineNumber(m_nLine);
	int nStartColumn = m_nPos - m_nLineStart + 1;

	// Parse Attributes
	while(true)
	{
		EatWhitespace();
		if(m_nPos >= m_nLength)
		{
			SetError("Expected an attribute, a '/', or a '>'");
			return NULL;
		}
		if(m_pFile[m_nPos] == '/' || m_pFile[m_nPos] == '>')
			break;
		GXMLAttribute* pNewAttr = ParseAttribute();
		if(!pNewAttr)
		{
			delete(pNewTag);
			return NULL;
		}
		pNewTag->AddAttribute(pNewAttr);
	}
	int nEndColumn = m_nPos - m_nLineStart + 1;
	pNewTag->SetColumnAndWidth(nStartColumn, nEndColumn - nStartColumn);

	if(m_pFile[m_nPos] == '>')
	{
		// Parse the children
		m_nPos++;
		while(!IsTheNextOneACloser())
		{
			GXMLTag* pChildTag = ParseTag();
			if(!pChildTag)
			{
				delete(pNewTag);
				return NULL;
			}
			pNewTag->AddChildTag(pChildTag);
		}

		// Parse the closer
		EatWhitespace();
		if(!ParseCloser(nNameStart, nNameLength))
		{
			delete(pNewTag);
			return NULL;
		}
	}
	else
	{
		// Parse the end of the tag
		GAssert(m_pFile[m_nPos] == '/', "internal error");
		m_nPos++;
		EatWhitespace();
		if(m_nPos >= m_nLength || m_pFile[m_nPos] != '>')
		{
			SetError("Expected a '>'");
			delete(pNewTag);
			return NULL;
		}
		m_nPos++;
	}
	return pNewTag;
}

int GXMLParser::ParseName()
{
	EatWhitespace();
	char c;
	int nLength = 0;
	while(true)
	{
		if(m_nPos >= m_nLength)
			break;
		c = m_pFile[m_nPos];
		if(c == '/' || c == '>' || c == '=')
			break;
		else if(c == '\n')
		{
			m_nLine++;
			m_nLineStart = m_nPos + 1;
			break;
		}
		else if(isWhitespace(c))
			break;
		m_nPos++;
		nLength++;
	}
	return nLength;
}

bool GXMLParser::IsTheNextOneACloser()
{
	int nPos = m_nPos;
	while(nPos < m_nLength && isWhitespace(m_pFile[nPos]))
		nPos++;
	if(nPos >= m_nLength || m_pFile[nPos] != '<')
		return false;
	nPos++;
	while(nPos < m_nLength && isWhitespace(m_pFile[nPos]))
		nPos++;
	if(nPos >= m_nLength || m_pFile[nPos] != '/')
		return false;
	return true;
}

bool GXMLParser::ParseCloser(int nNameStart, int nNameLength)
{
	if(m_nPos >= m_nLength || m_pFile[m_nPos] != '<')
	{
		SetError("Expected a '<'");
		return false;
	}
	m_nPos++;
	EatWhitespace();
	if(m_nPos >= m_nLength || m_pFile[m_nPos] != '/')
	{
		SetError("Expected a '/'");
		return false;
	}
	m_nPos++;
	EatWhitespace();
	int nCloserStart = m_nPos;
	int nCloserLength = ParseName();
	if(nCloserLength != nNameLength || memcmp(&m_pFile[nNameStart], &m_pFile[nCloserStart], nCloserLength) != 0)
	{
		SetError("Closer name doesn't match tag name");
		return false;
	}
	EatWhitespace();
	if(m_nPos >= m_nLength || m_pFile[m_nPos] != '>')
	{
		SetError("Expected a '>'");
		return false;
	}
	m_nPos++;
	return true;
}



GXMLAttribute* GXMLParser::ParseAttribute()
{
	int nNameStart = m_nPos;
	int nNameLength = ParseName();
	if(nNameLength < 1)
	{
		SetError("Expected an attribute name");
		return NULL;
	}
	EatWhitespace();
	if(m_nPos >= m_nLength || m_pFile[m_nPos] != '=')
	{
		SetError("Expected a '='");
		return NULL;
	}
	m_nPos++;
	EatWhitespace();
	if(m_nPos >= m_nLength || m_pFile[m_nPos] != '"')
	{
		SetError("Expected a '\"'");
		return NULL;
	}
	m_nPos++;
	int nValueStart = m_nPos;
	char c;
	int nValueLength = 0;
	while(true)
	{
		if(m_nPos >= m_nLength)
		{
			SetError("Expected a '\"'");
			return NULL;
		}
		c = m_pFile[m_nPos];
		if(c == '"')
			break;
		else if(c == '\n')
		{
			SetError("Expected a '\"'");
			return NULL;
		}
		m_nPos++;
		nValueLength++;
	}
	if(m_nPos >= m_nLength || m_pFile[m_nPos] != '"')
	{
		SetError("Expected a '\"'");
		return NULL;
	}
	m_nPos++;
	
	char szTmp[512];
	char* szBuff = szTmp;
	if(nValueLength >= 512)
		szBuff = new char[nValueLength + 1];
	if(!UnescapeAttrValue(&m_pFile[nValueStart], nValueLength, szBuff))
	{
		SetError("Unrecognized escape sequence");
		return NULL;
	}
	GXMLAttribute* pAttr = new GXMLAttribute(&m_pFile[nNameStart], nNameLength, szBuff);
	if(szBuff != szTmp)
		delete(szBuff);
	return pAttr;
}

bool GXMLParser::UnescapeAttrValue(const char* pValue, int nLength, char* pBuffer)
{
	while(nLength > 0)
	{
		if(*pValue == '&')
		{
			pValue++;
			nLength--;
			switch(*pValue)
			{
			case '#':
				{
					pValue++;
					nLength--;
					if(UCHAR(*pValue) != 'X')
						return false;
					int nNum = 0;
					while(*pValue != ';' && nLength > 0)
					{
						if(UCHAR(*pValue) < '0' || UCHAR(*pValue) > 'F')
							return false;
						if(UCHAR(*pValue) > '9' && UCHAR(*pValue) < 'A')
							return false;
						nNum <<= 4;
						if(UCHAR(*pValue) <= '9')
							nNum += UCHAR(*pValue) - '0';
						else
							nNum += UCHAR(*pValue) - 'A' + 10;
					}
					if(nLength < 1)
						return false;
					pValue++;
					nLength--;
					*pBuffer = nNum;
				}
				break;
			case 'a':
			case 'A':
				pValue++;
				nLength--;
				if(UCHAR(*pValue) != 'M')
					return false;
				pValue++;
				nLength--;
				if(UCHAR(*pValue) != 'P')
					return false;
				pValue++;
				nLength--;
				if(*pValue != ';')
					return false;
				*pBuffer = '&';
				pValue++;
				nLength--;
				break;
			case 'g':
			case 'G':
				pValue++;
				nLength--;
				if(UCHAR(*pValue) != 'T')
					return false;
				pValue++;
				nLength--;
				if(*pValue != ';')
					return false;
				*pBuffer = '>';
				pValue++;
				nLength--;
				break;
			case 'l':
			case 'L':
				pValue++;
				nLength--;
				if(UCHAR(*pValue) != 'T')
					return false;
				pValue++;
				nLength--;
				if(*pValue != ';')
					return false;
				*pBuffer = '<';
				pValue++;
				nLength--;
				break;
			case 'q':
			case 'Q':
				pValue++;
				nLength--;
				if(UCHAR(*pValue) != 'U')
					return false;
				pValue++;
				nLength--;
				if(UCHAR(*pValue) != 'O')
					return false;
				pValue++;
				nLength--;
				if(UCHAR(*pValue) != 'T')
					return false;
				pValue++;
				nLength--;
				if(*pValue != ';')
					return false;
				*pBuffer = '"';
				pValue++;
				nLength--;
				break;
			default:
				return false;
			}
		}
		else
		{
			*pBuffer = *pValue;
			pValue++;
			nLength--;
		}
		pBuffer++;
	}
	*pBuffer = '\0';
	return true;
}








GXMLAttribute::GXMLAttribute(const char* szName, const char* szValue) : GBucket()
{
	m_pName = NULL;
	m_pValue = NULL;
	SetName(szName);
	SetValue(szValue);
}

GXMLAttribute::GXMLAttribute(const char* pName, int nNameLength, const char* pValue) : GBucket()
{
	m_pName = new char[nNameLength + 1];
	memcpy(m_pName, pName, nNameLength);
	m_pName[nNameLength] = '\0';
	m_pValue = NULL;
	SetValue(pValue);
}

GXMLAttribute::GXMLAttribute(const char* pName, const char* pValue, int nValueLength) : GBucket()
{
	m_pName = NULL;
	SetName(pName);
	m_pValue = new char[nValueLength + 1];
	memcpy(m_pValue, pValue, nValueLength);
	m_pValue[nValueLength] = '\0';
}

GXMLAttribute::~GXMLAttribute()
{
	delete(m_pName);
	delete(m_pValue);
}

void GXMLAttribute::SetName(const char* szName)
{
	delete(m_pName);
	m_pName = new char[strlen(szName) + 1];
	strcpy(m_pName, szName);
	int n;
	for(n = 0; m_pName[n] != '\0'; n++)
	{
		if(m_pName[n] < ' ')
			m_pName[n] = ' ';
	}
}

void GXMLAttribute::SetValue(const char* szValue)
{
	delete(m_pValue);
	m_pValue = new char[strlen(szValue) + 1];
	strcpy(m_pValue, szValue);
}

int GXMLAttribute::Compare(GBucket* pBucket)
{
	return stricmp(m_pName, ((GXMLAttribute*)pBucket)->m_pName);
}

GXMLAttribute* GXMLAttribute::Copy()
{
	return new GXMLAttribute(GetName(), GetValue());
}

int EscapeAttrChar(char c, char* pBuffer)
{
	if(c < ' ')
	{
		char szTmp[32];
		sprintf(szTmp, "%x", c);
		if(pBuffer)
		{
			strcpy(pBuffer, "&#x");
			strcat(pBuffer, szTmp);
			strcat(pBuffer, ";");
		}
		return 4 + strlen(szTmp);
	}
	else
	{
		switch(c)
		{
		case '&':
			if(pBuffer)
				strcpy(pBuffer, "&amp;");
			return 5;
		case '<':
			if(pBuffer)
				strcpy(pBuffer, "&lt;");
			return 4;
		case '>':
			if(pBuffer)
				strcpy(pBuffer, "&gt;");
			return 4;
		case '"':
			if(pBuffer)
				strcpy(pBuffer, "&quot;");
			return 6;
		default:
			if(pBuffer)
				*pBuffer = c;
			return 1;
		}
	}
}

int GXMLAttribute::ToString(char* pBuffer, bool bEscapeQuotes)
{
	int nPos = 0;
	if(pBuffer)
		pBuffer[nPos] = ' ';
	nPos++;
	if(pBuffer)
		strcpy(&pBuffer[nPos], m_pName);
	nPos += strlen(m_pName);
	if(bEscapeQuotes)
	{
		if(pBuffer)
			strcpy(&pBuffer[nPos], "=\\\"");
		nPos += 3;
	}
	else
	{
		if(pBuffer)
			strcpy(&pBuffer[nPos], "=\"");
		nPos += 2;
	}
	char* pChar = m_pValue;
	while(*pChar != '\0')
	{
		nPos += EscapeAttrChar(*pChar, pBuffer ? &pBuffer[nPos] : NULL);
		pChar++;
	}
	if(bEscapeQuotes)
	{
		if(pBuffer)
			strcpy(&pBuffer[nPos], "\\\"");
		nPos += 2;
	}
	else
	{
		if(pBuffer)
			pBuffer[nPos] = '"';
		nPos++;
	}
	return nPos;
}



// **************************************************************

GXMLTag::GXMLTag(const char* szName) : GBucket()
{
	m_pChildren = new GLList();
	m_pAttributes = new GLList();
	m_pParent = NULL;
	m_pName = NULL;
	SetName(szName);
	m_nLineNumber = 0;
	m_nColumnAndWidth = 0;
#ifdef _DEBUG
	m_DEBUG_ONLY_attributes[0] = NULL;
	m_DEBUG_ONLY_attributes[1] = NULL;
	m_DEBUG_ONLY_attributes[2] = NULL;
	m_DEBUG_ONLY_attributes[3] = NULL;
#endif
}

GXMLTag::GXMLTag(const char* pName, int nLength) : GBucket()
{
	m_pChildren = new GLList();
	m_pAttributes = new GLList();
	m_pParent = NULL;
	m_pName = new char[nLength + 1];
	memcpy(m_pName, pName, nLength);
	m_pName[nLength] = '\0';
	m_nLineNumber = 0;
#ifdef _DEBUG
	m_DEBUG_ONLY_attributes[0] = NULL;
	m_DEBUG_ONLY_attributes[1] = NULL;
	m_DEBUG_ONLY_attributes[2] = NULL;
	m_DEBUG_ONLY_attributes[3] = NULL;
#endif
}

GXMLTag::~GXMLTag()
{
	delete(m_pChildren);
	delete(m_pAttributes);
	delete(m_pName);
}

void GXMLTag::SetName(const char* szName)
{
	delete(m_pName);
	m_pName = new char[strlen(szName) + 1];
	strcpy(m_pName, szName);
}

int GXMLTag::Compare(GBucket* pBucket)
{
	return stricmp(m_pName, ((GXMLTag*)pBucket)->m_pName);
}

GXMLTag* GXMLTag::Copy()
{
	GXMLTag* pNewTag = new GXMLTag(GetName());
	GXMLAttribute* pAttr;
	for(pAttr = GetFirstAttribute(); pAttr; pAttr = GetNextAttribute(pAttr))
		pNewTag->AddAttribute(pAttr->Copy());
	GXMLTag* pChildTag;
	for(pChildTag = GetFirstChildTag(); pChildTag; pChildTag = GetNextChildTag(pChildTag))
		pNewTag->AddChildTag(pChildTag->Copy());
	return pNewTag;
}

GXMLAttribute* GXMLTag::GetAttribute(const char* szName)
{
	GXMLAttribute* pAttr;
	for(pAttr = GetFirstAttribute(); pAttr; pAttr = GetNextAttribute(pAttr))
	{
		if(stricmp(szName, pAttr->GetName()) == 0)
			return pAttr;
	}
	return NULL;
}

GXMLTag* GXMLTag::GetChildTag(const char* szName)
{
	GXMLTag* pChildTag;
	for(pChildTag = GetFirstChildTag(); pChildTag; pChildTag = GetNextChildTag(pChildTag))
	{
		if(stricmp(szName, pChildTag->GetName()) == 0)
			return pChildTag;
	}
	return NULL;
}

GXMLTag* GXMLTag::FindChildWithAttribute(const char* szAttrName, const char* szAttrValue)
{
	GXMLTag* pChildTag;
	GXMLAttribute* pAttr;
	for(pChildTag = GetFirstChildTag(); pChildTag; pChildTag = GetNextChildTag(pChildTag))
	{
		pAttr = pChildTag->GetAttribute(szAttrName);
		if(!pAttr)
			continue;
		if(stricmp(pAttr->GetValue(), szAttrValue) == 0)
			return pChildTag;
	}
	return NULL;
}

void GXMLTag::AddAttribute(GXMLAttribute* pAttribute)
{
	GAssert(pAttribute,"Null parameter");
	if(!pAttribute)
		return;
	m_pAttributes->Link(pAttribute);
#ifdef _DEBUG
	if(m_pAttributes->GetCount() < 5)
		m_DEBUG_ONLY_attributes[m_pAttributes->GetCount() - 1] = pAttribute;
#endif
}

char* GXMLTag::ToString(const char* szLineStart, const char* szLineEnd, bool bEscapeQuotes)
{
	int nSize = ToString(NULL, 0, szLineStart, szLineEnd, bEscapeQuotes);
	char* pBuffer = new char[nSize + 1];
	int nSize2 = ToString(pBuffer, 0, szLineStart, szLineEnd, bEscapeQuotes);
	GAssert(nSize2 == nSize, "size changed");
	pBuffer[nSize] = '\0';
	return pBuffer;
}

char* GXMLTag::ToString()
{
#ifdef WIN32
	return ToString("", "\r\n", false);
#else // WIN32
	return ToString("", "\n", false);
#endif // !WIN32
}

int GXMLTag::ToString(char* pBuffer, int nTabs, const char* szLineStart, const char* szLineEnd, bool bEscapeQuotes)
{
	int nPos = 0;
	if(pBuffer)
		memset(pBuffer, '\t', nTabs);
	nPos += nTabs;
	int nLineStartLength = strlen(szLineStart);
	int nLineEndLength = strlen(szLineEnd);
	if(pBuffer)
		memcpy(&pBuffer[nPos], szLineStart, nLineStartLength);
	nPos += nLineStartLength;
	if(pBuffer)
		pBuffer[nPos] = '<';
	nPos++;
	if(pBuffer)
		strcpy(&pBuffer[nPos], m_pName);
	nPos += strlen(m_pName);
	GXMLAttribute* pAttr;
	for(pAttr = GetFirstAttribute(); pAttr; pAttr = GetNextAttribute(pAttr))
		nPos += pAttr->ToString(pBuffer ? &pBuffer[nPos] : NULL, bEscapeQuotes);
	if(m_pChildren->GetCount() > 0)
	{
		if(pBuffer)
			strcpy(&pBuffer[nPos], ">");
		nPos += 1;
		if(pBuffer)
			memcpy(&pBuffer[nPos], szLineEnd, nLineEndLength);
		nPos += nLineEndLength;
		GXMLTag* pChild;
		for(pChild = GetFirstChildTag(); pChild; pChild = GetNextChildTag(pChild))
			nPos += pChild->ToString(pBuffer ? &pBuffer[nPos] : NULL, nTabs + 1, szLineStart, szLineEnd, bEscapeQuotes);
		if(pBuffer)
			memset(&pBuffer[nPos], '\t', nTabs);
		nPos += nTabs;
		if(pBuffer)
			memcpy(&pBuffer[nPos], szLineStart, nLineStartLength);
		nPos += nLineStartLength;
		if(pBuffer)
			strcpy(&pBuffer[nPos], "</");
		nPos += 2;
		if(pBuffer)
			strcpy(&pBuffer[nPos], m_pName);
		nPos += strlen(m_pName);
		if(pBuffer)
			strcpy(&pBuffer[nPos], ">");
		nPos += 1;
		if(pBuffer)
			memcpy(&pBuffer[nPos], szLineEnd, nLineEndLength);
		nPos += nLineEndLength;
	}
	else
	{
		if(pBuffer)
			strcpy(&pBuffer[nPos], " />");
		nPos += 3;
		if(pBuffer)
			memcpy(&pBuffer[nPos], szLineEnd, nLineEndLength);
		nPos += nLineEndLength;
	}
	return nPos;
}

/*static*/ GXMLTag* GXMLTag::FromString(const char* pBuffer, int nSize, const char** pszErrorMessage /*=NULL*/, int* pnErrorOffset /*=NULL*/, int* pnErrorLine /*=NULL*/, int* pnErrorColumn /*=NULL*/)
{
	if(nSize < 1)
	{
		if(pszErrorMessage)
			*pszErrorMessage = "an empty string is not valid XML";
		if(pnErrorOffset)
			*pnErrorOffset = 0;
		if(pnErrorLine)
			*pnErrorLine = 1;
		if(pnErrorColumn)
			*pnErrorColumn = 0;
		return NULL;
	}
	GXMLParser parser(pBuffer, nSize);
	return parser.Parse(pszErrorMessage, pnErrorOffset, pnErrorLine, pnErrorColumn);
}

bool GXMLTag::ToFile(const char* szFilename)
{
	FileHolder hFile(fopen(szFilename, "w"));
	FILE* pFile = hFile.Get();
	if(!pFile)
		return false;
	return ToFile(pFile);
}

bool GXMLTag::ToFile(FILE* pFile)
{
	Holder<char*> hBuffer(ToString());
	char* pBuffer = hBuffer.Get();
	if(!pBuffer)
		return false;
	fwrite(pBuffer, strlen(pBuffer) + 1, 1, pFile);
	return true;
}

bool GXMLTag::ToCppFile(const char* szFilename, const char* szVarName, const char* szHeader)
{
	FILE* pFile = fopen(szFilename, "w");
	if(!pFile)
		return false;
	char* pBuffer = ToString("\"", "\"\n", true);
	if(!pBuffer)
	{
		fclose(pFile);
		return false;
	}
	fwrite(szHeader, strlen(szHeader), 1, pFile);
	const char* szDecl = "const char* ";
	fwrite(szDecl, strlen(szDecl), 1, pFile);
	fwrite(szVarName, strlen(szVarName), 1, pFile);
	const char* szHead = " = \n";
	fwrite(szHead, strlen(szHead), 1, pFile);
	fwrite(pBuffer, strlen(pBuffer), 1, pFile);
	const char* szTail = ";\n\n";
	fwrite(szTail, strlen(szTail), 1, pFile);
	fclose(pFile);
	delete(pBuffer);
	return true;
}

/*static*/ GXMLTag* GXMLTag::FromFile(const char* szFilename, const char** pszErrorMessage /*=NULL*/, int* pnErrorOffset /*=NULL*/, int* pnErrorLine /*=NULL*/, int* pnErrorColumn /*=NULL*/)
{
	if(pszErrorMessage)
		*pszErrorMessage = "<unknown error>";
	if(pnErrorOffset)
		*pnErrorOffset = 0;
	if(pnErrorLine)
		*pnErrorLine = 0;
	if(pnErrorColumn)
		*pnErrorColumn = 0;

	FILE* pFile = fopen(szFilename, "rb");
	if(!pFile)
	{
		if(pszErrorMessage)
			*pszErrorMessage = "File not found";
		return NULL;
	}
	int nFileSize = filelength(fileno(pFile));
	char* pBuffer = new char[nFileSize + 1];
	if(!pBuffer)
	{
		if(pszErrorMessage)
			*pszErrorMessage = "Out of memory";
		fclose(pFile);
		return NULL;
	}
	int nBytesRead = fread(pBuffer, sizeof(char), nFileSize, pFile);
	int err = ferror(pFile);
	if(err != 0 || nBytesRead != nFileSize)
	{
		GAssert(false, "Error reading file");
		if(pszErrorMessage)
			*pszErrorMessage = "IO error reading file";
		fclose(pFile);
		delete(pBuffer);
		return NULL;
	}
	pBuffer[nFileSize] = '\0';
	GXMLTag* pTag = FromString(pBuffer, nFileSize, pszErrorMessage, pnErrorOffset, pnErrorLine, pnErrorColumn);
	fclose(pFile);
	delete(pBuffer);
	return pTag;
}

