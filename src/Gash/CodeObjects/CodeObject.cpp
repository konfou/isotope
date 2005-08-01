/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "CodeObject.h"
#include "../../GClasses/GHashTable.h"

#ifdef _DEBUG
unsigned int CodeObject::s_nAllocs = 0;
unsigned int CodeObject::s_nDeletes = 0;
#endif // _DEBUG
//GHashTable* CodeObject::s_pLeakedObjects = new GHashTable(101);

CodeObject::CodeObject(int nLineNumber, int nColumn, int nWidth)
{
	SetLineNumber(nLineNumber);
	SetColumnAndWidth(nColumn, nWidth);

#ifdef _DEBUG
//	GAssert(s_nAllocs != 769, "break");
//	s_pLeakedObjects->Add(this, (void*)s_nAllocs);
	s_nAllocs++;
#endif // _DEBUG
}

CodeObject::~CodeObject()
{
#ifdef _DEBUG
	s_nDeletes++;
//	s_pLeakedObjects->Remove(this);
#endif // _DEBUG
}

