/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GAVLTREE_H__
#define __GAVLTREE_H__

class GAVLTree;
class GAVLNode;
class GAVLEnumerator;

// The base class for a node in an AVL tree. You will need to inherit from this
// class in order to put your data in an AVL tree. A good implementation is to
// make a class, perhaps named "KeyNode", that inherits from GAVLNode and
// contains only the info necessary to implement the compare function, and
// another class, perhaps named "DataNode", that inherits from KeyNode and also
// includes a payload area for useful data. In this system, pLikeMe would be an
// instance of KeyNode, but the value this method returns would be an instance
// of DataNode because you only insert instances of DataNode into the tree.
class GAVLNode
{
friend inline int _GetHeight(GAVLNode* pNode);
friend inline int _GetSize(GAVLNode* pNode);
friend class GAVLTree;
friend class GAVLEnumerator;
private:
	GAVLNode* m_pLeftChild;
	GAVLNode* m_pRightChild;
	int m_nHeight;
	int m_nSize;

public:
	GAVLNode() : m_pLeftChild(NULL), m_pRightChild(NULL), m_nHeight(1), m_nSize(1)
	{
	}

	virtual ~GAVLNode()
	{
		delete(m_pLeftChild);
		delete(m_pRightChild);
	}

	// Returns -1 if this compares less than pThat
	// Returns 0 if this is the same as pThat
	// Returns 1 if this compares greater than pThat
	virtual int Compare(GAVLNode* pThat) = 0;

	// Returns the maximum tree depth from this node to the deepest child.  A
	// node with no children has a depth of 1.
	int GetSize() { return m_nSize; }

protected:
	GAVLNode* Insert(GAVLNode* pThat);
	GAVLNode* Unlink(GAVLNode** ppInOutThat);
	GAVLNode* Unlink(int nIndex, GAVLNode** ppOutThat);
	GAVLNode* RotateLeft();
	GAVLNode* RotateRight();
	GAVLNode* GetLeftMost(GAVLNode** ppPar);
	GAVLNode* GetRightMost(GAVLNode** ppPar);
	GAVLNode* FindNode(GAVLNode* pLikeMe, int* pnOutIndex);
	GAVLNode* FindNode(int n);
	void ReCalcLeftLineSizes();
	void ReCalcRightLineSizes();
	GAVLNode* UnlinkLeftMost(GAVLNode** ppOutThat);
	GAVLNode* UnlinkRightMost(GAVLNode** ppOutThat);
	GAVLNode* UnlinkThisNode();

	inline GAVLNode* Balance()
	{
		int nBal = _GetHeight(m_pRightChild) - _GetHeight(m_pLeftChild);
		if(nBal > 1)
		{
			if(_GetHeight(m_pRightChild->m_pRightChild) - _GetHeight(m_pRightChild->m_pLeftChild) < 0)
				m_pRightChild = m_pRightChild->RotateRight();
			return RotateLeft();
		}
		else if(nBal < -1)
		{
			if(_GetHeight(m_pLeftChild->m_pRightChild) - _GetHeight(m_pLeftChild->m_pLeftChild) > 0)
				m_pLeftChild = m_pLeftChild->RotateLeft();
			return RotateRight();
		}
		return this;
	}
};

// A balanced binary tree. Good for priority queues.
class GAVLTree
{
friend class GAVLEnumerator;
protected:
	GAVLNode* m_pRoot;

public:
	GAVLTree() : m_pRoot(NULL)
	{
	}

	virtual ~GAVLTree()
	{
		delete(m_pRoot);
	}

	// Inserts a node into the tree
	void Insert(GAVLNode* pNode);

	// Gets a node that compares equal to pLikeMe.
	GAVLNode* GetNode(GAVLNode* pLikeMe, int* pnOutIndex);

	// Removes a node from the tree and returns it to you so you can delete it
	GAVLNode* Unlink(GAVLNode* pLikeMe);

	// Removes a node from the tree and returns it to you so you can delete it
	GAVLNode* Unlink(int nIndex);

	// Removes and deletes a node from the tree.  Returns true if a node was found to delete
	bool Delete(GAVLNode* pLikeMe);

	// Removes and deletes a node from the tree.  Returns true if a node was found to delete
	bool Delete(int nIndex);

	// Returns a node from the tree
	GAVLNode* GetNode(int n);

	// Returns the number of nodes in the tree
	int GetSize();

protected:
	GAVLNode* GetRoot() { return m_pRoot; }

};

class GAVLEnumerator
{
protected:
	GAVLNode* m_stack[sizeof(unsigned int) * 8];
	int m_nStackPointer;

public:
	// Start at the left-most node
	GAVLEnumerator(GAVLTree* pTree);

	// Start at the node most like me
	GAVLEnumerator(GAVLTree* pTree, GAVLNode* pLikeMe);

	virtual ~GAVLEnumerator() {}

	// Move to the next node and return it.  Returns NULL
	// when it reaches the end of the collection.
	GAVLNode* GetNext();
};

inline int _GetHeight(GAVLNode* pNode)
{
	return pNode ? pNode->m_nHeight : 0;
}

inline int _GetSize(GAVLNode* pNode)
{
	return pNode ? pNode->m_nSize : 0;
}


#endif // __GAVLTREE_H__
