#include "GHtml.h"
#include "GHashTable.h"

typedef void (GHtml::*GHtmlTagHandler)();

struct GHtmlTagHandlerStruct
{
	const char* szTagName;
	GHtmlTagHandler pHandler;
};

struct GHtmlTagHandlerStruct GHtml::g_GHtmlTagHandlerTable[] = 
{
	{"!DOCTYPE", &GHtml::IgnoreTag},
	{"a", &GHtml::OpenLinkTag},
	{"/a", &GHtml::CloseLinkTag},
	{"b", &GHtml::OpenBoldTag},
	{"/b", &GHtml::CloseBoldTag},
	{"big", &GHtml::OpenBigTag},
	{"/big", &GHtml::CloseBigTag},
	{"body", &GHtml::OpenBodyTag},
	{"/body", &GHtml::CloseBodyTag},
	{"br", &GHtml::IgnoreTag},
	{"center", &GHtml::OpenCenterTag},
	{"/center", &GHtml::CloseCenterTag},
	{"font", &GHtml::IgnoreTag},
	{"/font", &GHtml::IgnoreTag},
	{"head", &GHtml::OpenHeadTag},
	{"/head", &GHtml::CloseHeadTag},
	{"html", &GHtml::OpenHtmlTag},
	{"/html", &GHtml::CloseHtmlTag},
	{"html", &GHtml::OpenItalicsTag},
	{"/html", &GHtml::CloseItalicsTag},
	{"img", &GHtml::IgnoreTag},
	{"link", &GHtml::IgnoreTag},
	{"meta", &GHtml::IgnoreTag},
	{"p", &GHtml::IgnoreTag},
	{"/p", &GHtml::IgnoreTag},
	{"script", &GHtml::IgnoreTag},
	{"/script", &GHtml::IgnoreTag},
	{"table", &GHtml::OpenTableTag},
	{"/table", &GHtml::CloseTableTag},
	{"tbody", &GHtml::IgnoreTag},
	{"/tbody", &GHtml::IgnoreTag},
	{"title", &GHtml::OpenTitleTag},
	{"/title", &GHtml::CloseTitleTag},
	{"td", &GHtml::IgnoreTag},
	{"/td", &GHtml::IgnoreTag},
	{"tr", &GHtml::IgnoreTag},
	{"/tr", &GHtml::IgnoreTag},
	{"u", &GHtml::OpenUnderlineTag},
	{"/u", &GHtml::CloseUnderlineTag},
	{"w", &GHtml::IgnoreTag},
	{"/w", &GHtml::IgnoreTag},
};

#define GHTML_TAG_HANDLER_TABLE_SIZE sizeof(g_GHtmlTagHandlerTable) / sizeof(struct GHtmlTagHandlerStruct)

// -----------------------------------------------------------------

GHtml::GHtml(char* pDoc, int nSize, bool bOwnDoc)
{
	m_bOwnDoc = bOwnDoc;
	m_pDoc = pDoc;
	m_nSize = nSize;
	m_nPos = 0;

	// Properties
	m_eProperties = 0;
	m_nTextSize = 0;
	m_nTableNests = 0;

	// Tag table
	m_pTagTable = new GConstStringHashTable(73, false);
	int n;
	for(n = 0; (unsigned int)n < GHTML_TAG_HANDLER_TABLE_SIZE; n++)
		m_pTagTable->Add(g_GHtmlTagHandlerTable[n].szTagName, &g_GHtmlTagHandlerTable[n]);
}

GHtml::~GHtml()
{
	if(m_bOwnDoc)
		delete(m_pDoc);
}

