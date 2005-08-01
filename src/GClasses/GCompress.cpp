/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GCompress.h"
#include "GHashTable.h"

class GBlobHashTable : public GHashTableBase
{
protected:
	int m_nBlobSize;

public:
	GBlobHashTable(int nInitialBucketCount, int nBlobSize)
		: GHashTableBase(nInitialBucketCount)
	{
		m_nBlobSize = nBlobSize;
	}

	virtual ~GBlobHashTable()
	{
	}


	virtual unsigned int Hash(const char* pKey, int nBucketCount)
	{
		unsigned int n = 0;
		int i;
		for(i = 0; i < m_nBlobSize; i++)
			n += (unsigned char)pKey[i];
		return n % nBucketCount;
	}

	virtual bool AreKeysEqual(const char* pKey1, const char* pKey2)
	{
		int i;
		for(i = 0; i < m_nBlobSize; i++)
		{
			if(pKey1[i] != pKey2[i])
				return false;
		}
		return true;
	}

	void Add(const char* pKey, const void* pValue)
	{
		_Add(pKey, pValue);
	}

	int Count(const char* pKey)
	{
		return _Count(pKey);
	}

	bool Get(const char* pKey, void** ppOutValue)
	{
		return _Get(pKey, ppOutValue);
	}

	void Remove(const char* pKey)
	{
		_Remove(pKey);
	}
};


// -----------------------------------------------------------------------------

// This expectes pOut to also be a buffer of size nSize.  It returns the size used, or -1 if failure.
/*static*/ int GCompress::CompressPass(const unsigned char* pIn, unsigned char* pOut, int nSize, int nBlobSize)
{
	// Pass 1 -- Fill the hash tables
	int nMinRecurrence = 2 + 2 * sizeof(unsigned int) - nBlobSize;
	if(nMinRecurrence < 2)
		nMinRecurrence = 2;
	GAssert(nBlobSize > sizeof(unsigned int), "nBlobSize must be more than sizeof(unsigned int)");
	GBlobHashTable ht(nSize, nBlobSize);
	GHashTable htSmall(nSize);
	int n = 0;
	int bogus;
	while(true)
	{
		if(n + nBlobSize <= nSize)
		{
			if(ht.Count((const char*)&pIn[n]) < nMinRecurrence)
				ht.Add((const char*)&pIn[n], NULL);
		}
		else
			break;
		if(n + (int)sizeof(unsigned int) <= nSize)
		{
			unsigned int val = *(unsigned int*)&pIn[n];
			if(val != 0)
			{
				if(!htSmall.Get((const char*)val, (void**)&bogus))
					htSmall.Add((const char*)val, NULL);
			}
		}
		n++;
	}

	// Pass 2 -- Build replacement table
	int* pData = (int*)pOut;
	pData[0] = nBlobSize;
	pData[1] = 0; // Holds the count of blobs to replace
	int nOutPos = 2 * sizeof(int);
	n = 0;
	while(true)
	{
		if(n + nBlobSize > nSize)
			break;
		int nOccurrences = ht.Count((const char*)&pIn[n]);
		if(nOccurrences >= nMinRecurrence)
		{
			// Check if there's alreay a replacement for this blob
			unsigned int replacement;
			bool bOK = ht.Get((const char*)&pIn[n], (void**)&replacement);
			GAssert(bOK, "problem with hash table");
			if(replacement == 0)
			{
				// Find a new replacement for this blob
				unsigned int bogus;
				int i;
				for(i = 0; i < 30; i++)
				{
					replacement = GetRandUInt();
					if(replacement == 0)
					{
						i--;
						continue;
					}
					if(!htSmall.Get((const char*)replacement, (void**)&bogus))
						break;
					replacement = 0;
				}
				if(replacement == 0)
				{
					GAssert(false, "failed to find a suitable replacement.  This should almost never happen.");
					return -1;
				}

				// Remember the replacement for this blob
				for(i = 0; i < nOccurrences; i++)
					ht.Remove((const char*)&pIn[n]);
				for(i = 0; i < nMinRecurrence; i++)
					ht.Add((const char*)&pIn[n], (const void*)replacement);

				// Add an entry to the replacement table
				if(nOutPos + (int)sizeof(unsigned int) + nBlobSize > nSize)
				{
					//GAssert(false, "replacement table larger than original data");
					return -1;
				}
				*((unsigned int*)&pOut[nOutPos]) = replacement;
				nOutPos += sizeof(unsigned int);
				memcpy(&pOut[nOutPos], &pIn[n], nBlobSize);
				nOutPos += nBlobSize;
				pData[1]++; // increment the number of blobs to replace
				n += (nBlobSize - 1);
			}
		}
		n++;
	}
	if(pData[1] <= 0)
		return -1; // nothing was found worth replacing

	// Pass 3 -- Write the data
	n = 0;
	while(n < nSize)
	{
		if(n + nBlobSize <= nSize)
		{
			unsigned int replacement;
			bool bOK = ht.Get((const char*)&pIn[n], (void**)&replacement);
			GAssert(bOK, "problem with hash table");
			if(replacement != 0)
			{
				if(nOutPos + (int)sizeof(unsigned int) > nSize)
					return -1;
				memcpy(&pOut[nOutPos], &replacement, sizeof(unsigned int));
				nOutPos += sizeof(unsigned int);
				n += nBlobSize;
				continue;
			}
		}
		if(nOutPos >= nSize)
			return -1;
		pOut[nOutPos] = pIn[n];
		nOutPos++;
		n++;
	}

	return nOutPos;
}

