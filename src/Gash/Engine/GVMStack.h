/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GVMSTACK_H__
#define __GVMSTACK_H__

#include "../../GClasses/GArray.h"

struct CallStackLayer
{
	unsigned char* pCodePos;
	int nStackTop;
	int nMethodID;
	int nOffset;
};

// Dynamic array of CallStackLayer's
class GCallStackLayerArray : public GSmallArray
{
public:
	GCallStackLayerArray(int nGrowBy) : GSmallArray(sizeof(struct CallStackLayer), nGrowBy) { }
	virtual ~GCallStackLayerArray() { }

	struct CallStackLayer* GetLayer(int nIndex) { return (struct CallStackLayer*)_GetCellRef(nIndex); }
	void AddLayer(struct CallStackLayer* pLayer) { _AddCellByRef(pLayer); }
	void InsertLayer(int nPos, struct CallStackLayer* pLayer) { _InsertCellByRef(nPos, pLayer); }
	void SetLayer(int nCell, struct CallStackLayer* pLayer) { _SetCellByRef(nCell, pLayer); }
};

#endif // __GVMSTACK_H__
