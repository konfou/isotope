/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GHASHTABLE_H__
#define __GHASHTABLE_H__

#include <stdio.h>
#include "GMacros.h"

class GQueue;
struct HashBucket;

// The base class of hash tables
class GHashTableBase
{
friend class GHashTableEnumerator;
protected:
	struct HashBucket* m_pBuckets;
	struct HashBucket* m_pFirstEmpty;
	int m_nBucketCount;
	int m_nCount;
	int m_nModCount;

	GHashTableBase(int nInitialBucketCount);

public:
	virtual ~GHashTableBase();

	int GetCount() { return m_nCount; }
	int GetModCount() { return m_nModCount; }

protected:
	virtual unsigned int Hash(const char* pKey, int nBucketCount) = 0;
	virtual bool AreKeysEqual(const char* pKey1, const char* pKey2) = 0;
	void _Resize(int nNewSize);

	// Adds a key/value pair to the hash table
	void _Add(const char* pKey, const void* pValue);

	// Returns true and the first occurrence of a value with the specified key if one exists
	bool _Get(const char* pKey, void** pOutValue);

	// Removes the first found occurrence of the specified key
	void _Remove(const char* pKey);

	// Returns the number of values with the specified key
	int _Count(const char* pKey);
};




#define UNCOMMON_INT 0x80000000

// This class makes it easy to iterate through the values in a hash table
class GHashTableEnumerator
{
protected:
	GHashTableBase* m_pHashTable;
	int m_nPos;
	int m_nModCount;

public:
	GHashTableEnumerator(GHashTableBase* pHashTable)
	{
		m_pHashTable = pHashTable;
		m_nModCount = m_pHashTable->GetModCount();
		m_nPos = 0;
	}

	// Gets the next element in the hash table. ppValue is set to
	// the value and the return value is the key. Returns NULL when
	// it reaches the end of the collection. (The first time it is
	// called, it returns the first item in the collection.)
	const char* GetNext(void** ppOutValue);

	// Sets the value at pOutInt to the next integer key in the hash table.  Returns false when it
	// reaches the end of the collection.
	bool GetNextIntKey(int* pOutKey, void** ppOutValue)
	{
		unsigned int n = (unsigned int)GetNext(ppOutValue);
		if(n == 0)
			return false;
		if(n == UNCOMMON_INT)
			*pOutKey = 0;
		else
			*pOutKey = (int)n;
		return true;
	}

	// Returns the value associated with the current key
	void* GetCurrentValue();
};





class GHashTable : public GHashTableBase
{
public:
	GHashTable(int nInitialBucketCount)
		: GHashTableBase(nInitialBucketCount)
	{
	}

	virtual ~GHashTable()
	{
	}


	virtual unsigned int Hash(const char* pKey, int nBucketCount)
	{
		return ((unsigned int)pKey) % nBucketCount;
	}

	virtual bool AreKeysEqual(const char* pKey1, const char* pKey2)
	{
		return pKey1 == pKey2;
	}

	// Adds a pointer key and value pair to the hash table.  The key can not be NULL
	void Add(const void* pKey, const void* pValue)
	{
		_Add((const char*)pKey, pValue);
	}

	// Adds an integer key and value pair to the hash table.  The key can not be 0x80000000
	void Add(int nKey, const void* pValue)
	{
		GAssert(nKey != (int)UNCOMMON_INT, "key can't be 0x80000000");
		if(nKey == 0)
			_Add((const char*)UNCOMMON_INT, pValue);
		else
			_Add((const char*)nKey, pValue);
	}

	// Gets a value based on the key
	bool Get(const void* pKey, void** ppOutValue)
	{
		return _Get((const char*)pKey, ppOutValue);
	}

	// Gets a value based on the key
	bool Get(int nKey, void** ppOutValue)
	{
		GAssert(nKey != (int)UNCOMMON_INT, "key can't be 0x80000000");
		if(nKey == 0)
			return _Get((const char*)UNCOMMON_INT, ppOutValue);
		else
			return _Get((const char*)nKey, ppOutValue);
	}

