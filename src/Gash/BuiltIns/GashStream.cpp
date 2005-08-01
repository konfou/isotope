/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GashStream.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GPointerQueue.h"

void RegisterGashStream(GConstStringHashTable* pTable)
{
	pTable->Add("method !new()", new EMethodPointerHolder((MachineMethod0)&GashStream::allocate));
    pTable->Add("method getSize(&Integer)", new EMethodPointerHolder((MachineMethod1)&GashStream::getSize));
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&GashStream::toStream));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&GashStream::fromStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&GashStream::setRefs));
	pTable->Add("method &writeObject(Object)", new EMethodPointerHolder((MachineMethod1)&GashStream::writeObject));
	pTable->Add("method &readObject(!Object)", new EMethodPointerHolder((MachineMethod1)&GashStream::readObject));
}

void GashStream::toStream(Engine* pEngine, EVar* pStream, EVar* pRefs)
{
	GAssert(false, "todo: write me");
}

void GashStream::fromStream(Engine* pEngine, EVar* pStream)
{
	GAssert(false, "todo: write me");
}

void GashStream::setRefs(Engine* pEngine, EVar* pRefs)
{
	GAssert(false, "todo: throw--this should never be called");
}
