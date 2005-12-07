/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/


#ifndef __GCOMPRESS_H__
#define __GCOMPRESS_H__

#define GCOMPRESS_CYCLES 4

// This is supposed to be a compression utility, but it doesn't really work
class GCompress
{
public:
	// Note that the buffer this returns will be as big as the original one, so if you plan
	// keep it around in memory you should reallocate it.  You must delete the buffer it returns.
	static unsigned char* Compress(const unsigned char* pBytes, int nSize, int* pnOutNewSize);

	// You must delete the buffer this returns.  Returns NULL on failure.
	static unsigned char* Decompress(const unsigned char* pBytes, int nSize, int* pnOutNewSize);

protected:
	static unsigned char* CompressBlock(const unsigned char* pBytes, int nSize, int* pnOutNewSize);
	static unsigned char* DecompressBlock(const unsigned char* pBytes, int nSize, int* pnOutNewSize);
	static int CompressPass(const unsigned char* pIn, unsigned char* pOut, int nSize, int nBlobSize);
	static int DecompressPass(const unsigned char* pIn, unsigned char* pOut, int nInSize, int nFinalSize);
};

#endif // __GCOMPRESS_H__
