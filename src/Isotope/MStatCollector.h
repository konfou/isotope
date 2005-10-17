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