	// Removes an entry from the hash table
	void Remove(const void* pKey)
	{
		_Remove((const char*)pKey);
	}

	// Removes an entry from the hash table
	void Remove(int nKey)
	{
		GAssert(nKey != (int)UNCOMMON_INT, "key can't be 0x80000000");
		if(nKey == 0)
			Remove((const char*)UNCOMMON_INT);
		else
			Remove((const char*)nKey);
	}
};





// Hash table based on keys of constant strings (or at least strings
// that won't change during the lifetime of the hash table).  It's a
// good idea to use a GStringHeap in connection with this class.
class GConstStringHashTable : public GHashTableBase
{
protected:
	bool m_bCaseSensitive;

public:
	GConstStringHashTable(int nInitialBucketCount, bool bCaseSensitive)
		: GHashTableBase(nInitialBucketCount)
	{
		m_bCaseSensitive = bCaseSensitive;
	}

	virtual ~GConstStringHashTable()
	{
	}


	virtual unsigned int Hash(const char* pKey, int nBucketCount)
	{
		unsigned int n = 0;
		if(m_bCaseSensitive)
		{
			while(*pKey != '\0')
			{
				n += (*pKey);
				pKey++;
			}
		}
		else
		{
			while(*pKey != '\0')
			{
				n += ((*pKey) & ~0x20);
				pKey++;
			}
		}
		return n % nBucketCount;
	}

	virtual bool AreKeysEqual(const char* pKey1, const char* pKey2)
	{
		if(m_bCaseSensitive)
			return(strcmp(pKey1, pKey2) == 0);
		else
			return(stricmp(pKey1, pKey2) == 0);
	}

	// Adds a key and value pair to the hash table.  The key should be a constant
	// string (or at least a string that won't change over the lifetime of the
	// hash table).  The GStringHeap class provides a good place to store such a
	// string.
	void Add(const char* pKey, const void* pValue)
	{
		_Add(pKey, pValue);
	}

	// Gets the value for the specified key
	bool Get(const char* pKey, void** ppOutValue)
	{
		return _Get(pKey, ppOutValue);
	}

	// Removes an entry from the hash table
	void Remove(const char* pKey)
	{
		_Remove(pKey);
	}
};




// Provides a heap in which to put a string
class GStringHeap
{
protected:
	char* m_pCurrentBlock;
	int m_nMinBlockSize;
	int m_nCurrentPos;

public:
	GStringHeap(int nMinBlockSize)
	{
		m_pCurrentBlock = NULL;
		m_nMinBlockSize = nMinBlockSize;
		m_nCurrentPos = nMinBlockSize;
	}

	virtual ~GStringHeap()
	{
		while(m_pCurrentBlock)
		{
			char* pNext = *(char**)m_pCurrentBlock;
			delete(m_pCurrentBlock);
			m_pCurrentBlock = pNext;
		}
	}

	// Allocate space in the heap and copy a string to it.  Returns
	// a pointer to the string
	char* Add(const char* szString)
	{
		return Add(szString, strlen(szString));
	}

	// Allocate space in the heap and copy a string to it.  Returns
	// a pointer to the string
	char* Add(const char* pString, int nLength)
	{
		char* pNewString = Allocate(nLength + 1);
		memcpy(pNewString, pString, nLength);
		pNewString[nLength] = '\0';
		return pNewString;
	}

	// Allocate space big enough to hold the contents of the queue and
	// dump the queue into the space.  Returns a pointer to the buffer.
	char* Add(GQueue* pQ);

	// Allocate space in the heap and return a pointer to it
	char* Allocate(int nLength)
	{
		if(nLength > m_nMinBlockSize - m_nCurrentPos)
		{
			char* pNewBlock = new char[sizeof(char*) + MAX(nLength, m_nMinBlockSize)];
			*(char**)pNewBlock = m_pCurrentBlock;
			m_pCurrentBlock = pNewBlock;
			m_nCurrentPos = 0;
		}
		char* pNewString = m_pCurrentBlock + sizeof(char*) + m_nCurrentPos;
		m_nCurrentPos += nLength;
		return pNewString;
	}
};


#endif // __GHASHTABLE_H__