/*static*/ int GCompress::DecompressPass(const unsigned char* pIn, unsigned char* pOut, int nInSize, int nFinalSize)
{
	// Parse the replacement table
	int* pData = (int*)pIn;
	int nBlobSize = pData[0];
	int nReplacementCount = pData[1];
	int nInPos = 2 * sizeof(int);
	GHashTable ht(nReplacementCount);
	int n;
	for(n = 0; n < nReplacementCount; n++)
	{
		if(nInPos >= nInSize)
			return -1;
		unsigned int replacement = *((unsigned int*)&pIn[nInPos]);
		nInPos += sizeof(unsigned int);
		const unsigned char* pBlob = &pIn[nInPos];
		nInPos += nBlobSize;
		ht.Add((const void*)replacement, pBlob);
	}

	// Decode the data
	int nOutPos = 0;
	while(nInPos < nInSize)
	{
		if(nInPos + (int)sizeof(unsigned int) <= nInSize)
		{
			const unsigned char* pBlob;
			unsigned int data = *((unsigned int*)&pIn[nInPos]);
			if(data != 0 && ht.Get((const void*)data, (void**)&pBlob))
			{
				if(nOutPos + nBlobSize > nFinalSize)
					return -1;
				memcpy(&pOut[nOutPos], pBlob, nBlobSize);
				nOutPos += nBlobSize;
				nInPos += sizeof(unsigned int);
				continue;
			}
		}
		if(nOutPos >= nFinalSize)
			return -1;
		pOut[nOutPos] = pIn[nInPos];
		nOutPos++;
		nInPos++;
	}

	return nOutPos;
}

/*static*/ unsigned char* GCompress::CompressBlock(const unsigned char* pBytes, int nSize, int* pnOutNewSize)
{
	unsigned char* pIn = new unsigned char[nSize];
	memcpy(pIn, pBytes, nSize);
	unsigned char* pOut = new unsigned char[nSize + 2 * sizeof(unsigned int)];
	unsigned char* pOutBuffer = pOut + 2 * sizeof(unsigned int);
	int* pData = (int*)pOut;
	pData[0] = nSize; // original size
	pData[1] = 0; // number of passes
	int nBlobSize = 64;
	while(true)
	{
		int nNewSize = CompressPass(pIn, pOutBuffer, nSize, nBlobSize);
		nBlobSize /= 2;
		if(nNewSize >= 0 && nNewSize < nSize)
		{
			memcpy(pIn, pOutBuffer, nNewSize);
			nSize = nNewSize;
			pData[1]++;
			continue;
		}
		if(nBlobSize <= sizeof(unsigned int))
			break;
	}
	memcpy(pOutBuffer, pIn, nSize);
	delete(pIn);
	*pnOutNewSize = nSize + 2 * sizeof(unsigned int);
	return pOut;
}

