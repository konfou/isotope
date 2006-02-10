/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GDataBase.h"
#include "GMacros.h"
#include "GAVLTree.h"
#include <stdlib.h>

// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------

#define GDB_PAGE_SIZE 4096
#define BITS_IN_PAGE_SIZE 12
#define BYTES_IN_PAGE_MASK ((1 << BITS_IN_PAGE_SIZE) - 1)
#define GDB_PAGES_PER_CHAPTER 256
#define BITS_IN_CHAPTER_SIZE 8
#define PAGE_IN_CHAPTER_MASK ((1 << BITS_IN_CHAPTER_SIZE) - 1)
#define PAGES_TO_CACHE 2048

// -------------------------------------------------------------------
// File Structures
// -------------------------------------------------------------------

struct GDataBaseHeader
{
	int nDataBaseSize;
	int nRootTable;
};

enum GDBRecordType
{
	GDBRT_DELETED = 100,
	GDBRT_TABLE = 101,
	GDBRT_TUPLE = 102,
};

struct GDBRecordHeader
{
	int nState; // a GDBRecordType enum
	int nRecordSize; // size in bytes of the entire record
	int nFieldCount; // count of the values in this record
	// GDBValue pValues[];
};

struct GDBValue
{
	int nLeft; // For Table values, this refers to the head tuple
	int nRight;
	int nParent;
	int nHeight; // The max of the heights of both children plus 1
	int nRecord; // The record position of the record that contains this value
	int nValueSize;
	// char sValue[];
};


// -------------------------------------------------------------------
// Page
// -------------------------------------------------------------------

class GDBPage
{
public:
	char m_data[GDB_PAGE_SIZE];
	int m_nPos;
	bool m_bDirty;

	// Pages are linked in a list in order of most recently used
	// so that when we run out of RAM we can throw out the least
	// recently used page.
	GDBPage* m_pPrev;
	GDBPage* m_pNext;

	GDBPage(int nPos)
	{
		GAssert((nPos & BYTES_IN_PAGE_MASK) == 0, "Page not aligned properly");
		m_nPos = nPos;
		m_bDirty = false;
		m_pPrev = NULL;
		m_pNext = NULL;
	}

	virtual ~GDBPage()
	{
	}
};

// -------------------------------------------------------------------
// Chapter
// -------------------------------------------------------------------


class GDBChapterLookup : public GAVLNode
{
public:
	int m_nChapter;

	GDBChapterLookup(int nChapter)
	{
		m_nChapter = nChapter;
	}

	virtual ~GDBChapterLookup()
	{
	}

	virtual int Compare(GAVLNode* pThat)
	{
		if(m_nChapter < ((GDBChapterLookup*)pThat)->m_nChapter)
			return -1;
		if(m_nChapter > ((GDBChapterLookup*)pThat)->m_nChapter)
			return 1;
		return 0;
	}
};

class GDBChapter : public GDBChapterLookup
{
public:
	GDBPage* m_pPages[GDB_PAGES_PER_CHAPTER];
	
	GDBChapter(int nChapter) : GDBChapterLookup(nChapter)
	{
		memset(m_pPages, '\0', sizeof(GDBPage*) * GDB_PAGES_PER_CHAPTER);
	}

	virtual ~GDBChapter()
	{
		int n;
		for(n = 0; n < GDB_PAGES_PER_CHAPTER; n++)
			delete(m_pPages[n]);
	}
};

// -------------------------------------------------------------------
// GDBRecord
// -------------------------------------------------------------------

GDBRecord::GDBRecord()
{
	m_nFieldCount = 0;
	m_nRecordPos = 0;
	m_pFields = NULL;
	m_bValidTable = false;
}

GDBRecord::~GDBRecord()
{
	Clear();
}

void GDBRecord::Clear()
{
	int n;
	for(n = 0; n < m_nFieldCount; n++)
		delete(m_pFields[n].pValue);
	delete(m_pFields);
	m_nFieldCount = 0;
	m_pFields = NULL;
}

void GDBRecord::SetFieldCount(int nFieldCount)
{
	Clear();
	if(nFieldCount > 0)
	{
		m_pFields = new struct GDBRecordField[nFieldCount];
		memset(m_pFields, '\0', sizeof(struct GDBRecordField) * nFieldCount);
	}
	else
		m_pFields = NULL;
	m_nFieldCount = nFieldCount;
}

// note that this will take ownership of pValue
void GDBRecord::SetField(int nField, int nValuePos, int nValueSize, int nValueSizeLoaded, void* pValue)
{
	GAssert(nField >= 0 && nField < m_nFieldCount, "Out of range (725)");
	m_pFields[nField].nRootPos = 0;
	m_pFields[nField].nRootValue = 0;
	m_pFields[nField].nValuePos = nValuePos;
	m_pFields[nField].nValueSize = nValueSize;
	m_pFields[nField].nValueSizeLoaded = nValueSizeLoaded;
	delete(m_pFields[nField].pValue);
	m_pFields[nField].pValue = (char*)pValue;
}

void GDBRecord::SetField(int nField, const void* pValue, int nValueSize)
{
	char* pNewValue = new char[nValueSize + 2];
	memcpy(pNewValue, pValue, nValueSize);
	pNewValue[nValueSize] = '\0';
	pNewValue[nValueSize + 1] = '\0';
	SetField(nField, 0, nValueSize, nValueSize, pNewValue);
}

int GDBRecord::GetRecordSize()
{
	int nSize = sizeof(struct GDBRecordHeader);
	nSize += m_nFieldCount * sizeof(struct GDBValue);
	int n;
	for(n = 0; n < m_nFieldCount; n++)
		nSize += m_pFields[n].nValueSize;
	return nSize;
}

void GDBRecord::SetRoot(int nField, int nRootPos, int nRootValue)
{
	m_pFields[nField].nRootPos = nRootPos;
	m_pFields[nField].nRootValue = nRootValue;
}

int GDBRecord::GetRootPos(int nField)
{
	GAssert(m_pFields[nField].nRootPos, "The root pos should never be zero--this happens if it's not a table or the table wasn't added to the database");
	return m_pFields[nField].nRootPos;
}

int GDBRecord::GetRootValue(int nField)
{
	return m_pFields[nField].nRootValue;
}

// -------------------------------------------------------------------
// GDBQueryEnumerator
// -------------------------------------------------------------------

