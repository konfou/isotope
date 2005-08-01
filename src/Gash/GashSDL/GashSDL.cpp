/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "../../GClasses/GHashTable.h"
#include "../Include/GashEngine.h"

GConstStringHashTable* g_pSdlMachineObjects = NULL;

void RegisterSdlWindow(GConstStringHashTable* pTable);
void RegisterSdlFrame(GConstStringHashTable* pTable);
void RegisterSdlScene(GConstStringHashTable* pTable);

void RegisterSdlMachineClasses()
{
	WrapperObject::RegisterMachineClass(&g_pSdlMachineObjects, "SdlWindow", RegisterSdlWindow);
	WrapperObject::RegisterMachineClass(&g_pSdlMachineObjects, "SdlFrame", RegisterSdlFrame);
	WrapperObject::RegisterMachineClass(&g_pSdlMachineObjects, "SdlScene", RegisterSdlScene);
}
