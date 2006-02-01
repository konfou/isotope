/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "GGrammar.h"
#include "GParseTree.h"
#include "GStream.h"

// ***** Static globals
GGrammarRule::PARSE_ERRORS GGrammarRule::s_nLastError = GGrammarRule::PE_NO_ERROR;
// *****

GGrammarRule::GGrammarRule()
{
	m_nStackBase = -1;
	m_pPTNStack = NULL;
}

GGrammarRule::~GGrammarRule()
{
	m_nStackBase = -1;
	Flush();
}

void GGrammarRule::Flush()
{
	if(m_pPTNStack)
	{
		int n;
		int nCount = m_pPTNStack->GetSize();
		for(n = nCount - 1; n > m_nStackBase; n--)
		{
			delete((GParseTreeNode*)m_pPTNStack->GetPointer(n));
			m_pPTNStack->DeleteCell(n);
		}
		if(m_nStackBase < 0)
		{
			delete(m_pPTNStack);
			m_pPTNStack = NULL;
		}
	}
}

void GGrammarRule::EatWhiteSpace(GStream* pStream)
{
	while(IsWhiteSpace(pStream->Peek(0)))
		pStream->Eat(1);
}

GParseTreeNode* GGrammarRule::GetParseTreeNode()
{
	if(m_pPTNStack)
	{
		if(m_nStackBase < m_pPTNStack->GetSize())
			return (GParseTreeNode*)m_pPTNStack->GetPointer(m_nStackBase);
		else
			return NULL;
	}
	else
		return NULL;
}

void GGrammarRule::SetParseTreeNode(GParseTreeNode* pPTN)
{
	if(!m_pPTNStack)
		m_pPTNStack = new GPointerArray(16);
	while(m_pPTNStack->GetSize() <= m_nStackBase)
		m_pPTNStack->AddPointer(NULL);
	m_pPTNStack->SetPointer(m_nStackBase, pPTN);
}


// ***********************************************************

GStructureGrammarRule::GStructureGrammarRule() : GGrammarRule()
{
	m_pFields = new GPointerArray(16);
}

GStructureGrammarRule::~GStructureGrammarRule()
{
	delete(m_pFields);
}

int GStructureGrammarRule::GetChildCount()
{
	GStructureParseTreeNode* pStructurePTN = (GStructureParseTreeNode*)GetParseTreeNode();
	if(pStructurePTN)
		return pStructurePTN->GetChildCount();
	else
		return 0;
}

void GStructureGrammarRule::AddParsedChild(GParseTreeNode* pChild)
{
	GStructureParseTreeNode* pStructurePTN = (GStructureParseTreeNode*)GetParseTreeNode();
	if(!pStructurePTN)
	{
		if(GetParseTreeNode() != NULL)
			GAssert(false, "Error, memory leak!");
		pStructurePTN = new GStructureParseTreeNode(this);
		SetParseTreeNode(pStructurePTN);
	}
	pStructurePTN->AddChild(pChild);
}

void GStructureGrammarRule::AddField(GGrammarRule* pField)
{
	m_pFields->AddPointer(pField);
}

GParseTreeNode* GStructureGrammarRule::Parse(GStream* pStream)
{
	// Increment stack base now and decrement it when this method returns
	StackBaseAjuster SBATmp(this);

	unsigned int nStartPos = pStream->GetCount();

	// Try to parse a child for each field
	GGrammarRule* pField;
	GParseTreeNode* pChild;
	int n;
	while((n = GetChildCount()) < m_pFields->GetSize())
	{
		pField = (GGrammarRule*)m_pFields->GetPointer(n);
		pChild = pField->Parse(pStream);
		if(pChild)
			AddParsedChild(pChild);
		else
			return NULL; // Failed
	}

	// Successful
	GParseTreeNode* pPTN = GetParseTreeNode();
	if(!pPTN)
	{
		// Special case for empty structure grammar rule
		pPTN = new GStructureParseTreeNode(this); // This is here so it won't fail to parse and empty structure rule
		pPTN->m_nStartPos = pStream->GetCount();
	}
	pPTN->m_nStartPos = nStartPos;
	pPTN->m_nEndPos = pStream->GetCount();
	SetParseTreeNode(NULL);

	return pPTN;
}

// ***********************************************************

GListGrammarRule::GListGrammarRule() : GGrammarRule()
{
	m_pListRule = NULL;
}

