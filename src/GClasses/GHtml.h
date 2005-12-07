#ifndef __GHTML_H__
#define __GHTML_H__

class GConstStringHashTable;
struct GHtmlTagHandlerStruct;

// This class is for parsing HTML files. It's designed to be simple, not
// standards compliant, so it doesn't support anything but the most common
// HTML features. That's a feature, not a bug. This would be a very useful
// class for building a web-crawler that was interested in searching
// plain-text.
class GHtml
{
public:
	enum HtmlProperties
	{
		html		= 0x00000001,
		head		= 0x00000002,
		body		= 0x00000004,
		title		= 0x00000008,
		link		= 0x00000010,
		center		= 0x00000020,
		underline	= 0x00000040,
		bold		= 0x00000080,
		italics		= 0x00000100,
	};

protected:
	char* m_pDoc;
	int m_nSize;
	bool m_bOwnDoc;
	int m_nPos;
	GConstStringHashTable* m_pTagTable;
	static struct GHtmlTagHandlerStruct g_GHtmlTagHandlerTable[];

	// Properties
	unsigned int m_eProperties;
	int m_nTextSize;
	int m_nTableNests;

public:
	GHtml(char* pDoc, int nSize, bool bOwnDoc);
	virtual ~GHtml();

	void Parse();

	// This method will be called whenever the parser finds a section of
	// display text. You can get the meta-properties of the text chunk
	// from the member variables in this class.
	virtual void OnTextChunk(const char* pChunk, int nChunkSize) = 0;

	// This method will be called for each parameter attribute in a tag.
	// If you just want the display text, you can use an empty method.
	// If you're interested in hyperlink targets, you will receive that
	// information in here.
	virtual void OnTagParam(const char* pTagName, int nTagNameLen, const char* pParamName, int nParamNameLen, const char* pValue, int nValueLen) = 0;

protected:
	void ParseTag();
	void IgnoreTag();
	void OpenHtmlTag();
	void CloseHtmlTag();
	void OpenHeadTag();
	void CloseHeadTag();
	void OpenBodyTag();
	void CloseBodyTag();
	void OpenBigTag();
	void CloseBigTag();
	void OpenCenterTag();
	void CloseCenterTag();
	void OpenTitleTag();
	void CloseTitleTag();
	void OpenTableTag();
	void CloseTableTag();
	void OpenLinkTag();
	void CloseLinkTag();
	void OpenUnderlineTag();
	void CloseUnderlineTag();
	void OpenBoldTag();
	void CloseBoldTag();
	void OpenItalicsTag();
	void CloseItalicsTag();
};

#endif // __GHTML_H__
