/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "MConsole.h"
#include "MachineObjects.h"
#include "../../GClasses/GHashTable.h"
#include "../Include/GaspLib.h"

void RegisterMConsole(GConstStringHashTable* pTable)
{
	pTable->Add("method print(String)", new EMethodPointerHolder((MachineMethod1)&MConsole::print));
	pTable->Add("method print(Integer)", new EMethodPointerHolder((MachineMethod1)&MConsole::printInt));
	pTable->Add("method print(Float)", new EMethodPointerHolder((MachineMethod1)&MConsole::printFloat));
}