inline int CompareValues(const char* sThis, int nThisSize, const char* sThat, int nThatSize)
{
	// Compare the values
	int nCmp = 0;
	int nSize = nThisSize;
	if(nThisSize > nThatSize)
	{
		nSize = nThatSize;
		nCmp = 1;
	}
	else if(nThisSize < nThatSize)
		nCmp = -1;
	int n;
	for(n = 0; n < nSize; n++)
	{
		if(sThis[n] > sThat[n])
		{
			nCmp = 1;
			break;
		}
		if(sThis[n] < sThat[n])
		{
			nCmp = -1;
			break;
		}
	}
	return nCmp;
}

bool GDBQueryEnumerator::GetNext(GDBRecord* pOutRecord)
{
	// Check if we already finished
	if(!m_pDataBase)
		return false;

#ifdef _DEBUG
	// Make sure we have a good database object
	if(!m_pDataBase->Check())
	{
		GAssert(false, "DataBase object corrupt or deleted!");
		return false;
	}
#endif // _DEBUG

	// Make sure we know the next record pos
	if(m_nBufferedRecordPos == 0)
	{
		// Find the next record
		m_nBufferedRecordPos = m_pDataBase->GetNextRecord(&m_nCurrentValuePos, m_pTable, m_nField, m_bLeftToRight);
		if(m_nBufferedRecordPos < 1)
		{
			GAssert(m_nBufferedRecordPos == 0, "bad record pos");
			m_pDataBase = NULL; // indicates we are done
			return false;
		}
	}

	// Read the record
	GAssert(m_nBufferedRecordPos > 0, "bad record pos");
	if(!m_pDataBase->ReadRecord(pOutRecord, m_nBufferedRecordPos, m_nValueSizeCap))
	{
		GAssert(false, "failed to read record");
		m_pDataBase = NULL; // indicates we are done
		return false;
	}
	m_nBufferedRecordPos = 0;

	// Check the value
	if(!m_bNeedCompareValue)
		return true;
	int nCmp = CompareValues(pOutRecord->GetField(m_nField), pOutRecord->GetFieldSize(m_nField), m_sCompareValue, m_nCompareValueSize);
	switch(m_eQueryType)
	{
		case GDataBase::equal:
			if(nCmp != 0)
			{
				GAssert(nCmp = m_bLeftToRight ? 1 : -1, "sorting problem");
				m_pDataBase = NULL;
				return false;
			}
			break;

		case GDataBase::less:
			if(nCmp >= 0)
			{
				m_pDataBase = NULL;
				return false;
			}
			break;

		case GDataBase::greater:
			if(nCmp <= 0)
			{
				m_pDataBase = NULL;
				return false;
			}
			break;

		case GDataBase::lessOrEqual:
			if(nCmp > 0)
			{
				m_pDataBase = NULL;
				return false;
			}
			break;

		case GDataBase::greaterOrEqual:
			if(nCmp < 0)
			{
				m_pDataBase = NULL;
				return false;
			}
			break;

		case GDataBase::betweenInclusive:
			if(m_bLeftToRight)
			{
				// same as lessOrEqual
				if(nCmp > 0)
				{
					m_pDataBase = NULL;
					return false;
				}
			}
			else
			{
				// same as greaterOrEqual
				if(nCmp < 0)
				{
					m_pDataBase = NULL;
					return false;
				}
			}
			break;

		case GDataBase::betweenExclusive:
			if(m_bLeftToRight)
			{
				// same as less
				if(nCmp >= 0)
				{
					m_pDataBase = NULL;
					return false;
				}
			}
			else
			{
				// same as greater
				if(nCmp <= 0)
				{
					m_pDataBase = NULL;
					return false;
				}
			}
			break;

		default:
			GAssert(false, "unexpected enum value");
			break;
	}

	return true;
}

// -------------------------------------------------------------------
// GDataBase
// -------------------------------------------------------------------

#define ReadHeaderValue(field) ReadIntValue(MEMBEROFFSET(struct GDataBaseHeader, field))
#define WriteHeaderValue(field, value) WriteIntValue(MEMBEROFFSET(struct GDataBaseHeader, field), value)

GDataBase::GDataBase(FILE* pFile)
{
	GAssert((1 << BITS_IN_PAGE_SIZE) == GDB_PAGE_SIZE, "Page size problem");
	GAssert((1 << BITS_IN_CHAPTER_SIZE) == GDB_PAGES_PER_CHAPTER, "Chapter size problem");
	m_pFile = pFile;
	m_pChapters = new GAVLTree();
	m_nChaptersCreated = 0;
	m_pCurrentChapter = NULL;
	m_pMostRecentlyUsedPage = NULL;
	m_pLeastRecentlyUsedPage = NULL;
	m_nPageCacheSize = 0;
#ifdef _DEBUG
	m_nMagic = 0x31636a54; // This value is so we can check that the database hasn't been deleted
#endif // _DEBUG
}

GDataBase::~GDataBase()
{
#ifdef _DEBUG
	m_nMagic = 0;
#endif // _DEBUG
	Flush();
	fclose(m_pFile);
	delete(m_pChapters);
}

/*static*/ bool GDataBase::Create(const char* szFilename)
{
	// Create a file
	FILE* pFile = fopen(szFilename, "wb");
	if(!pFile)
	{
		GAssert(false, "failed to create file");
		return false;
	}

	// Create a header
	struct GDataBaseHeader header;
	header.nRootTable = 0;
	header.nDataBaseSize = sizeof(struct GDataBaseHeader);

	// Write it to the file
	fwrite(&header, sizeof(struct GDataBaseHeader), 1, pFile); // todo: check for errors
	fclose(pFile);
	return true;
}

/*static*/ GDataBase* GDataBase::Open(const char* szFilename)
{
	FILE* pFile = fopen(szFilename, "rb+");
	if(!pFile)
	{
		GAssert(false, "failed to open file");
		return NULL;
	}
	GDataBase* pDataBase = new GDataBase(pFile);
	return pDataBase;
}

int GDataBase::ReadIntValue(int nPos)
{
	int nValue;
	if(!Read((char*)&nValue, nPos, sizeof(int)))
	{
		GAssert(false, "error reading int");
		return 0;
	}
	return nValue;
}

void GDataBase::WriteIntValue(int nPos, int nValue)
{
	if(!Write((char*)&nValue, nPos, sizeof(int)))
		GAssert(false, "error writing int");
}

