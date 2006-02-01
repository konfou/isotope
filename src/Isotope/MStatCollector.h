/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MSTATCOLLECTOR_H__
#define __MSTATCOLLECTOR_H__

#include <stdio.h>

class GQueue;
class GPointerArray;

class MStatCollector
{
protected:
	GQueue* m_pQ;
	FILE* m_pFile;

public:
	MStatCollector();
	~MStatCollector();

	void ReportStats(GPointerArray* pNameValuePairs);

protected:
	void ReportValues(const char* szLine);

};

#endif // __MSTATCOLLECTOR_H__
