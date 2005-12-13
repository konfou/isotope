/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "../../GClasses/GHashTable.h"
#include "../Include/GaspEngine.h"

GConstStringHashTable* g_pBuiltInMachineObjects = NULL;


void RegisterGaspFloat(GConstStringHashTable* pTable);
void RegisterGaspStream(GConstStringHashTable* pTable);
void RegisterGaspString(GConstStringHashTable* pTable);


void RegisterBuiltInClasses()
{
	WrapperObject::RegisterMachineClass(&g_pBuiltInMachineObjects, "Float", RegisterGaspFloat);
	WrapperObject::RegisterMachineClass(&g_pBuiltInMachineObjects, "Stream", RegisterGaspStream);
	WrapperObject::RegisterMachineClass(&g_pBuiltInMachineObjects, "String", RegisterGaspString);
}