bool GDataBase::AddRecord(GDBRecord* pRecord, GDBRecord* pTableRecord)
{
	GAssert(!pTableRecord || pTableRecord->IsValidTable(), "This table is no longer valid.  (You must get the table again after you add a record to it.)");
	GAssert(!pTableRecord || pTableRecord->GetFieldCount() - 1 == pRecord->GetFieldCount(), "Field count mismatch");
	if(pTableRecord)
		pTableRecord->SetValidTable(false);
	
	// Allocate space for a record
	int nSize = pRecord->GetRecordSize();
	GTEMPBUF(pRec, nSize); // this macro allocates a temporary buffer in pRec

	// Fill in the struct values
	struct GDBRecordHeader* pHeader = (struct GDBRecordHeader*)pRec;
	pHeader->nFieldCount = pRecord->GetFieldCount();
	pHeader->nRecordSize = nSize;
	pHeader->nState = pTableRecord ? GDBRT_TUPLE : GDBRT_TABLE;

	// Find a spot for it
	int nPos = FindDeletedRecordToReuse(pHeader->nRecordSize);
	if(nPos == 0)
	{
		nPos = ReadHeaderValue(nDataBaseSize);
		WriteHeaderValue(nDataBaseSize, nPos + pHeader->nRecordSize);
	}

	// Set all the field names
	char* pTmp = pRec + sizeof(struct GDBRecordHeader);
	struct GDBValue* pValue;
	int nFieldCount = pHeader->nFieldCount;
	int n;
	for(n = 0; n < nFieldCount; n++)
	{
		pValue = (struct GDBValue*)pTmp;
		pValue->nLeft = 0;
		pValue->nRight = 0;
		pValue->nParent = 0;
		pValue->nHeight = 1;
		pValue->nRecord = nPos;
		pValue->nValueSize = pRecord->GetFieldSize(n);
		pTmp += sizeof(struct GDBValue);
		memcpy(pTmp, pRecord->GetField(n), pValue->nValueSize);
		pTmp += pValue->nValueSize;
	}
	GAssert(pTmp - pRec == nSize, "alignment error");

	// Write the record
	if(!Write(pRec, nPos, nSize))
		return false;

	// Insert the table
	if(pTableRecord)
	{
		// Insert each field
		pTmp = pRec + sizeof(struct GDBRecordHeader);
		for(n = 0; n < nFieldCount; n++)
		{
			pValue = (struct GDBValue*)pTmp;
			pTmp += sizeof(struct GDBValue);
			pTmp += pValue->nValueSize;
			InsertValue(pTableRecord->GetRootPos(n), nPos + ((char*)pValue - (char*)pRec));
		}
	}
	else
	{
		// Update root values
		pTmp = pRec + sizeof(struct GDBRecordHeader);
		for(n = 0; n < nFieldCount - 1; n++)
		{
			pValue = (struct GDBValue*)pTmp;
			pTmp += sizeof(struct GDBValue);
			pTmp += pValue->nValueSize;
			pRecord->SetRoot(n, nPos + ((char*)pValue - (char*)pRec) + MEMBEROFFSET(struct GDBValue, nLeft), pValue->nLeft);
		}

		// Only insert the last field
		InsertValue(MEMBEROFFSET(struct GDataBaseHeader, nRootTable), nPos + ((char*)pTmp - pRec));
	}

	return true;
}

bool GDataBase::AddTable(GDBRecord* pRecord)
{
	return AddRecord(pRecord, NULL);
}

bool GDataBase::AddTuple(GDBRecord* pRecord, GDBRecord* pTableRecord)
{
	GAssert(pTableRecord, "pTableRecord can't be NULL");
	return AddRecord(pRecord, pTableRecord);
}

int GDataBase::FindRecord(const char* szValue, int nValueSize, bool bRightMost, bool bClosest, int nRootValue, int* pnOutValPos)
{
	*pnOutValPos = Find(szValue, nValueSize, bRightMost, bClosest, nRootValue);
	if(*pnOutValPos < 1)
		return 0;
	struct GDBValue valThis;
	if(!Read((char*)&valThis, *pnOutValPos, sizeof(struct GDBValue)))
	{
		GAssert(false, "error reading value");
		return 0;
	}
	GAssert(valThis.nRecord > 0, "bad record pos");
	return valThis.nRecord;
}

bool GDataBase::GetTable(GDBRecord* pOutTable, const void* pName, int nNameSize)
{
	int nTmp;
	int nTablePos = FindRecord((const char*)pName, nNameSize, false, false, ReadHeaderValue(nRootTable), &nTmp);
	if(nTablePos < 1)
		return false;
	bool bRet = ReadRecord(pOutTable, nTablePos, MAX_VALUE_CAP);
	if(bRet)
		pOutTable->SetValidTable(true);
	return bRet;
}

bool GDataBase::GetTuple(GDBRecord* pOutTuple, GDBRecord* pInTable, int nField, const void* pValue, int nValueSize, int nValueSizeCap/*=MAX_VALUE_CAP*/)
{
	int nTmp;
	int nTuplePos = FindRecord((const char*)pValue, nValueSize, false, false, pInTable->GetRootValue(nField), &nTmp);
	if(nTuplePos < 1)
		return false;
	return ReadRecord(pOutTuple, nTuplePos, nValueSizeCap);
}

