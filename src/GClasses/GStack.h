/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GSTACK_H__
#define __GSTACK_H__

class GLList;
class GStackChunk;


class GStack
{
protected:
	int m_nChunkSize;
	int m_nPos;
	int m_nDataSize;
	GLList* m_pChunks;
	GStackChunk* m_pExtra;

public:
	GStack(int nChunkSize = 1024);
	virtual ~GStack();

	void Flush();
	void Push(char c);
	void Push(int n);
	bool Pop(char* pc);
	bool Pop(int* pn);
	int GetSize() { return m_nDataSize; }
};

#endif // __GSTACK_H__