GListGrammarRule::~GListGrammarRule()
{

}

void GListGrammarRule::SetListRule(GGrammarRule* pListRule)
{
	m_pListRule = pListRule;
}

GParseTreeNode* GListGrammarRule::Parse(GStream* pStream)
{
	// Increment stack base now and decrement it when this method returns
	StackBaseAjuster SBATmp(this);

	if(!m_pListRule)
		GAssert(false, "Error, no list rule set");

	// Make the Under-Construction node
	GListParseTreeNode* pListPTN = (GListParseTreeNode*)GetParseTreeNode();
	if(!pListPTN)
	{
		pListPTN = new GListParseTreeNode(this);
		SetParseTreeNode(pListPTN);
		pListPTN->m_nStartPos = pStream->GetCount();
	}
	
	// Parse as many children as we can get
	GParseTreeNode* pPTN;
	while(true)
	{
		pPTN = m_pListRule->Parse(pStream);
		if(pPTN)
			pListPTN->AddChild(pPTN);
		else
		{
			if(s_nLastError == PE_NEED_MORE_DATA)
				return NULL;
			else
			{
				// Successful
				pListPTN->m_nEndPos = pStream->GetCount();
				SetParseTreeNode(NULL);
				return pListPTN;
			}
		}
	}
}

// ***********************************************************

GNonDetGrammarRule::GNonDetGrammarRule() : GGrammarRule()
{
	m_pChoices = new GPointerArray(16);
}

GNonDetGrammarRule::~GNonDetGrammarRule()
{
	delete(m_pChoices);
}

void GNonDetGrammarRule::AddChoice(GGrammarRule* pChoice)
{
	m_pChoices->AddPointer(pChoice);
}


GParseTreeNode* GNonDetGrammarRule::Parse(GStream* pStream)
{
	// Increment stack base now and decrement it when this method returns
	StackBaseAjuster SBATmp(this);

	int nChoices = m_pChoices->GetSize();
	int nChoice = 0;
	GNonDetParseTreeNode* pNonDetPTN = (GNonDetParseTreeNode*)GetParseTreeNode();
	unsigned int nStreamStartPos;
	if(pNonDetPTN)
	{
		nChoice = pNonDetPTN->m_nWhichOne;
		nStreamStartPos = pNonDetPTN->m_nStartPos;
	}
	else
		nStreamStartPos = pStream->GetCount();
	GParseTreeNode* pPTN;
	GGrammarRule* pChoice;
	while(nChoice < nChoices)
	{
		pChoice = (GGrammarRule*)m_pChoices->GetPointer(nChoice);
		pPTN = pChoice->Parse(pStream);
		if(pPTN)
		{
			// Successful
			if(!pNonDetPTN)
			{
				pNonDetPTN = new GNonDetParseTreeNode(this);
				pNonDetPTN->m_nStartPos = nStreamStartPos;
				pNonDetPTN->m_nWhichOne = nChoice;
			}
			else
				SetParseTreeNode(NULL);
			if(pNonDetPTN->m_pChild)
				GAssert(false, "Memory leak!");
			pNonDetPTN->m_pChild = pPTN;
			pNonDetPTN->m_nEndPos = pStream->GetCount();
			return(pNonDetPTN);
		}
		else
		{
			if(s_nLastError == PE_NEED_MORE_DATA)
			{
				if(!pNonDetPTN && pStream->GetCount() != nStreamStartPos)
				{
					// Remember which one for next time
					pNonDetPTN = new GNonDetParseTreeNode(this);
					pNonDetPTN->m_nStartPos = nStreamStartPos;
					pNonDetPTN->m_nWhichOne = nChoice;
					SetParseTreeNode(pNonDetPTN);
				}
				return NULL;
			}
			else
			{
				if(pNonDetPTN || pStream->GetCount() != nStreamStartPos)
					return NULL;
			}
		}

		if(pNonDetPTN)
			break;
		nChoice++;
	}

	s_nLastError = PE_EXPECTED_SOMETHING_ELSE;
	return NULL;
}


// ***********************************************************

GSyntaxGrammarRule::GSyntaxGrammarRule() : GGrammarRule()
{
	m_szSyntax = NULL;
}

GSyntaxGrammarRule::~GSyntaxGrammarRule()
{
	delete(m_szSyntax);
}