void GDataBase::Query(GDBQueryEnumerator* pOutEnumerator, GDBRecord* pTableRecord, int nField, QueryType eQueryType, const void* pValue1, int nValue1Size, const void* pValue2 /*=NULL*/, int nValue2Size /*=0*/, bool bLeftToRight /*=true*/, int nValueSizeCap /*=MAX_VALUE_CAP*/)
{
	pOutEnumerator->m_pDataBase = NULL;

	// Check parameters, swap values if necessary, and figure out which value to search on
	const char* pSearchValue = (const char*)pValue1;
	int nSearchValueSize = nValue1Size;
	bool bRightMost = !bLeftToRight;
	bool bEnd = false;
	bool bClosest = true;
	switch(eQueryType)
	{
		case less:
			bRightMost = bLeftToRight;
			// intentional fall through
		case lessOrEqual:
			GAssert(!pValue2, "Only a one-value operation");
			if(bLeftToRight)
				bEnd = true;
			break;

		case greater:
			bRightMost = bLeftToRight;
			// intentional fall through
		case greaterOrEqual:
			GAssert(!pValue2, "Only a one-value operation");
			if(!bLeftToRight)
				bEnd = true;
			break;

		case equal:
			GAssert(!pValue2, "Only a one-value operation");
			bClosest = false;
			break;

		case betweenExclusive:
			bRightMost = bLeftToRight;
			// intentional fall-through
		case betweenInclusive:
			GAssert(pValue2, "Expected a second value");
			if(CompareValues((const char*)pValue1, nValue1Size, (const char*)pValue2, nValue2Size) > 0)
			{
				const char* pTmp = (const char*)pValue2;
				int nTmpSize = nValue2Size;
				pValue2 = pValue1;
				nValue2Size = nValue1Size;
				pValue1 = pTmp;
				nValue1Size = nTmpSize;
				pSearchValue = (const char*)pValue1;
				nSearchValueSize = nValue1Size;
			}
			if(!bLeftToRight)
			{
				pSearchValue = (const char*)pValue2;
				nSearchValueSize = nValue2Size;
			}
			break;

		default:
			GAssert(false, "unexpected value");
			break;
	}

	int nValPos;
	if(bEnd)
		nValPos = GetLeftOrRightMost(pTableRecord->GetRootValue(nField), !bLeftToRight);
	else
	{
		// Search for the first record
		nValPos = Find(pSearchValue, nSearchValueSize, bRightMost, bClosest, pTableRecord->GetRootValue(nField));

		// See if we need to get the next record
		bool bNeedNext = false;
		if(nValPos <= 0)
		{
			switch(eQueryType)
			{
				case equal:
				case less:
				case greater:
				case betweenExclusive:
					return;

				case lessOrEqual:
				case greaterOrEqual:
				case betweenInclusive:
					bNeedNext = true;
					break;

				default:
					GAssert(false, "unexpected value");
					break;
			}
		}
		else
		{
			if(eQueryType != equal)
			{
				// Load the value
				struct GDBValue valFirst;
				if(!Read((char*)&valFirst, nValPos, sizeof(struct GDBValue)))
				{
					GAssert(false, "failed to read");
					return;
				}
				char sFirst[GDB_VALUE_SORTING_PART_SIZE];
				int nFirstSize = MIN(valFirst.nValueSize, GDB_VALUE_SORTING_PART_SIZE);
				if(!Read(sFirst, nValPos + sizeof(struct GDBValue), nFirstSize))
				{
					GAssert(false, "failed to read");
					return;
				}

				// See if it's in range
				int nCmp = CompareValues(sFirst, nFirstSize, pSearchValue, nSearchValueSize);
				switch(eQueryType)
				{
					case less:
						GAssert(nCmp <= 0, "Find returned bad results");
						if(nCmp >= 0)
							bNeedNext = true;
						break;

					case lessOrEqual:
						if(nCmp > 0)
							bNeedNext = true;
						break;

					case greater:
						GAssert(nCmp >= 0, "Find returned bad results");
						if(nCmp <= 0)
							bNeedNext = true;
						break;

					case greaterOrEqual:
						if(nCmp < 0)
							bNeedNext = true;
						break;

					case betweenExclusive:
						if(bLeftToRight)
						{
							// Same as greater
							GAssert(nCmp >= 0, "Find returned bad results");
							if(nCmp <= 0)
								bNeedNext = true;
						}
						else
						{
							// Same as less
							GAssert(nCmp <= 0, "Find returned bad results");
							if(nCmp >= 0)
								bNeedNext = true;
						}
						break;

					case betweenInclusive:
						if(bLeftToRight)
						{
							// Same as greaterOrEqual
							if(nCmp < 0)
								bNeedNext = true;
						}
						else
						{
							// Same as lessOrEqual
							if(nCmp > 0)
								bNeedNext = true;
						}
						break;

					default:
						GAssert(false, "unexpected value");
						break;
				}
			}
		}

		// Get the next value (if necessary)
		if(bNeedNext)
			nValPos = GetNextValue(nValPos, pTableRecord, nField, bLeftToRight);
	}

	// Convert the value to a record pos
	if(nValPos <= 0)
		return;
	struct GDBValue valThis;
	if(!Read((char*)&valThis, nValPos, sizeof(struct GDBValue)))
	{
		GAssert(false, "error reading value");
		return;
	}
	GAssert(valThis.nRecord > 0, "bad record pos");

	// Set up the query enumerator
	pOutEnumerator->m_bLeftToRight = bLeftToRight;
	pOutEnumerator->m_nBufferedRecordPos = valThis.nRecord;
	pOutEnumerator->m_nCurrentValuePos = nValPos;
	pOutEnumerator->m_nField = nField;
	pOutEnumerator->m_nValueSizeCap = nValueSizeCap;
	pOutEnumerator->m_pTable = pTableRecord;
	pOutEnumerator->m_pDataBase = this;
	pOutEnumerator->m_eQueryType = eQueryType;

	// See if the enumerator needs to check the value
	pOutEnumerator->m_bNeedCompareValue = true;
	switch(eQueryType)
	{
		case less:
		case lessOrEqual:
			if(!bLeftToRight)
				pOutEnumerator->m_bNeedCompareValue = false;
			break;
		case greater:
		case greaterOrEqual:
			if(bLeftToRight)
				pOutEnumerator->m_bNeedCompareValue = false;
			break;
		default:
			break;
	}
	if(pOutEnumerator->m_bNeedCompareValue)
	{
		bool bFirstValue = true;
		if(eQueryType == betweenInclusive || eQueryType == betweenExclusive)
		{
			if(bLeftToRight)
				bFirstValue = false;
		}
		pOutEnumerator->m_nCompareValueSize = MIN(bFirstValue ? nValue1Size : nValue2Size, GDB_VALUE_SORTING_PART_SIZE);
		memcpy(pOutEnumerator->m_sCompareValue, bFirstValue ? pValue1 : pValue2, pOutEnumerator->m_nCompareValueSize);
	}
	else
		pOutEnumerator->m_nCompareValueSize = 0;
}

int GDataBase::GetNextRecord(int* pnValPos, GDBRecord* pTableRecord, int nField, bool bRight)
{
	*pnValPos = GetNextValue(*pnValPos, pTableRecord, nField, bRight);
	if(*pnValPos < 1)
	{
		GAssert(*pnValPos == 0, "bad value");
		return 0;
	}
	struct GDBValue valNext;
	if(!Read((char*)&valNext, *pnValPos, sizeof(struct GDBValue)))
	{
		GAssert(false, "error reading value");
		return 0;
	}
	GAssert(valNext.nRecord > 0, "bad record value");
	return valNext.nRecord;
}

