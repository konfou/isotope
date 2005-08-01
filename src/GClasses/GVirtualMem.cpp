/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

/*
#include "GnuSDK.h"
#include "GMemChunk.h"
#ifndef WIN32
#include <string.h> // (for memset)
#endif // not WIN32

GMemChunk::GMemChunk(long addr) : GHashBucket()
{
	nStartAddr = addr;
	memset(&mem, 0, sizeof(mem));
}

GMemChunk::~GMemChunk()
{

}

int GMemChunk::Compare(GBucket* pBucket)
{
	GMemChunk* pTmp = (GMemChunk*)pBucket;
	if(nStartAddr < pTmp->nStartAddr)
		return -1;
	if(nStartAddr > pTmp->nStartAddr)
		return 1;
	return 0;
}

int GMemChunk::GetHashIndex(int nMaxLists)
{
	return nStartAddr % nMaxLists;
}

///////////////////////////////////////////////////////////////////

GVirtualMem::GVirtualMem()
{
	pHash = new GHashTable(150);
}

GVirtualMem::~GVirtualMem()
{
	delete(pHash);
}

void GVirtualMem::WriteByte(long addr, unsigned char b)
{
	long nBase = (addr >> 10) << 10;
	long nOffs = addr - nBase;
	GMemChunk tmp(nBase);
	GMemChunk* pMem = (GMemChunk*)pHash->GetBucket(&tmp);
	if(!pMem)
	{
		pMem = new GMemChunk(nBase);
		pHash->LinkBucket(pMem);
	}
	pMem->mem[nOffs] = b;
}

unsigned char GVirtualMem::ReadByte(long addr)
{
	long nBase = (addr >> 10) << 10;
	long nOffs = addr - nBase;
	GMemChunk tmp(nBase);
	GMemChunk* pMem = (GMemChunk*)pHash->GetBucket(&tmp);
	if(!pMem)
		return '\0';
	return(pMem->mem[nOffs]);
}

void GVirtualMem::WriteWord(long addr, unsigned long l)
{
	unsigned char* pChar = (unsigned char*)&l;
	WriteByte(addr, pChar[0]);
	WriteByte(addr + 1, pChar[1]);
	WriteByte(addr + 2, pChar[2]);
	WriteByte(addr + 3, pChar[3]);
}

unsigned long GVirtualMem::ReadWord(long addr)
{
	unsigned long l;
	unsigned char* pChar = (unsigned char*)&l;
	pChar[0] = ReadByte(addr);
	pChar[1] = ReadByte(addr + 1);
	pChar[2] = ReadByte(addr + 2);
	pChar[3] = ReadByte(addr + 3);
	return l;
}
*/