void GSyntaxGrammarRule::SetSyntax(char* szSyntax)
{
	delete(m_szSyntax);
	m_szSyntax = new char[strlen(szSyntax) + 1];
	strcpy(m_szSyntax, szSyntax);
}

GParseTreeNode* GSyntaxGrammarRule::Parse(GStream* pStream)
{
	if(!m_szSyntax)
		GAssert(false, "Error, syntax not set");
	
	// See if we got all the stuff we need
	char cTheStoopidRule = '\0';
	unsigned int nStreamPos = 0;
	unsigned int nSize = pStream->GetSize();
	unsigned int n;
	for(n = 0; m_szSyntax[n] != '\0'; n++)
	{
		// Special cases (enclosed in brackets)
		if(m_szSyntax[n] == '[')
		{
			bool bIsRepeatBrace = false;
			switch(m_szSyntax[n + 1])
			{
				case '0':
				case 'O':
				case 'o':	cTheStoopidRule = m_szSyntax[n + 2];	break;
				case '[':	bIsRepeatBrace = true;					break;
				default:											break;
			}
			if(bIsRepeatBrace)
				n++;
			else
			{
				while(m_szSyntax[n] != ']' && m_szSyntax[n + 1] != '\0')
					n++;
				continue;
			}
		}
		if(nStreamPos >= nSize)
		{
			s_nLastError = PE_NEED_MORE_DATA;
			return NULL;
		}
		while(IsWhiteSpace(pStream->Peek(nStreamPos)))
		{
			nStreamPos++;
			if(nStreamPos >= nSize)
			{
				s_nLastError = PE_NEED_MORE_DATA;
				return NULL;
			}
		}
		if(IsSame(pStream->Peek(nStreamPos), m_szSyntax[n]))
			nStreamPos++;
		else
		{
			// The syntax doesn't match.  This will be an error unless the Stoopid rule applies here
			if(cTheStoopidRule != '\0' && IsSame(pStream->Peek(nStreamPos), cTheStoopidRule))
			{
				// The stoopid rule says that if the syntax is "... <o)>, ..." then the ','
				// is optional if the token where we would expect the comma is a ')'.  If
				// that happens, then we act as if there were a ',' and don't eat the ')'
				// and continue as normal.  It works with other characters too, not just
				// ',' and ')'.  I know that's a stoopid rule, but it's necessary so that
				// lists of things separated by commas don't have to end with a comma
				// after the last item in the list.
			}
			else
			{
				s_nLastError = PE_EXPECTED_SOMETHING_ELSE;
				return NULL;
			}
		}
		cTheStoopidRule = '\0';
	}

	// Successful
	GParseTreeNode* pPTN = new GSyntaxParseTreeNode(this);
	pPTN->m_nStartPos = pStream->GetCount();
	pStream->Eat(nStreamPos);
	pPTN->m_nEndPos = pStream->GetCount();
	return pPTN;
}

// ***********************************************************

GTextGrammarRule::GTextGrammarRule() : GGrammarRule()
{

}

GTextGrammarRule::~GTextGrammarRule()
{

}

bool GTextGrammarRule::IsRightTypeOfChar(char c)
{
	switch(m_nType)
	{
		case TT_DECIMAL_VALUE:		return IsNumber(c);
		case TT_IDENTIFIER:			return IsAlphaNumeric(c);
		default:					GAssert(false, "Error, unknown type");
	}
	return false;
}

GParseTreeNode* GTextGrammarRule::ParseQuotedString(GStream* pStream)
{
	// Move past white space
	unsigned int nSize = pStream->GetSize();
	unsigned int n;
	for(n = 0; IsWhiteSpace(pStream->Peek(n)); n++)
	{
		if(n >= nSize)
		{
			s_nLastError = PE_NEED_MORE_DATA;
			return NULL;
		}
	}
	unsigned int nStartPos = pStream->GetCount();

	// Check for quotation mark
	if(pStream->Peek(n) != '"')
	{
		s_nLastError = PE_EXPECTED_SOMETHING_ELSE;
		return NULL;
	}
	n++;
	if(n >= nSize)
	{
		s_nLastError = PE_NEED_MORE_DATA;
		return NULL;
	}

	// Move to the closing quotation mark
	char c;
	while((c = pStream->Peek(n)) != '"')
	{
		if(c < 32)
		{
			s_nLastError = PE_EXPECTED_SOMETHING_ELSE;
			return NULL;
		}
		n++;
		if(n >= nSize)
		{
			s_nLastError = PE_NEED_MORE_DATA;
			return NULL;
		}
	}
	n++;

	// Succeed
	GParseTreeNode* pPTN = new GTextParseTreeNode(GTextParseTreeNode::PTNT_QUOTED_STRING, this);
	pPTN->m_nStartPos = nStartPos;
	pStream->Eat(n);
	pPTN->m_nEndPos = pStream->GetCount();
	return pPTN;
}

