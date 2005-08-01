/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "GAVLTree.h"
#include "GMacros.h"

GAVLNode* GAVLNode::Insert(GAVLNode* pThat)
{
	// Check preconditions
	GAssert(m_nHeight == MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1, "Height problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) < 2, "Balance problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) > -2, "Balance problem");
	GAssert(m_nSize == _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1, "Size problem");

	// Insert the node
	if(Compare(pThat) > 0)
	{
		if(m_pLeftChild)
		{
			m_pLeftChild = m_pLeftChild->Insert(pThat);
			m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
			m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
			return Balance();
		}
		else
		{
			m_pLeftChild = pThat;
			m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
			m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
			return this;
		}
	}
	else
	{
		if(m_pRightChild)
		{
			m_pRightChild = m_pRightChild->Insert(pThat);
			m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
			m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
			return Balance();
		}
		else
		{
			m_pRightChild = pThat;
			m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
			m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
			return this;
		}
	}
}

GAVLNode* GAVLNode::Unlink(GAVLNode** ppInOutThat)
{
	// Check preconditions
	GAssert(m_nHeight == MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1, "Height problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) < 2, "Balance problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) > -2, "Balance problem");
	GAssert(m_nSize == _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1, "Size problem");

	// Find the node
	int nCmp = Compare(*ppInOutThat);
	if(nCmp > 0)
	{
		if(m_pLeftChild)
			m_pLeftChild = m_pLeftChild->Unlink(ppInOutThat);
		else
		{
			*ppInOutThat = NULL; // Can't find a matching node to delete
			return this;
		}
	}
	else if(nCmp < 0)
	{
		if(m_pRightChild)
			m_pRightChild = m_pRightChild->Unlink(ppInOutThat);
		else
		{
			*ppInOutThat = NULL; // Can't find a matching node to delete
			return this;
		}
	}
	else
	{
		*ppInOutThat = this;
		return UnlinkThisNode();
	}
	m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
	m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
	return Balance();
}

GAVLNode* GAVLNode::Unlink(int nIndex, GAVLNode** ppOutThat)
{
	// Check preconditions
	GAssert(nIndex >= 0 && nIndex < m_nSize, "Out of range (720)");
	GAssert(m_nHeight == MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1, "Height problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) < 2, "Balance problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) > -2, "Balance problem");
	GAssert(m_nSize == _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1, "Size problem");
	
	// Find the node
	int nLeftSize = _GetSize(m_pLeftChild);
	if(nIndex < nLeftSize)
		m_pLeftChild = m_pLeftChild->Unlink(nIndex, ppOutThat);
	else if(nIndex == nLeftSize)
	{
		*ppOutThat = this;
		return UnlinkThisNode();
	}
	else
		m_pRightChild = m_pRightChild->Unlink(nIndex - nLeftSize - 1, ppOutThat);
	m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
	m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
	return Balance();
}

GAVLNode* GAVLNode::UnlinkThisNode()
{
	// If this node doesn't have a left or right child, it's easy
	if(!m_pRightChild)
	{
		GAVLNode* pTmp = m_pLeftChild;
		m_pLeftChild = NULL;
		m_nHeight = 1;
		m_nSize = 1;
		return pTmp;
	}
	if(!m_pLeftChild)
	{
		GAVLNode* pTmp = m_pRightChild;
		m_pRightChild = NULL;
		m_nHeight = 1;
		m_nSize = 1;
		return pTmp;
	}
	
	// I've got two children--I'll have to use my left-child's right-most
	// descendant to fill the vacant spot I leave when I get unlinked.
	if(m_pLeftChild->m_pRightChild)
	{
		// Find right-most child of my left child to be my replacement
		GAVLNode* pReplacement;
		m_pLeftChild = m_pLeftChild->UnlinkRightMost(&pReplacement);
		pReplacement->m_pLeftChild = m_pLeftChild;
		pReplacement->m_pRightChild = m_pRightChild;
		pReplacement->m_nHeight = MAX(_GetHeight(pReplacement->m_pLeftChild), _GetHeight(pReplacement->m_pRightChild)) + 1;
		pReplacement->m_nSize = _GetSize(pReplacement->m_pLeftChild) + _GetSize(pReplacement->m_pRightChild) + 1;
		m_pLeftChild = NULL;
		m_pRightChild = NULL;
		m_nHeight = 1;
		m_nSize = 1;
		return pReplacement->Balance();
	}
	else
	{
		// Replace me with my left child
		m_pLeftChild->m_pRightChild = m_pRightChild;
		m_pLeftChild->m_nHeight = MAX(_GetHeight(m_pLeftChild->m_pLeftChild), _GetHeight(m_pLeftChild->m_pRightChild)) + 1;
		m_pLeftChild->m_nSize = _GetSize(m_pLeftChild->m_pLeftChild) + _GetSize(m_pLeftChild->m_pRightChild) + 1;
		GAVLNode* pTmp = m_pLeftChild;
		m_pLeftChild = NULL;
		m_pRightChild = NULL;
		m_nHeight = 1;
		m_nSize = 1;
		return pTmp->Balance();
	}
}

