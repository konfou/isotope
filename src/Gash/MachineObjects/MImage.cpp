/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "MImage.h"
#include "MachineObjects.h"
#include "../../GClasses/GHashTable.h"
#include "../Include/GashLib.h"

void RegisterMImage(GConstStringHashTable* pTable)
{
	pTable->Add("method !new()", new EMethodPointerHolder((MachineMethod0)&MImage::allocate));
	pTable->Add("method &setSize(Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&MImage::setSize));
	pTable->Add("method &setPixel(Integer, Integer, Color)", new EMethodPointerHolder((MachineMethod3)&MImage::setPixel));
	pTable->Add("method getPixel(&Color, Integer, Integer)", new EMethodPointerHolder((MachineMethod3)&MImage::getPixel));
	pTable->Add("method getRect(&Rect)", new EMethodPointerHolder((MachineMethod1)&MImage::getRect));
	pTable->Add("method getSize(&Integer, &Integer)", new EMethodPointerHolder((MachineMethod2)&MImage::getSize));
	pTable->Add("method &load(String)", new EMethodPointerHolder((MachineMethod1)&MImage::load));
	pTable->Add("method save(String)", new EMethodPointerHolder((MachineMethod1)&MImage::save));
}