GParseTreeNode* GTextGrammarRule::ParseWord(GStream* pStream)
{
	// Move past white space
	unsigned int nSize = pStream->GetSize();
	unsigned int n;
	for(n = 0; IsWhiteSpace(pStream->Peek(n)); n++)
	{
		if(n >= nSize)
		{
			s_nLastError = PE_NEED_MORE_DATA;
			return NULL;
		}
	}
	unsigned int nStartPos = pStream->GetCount() + n;

	// Make sure there's at least one of what we want
	char c = pStream->Peek(n);
	if(!IsRightTypeOfChar(c) || (m_nType == TT_IDENTIFIER && !IsAlpha(c)))
	{
		s_nLastError = PE_EXPECTED_SOMETHING_ELSE;
		return NULL;
	}
	n++;
	if(n >= nSize)
	{
		s_nLastError = PE_NEED_MORE_DATA;
		return NULL;
	}

	// Eat the rest of the token
	while(IsRightTypeOfChar(pStream->Peek(n)))
	{
		n++;
		if(n >= nSize)
		{
			s_nLastError = PE_NEED_MORE_DATA;
			return NULL;
		}
	}

	// Succeed
	GParseTreeNode* pPTN = NULL;
	switch(m_nType)
	{
		case TT_DECIMAL_VALUE:		pPTN = new GTextParseTreeNode(GTextParseTreeNode::PTNT_DECIMAL_VALUE, this);	break;
		case TT_IDENTIFIER:			pPTN = new GTextParseTreeNode(GTextParseTreeNode::PTNT_IDENTIFIER, this);		break;
		default:					GAssert(false, "Error, unknown type");
	}
	pPTN->m_nStartPos = nStartPos;
	pStream->Eat(n);
	pPTN->m_nEndPos = pStream->GetCount();
	return pPTN;
}

GParseTreeNode* GTextGrammarRule::ParseXMLText(GStream* pStream)
{
	// Move past white space
	unsigned int nSize = pStream->GetSize();
	unsigned int n;
	for(n = 0; IsWhiteSpace(pStream->Peek(n)); n++)
	{
		if(n >= nSize)
		{
			s_nLastError = PE_NEED_MORE_DATA;
			return NULL;
		}
	}
	unsigned int nStartPos = pStream->GetCount() + n;

	// Make sure there's at least one XML-text character
	char c = pStream->Peek(n);
	if(c == '<' || c == '>' || c < ' ')
	{
		s_nLastError = PE_EXPECTED_SOMETHING_ELSE;
		return NULL;
	}
	n++;
	if(n >= nSize)
	{
		s_nLastError = PE_NEED_MORE_DATA;
		return NULL;
	}

	// Eat the rest of the text
	while(pStream->Peek(n) != '<' && pStream->Peek(n) != '>')
	{
		n++;
		if(n >= nSize)
		{
			s_nLastError = PE_NEED_MORE_DATA;
			return NULL;
		}
	}

	// Back out the trailing white space
	int n2 = n;
	while(n2 > 0 && IsWhiteSpace(pStream->Peek(n2 - 1)))
		n2--;

	// Succeed
	GParseTreeNode* pPTN;
	pPTN = new GTextParseTreeNode(GTextParseTreeNode::PTNT_XML_TEXT, this);
	pPTN->m_nStartPos = nStartPos;
	pStream->Eat(n);
	pPTN->m_nEndPos = pStream->GetCount() - (n - n2);
	return pPTN;
}

GParseTreeNode* GTextGrammarRule::Parse(GStream* pStream)
{
	switch(m_nType)
	{
		case TT_QUOTED_STRING:
			return ParseQuotedString(pStream);
		case TT_DECIMAL_VALUE:
		case TT_IDENTIFIER:
			return ParseWord(pStream);
		case TT_XMLTEXT:
			return ParseXMLText(pStream);
		default:
			GAssert(false, "Error, unknown type");
	}
	return NULL;
}