int GDataBase::GetNextValue(int nPos, GDBRecord* pTableRecord, int nField, bool bRight)
{
	int nNextValPos;
	if(nPos == 0)
	{
		// Get first or last value in the database
		int nRootVal;
		if(pTableRecord)
			nRootVal = pTableRecord->GetRootValue(nField);
		else
			nRootVal = ReadHeaderValue(nRootTable);
		nNextValPos = GetLeftOrRightMost(nRootVal, !bRight);
	}
	else
	{
		// Find the next value
		GAssert(nPos > 0, "bad value pos");
		struct GDBValue valThis;
		if(!Read((char*)&valThis, nPos, sizeof(struct GDBValue)))
		{
			GAssert(false, "error reading value");
			return 0;
		}
		if(bRight)
			nNextValPos = GetNextRight(nPos, &valThis);
		else
			nNextValPos = GetNextLeft(nPos, &valThis);
		GAssert(nPos == 0 || nNextValPos != nPos, "it's the same value again");
	}

	return nNextValPos;
}

bool GDataBase::ReadRecord(GDBRecord* pOutRecord, int nPos, int nValueSizeCap)
{
	struct GDBRecordHeader header;
	if(!Read((char*)&header, nPos, sizeof(struct GDBRecordHeader)))
	{
		GAssert(false, "error reading record");
		return false;
	}
	pOutRecord->SetRecordPos(nPos);
	nPos += sizeof(struct GDBRecordHeader);
	pOutRecord->SetFieldCount(header.nFieldCount);
	struct GDBValue value;
	int n;
	for(n = 0; n < header.nFieldCount; n++)
	{
		int nValuePos = nPos;
		if(!Read((char*)&value, nPos, sizeof(struct GDBValue)))
		{
			GAssert(false, "error reading record");
			return false;
		}
		nPos += sizeof(struct GDBValue);
		int nReadSize = MIN(nValueSizeCap, value.nValueSize);
		char* pBuf = new char[nReadSize + 2];
		if(!Read(pBuf, nPos, nReadSize))
		{
			GAssert(false, "error reading record");
			return false;
		}
		pBuf[nReadSize] = '\0';
		pBuf[nReadSize + 1] = '\0';
		pOutRecord->SetField(n, nPos, value.nValueSize, nReadSize, pBuf);
		pOutRecord->SetRoot(n, nValuePos + MEMBEROFFSET(struct GDBValue, nLeft), value.nLeft);
		nPos += value.nValueSize;
	}
	return true;
}

bool GDataBase::DeleteRecord(GDBRecord* pTuple, GDBRecord* pTable)
{
	int nPos = pTuple->GetRecordPos();
	if(nPos <= 0)
		return false;

	// Read in the header (to get the record size)
	struct GDBRecordHeader header;
	if(!Read((char*)&header, nPos, sizeof(struct GDBRecordHeader)))
		return false;
	if(header.nState == GDBRT_DELETED)
		return false;

	// Unlink all the fields
	nPos += sizeof(struct GDBRecordHeader);
	struct GDBValue value;
	int n;
	GAssert(pTable->GetFieldCount() == header.nFieldCount + 1, "Table doesn't go with record");
	for(n = 0; n < header.nFieldCount; n++)
	{
		if(!Read((char*)&value, nPos, sizeof(struct GDBValue)))
		{
			GAssert(false, "error reading record");
			return false;
		}
		UnlinkValue(nPos, &value, pTable->GetRootPos(n));
		nPos += sizeof(struct GDBValue);
		nPos += value.nValueSize;
	}

	// Put in table of free area to reuse

	return true;
}

int GDataBase::FindDeletedRecordToReuse(int nSize)
{
	// todo: write this
	return 0;
}

void GDataBase::Flush()
{
	GDBPage* pPage;
	for(pPage = m_pMostRecentlyUsedPage; pPage; pPage = pPage->m_pNext)
		FlushPage(pPage);
	m_pMostRecentlyUsedPage = NULL;
}

void GDataBase::FlushPage(GDBPage* pPage)
{
	// Write the changes to disk
	if(!pPage->m_bDirty)
		return;
	fseek(m_pFile, pPage->m_nPos, SEEK_SET); // todo: check for errors
	fwrite(pPage->m_data, GDB_PAGE_SIZE, 1, m_pFile); // todo: check for errors
	pPage->m_bDirty = false;
}

GDBPage* GDataBase::GetPage(int nPos)
{
	// Calculate chapter and page
	GAssert(nPos >= 0, "out of range (726)");
	int nPage = nPos >> BITS_IN_PAGE_SIZE;
	int nChapter = nPage >> BITS_IN_CHAPTER_SIZE;
	nPage &= PAGE_IN_CHAPTER_MASK;

	// Find the right chapter
	GDBChapter* pChapter;
	if(m_pCurrentChapter && m_pCurrentChapter->m_nChapter == nChapter)
		pChapter = m_pCurrentChapter;
	else
	{
		GDBChapterLookup tmp(nChapter);
		int nIndex;
		pChapter = (GDBChapter*)m_pChapters->GetNode(&tmp, &nIndex);
		if(!pChapter)
		{
			pChapter = new GDBChapter(nChapter);
			m_pChapters->Insert(pChapter);
		}
	}

	// Get the right page
	GDBPage* pPage = pChapter->m_pPages[nPage];
	if(!pPage)
	{
		// Load the new page
		pPage = new GDBPage(nPos & (~BYTES_IN_PAGE_MASK));
		pChapter->m_pPages[nPage] = pPage;
		fseek(m_pFile, nPos & (~BYTES_IN_PAGE_MASK), SEEK_SET); // todo: check for errors
		fread(pPage->m_data, GDB_PAGE_SIZE, 1, m_pFile); // todo: check for errors

		// Throw out an old page (if necessary)
		if(m_nPageCacheSize > PAGES_TO_CACHE)
		{
			GDBPage* pCondemnedPage = m_pLeastRecentlyUsedPage;
			m_pLeastRecentlyUsedPage = m_pLeastRecentlyUsedPage->m_pPrev;
			if(m_pLeastRecentlyUsedPage)
				m_pLeastRecentlyUsedPage->m_pNext = NULL;
			m_nPageCacheSize--;
			FlushPage(pCondemnedPage);

			// Remove page from the chapter
			int nPage2 = pCondemnedPage->m_nPos >> BITS_IN_PAGE_SIZE;
			int nChapter2 = nPage2 >> BITS_IN_CHAPTER_SIZE;
			nPage2 &= PAGE_IN_CHAPTER_MASK;
			GDBChapterLookup tmp(nChapter2);
			int nIndex;
			pChapter = (GDBChapter*)m_pChapters->GetNode(&tmp, &nIndex);
			GAssert(pChapter, "Chapter for condemned page not found");
			GAssert(pChapter->m_pPages[nPage2] == pCondemnedPage, "Condemned page not in chapter");
			pChapter->m_pPages[nPage2] = NULL;

			// Delete the page
			delete(pCondemnedPage);
		}
	}

	// Move this page to the front of the list
	GAssert(pPage, "Should have found the page by now");
	if(m_pMostRecentlyUsedPage != pPage)
	{
		if(pPage->m_pPrev)
			pPage->m_pPrev->m_pNext = pPage->m_pNext;
		if(m_pLeastRecentlyUsedPage == pPage)
		{
			m_pLeastRecentlyUsedPage = pPage->m_pPrev;
			GAssert(pPage->m_pNext == NULL, "not the last in the list");
		}
		else if(pPage->m_pNext)
			pPage->m_pNext->m_pPrev = pPage->m_pPrev;
		pPage->m_pNext = m_pMostRecentlyUsedPage;
		pPage->m_pPrev = NULL;
		if(m_pMostRecentlyUsedPage)
			m_pMostRecentlyUsedPage->m_pPrev = pPage;
		m_pMostRecentlyUsedPage = pPage;
		if(!m_pLeastRecentlyUsedPage)
			m_pLeastRecentlyUsedPage = pPage;
	}

	return pPage;
}