// Returns NULL on failure
/*static*/ unsigned char* GCompress::DecompressBlock(const unsigned char* pBytes, int nSize, int* pnOutNewSize)
{
	// Get data
	if(nSize < 2 * sizeof(unsigned int))
	{
		GAssert(false, "bogus blob");
		return NULL;
	}
	int* pData = (int*)pBytes;
	int nFinalSize = pData[0];
	int nCycles = pData[1];
	if(nFinalSize < nSize - 2 * (int)sizeof(unsigned int))
	{
		GAssert(false, "final size is significantly smaller than original");
		return NULL;
	}
	unsigned char* pOut = new unsigned char[nFinalSize];
	*pnOutNewSize = nFinalSize;

	// Handle the non-compressed case
	if(nCycles <= 0)
	{
		if(nFinalSize != nSize - 2 * (int)sizeof(unsigned int))
		{
			GAssert(false, "bad blob");
			delete(pOut);
			return NULL;
		}
		memcpy(pOut, pBytes + 2 * sizeof(unsigned int), nFinalSize);
		return pOut;
	}

	// Decompress it
	unsigned char* pTemp = new unsigned char[nFinalSize];
	nSize -= (2 * sizeof(unsigned int));
	memcpy(pTemp, pBytes + 2 * sizeof(unsigned int), nSize);
	while(nCycles > 0)
	{
		nSize = DecompressPass(pTemp, pOut, nSize, nFinalSize);
		GAssert(nSize <= nFinalSize, "internal decompression error");
		nCycles--;
		if(nCycles > 0)
			memcpy(pTemp, pOut, nSize);
	}
	delete(pTemp);

	// Check results
	if(nSize != nFinalSize)
	{
		GAssert(false, "final size incorrect");
		delete(pOut);
		return NULL;
	}
	return pOut;
}

#define GCOMPRESS_BLOCK_SIZE 262144

/*static*/ unsigned char* GCompress::Compress(const unsigned char* pBytes, int nSize, int* pnOutNewSize)
{
	// Compress each block
	int nBlocks = (nSize + GCOMPRESS_BLOCK_SIZE - 1) / GCOMPRESS_BLOCK_SIZE;
	unsigned char** ppBlocks = new unsigned char*[nBlocks];
	int* pnCompressedSizes = new int[nBlocks];
	int n = 0;
	int nTotalSize = 0;
	while(nSize > 0)
	{
		int nNewSize;
		ppBlocks[n] = CompressBlock(pBytes, MIN(nSize, GCOMPRESS_BLOCK_SIZE), &nNewSize);
		if(!ppBlocks[n])
		{
			int i;
			for(i = n - 1; i >= 0; i--)
				delete(ppBlocks[i]);
			delete(ppBlocks);
			delete(pnCompressedSizes);
			return NULL;			
		}
		pnCompressedSizes[n] = nNewSize;
		nTotalSize += sizeof(int);
		nTotalSize += nNewSize;
		pBytes += GCOMPRESS_BLOCK_SIZE;
		nSize -= GCOMPRESS_BLOCK_SIZE;
		n++;
	}

	// Stitch all the compressed blocks together into a big blob
	unsigned char* pOut = new unsigned char[nTotalSize];
	int nOutPos = 0;
	int i;
	for(i = 0; i < nBlocks; i++)
	{
		*((int*)&pOut[nOutPos]) = pnCompressedSizes[i];
		nOutPos += sizeof(int);
		memcpy(&pOut[nOutPos], ppBlocks[i], pnCompressedSizes[i]);
		nOutPos += pnCompressedSizes[i];
		delete(ppBlocks[i]);
	}
	delete(ppBlocks);
	delete(pnCompressedSizes);
	*pnOutNewSize = nTotalSize;
	return pOut;
}

/*static*/ unsigned char* GCompress::Decompress(const unsigned char* pBytes, int nSize, int* pnOutNewSize)
{
	// Count the total size
	int nTotalSize = 0;
	int nPos = 0;
	while(true)
	{
		int nCompressedSize = ((int*)&pBytes[nPos])[0];
		int nUncompressedSize = ((int*)&pBytes[nPos])[1];
		nPos += sizeof(int);
		nPos += nCompressedSize;
		nTotalSize += nUncompressedSize;
		if(nPos >= nSize)
			break;
	}
	GAssert(nPos == nSize, "bad compressed data");

	// Uncompress everything
	unsigned char* pOut = new unsigned char[nTotalSize];
	nPos = 0;
	int nOutPos = 0;
	while(true)
	{
		int nCompressedSize = ((int*)&pBytes[nPos])[0];
		int nUncompressedSize = ((int*)&pBytes[nPos])[1];
		nPos += sizeof(int);
		int nNewSize;
		unsigned char* pTemp = DecompressBlock(&pBytes[nPos], nCompressedSize, &nNewSize);
		nPos += nCompressedSize;
		GAssert(nNewSize == nUncompressedSize, "bad compressed image");
		memcpy(&pOut[nOutPos], pTemp, nNewSize);
		nOutPos += nNewSize;
		delete(pTemp);
		if(nPos >= nSize)
			break;
	}
	GAssert(nPos == nSize, "bad compressed data");
	GAssert(nOutPos == nTotalSize, "bad compressed data");
	*pnOutNewSize = nTotalSize;
	return pOut;
}
