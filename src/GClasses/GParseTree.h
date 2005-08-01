/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GPARSETREE_H__
#define __GPARSETREE_H__

#include "GMacros.h"
#include "GArray.h"

class GGrammarRule;
class GStructureGrammarRule;
class GListGrammarRule;
class GNonDetGrammarRule;
class GSyntaxGrammarRule;
class GTextGrammarRule;


class GParseTreeNode
{
friend class GStructureGrammarRule;
friend class GListGrammarRule;
friend class GNonDetGrammarRule;
friend class GSyntaxGrammarRule;
friend class GTextGrammarRule;

protected:
	GGrammarRule* m_pRule;
	unsigned int m_nStartPos;
	unsigned int m_nEndPos;

public:
	enum ParseTreeNodeTypes
	{
		PTNT_STRUCTURE,
		PTNT_LIST,
		PTNT_NONDET,
		PTNT_SYNTAX,
		PTNT_IDENTIFIER,
		PTNT_DECIMAL_VALUE,
		PTNT_QUOTED_STRING,
		PTNT_XML_TEXT,
	};
	
	GParseTreeNode(GGrammarRule* pRule);
	virtual ~GParseTreeNode();

	ParseTreeNodeTypes GetType()		{ return m_eType; }
	GGrammarRule* GetRule()				{ return m_pRule; }
	unsigned int GetStartPos()			{ return m_nStartPos; }
	unsigned int GetEndPos()			{ return m_nEndPos; }

protected:
	ParseTreeNodeTypes m_eType;

};


class GStructureParseTreeNode : public GParseTreeNode
{
protected:
	GParseTreeNode** m_ppChildren;
	int m_nChildCount;
	int m_nFieldCount;

public:
	GStructureParseTreeNode(GStructureGrammarRule* pRule);
	virtual ~GStructureParseTreeNode();

	void AddChild(GParseTreeNode* pChild);
	int GetChildCount() { return m_nChildCount; }
	GParseTreeNode* GetChild(int n);
};


class GListParseTreeNode : public GParseTreeNode
{
protected:
	GSmallArray* m_pChildren;

public:
	GListParseTreeNode(GListGrammarRule* pRule);
	virtual ~GListParseTreeNode();

	void AddChild(GParseTreeNode* pNewChild) { m_pChildren->_AddCellByRef(&pNewChild); }
	GParseTreeNode* GetChild(int n) { return *(GParseTreeNode**)(m_pChildren->_GetCellRef(n)); }
	int GetChildCount() { return m_pChildren->GetSize(); }

};

class GNonDetParseTreeNode : public GParseTreeNode
{
friend class GNonDetGrammarRule;
protected:
	int m_nWhichOne;
	GParseTreeNode* m_pChild;

public:
	GNonDetParseTreeNode(GNonDetGrammarRule* pRule);
	virtual ~GNonDetParseTreeNode();

	GParseTreeNode* GetChild() { return m_pChild; }

};


class GSyntaxParseTreeNode : public GParseTreeNode
{
public:
	GSyntaxParseTreeNode(GGrammarRule* pRule);
	virtual ~GSyntaxParseTreeNode();

};


class GTextParseTreeNode : public GParseTreeNode
{
protected:
	char* szText;

public:
	GTextParseTreeNode(ParseTreeNodeTypes ePTNT, GGrammarRule* pRule);
	virtual ~GTextParseTreeNode();

};


#endif // __GPARSETREE_H__