bool GDataBase::Read(char* pBuf, int nPos, int nSize)
{
	GAssert(nPos > 0 || (nPos == 0 && nSize == sizeof(int)), "out of range (727)");
	while(nSize > 0)
	{
		GDBPage* pPage = GetPage(nPos);
		int nBytesForThisPage;
		if((nPos & BYTES_IN_PAGE_MASK) + nSize > GDB_PAGE_SIZE)
			nBytesForThisPage = GDB_PAGE_SIZE - (nPos & BYTES_IN_PAGE_MASK);
		else
			nBytesForThisPage = nSize;
		memcpy(pBuf, &pPage->m_data[nPos & BYTES_IN_PAGE_MASK], nBytesForThisPage);
		pBuf += nBytesForThisPage;
		nPos += nBytesForThisPage;
		nSize -= nBytesForThisPage;
	}
	return true;
}

bool GDataBase::Write(const char* pBuf, int nPos, int nSize)
{
	GAssert(nPos > 0 || (nPos == 0 && nSize == sizeof(int)), "out of range (728)");
	while(nSize > 0)
	{
		GDBPage* pPage = GetPage(nPos);
		pPage->m_bDirty = true;
		int nBytesForThisPage;
		if((nPos & BYTES_IN_PAGE_MASK) + nSize > GDB_PAGE_SIZE)
			nBytesForThisPage = GDB_PAGE_SIZE - (nPos & BYTES_IN_PAGE_MASK);
		else
			nBytesForThisPage = nSize;
		memcpy(&pPage->m_data[nPos & BYTES_IN_PAGE_MASK], pBuf, nBytesForThisPage);
		pBuf += nBytesForThisPage;
		nPos += nBytesForThisPage;
		nSize -= nBytesForThisPage;
	}
	return true;
}

// -------------------------------------------------------------------
// AVL Tree methods
// -------------------------------------------------------------------