void GHtml::ParseTag()
{
	// Parse the tag name
	char szBuf[64];
	GAssert(m_pDoc[m_nPos] == '<', "not a tag");
	m_nPos++;
	while(m_pDoc[m_nPos] <= ' ' && m_nPos < m_nSize)
		m_nPos++;
	int nTagStart = m_nPos;
	while(m_pDoc[m_nPos] > ' ' && m_pDoc[m_nPos] != '>' && m_nPos < m_nSize)
		m_nPos++;
	int nTagNameLen = m_nPos - nTagStart;
	if(nTagNameLen > 63)
		nTagNameLen = 63;
	memcpy(szBuf, &m_pDoc[nTagStart], nTagNameLen);
	szBuf[nTagNameLen] = '\0';

	// Handle comment tags
	if(strncmp(szBuf, "!--", 3) == 0)
	{
		// Find the end of the comment--todo: this isn't quite right
		while(m_nPos < m_nSize && m_pDoc[m_nPos] != '>')
			m_nPos++;

		// Handle the comment
		OnComment(&m_pDoc[nTagStart + 3], m_nPos - nTagStart - 5);

		// Advance past the '>'
		if(m_nPos < m_nSize)
			m_nPos++;

		// Eat whitespace
		while(m_nPos < m_nSize && m_pDoc[m_nPos] <= ' ')
			m_nPos++;
		return;
	}

	// Handle the tag
	struct GHtmlTagHandlerStruct* pTagHandlerStruct = NULL;
	if(m_pTagTable->Get(szBuf, (void**)&pTagHandlerStruct))
	{
		(this->*pTagHandlerStruct->pHandler)();
	}
	else
	{
		//printf("Unhandled tag: <%s>\n", szBuf);
	}

	// Eat whitespace
	while(m_nPos < m_nSize && m_pDoc[m_nPos] <= ' ')
		m_nPos++;

	// Handle the params
	int nParamStart, nParamLen, nValueStart, nValueLen;
	while(m_pDoc[m_nPos] != '>' && m_nPos < m_nSize)
	{
		// Find the equals
		nParamStart = m_nPos;
		while(m_pDoc[m_nPos] != '=' && m_pDoc[m_nPos] != '>' && m_nPos < m_nSize)
			m_nPos++;
		nParamLen = m_nPos - nParamStart;
		m_nPos++;

		// Eat whitespace
		while(m_nPos < m_nSize && m_pDoc[m_nPos] <= ' ')
			m_nPos++;

		if(m_pDoc[m_nPos] == '"')
		{
			// Move past the '"'
			m_nPos++;
			nValueStart = m_nPos;

			// Find the close quote
			while(m_nPos < m_nSize && m_pDoc[m_nPos] != '"')
				m_nPos++;
			if(m_nPos >= m_nSize)
				break;
			nValueLen = m_nPos - nValueStart;
			m_nPos++;
		}
		else
		{
			// Move until we hit whitespace again
			nValueStart = m_nPos;
			while(m_nPos < m_nSize && m_pDoc[m_nPos] > ' ' && m_pDoc[m_nPos] != '>')
				m_nPos++;
			nValueLen = m_nPos - nValueStart;
		}

		// Handle the parameter
		OnTagParam(&m_pDoc[nTagStart], nTagNameLen, &m_pDoc[nParamStart], nParamLen, &m_pDoc[nValueStart], nValueLen);

		// Eat whitespace
		while(m_nPos < m_nSize && m_pDoc[m_nPos] <= ' ')
			m_nPos++;
	}

	// Advance past the '>'
	if(m_nPos < m_nSize)
	{
		GAssert(m_pDoc[m_nPos] == '>', "unexpected condition");
		m_nPos++;
	}

	// Eat whitespace
	while(m_nPos < m_nSize && m_pDoc[m_nPos] <= ' ')
		m_nPos++;
}

void GHtml::Parse()
{
	while(m_nPos < m_nSize)
	{
		// Parse tags
		while(true)
		{
			if(m_pDoc[m_nPos] != '<')
				break;
			ParseTag();
			if(m_nPos >= m_nSize)
				return;
		}

		// Parse text
		int nChunkStart = m_nPos;
		m_nPos++;
		while(m_nPos < m_nSize && m_pDoc[m_nPos] != '<')
			m_nPos++;
		OnTextChunk(&m_pDoc[nChunkStart], m_nPos - nChunkStart);
	}
}

void GHtml::IgnoreTag()
{
}

void GHtml::OpenHtmlTag()
{
	m_eProperties |= html;
}

void GHtml::CloseHtmlTag()
{
	m_eProperties &= ~html;
}

void GHtml::OpenHeadTag()
{
	m_eProperties |= head;
}

void GHtml::CloseHeadTag()
{
	m_eProperties &= ~head;
}

void GHtml::OpenBodyTag()
{
	m_eProperties |= body;
}

void GHtml::CloseBodyTag()
{
	m_eProperties &= ~body;
}

void GHtml::OpenBigTag()
{
	m_nTextSize++;
}

void GHtml::CloseBigTag()
{
	m_nTextSize--;
}

void GHtml::OpenCenterTag()
{
	m_eProperties |= center;
}

void GHtml::CloseCenterTag()
{
	m_eProperties &= ~center;
}

void GHtml::OpenTitleTag()
{
	m_eProperties |= title;
}

void GHtml::CloseTitleTag()
{
	m_eProperties &= ~title;
}

void GHtml::OpenTableTag()
{
	m_nTableNests++;
}

void GHtml::CloseTableTag()
{
	m_nTableNests--;
}

void GHtml::OpenLinkTag()
{
	m_eProperties |= link;
}

void GHtml::CloseLinkTag()
{
	m_eProperties &= ~link;
}

void GHtml::OpenUnderlineTag()
{
	m_eProperties |= underline;
}

void GHtml::CloseUnderlineTag()
{
	m_eProperties &= ~underline;
}

void GHtml::OpenBoldTag()
{
	m_eProperties |= bold;
}

void GHtml::CloseBoldTag()
{
	m_eProperties &= ~bold;
}

void GHtml::OpenItalicsTag()
{
	m_eProperties |= italics;
}

void GHtml::CloseItalicsTag()
{
	m_eProperties &= ~italics;
}

