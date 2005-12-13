/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "TestDataBase.h"
#include "../../../GClasses/GDataBase.h"
#include "../ClassTests.h"
#include <stdlib.h>
#ifdef WIN32
#include <windows.h> // for DeleteFile
#endif // WIN32

void MakeRecord(GDBRecord* pRec, int nVal)
{
	char szTmp[32];
	int nSize = GDataBase::IntToVal(nVal, szTmp);
	pRec->SetFieldCount(2);
	pRec->SetField(0, szTmp, nSize);
	pRec->SetField(1, szTmp, nSize);
}

bool TestDataBase(ClassTests* pThis)
{
	if(!GDataBase::Create("TestDB.gdb"))
		return false;
	GDataBase* pDataBase = GDataBase::Open("TestDB.gdb");
	if(!pDataBase)
		return false;
	bool bRet = false;
	do
	{
		// Make a table
		GDBRecord tbl;
		tbl.SetFieldCount(3);
		tbl.SetField(0, "Val", 3);
		tbl.SetField(1, "Val2", 4);
		tbl.SetField(2, "MyTable", 7);
		if(!pDataBase->AddTable(&tbl))
			break;

		// Add some values to the table
		int nRecordCount = 256;
		int nQueryRuns = 20;
		int n;
		GAssert(((nRecordCount & 1) == 0) && nRecordCount > nQueryRuns && ((nQueryRuns & 1) == 0), "nRecordCount must be even and more than nQueryRuns and nQueryRuns must be even");
		for(n = 0; n < nRecordCount / 2; n += 2)
		{
			// Find the table
			if(!pDataBase->GetTable(&tbl, "MyTable", 7))
				break;

			// Add a record
			GDBRecord rec;
			MakeRecord(&rec, n);
			if(!pDataBase->AddTuple(&rec, &tbl))
				break;
		}
		if(n < nRecordCount / 2)
			break;
		for(n = nRecordCount - 2; n >= nRecordCount / 2; n -= 2)
		{
			// Find the table
			if(!pDataBase->GetTable(&tbl, "MyTable", 7))
				break;

			// Add a record
			GDBRecord rec;
			MakeRecord(&rec, n);
			if(!pDataBase->AddTuple(&rec, &tbl))
				break;
		}
		if(n >= nRecordCount / 2)
			break;

		// Get the table
		const char* pBologna = "Bologna";
		memset(&tbl, '\0', sizeof(GDBRecord));
		if(pDataBase->GetTable(&tbl, pBologna, strlen(pBologna)))
			break;
		if(!pDataBase->GetTable(&tbl, "MyTable", strlen("MyTable")))
			break;
		if(tbl.GetFieldCount() != 3)
			break;

		// Query equal (positive)
		GDBQueryEnumerator qe;
		GDBRecord rec;
		char pVal[256];
		int nValSize = GDataBase::IntToVal(nQueryRuns, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::equal, pVal, nValSize);
		if(!qe.GetNext(&rec))
			break;
		if(rec.GetFieldCount() != 2)
			break;
		if(GDataBase::ValToInt(rec.GetField(0)) != nQueryRuns)
			break;
		if(GDataBase::ValToInt(rec.GetField(1)) != nQueryRuns)
			break;

		// Delete this record
		if(!pDataBase->DeleteRecord(&rec, &tbl))
			break;

		// Query equal (negative never added)
		nValSize = GDataBase::IntToVal(nQueryRuns + 1, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::equal, pVal, nValSize);
		if(qe.GetNext(&rec))
			break;

		// Query equal (negative deleted)
		nValSize = GDataBase::IntToVal(nQueryRuns, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::equal, pVal, nValSize);
		if(qe.GetNext(&rec))
			break;

		// Query greater (left to right)
		int nStart = nRecordCount - nQueryRuns;
		nValSize = GDataBase::IntToVal(nStart, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::greater, pVal, nValSize);
		for(n = nStart + 2; n < nRecordCount; n += 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nRecordCount)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query greater (right to left)
		pDataBase->Query(&qe, &tbl, 1, GDataBase::greater, pVal, nValSize, NULL, 0, false);
		for(n = nRecordCount - 2; n > nStart; n -= 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nStart)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query greater (negative)
		nValSize = GDataBase::IntToVal(nRecordCount - 2, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::greater, pVal, nValSize);
		if(qe.GetNext(&rec))
			break;
		pDataBase->Query(&qe, &tbl, 0, GDataBase::greater, pVal, nValSize, NULL, 0, false);
		if(qe.GetNext(&rec))
			break;

		// Query less (right to left)
		int nEnd = nQueryRuns;
		nValSize = GDataBase::IntToVal(nEnd, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::less, pVal, nValSize, NULL, 0, false);
		for(n = nEnd - 2; n >= 0; n -= 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != -2)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query less (left to right)
		pDataBase->Query(&qe, &tbl, 1, GDataBase::less, pVal, nValSize);
		for(n = 0; n < nEnd; n += 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nEnd)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query less (negative)
		nValSize = GDataBase::IntToVal(0, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::less, pVal, nValSize, NULL, 0, false);
		if(qe.GetNext(&rec))
			break;
		pDataBase->Query(&qe, &tbl, 0, GDataBase::less, pVal, nValSize);
		if(qe.GetNext(&rec))
			break;

		// Query greaterOrEqual (left to right)
		nStart = nRecordCount - nQueryRuns;
		nValSize = GDataBase::IntToVal(nStart, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::greaterOrEqual, pVal, nValSize);
		for(n = nStart; n < nRecordCount; n += 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nRecordCount)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query greaterOrEqual (right to left)
		pDataBase->Query(&qe, &tbl, 1, GDataBase::greaterOrEqual, pVal, nValSize, NULL, 0, false);
		for(n = nRecordCount - 2; n >= nStart; n -= 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nStart - 2)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query greaterOrEqual (negative)
		nValSize = GDataBase::IntToVal(nRecordCount + 2, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::greaterOrEqual, pVal, nValSize);
		if(qe.GetNext(&rec))
			break;
		pDataBase->Query(&qe, &tbl, 0, GDataBase::greaterOrEqual, pVal, nValSize, NULL, 0, false);
		if(qe.GetNext(&rec))
			break;

		// Query lessOrEqual (right to left)
		nEnd = nQueryRuns;
		nValSize = GDataBase::IntToVal(nEnd, pVal);
		pDataBase->Query(&qe, &tbl, 0, GDataBase::lessOrEqual, pVal, nValSize, NULL, 0, false);
		for(n = nEnd; n >= 0; n -= 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != -2)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query lessOrEqual (left to right)
		pDataBase->Query(&qe, &tbl, 1, GDataBase::lessOrEqual, pVal, nValSize);
		for(n = 0; n <= nEnd; n += 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nEnd + 2)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query betweenInclusive (left to right)
		nStart = (nRecordCount - nQueryRuns) / 2;
		if(nStart & 1)
			nStart++;
		nEnd = (nRecordCount + nQueryRuns) / 2;
		if(nEnd & 1)
			nEnd--;
		nValSize = GDataBase::IntToVal(nStart, pVal);
		char pVal2[256];
		int nValSize2 = GDataBase::IntToVal(nEnd, pVal2);		
		pDataBase->Query(&qe, &tbl, 0, GDataBase::betweenInclusive, pVal, nValSize, pVal2, nValSize2);
		for(n = nStart; n <= nEnd; n += 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nEnd + 2)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query betweenInclusive (right to left), (swapped left/right)
		pDataBase->Query(&qe, &tbl, 0, GDataBase::betweenInclusive, pVal2, nValSize2, pVal, nValSize, false);
		for(n = nEnd; n >= nStart; n -= 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nStart - 2)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query betweenExclusive (left to right), (swapped left/right)
		pDataBase->Query(&qe, &tbl, 0, GDataBase::betweenExclusive, pVal2, nValSize2, pVal, nValSize);
		for(n = nStart + 2; n < nEnd; n += 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nEnd)
			break;
		if(qe.GetNext(&rec))
			break;

		// Query betweenExclusive (right to left)
		pDataBase->Query(&qe, &tbl, 0, GDataBase::betweenExclusive, pVal, nValSize, pVal2, nValSize2, false);
		for(n = nEnd - 2; n > nStart; n -= 2)
		{
			if(n == nQueryRuns)
				continue;
			if(!qe.GetNext(&rec))
				break;
			if(GDataBase::ValToInt(rec.GetField(0)) != n)
				break;
		}
		if(n != nStart)
			break;
		if(qe.GetNext(&rec))
			break;

		// It all passed!
		bRet = true;
	} while(false);

	// Clean Up
	delete(pDataBase);
#ifdef WIN32
	DeleteFile("TestDB.gdb");
#else
	// todo: what's the Linux equivalent of DeleteFile?
#endif // !WIN32
	return bRet;
}
