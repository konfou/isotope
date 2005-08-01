/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "TestAvlTree.h"
#include "../../../GClasses/GAVLTree.h"
#include "../ClassTests.h"
#include <stdlib.h>

class MyAvlNode : public GAVLNode
{
protected:
	int m_nValue;

public:
	MyAvlNode(int nValue)
	{
		m_nValue = nValue;
	}

	virtual ~MyAvlNode()
	{
	}

	virtual int Compare(GAVLNode* pThat)
	{
		if(m_nValue < ((MyAvlNode*)pThat)->m_nValue)
			return -1;
		if(m_nValue > ((MyAvlNode*)pThat)->m_nValue)
			return 1;
		return 0;
	}
};

bool TestGAvlTree(ClassTests* pThis)
{
	bool bRet = true;
	GAVLTree myTree;
	int nFirstValue = rand();
	int nVal = nFirstValue;
	int n;
	for(n = 0; n < 10000; n++)
	{
		MyAvlNode* pNode = new MyAvlNode(nVal);
		nVal = rand();
		myTree.Insert(pNode);
	}
	MyAvlNode tmp(nFirstValue);
	int nIndex;
	if(!myTree.GetNode(&tmp, &nIndex))
		bRet = false;

	return true;
}
