/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "../../GClasses/GHashTable.h"
#include "../Include/GashEngine.h"

GConstStringHashTable* g_pQtMachineObjects = NULL;

void RegisterPopUps(GConstStringHashTable* pTable);

void RegisterQtMachineClasses()
{
	WrapperObject::RegisterMachineClass(&g_pQtMachineObjects, "PopUps", RegisterPopUps);
}
