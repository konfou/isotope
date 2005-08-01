/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GSTREAM_H__
#define __GSTREAM_H__

// This works with a circular buffer
class GStream
{
protected:
	unsigned char* m_pBuff;
	unsigned int m_nBufferSize;
	unsigned int m_nDataSize;
	unsigned int m_nBufferPos;
	unsigned int m_nDataPos;
	bool m_bWritePermission;

public:
	GStream(const void* pBuff, unsigned int nBufferSize, unsigned int nUsedSize, bool bWritePermission);
	virtual ~GStream();

	unsigned char Peek(unsigned int nOffset); // see what's coming
	unsigned int Eat(unsigned int nSize); // like Read, but doesn't give you any data
	unsigned int Write(const void* pBuff, unsigned int nSize); // returns the size actually written
	unsigned int Read(void* pBuff, unsigned int nSize); // returns the size actually read
	unsigned int GetCount() { return m_nDataPos; } // How many bytes have been read or eaten
	unsigned int GetSize() { return m_nDataSize; } // How much is left in the stream to read or eat
};


#endif // __GSTREAM_H__
