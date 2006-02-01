/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MStatCollector.h"
#include "../GClasses/GQueue.h"
#include "../GClasses/GArray.h"
#include "../Gasp/BuiltIns/GaspString.h"
#include "Main.h"
#include <time.h>

MStatCollector::MStatCollector()
{
	m_pQ = new GQueue();
	m_pFile = fopen("stats.log", "a");
	if(!m_pFile)
		printf("Failed to create stats file");
}

MStatCollector::~MStatCollector()
{
	if(m_pFile)
		fclose(m_pFile);
}

void MStatCollector::ReportStats(GPointerArray* pNameValuePairs)
{
	if(!m_pFile)
		return;
	GAssert(m_pQ->GetSize() == 0, "There's already stuff in the queue");
	int n, i;
	char c;
	int nCount = pNameValuePairs->GetSize();
	nCount /= 2;
	for(n = 0; n < nCount; n++)
	{
		GaspString* pNameString = (GaspString*)pNameValuePairs->GetPointer(2 * n);
		GString* pName = &pNameString->m_value;
		GaspString* pValueString = (GaspString*)pNameValuePairs->GetPointer(2 * n + 1);
		GString* pValue = &pValueString->m_value;
		m_pQ->Push('|');
		int nLen = pName->GetLength();
		for(i = 0; i < nLen; i++)
		{
			c = (char)pName->GetWChar(i);
			m_pQ->Push(c);
		}
		m_pQ->Push('=');
		nLen = pValue->GetLength();
		for(i = 0; i < nLen; i++)
		{
			c = (char)pValue->GetWChar(i);
			m_pQ->Push(c);
		}
	}
	Holder<char*> hValues(m_pQ->DumpToString());
	ReportValues(hValues.Get());
}

void MStatCollector::ReportValues(const char* szLine)
{
	time_t t;
	time_t now = time(&t);
	const char* szNow = ctime(&now);
	char szTime[64];
	strcpy(szTime, szNow);
	int n;
	for(n = strlen(szTime) - 1; n > 0 && szTime[n] <= ' '; n--)
		szTime[n] = '\0';
	fputs(szTime, m_pFile);
	fputs(szLine, m_pFile);
	fputs("\n", m_pFile);
}
