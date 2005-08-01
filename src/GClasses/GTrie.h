/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GTRIE_H__
#define __GTRIE_H__

class GTrie
{
protected:
	struct GTrieNode* m_pRoot;
	unsigned int m_nCount;

	void RecurseDestructTrieNode(struct GTrieNode* pNode);
	struct GTrieNode* RecurseCopyTrieNode(struct GTrieNode* pNode);

public:
	GTrie();
	virtual ~GTrie();

	void CopyTrie(GTrie* pSource);
	unsigned int GetCount() { return m_nCount; }
	void Add(unsigned int nValue);
	bool Check(unsigned int nValue);
	bool Remove(unsigned int nValue);
	unsigned int GetSmallest();
	unsigned int GetBiggest();
};

#endif // __GTRIE_H__
