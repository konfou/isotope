/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include <stdio.h>
#include "Model.h"
#include "MRealm.h"
#include <time.h>
#include "../GClasses/GXML.h"
#include "Main.h"
#include "MStore.h"
#include "NRealmProtocol.h"
#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <sys/time.h>
#endif // !WIN32


Model::Model()
{
}

Model::~Model()
{
}
