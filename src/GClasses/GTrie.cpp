/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GTrie.h"
#include "GMacros.h"

struct GTrieNode
{
	struct GTrieNode* pZero;
	struct GTrieNode* pOne;
};

GTrie::GTrie()
{
	m_nCount = 0;
	m_pRoot = new struct GTrieNode;
	m_pRoot->pZero = NULL;
	m_pRoot->pOne = NULL;
}

GTrie::~GTrie()
{
	RecurseDestructTrieNode(m_pRoot);
}

void GTrie::CopyTrie(GTrie* pSource)
{
	RecurseDestructTrieNode(m_pRoot);
	m_pRoot = RecurseCopyTrieNode(pSource->m_pRoot);
	m_nCount = pSource->m_nCount;
}

struct GTrieNode* GTrie::RecurseCopyTrieNode(struct GTrieNode* pNode)
{
	struct GTrieNode* pNewTrieNode = new struct GTrieNode;
	if(pNode->pZero)
	{
		if(pNode->pZero == pNode)
			pNewTrieNode->pZero = pNewTrieNode;
		else
			pNewTrieNode->pZero = RecurseCopyTrieNode(pNode->pZero);
	}
	else
		pNewTrieNode->pZero = NULL;
	if(pNode->pOne)
	{
		if(pNode->pOne == pNode)
			pNewTrieNode->pOne = pNewTrieNode;
		else
			pNewTrieNode->pOne = RecurseCopyTrieNode(pNode->pOne);
	}
	else
		pNewTrieNode->pOne = NULL;
	return pNewTrieNode;
}

void GTrie::RecurseDestructTrieNode(struct GTrieNode* pNode)
{
	if(pNode->pZero && pNode->pZero != pNode)
		RecurseDestructTrieNode(pNode->pZero);
	if(pNode->pOne && pNode->pOne != pNode)
		RecurseDestructTrieNode(pNode->pOne);
	delete(pNode);
	m_nCount--;
}

void GTrie::Add(unsigned int nValue)
{
	struct GTrieNode* pNode = m_pRoot;
	unsigned int nMask = 0x80000000;
	int n;
	for(n = 0; n < (int)sizeof(unsigned int) * 8 - 1; n++)
	{
		if(nValue & nMask)
		{
			if(!pNode->pOne)
			{
				pNode->pOne = new struct GTrieNode;
				pNode->pOne->pZero = NULL;
				pNode->pOne->pOne = NULL;
			}
			pNode = pNode->pOne;
		}
		else
		{
			if(!pNode->pZero)
			{
				pNode->pZero = new struct GTrieNode;
				pNode->pZero->pZero = NULL;
				pNode->pZero->pOne = NULL;
			}
			pNode = pNode->pZero;
		}
		nMask >>= 1;
	}
	GAssert(nMask == 1, "internal error");
	if(nValue & 1)
		pNode->pOne = pNode; // The last node points back to itself for true, NULL for false
	else
		pNode->pZero = pNode;  // The last node points back to itself for true, NULL for false
	m_nCount++;
}

bool GTrie::Check(unsigned int nValue)
{
	struct GTrieNode* pNode = m_pRoot;
	unsigned int nMask = 0x80000000;
	int n;
	for(n = 0; n < (int)sizeof(unsigned int) * 8 - 1; n++)
	{
		if(nValue & nMask)
			pNode = pNode->pOne;
		else
			pNode = pNode->pZero;
		if(!pNode)
			return false;
		nMask >>= 1;
	}
	return true;
}

bool GTrie::Remove(unsigned int nValue)
{
	struct GTrieNode* pStack[sizeof(unsigned int) * 8];
	struct GTrieNode* pNode = m_pRoot;
	unsigned int nMask = 0x80000000;
	int n;
	for(n = 0; n < (int)sizeof(unsigned int) * 8; n++)
	{
		pStack[n] = pNode;
		if(nValue & nMask)
			pNode = pNode->pOne;
		else
			pNode = pNode->pZero;
		if(!pNode)
			return false;
		nMask >>= 1;
	}
	if(nValue & 1)
		pNode->pOne = NULL;
	else
		pNode->pZero = NULL;
	for(n = sizeof(unsigned int) * 8 - 1; n > 0; n--)
	{
		if(pStack[n]->pOne == NULL && pStack[n]->pZero == NULL)
		{
			if(pStack[n - 1]->pZero == pStack[n])
			{
				pStack[n - 1]->pZero = NULL;
			}
			else
			{
				GAssert(pStack[n - 1]->pOne == pStack[n], "internal error");
				pStack[n - 1]->pOne = NULL;
			}
		}
		else
			break;
		delete(pStack[n]);
	}
	m_nCount--;
	return true;
}

unsigned int GTrie::GetSmallest()
{
	GAssert(m_nCount > 0, "No values to get");
	struct GTrieNode* pNode = m_pRoot;
	unsigned int nValue = 0;
	int n;
	for(n = 0; n < (int)sizeof(unsigned int) * 8; n++)
	{
		nValue <<= 1;
		if(pNode->pZero)
			pNode = pNode->pZero;
		else
		{
			pNode = pNode->pOne;
			GAssert(pNode, "Internal error");
			nValue |= 1;
		}
	}
	return nValue;
}

unsigned int GTrie::GetBiggest()
{
	GAssert(m_nCount > 0, "No values to get");
	struct GTrieNode* pNode = m_pRoot;
	unsigned int nValue = 0;
	int n;
	for(n = 0; n < (int)sizeof(unsigned int) * 8; n++)
	{
		nValue <<= 1;
		if(pNode->pOne)
		{
			pNode = pNode->pOne;
			nValue |= 1;
		}
		else
		{
			pNode = pNode->pZero;
			GAssert(pNode, "Internal error");
		}
	}
	return nValue;
}

