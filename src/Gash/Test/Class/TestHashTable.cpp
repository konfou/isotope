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
#include "../../../GClasses/GHashTable.h"
#include "../ClassTests.h"
#include <stdlib.h>

#define TEST_HASH_TABLE_ELEMENTS 32000

bool VerifyBucketCount(GHashTableBase* pHT)
{
	GHashTableEnumerator hte(pHT);
	int n = 0;
	while(hte.GetNextKey())
		n++;
	if(n != pHT->GetCount())
		return false;
	return true;
}

bool TestGHashTable(ClassTests* pThis)
{
	int nElements = TEST_HASH_TABLE_ELEMENTS;
	GHashTable ht(13);
	int n;
	for(n = 0; n < nElements; n++)
		ht.Add(n * n, (const void*)n);
	for(n = 0; n < nElements; n += 7)
		ht.Remove(n * n);
	if(!VerifyBucketCount(&ht))
		return false;
	int nVal;
	for(n = 0; n < nElements; n++)
	{
		if(n % 7 == 0)
		{
			if(ht.Get(n * n, (void**)&nVal))
				return false;
		}
		else
		{
			if(!ht.Get(n * n, (void**)&nVal))
				return false;
			if(nVal != n)
				return false;
		}
	}

	return true;
}
