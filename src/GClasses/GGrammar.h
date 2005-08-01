/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GGRAMMAR_H__
#define __GGRAMMAR_H__

#include "GMacros.h"

//class GHash;
class GGrammarRule;
class GParseTreeNode;
class GLList;
class GStream;
class GStructureParseTreeNode;
class GListParseTreeNode;
class GPointerArray;

class GGrammar
{
protected:
//	GHash* m_pRules;
	GGrammarRule* m_pRoot;

public:
	GGrammar();
	virtual ~GGrammar();

};

class GGrammarRule
{
public:
	GGrammarRule();
	virtual ~GGrammarRule();

	enum PARSE_ERRORS
	{
		PE_NO_ERROR,
		PE_NEED_MORE_DATA,
		PE_EXPECTED_SOMETHING_ELSE,
	};

	virtual void Flush();

	// **** PURE VIRTUAL METHODS ***
	virtual GParseTreeNode* Parse(GStream* pStream) = 0;

protected:
	GPointerArray* m_pPTNStack;
	int m_nStackBase;

	// **** Static variables ***
	static PARSE_ERRORS s_nLastError;

	bool IsSame(char c1, char c2) { return((UCHAR(c1) == UCHAR(c2)) ? true : false); } // So you can turn case sensitivity on or off by rewriting this method
	bool IsWhiteSpace(char c) { return((c <= 32 && c > 0) ? true : false); }
	bool IsNumber(char c) { return((c >= '0' && c <= '9') ? true : false); }
	bool IsAlpha(char c) { return(( (c >= 'a' && c <= 'z') ||
									(c >= 'A' && c <= 'Z') ||
									(c == '_')
									) ? true : false); }
	bool IsAlphaNumeric(char c) { return(IsAlpha(c) || IsNumber(c)); }
	void EatWhiteSpace(GStream* pStream);
	GParseTreeNode* GetParseTreeNode();
	void SetParseTreeNode(GParseTreeNode* pPTN);

	// The following class is for use within GGrammarRule::Parse() ONLY.
	class StackBaseAjuster
	{
	protected:
		GGrammarRule* m_pParentThis;

	public:
		StackBaseAjuster(GGrammarRule* pParentThis)	{ m_pParentThis = pParentThis; m_pParentThis->m_nStackBase++; }
		~StackBaseAjuster()	{ m_pParentThis->m_nStackBase--; }
	};
friend class StackBaseAjuster;
};

class GStructureGrammarRule : public GGrammarRule
{
friend class GStructureParseTreeNode;
protected:
	GPointerArray* m_pFields;

	int GetChildCount();
	void AddParsedChild(GParseTreeNode* pChild);

public:
	GStructureGrammarRule();
	virtual ~GStructureGrammarRule();
	
	virtual GParseTreeNode* Parse(GStream* pStream);
	void AddField(GGrammarRule* pField);

};


class GListGrammarRule : public GGrammarRule
{
protected:
	GGrammarRule* m_pListRule;

public:
	GListGrammarRule();
	virtual ~GListGrammarRule();

	virtual GParseTreeNode* Parse(GStream* pStream);
	void SetListRule(GGrammarRule* pListRule);

};


class GNonDetGrammarRule : public GGrammarRule
{
protected:
	GPointerArray* m_pChoices;

public:
	GNonDetGrammarRule();
	virtual ~GNonDetGrammarRule();

	virtual GParseTreeNode* Parse(GStream* pStream);
	void AddChoice(GGrammarRule* pChoice);

};

class GSyntaxGrammarRule : public GGrammarRule
{
protected:
	char* m_szSyntax;

public:
	GSyntaxGrammarRule();
	virtual ~GSyntaxGrammarRule();

	virtual GParseTreeNode* Parse(GStream* pStream);
	void SetSyntax(char* szSyntax);

};

class GTextGrammarRule : public GGrammarRule
{
public:
	enum TEXT_TYPES
	{
		TT_QUOTED_STRING,
		TT_DECIMAL_VALUE,
		TT_IDENTIFIER,
		TT_XMLTEXT,
	};

	GTextGrammarRule();
	virtual ~GTextGrammarRule();

	virtual GParseTreeNode* Parse(GStream* pStream);
	void SetType(TEXT_TYPES tt) { m_nType = tt; }

protected:
	TEXT_TYPES m_nType;

	bool IsRightTypeOfChar(char c);
	GParseTreeNode* ParseQuotedString(GStream* pStream);
	GParseTreeNode* ParseWord(GStream* pStream);
	GParseTreeNode* ParseXMLText(GStream* pStream);

};


#endif // __GGRAMMAR_H__