GAVLNode* GAVLNode::UnlinkLeftMost(GAVLNode** ppOutThat)
{
	// Check preconditions
	GAssert(m_nHeight == MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1, "Height problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) < 2, "Balance problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) > -2, "Balance problem");
	GAssert(m_nSize == _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1, "Size problem");
	
	if(m_pLeftChild)
	{
		m_pLeftChild = m_pLeftChild->UnlinkLeftMost(ppOutThat);
		m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
		m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
		return Balance();
	}
	else
	{
		*ppOutThat = m_pLeftChild;
		GAVLNode* pTmp = m_pRightChild;
		m_pRightChild = NULL;
		m_nHeight = 1;
		m_nSize = 1;
		return pTmp;
	}
}

GAVLNode* GAVLNode::UnlinkRightMost(GAVLNode** ppOutThat)
{
	// Check preconditions
	GAssert(m_nHeight == MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1, "Height problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) < 2, "Balance problem");
	GAssert(_GetHeight(m_pLeftChild) - _GetHeight(m_pRightChild) > -2, "Balance problem");
	GAssert(m_nSize == _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1, "Size problem");
	
	if(m_pRightChild)
	{
		m_pRightChild = m_pRightChild->UnlinkRightMost(ppOutThat);
		m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
		m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
		return Balance();
	}
	else
	{
		*ppOutThat = this;
		GAVLNode* pTmp = m_pLeftChild;
		m_pLeftChild = NULL;
		m_nHeight = 1;
		m_nSize = 1;
		return pTmp;
	}
}

GAVLNode* GAVLNode::RotateRight()
{
//	if(m_pLeftChild && _GetHeight(m_pLeftChild->m_pRightChild) - _GetHeight(m_pLeftChild->m_pLeftChild) > 0)
//		m_pLeftChild = m_pLeftChild->RotateLeft();
	GAssert(m_pLeftChild, "Can't rotate right if there's no left child");
	GAVLNode* pTmp1 = m_pLeftChild->m_pRightChild;
	m_pLeftChild->m_pRightChild = this;
	GAVLNode* pTmp2 = m_pLeftChild;
	m_pLeftChild = pTmp1;
	m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
	m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
	pTmp2->m_nHeight = MAX(_GetHeight(pTmp2->m_pLeftChild), _GetHeight(pTmp2->m_pRightChild)) + 1;
	pTmp2->m_nSize = _GetSize(pTmp2->m_pLeftChild) + _GetSize(pTmp2->m_pRightChild) + 1;
	return pTmp2;
}

GAVLNode* GAVLNode::RotateLeft()
{
//	if(m_pRightChild && _GetHeight(m_pRightChild->m_pRightChild) - _GetHeight(m_pRightChild->m_pLeftChild) < 0)
//		m_pRightChild = m_pRightChild->RotateRight();
	GAssert(m_pRightChild, "Can't rotate left if there's no right child");
	GAVLNode* pTmp1 = m_pRightChild->m_pLeftChild;
	m_pRightChild->m_pLeftChild = this;
	GAVLNode* pTmp2 = m_pRightChild;
	m_pRightChild = pTmp1;
	m_nHeight = MAX(_GetHeight(m_pLeftChild), _GetHeight(m_pRightChild)) + 1;
	m_nSize = _GetSize(m_pLeftChild) + _GetSize(m_pRightChild) + 1;
	pTmp2->m_nHeight = MAX(_GetHeight(pTmp2->m_pLeftChild), _GetHeight(pTmp2->m_pRightChild)) + 1;
	pTmp2->m_nSize = _GetSize(pTmp2->m_pLeftChild) + _GetSize(pTmp2->m_pRightChild) + 1;
	return pTmp2;
}

GAVLNode* GAVLNode::GetLeftMost(GAVLNode** ppPar)
{
	GAVLNode* pPar = NULL;
	GAVLNode* pCurrent = this;
	while(pCurrent->m_pLeftChild)
	{
		pPar = pCurrent;
		pCurrent = pCurrent->m_pLeftChild;
	}
	*ppPar = pPar;
	return pCurrent;
}

GAVLNode* GAVLNode::GetRightMost(GAVLNode** ppPar)
{
	GAVLNode* pPar = NULL;
	GAVLNode* pCurrent = this;
	while(pCurrent->m_pRightChild)
	{
		pPar = pCurrent;
		pCurrent = pCurrent->m_pRightChild;
	}
	*ppPar = pPar;
	return pCurrent;
}

GAVLNode* GAVLNode::FindNode(GAVLNode* pLikeMe, int* pnOutIndex)
{
	int nCmp = Compare(pLikeMe);
	if(nCmp > 0)
		return m_pLeftChild ? m_pLeftChild->FindNode(pLikeMe, pnOutIndex) : NULL;
	else if(nCmp < 0)
	{
		if(m_pRightChild)
		{
			GAVLNode* pTmp = m_pRightChild->FindNode(pLikeMe, pnOutIndex);
			*pnOutIndex += (_GetSize(m_pLeftChild) + 1);
			return pTmp;
		}
		return NULL;
	}
	*pnOutIndex = _GetSize(m_pLeftChild);
	return this;
}

GAVLNode* GAVLNode::FindNode(int n)
{
	GAssert(n >= 0 && n < m_nSize, "Out of range (721)");
	int nLeftSize = _GetSize(m_pLeftChild);
	if(n < nLeftSize)
		return m_pLeftChild->FindNode(n);
	n -= nLeftSize;
	if(n == 0)
		return this;
	n--;
	return m_pRightChild->FindNode(n);
}

// --------------------------------------------------------------------------------

void GAVLTree::Insert(GAVLNode* pNode)
{
	GAssert(pNode->m_nHeight == 1, "Inserting trees not supported");
	if(m_pRoot)
		m_pRoot = m_pRoot->Insert(pNode);
	else
		m_pRoot = pNode;
}

GAVLNode* GAVLTree::GetNode(GAVLNode* pLikeMe, int* pnOutIndex)
{
	if(m_pRoot)
		return m_pRoot->FindNode(pLikeMe, pnOutIndex);
	*pnOutIndex = -1;
	return NULL;
}

GAVLNode* GAVLTree::GetNode(int n)
{
	GAssert(m_pRoot, "Out of range (722)");
	return m_pRoot->FindNode(n);
}

int GAVLTree::GetSize()
{
	return m_pRoot ? m_pRoot->m_nSize : 0;
}

GAVLNode* GAVLTree::Unlink(GAVLNode* pLikeMe)
{
	if(!m_pRoot)
		return NULL;
	m_pRoot = m_pRoot->Unlink(&pLikeMe);
#ifdef _DEBUG
	if(pLikeMe)
	{
		GAssert(pLikeMe->m_pRightChild == 0, "Shouldn't have children");
		GAssert(pLikeMe->m_pLeftChild == 0, "Shouldn't have children");
		GAssert(pLikeMe->m_nHeight == 1, "Height Problem");
		GAssert(pLikeMe->m_nSize == 1, "Size Problem");
	}
#endif // _DEBUG
	return pLikeMe;
}

GAVLNode* GAVLTree::Unlink(int nIndex)
{
	GAVLNode* pThat;
	m_pRoot = m_pRoot->Unlink(nIndex, &pThat);
	return pThat;
}

bool GAVLTree::Delete(GAVLNode* pLikeMe)
{
	GAVLNode* pNode = Unlink(pLikeMe);
	bool bRet = pNode ? true : false;
	delete(pNode);
	return bRet;
}

bool GAVLTree::Delete(int nIndex)
{
	GAVLNode* pNode = Unlink(nIndex);
	bool bRet = pNode ? true : false;
	delete(pNode);
	return bRet;
}

// --------------------------------------------------------------------------------

GAVLEnumerator::GAVLEnumerator(GAVLTree* pTree)
{
	m_nStackPointer = 0;
	GAVLNode* pNode = pTree->GetRoot();
	while(pNode)
	{
		m_stack[m_nStackPointer] = pNode;
		m_nStackPointer++;
		pNode = pNode->m_pLeftChild;
	}
}

GAVLEnumerator::GAVLEnumerator(GAVLTree* pTree, GAVLNode* pLikeMe)
{
	m_nStackPointer = 0;
	GAVLNode* pNode = pTree->GetRoot();
	while(pNode)
	{
		m_stack[m_nStackPointer] = pNode;
		m_nStackPointer++;
		int nCmp = pNode->Compare(pLikeMe);
		if(nCmp > 0)
			pNode = pNode->m_pLeftChild;
		else if(nCmp < 0)
			pNode = pNode->m_pRightChild;
		else
			return;
	}
}

GAVLNode* GAVLEnumerator::GetNext()
{
	if(m_nStackPointer < 1)
		return NULL;
	GAVLNode* pNode = m_stack[m_nStackPointer - 1];
	if(pNode->m_pRightChild)
	{
		GAVLNode* pNext = pNode->m_pRightChild;
		m_stack[m_nStackPointer] = pNext;
		m_nStackPointer++;
		while(pNext->m_pLeftChild)
		{
			m_stack[m_nStackPointer] = pNext->m_pLeftChild;
			m_nStackPointer++;
			pNext = pNext->m_pLeftChild;
		}
	}
	else
	{
		while(true)
		{
			m_nStackPointer--;
			if(m_nStackPointer < 1)
				break;
			if(m_stack[m_nStackPointer - 1]->m_pLeftChild == m_stack[m_nStackPointer])
				break;
		}
	}
	return pNode;
}
