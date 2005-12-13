/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "MachineObjects.h"
#include "../../GClasses/GHashTable.h"
#include "../Include/GaspEngine.h"

GConstStringHashTable* g_pGaspLibMachineObjects = NULL;


void RegisterMEngine(GConstStringHashTable* pTable);
void RegisterMArray(GConstStringHashTable* pTable);
void RegisterMBigInt(GConstStringHashTable* pTable);
void RegisterMImage(GConstStringHashTable* pTable);
void RegisterMConsole(GConstStringHashTable* pTable);
void RegisterMLibrary(GConstStringHashTable* pTable);


void RegisterGaspMachineClasses()
{
	WrapperObject::RegisterMachineClass(&g_pGaspLibMachineObjects, "Array", RegisterMArray);
	WrapperObject::RegisterMachineClass(&g_pGaspLibMachineObjects, "BigInt", RegisterMBigInt);
	WrapperObject::RegisterMachineClass(&g_pGaspLibMachineObjects, "Image", RegisterMImage);
	WrapperObject::RegisterMachineClass(&g_pGaspLibMachineObjects, "Engine", RegisterMEngine);
	WrapperObject::RegisterMachineClass(&g_pGaspLibMachineObjects, "Console", RegisterMConsole);
	WrapperObject::RegisterMachineClass(&g_pGaspLibMachineObjects, "Library", RegisterMLibrary);
}