void GDataBase::GetChildrenHeights(struct GDBValue* pVal, int* pnLeft, int* pnRight)
{
	struct GDBValue valTmp;

	// Left
	*pnLeft = 0;
	if(pVal->nLeft)
	{
		if(!Read((char*)&valTmp, pVal->nLeft, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read while fixing height");
			valTmp.nHeight = 1; // guess (best hope for recovery)
		}
		*pnLeft = valTmp.nHeight;
	}

	// Right
	*pnRight = 0;
	if(pVal->nRight)
	{
		if(!Read((char*)&valTmp, pVal->nRight, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read while fixing height");
			valTmp.nHeight = 1; // guess (best hope for recovery)
		}
		*pnRight = valTmp.nHeight;
	}
}

void GDataBase::FixHeight(struct GDBValue* pVal)
{
	int nLeftHeight;
	int nRightHeight;
	GetChildrenHeights(pVal, &nLeftHeight, &nRightHeight);
	pVal->nHeight = MAX(nLeftHeight, nRightHeight) + 1;
}

// This method must be kept in sync with RotateRight
int GDataBase::RotateLeft(int nPos, GDBValue* pVal)
{
	// Read in right record
	int nRightPos = pVal->nRight;
	if(nRightPos <= 0)
	{
		GAssert(false, "Can't rotate left if there's no right child");
		return 0;
	}
	struct GDBValue valRight;
	if(!Read((char*)&valRight, nRightPos, sizeof(struct GDBValue)))
	{
		GAssert(false, "failed to read while rotating");
		return 0;
	}

	// Do the rotation
	int nTmp = valRight.nLeft;
	valRight.nLeft = nPos;
	int nOldPar = pVal->nParent;
	pVal->nParent = nRightPos;
	valRight.nParent = nOldPar;
	pVal->nRight = nTmp;
	if(nTmp > 0)
		WriteIntValue(nTmp + MEMBEROFFSET(struct GDBValue, nParent), nPos);
	FixHeight(pVal);
	if(!Write((char*)pVal, nPos, sizeof(struct GDBValue)))
		GAssert(false, "failed to write while rotating");
	FixHeight(&valRight);
	if(!Write((char*)&valRight, nRightPos, sizeof(struct GDBValue)))
		GAssert(false, "failed to write while rotating");

	return nRightPos;
}

// This method must be kept in sync with RotateLeft
int GDataBase::RotateRight(int nPos, GDBValue* pVal)
{
	// Read in left record
	int nLeftPos = pVal->nLeft;
	if(nLeftPos <= 0)
	{
		GAssert(false, "Can't rotate right if there's no left child");
		return 0;
	}
	struct GDBValue valLeft;
	if(!Read((char*)&valLeft, nLeftPos, sizeof(struct GDBValue)))
	{
		GAssert(false, "failed to read while rotating");
		return 0;
	}

	// Do the rotation
	int nTmp = valLeft.nRight;
	valLeft.nRight = nPos;
	int nOldPar = pVal->nParent;
	pVal->nParent = nLeftPos;
	valLeft.nParent = nOldPar;
	pVal->nLeft = nTmp;
	if(nTmp > 0)
		WriteIntValue(nTmp + MEMBEROFFSET(struct GDBValue, nParent), nPos);
	FixHeight(pVal);
	if(!Write((char*)pVal, nPos, sizeof(struct GDBValue)))
		GAssert(false, "failed to write while rotating");
	FixHeight(&valLeft);
	if(!Write((char*)&valLeft, nLeftPos, sizeof(struct GDBValue)))
		GAssert(false, "failed to write while rotating");

	return nLeftPos;
}

int GDataBase::Balance(int nPos, GDBValue* pVal)
{
	int nLeft = 0;
	int nRight = 0;
	GetChildrenHeights(pVal, &nLeft, &nRight);
	int nBal = nRight - nLeft;
	if(nBal > 1)
	{
		// Load the right child
		struct GDBValue valRight;
		if(!Read((char*)&valRight, pVal->nRight, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read while balancing");
			return 0;
		}

		// Rotate the right child right (if necessary)
		int nLeftHeight = 0;
		int nRightHeight = 0;
		GetChildrenHeights(&valRight, &nLeftHeight, &nRightHeight);
		if(nRightHeight - nLeftHeight < 0)
			pVal->nRight = RotateRight(pVal->nRight, &valRight);

		// Rotate left
		return RotateLeft(nPos, pVal);
	}
	if(nBal < -1)
	{
		// Load the right child
		struct GDBValue valLeft;
		if(!Read((char*)&valLeft, pVal->nLeft, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read while balancing");
			return 0;
		}

		// Rotate the right child right (if necessary)
		int nLeftHeight = 0;
		int nRightHeight = 0;
		GetChildrenHeights(&valLeft, &nLeftHeight, &nRightHeight);
		if(nRightHeight - nLeftHeight > 0)
			pVal->nLeft = RotateLeft(pVal->nLeft, &valLeft);

		// Rotate left
		return RotateRight(nPos, pVal);
	}

	// This method guarantees to write out the value, so do it now
	if(!Write((const char*)pVal, nPos, sizeof(struct GDBValue)))
	{
		GAssert(false, "failed to write while balancing");
		return 0;
	}

	return nPos;
}

bool GDataBase::InsertValue(int nHeadPos, int nValuePos)
{
	// Read in the position of the head value
	int nHeadValuePos;
	if(!Read((char*)&nHeadValuePos, nHeadPos, sizeof(int)))
		return false;

	// Read the value
	struct GDBValue valThat;
	if(!Read((char*)&valThat, nValuePos, sizeof(struct GDBValue)))
	{
		GAssert(false, "failed to read");
		return 0;
	}

	// Insert the value
	if(nHeadValuePos <= 0)
	{
		GAssert(nHeadValuePos == 0, "Bad value");
		nHeadValuePos = nValuePos;

		// Update this value
		valThat.nHeight = 1;
		valThat.nLeft = 0;
		valThat.nRight = 0;
		valThat.nParent = 0;

		// Write this value
		if(!Write((char*)&valThat, nValuePos, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read");
			return 0;
		}
	}
	else
	{
		char sThat[GDB_VALUE_SORTING_PART_SIZE];
		int nThatSize = MIN(valThat.nValueSize, GDB_VALUE_SORTING_PART_SIZE);
		if(!Read(sThat, nValuePos + sizeof(struct GDBValue), nThatSize))
		{
			GAssert(false, "failed to read");
			return 0;
		}

		nHeadValuePos = Insert(nHeadValuePos, nValuePos, &valThat, nThatSize, sThat);
	}

	// Write the head position
	if(nHeadValuePos <= 0)
	{
		GAssert(false, "failed to insert value");
		return false;
	}
	if(!Write((char*)&nHeadValuePos, nHeadPos, sizeof(int)))
		return false;

	return true;
}

// "This" is the node already in the tree, and
// "That" is the node you want to insert into the tree
int GDataBase::Insert(int nThisPos, int nThatPos, GDBValue* pValThat, int nThatSize, char* sThat)
{
	// Check that value
	GAssert(pValThat->nHeight == 1 &&
			pValThat->nLeft == 0 &&
			pValThat->nRight == 0 &&
			pValThat->nParent == 0, "Value to insert not prepared properly");

	// Read this value
	struct GDBValue valThis;
	if(!Read((char*)&valThis, nThisPos, sizeof(struct GDBValue)))
	{
		GAssert(false, "failed to read");
		return 0;
	}
	char sThis[GDB_VALUE_SORTING_PART_SIZE];
	int nThisSize = MIN(valThis.nValueSize, GDB_VALUE_SORTING_PART_SIZE);
	if(!Read(sThis, nThisPos + sizeof(struct GDBValue), nThisSize))
	{
		GAssert(false, "failed to read");
		return 0;
	}

	// Compare the values
	int nCmp = CompareValues(sThis, nThisSize, sThat, nThatSize);

	// Insert the node
	int nRetVal = 0;
	if(nCmp > 0)
	{
		if(valThis.nLeft > 0)
		{
			valThis.nLeft = Insert(valThis.nLeft, nThatPos, pValThat, nThatSize, sThat);
			FixHeight(&valThis);
			nRetVal = Balance(nThisPos, &valThis);
		}
		else
		{
			valThis.nLeft = nThatPos;
			valThis.nHeight = 1;
			nRetVal = nThisPos;
			WriteIntValue(nThatPos + MEMBEROFFSET(struct GDBValue, nParent), nThisPos);
			if(!Write((char*)&valThis, nThisPos, sizeof(struct GDBValue)))
			{
				GAssert(false, "failed to write");
				return 0;
			}
		}
	}
	else
	{
		if(valThis.nRight > 0)
		{
			valThis.nRight = Insert(valThis.nRight, nThatPos, pValThat, nThatSize, sThat);
			FixHeight(&valThis);
			nRetVal = Balance(nThisPos, &valThis);
		}
		else
		{
			valThis.nRight = nThatPos;
			valThis.nHeight = 1;
			nRetVal = nThisPos;
			WriteIntValue(nThatPos + MEMBEROFFSET(struct GDBValue, nParent), nThisPos);
			if(!Write((char*)&valThis, nThisPos, sizeof(struct GDBValue)))
			{
				GAssert(false, "failed to write");
				return 0;
			}
		}
	}
	return nRetVal;
}

// This guarantees to find the left-most (or right-most if bRightMost is true) value
// that matches sVal.  If there is no matching value, it will return 0.  If bClosest
// is true, it will return the closest value (the value just less than if bRightMost
// is false, and just greater than if bRightMost is true), or 0 if there isn't one.
int GDataBase::Find(const char* sVal, int nValSize, bool bRightMost, bool bClosest, int nNodePos)
{
	if(nNodePos <= 0)
	{
		GAssert(nNodePos == 0, "Bad value");
		return 0;
	}
	struct GDBValue valThis;
	char sThis[GDB_VALUE_SORTING_PART_SIZE];
	while(true)
	{
		// Read this value
		if(!Read((char*)&valThis, nNodePos, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read");
			return 0;
		}
		int nThisSize = MIN(valThis.nValueSize, GDB_VALUE_SORTING_PART_SIZE);
		if(!Read(sThis, nNodePos + sizeof(struct GDBValue), nThisSize))
		{
			GAssert(false, "failed to read");
			return 0;
		}

		// Compare
		int nCmp = CompareValues(sThis, nThisSize, sVal, nValSize);

		if(nCmp < 0)
		{
			if(valThis.nRight > 0)
				nNodePos = valThis.nRight;
			else
			{
				if(bClosest)
				{
					if(bRightMost)
						return GetNextRight(nNodePos, &valThis);
					else
						return nNodePos;
				}
				else
					return 0;
			}
		}
		else if(nCmp > 0)
		{
			if(valThis.nLeft > 0)
				nNodePos = valThis.nLeft;
			else
			{
				if(bClosest)
				{
					if(bRightMost)
						return nNodePos;
					else
						return GetNextLeft(nNodePos, &valThis);
				}
				else
					return 0;
			}
		}
		else
		{
			if(bRightMost)
			{
				if(valThis.nRight > 0)
				{
					int nTmp = Find(sVal, nValSize, bRightMost, false, valThis.nRight);
					if(nTmp > 0)
						return nTmp;
				}
			}
			else
			{
				if(valThis.nLeft > 0)
				{
					int nTmp = Find(sVal, nValSize, bRightMost, false, valThis.nLeft);
					if(nTmp > 0)
						return nTmp;
				}
			}
			return nNodePos;
		}
	}
}

void GDataBase::UnlinkValue(int nValPos, struct GDBValue* pVal, int nHeadPos)
{
	int nReplacement = GetReplacementValue(pVal);
	WriteIntValue(nReplacement + MEMBEROFFSET(struct GDBValue, nParent), pVal->nParent);
	GDBValue valPar;
	int nParPos = pVal->nParent;
	while(nParPos > 0)
	{
		if(!Read((char*)&valPar, nParPos, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read");
			return;
		}
		if(valPar.nLeft == nValPos)
			valPar.nLeft = nReplacement;
		else
		{
			GAssert(valPar.nRight == nValPos, "parent-child mismatch");
			valPar.nRight = nReplacement;
		}
		nValPos = nParPos;
		nParPos = valPar.nParent;
		nReplacement = Balance(nValPos, &valPar);
	}
	WriteIntValue(nHeadPos, nReplacement);
}

int GDataBase::GetReplacementValue(struct GDBValue* pVal)
{
	// If this value doesn't have a left or right child, it's easy
	if(pVal->nRight <= 0)
		return pVal->nLeft;
	if(pVal->nLeft <= 0)
		return pVal->nRight;

	// Load the left child (because we're going to use the left-child's
	// right-most descendant to fill the vacant spot it leaves when it
	// gets unlinked.)
	GDBValue valLeft;
	if(!Read((char*)&valLeft, pVal->nLeft, sizeof(struct GDBValue)))
	{
		GAssert(false, "failed to read");
		return 0;
	}

	// See if the left child has a right child
	if(valLeft.nRight > 0)
	{
		// Find right-most child of my left child to be my replacement
		int nReplacement;
		pVal->nLeft = UnlinkRightMost(pVal->nLeft, &valLeft, &nReplacement);

		// Load the replacement value
		GDBValue valReplacement;
		if(!Read((char*)&valReplacement, nReplacement, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read");
			return 0;
		}

		// Insert the replacement value
		valReplacement.nLeft = pVal->nLeft;
		valReplacement.nRight = pVal->nRight;
		FixHeight(&valReplacement);
		return Balance(nReplacement, &valReplacement);
	}
	else
	{
		// Replace me with its left child
		valLeft.nRight = pVal->nRight;
		FixHeight(&valLeft);
		return Balance(pVal->nLeft, &valLeft);
	}
}

int GDataBase::UnlinkRightMost(int nVal, struct GDBValue* pVal, int* pnOutThat)
{
	if(pVal->nRight > 0)
	{
		// Load the right child
		struct GDBValue valRight;
		if(!Read((char*)&valRight, pVal->nRight, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read while unlinking");
			return 0;
		}

		// Recurse
		pVal->nRight = UnlinkRightMost(pVal->nRight, &valRight, pnOutThat);
		FixHeight(pVal);
		return Balance(nVal, pVal);
	}
	else
	{
		*pnOutThat = nVal;
		return pVal->nLeft;
	}
}

int GDataBase::GetNextRight(int nPos, struct GDBValue* pVal)
{
	if(pVal->nRight > 0)
		return GetLeftOrRightMost(pVal->nRight, false);
	while(pVal->nParent > 0)
	{
		int nParPos = pVal->nParent;
		if(!Read((char*)pVal, nParPos, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read");
			return 0;
		}
		if(pVal->nLeft == nPos)
			return nParPos;
		GAssert(pVal->nRight == nPos, "parent-child mismatch");
		nPos = nParPos;
	}
	return 0;
}

int GDataBase::GetNextLeft(int nPos, struct GDBValue* pVal)
{
	if(pVal->nLeft > 0)
		return GetLeftOrRightMost(pVal->nLeft, true);
	while(pVal->nParent > 0)
	{
		int nParPos = pVal->nParent;
		if(!Read((char*)pVal, nParPos, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read");
			return 0;
		}
		if(pVal->nRight == nPos)
			return nParPos;
		GAssert(pVal->nLeft == nPos, "parent-child mismatch");
		nPos = nParPos;
	}
	return 0;
}

int GDataBase::GetLeftOrRightMost(int nValPos, bool bRight)
{
	if(nValPos <= 0)
	{
		GAssert(nValPos == 0, "bad value");
		return 0;
	}
	struct GDBValue valThis;
	int nNextVal;
	while(true)
	{
		if(!Read((char*)&valThis, nValPos, sizeof(struct GDBValue)))
		{
			GAssert(false, "failed to read");
			return 0;
		}
		nNextVal = bRight ? valThis.nRight : valThis.nLeft;
		if(nNextVal > 0)
			nValPos = nNextVal;
		else
			return nValPos;
	}
}

int GDataBase::ValToInt(const char* pBuf)
{
	char szTmp[32];
	int nSize = (int)*pBuf;
	memcpy(szTmp, pBuf + 1, nSize);
	szTmp[nSize] = '\0';
	return atoi(szTmp);
}

int GDataBase::IntToVal(int nVal, char* pBuf)
{
	itoa(nVal, pBuf + 1, 10);
	int nSize = strlen(pBuf + 1);
	pBuf[0] = (char)nSize;
	return nSize + 1;
}

