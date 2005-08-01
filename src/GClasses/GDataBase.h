/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GDATABASE_H__
#define __GDATABASE_H__

#include <stdio.h>

// When sorting values, data after this number of bytes will be ignored.
// (for example, if two very long strings have the same beginning for
//  this number of bytes, the values will compare equal.  This saves the
//  database from having to do ridiculously long comparisons which would
//  slow it down.)
#define GDB_VALUE_SORTING_PART_SIZE 128

#define MAX_VALUE_CAP 0x7fffffff

class GAVLTree;
class GDBChapter;
class GDBPage;
class GDataBase;
class GDBQueryEnumerator;

struct GDBRecordField
{
	int nRootPos; // only valid for table records
	int nRootValue; // only valid for table records
	int nValuePos;
	int nValueSize;
	int nValueSizeLoaded;
	char* pValue;
};


class GDBRecord
{
friend class GDataBase;
protected:
	int m_nRecordPos;
	int m_nFieldCount;
	struct GDBRecordField* m_pFields;
	bool m_bValidTable;

public:
	GDBRecord();
	virtual ~GDBRecord();

	void Clear();
	int GetFieldCount() { return m_nFieldCount; }
	void SetFieldCount(int nFieldCount);
	void SetField(int nField, const void* pValue, int nValueSize);
	int GetFieldSize(int nField) { return m_pFields[nField].nValueSize; }
	const char* GetField(int nField) { return m_pFields[nField].pValue; }

protected:
	int GetRecordSize();
	void SetRoot(int nField, int nRootPos, int nRootValue);
	int GetRootPos(int nField);
	int GetRootValue(int nField);
	void SetRecordPos(int nPos) { m_nRecordPos = nPos; }
	int GetRecordPos() { return m_nRecordPos; }
	bool IsValidTable() { return m_bValidTable; }
	void SetValidTable(bool bValid) { m_bValidTable = bValid; }

	// note that this will take ownership of pValue
	void SetField(int nField, int nValuePos, int nValueSize, int nValueSizeLoaded, void* pValue);
};


class GDataBase
{
friend class GDBQueryEnumerator;
protected:
	GAVLTree* m_pChapters;
	FILE* m_pFile;
	int m_nChaptersCreated;
	GDBChapter* m_pCurrentChapter;
	GDBPage* m_pMostRecentlyUsedPage;
	GDBPage* m_pLeastRecentlyUsedPage;
	int m_nPageCacheSize;
#ifdef _DEBUG
	int m_nMagic;
#endif // _DEBUG

	// The constructor is protected because you should use a public static
	// method to create or open a database.
	GDataBase(FILE* pFile);

public:
	enum QueryType
	{
		equal,
		less,
		greater,
		lessOrEqual,
		greaterOrEqual,
		betweenInclusive,
		betweenExclusive,
	};

	virtual ~GDataBase();

	// This will return true if it creates the database with the specified filename
	static bool Create(const char* szFilename);

	// This will return a pointer to a GDataBase object if it succeeds at
	// opening the database.  You must delete it when you're done (which will
	// close the database).  This will return NULL if it fails to open the
	// database.
	static GDataBase* Open(const char* szFilename);

	// Adds a new table to the database
	bool AddTable(GDBRecord* pRecord);

	// Gets a table from the database.  *Caution*: if you add a tuple to
	// a table, you must call GetTable again because the old one will
	// contain old information.  Even if you just want to add another
	// record, you still have to do it because the table contains a
	// pointer to the root of the AVL-tree, which might change.
	bool GetTable(GDBRecord* pOutTable, const void* pName, int nNameSize);

	// Adds a new tuple to the specified table
	bool AddTuple(GDBRecord* pRecord, GDBRecord* pTableRecord);

	// Finds the first exact match of a tuple by a singe field
	bool GetTuple(GDBRecord* pOutTuple, GDBRecord* pInTable, int nField, const void* pValue, int nValueSize, int nValueSizeCap = MAX_VALUE_CAP);

	// Finds a set of records
	void Query(GDBQueryEnumerator* pOutEnumerator, GDBRecord* pTableRecord, int nField, QueryType eQueryType, const void* pValue1, int nValue1Size, const void* pValue2 = NULL, int nValue2Size = 0, bool bLeftToRight = true, int nValueSizeCap = MAX_VALUE_CAP);

	// Delete a record
	bool DeleteRecord(GDBRecord* pTuple, GDBRecord* pTable);
	
	// Make sure everything has been written to disk
	void Flush();

	// Value conversion
	static int ValToInt(const char* pBuf);
	static int IntToVal(int nVal, char* pBuf);

#ifdef _DEBUG
	bool Check() { return (m_nMagic == 0x31636a54); }
#endif // _DEBUG
protected:
	// Database access
	bool AddRecord(GDBRecord* pRecord, GDBRecord* pTableRecord);
	int FindRecord(const char* szValue, int nValueSize, bool bRightMost, bool bClosest, int nRootValue, int* pnOutValPos);
	int GetNextValue(int nPos, GDBRecord* pTableRecord, int nField, bool bRight);
	int GetNextRecord(int* pnValPos, GDBRecord* pTableRecord, int nField, bool bRight);
	bool Read(char* pBuf, int nPos, int nSize);
	int FindTuple(GDBRecord* pTableRecord, int nField, const char* pValue, int nValueSize, int* pTrailBuffer, int nTrailBufferSize, int* pnTrailUsed, bool bRightMost, bool bClosest);
	bool ReadRecord(GDBRecord* pOutRecord, int nPos, int nValueSizeCap);

	// Paging system
	int FindDeletedRecordToReuse(int nSize);
	GDBPage* GetPage(int nPos);
	void FlushPage(GDBPage* pPage);
	bool Write(const char* pBuf, int nPos, int nSize);
	int ReadIntValue(int nPos);
	void WriteIntValue(int nPos, int nValue);

	// AVL Tree methods
	void GetChildrenHeights(struct GDBValue* pVal, int* pnLeft, int* pnRight);
	void FixHeight(struct GDBValue* pVal);
	int RotateLeft(int nPos, GDBValue* pVal);
	int RotateRight(int nPos, GDBValue* pVal);
	int Balance(int nPos, GDBValue* pVal); // Guarantees to write pVal to the database
	bool InsertValue(int nHeadPos, int nValuePos);
	int Insert(int nThisPos, int nThatPos, GDBValue* pValThat, int nThatSize, char* sThat);
	int Find(const char* sVal, int nValSize, bool bRightMost, bool bClosest, int nNodePos);
	void UnlinkValue(int nValPos, struct GDBValue* pVal, int nHeadPos);
	int GetReplacementValue(struct GDBValue* pVal);
	int UnlinkRightMost(int nVal, struct GDBValue* pVal, int* pnOutThat);
	int GetNextRight(int nPos, struct GDBValue* pVal);
	int GetNextLeft(int nPos, struct GDBValue* pVal);
	int GetLeftOrRightMost(int nValPos, bool bRight);
};


class GDBQueryEnumerator
{
friend class GDataBase;
protected:
	GDataBase* m_pDataBase;
	GDBRecord* m_pTable;
	int m_nField;
	int m_nCurrentValuePos;
	int m_nBufferedRecordPos;
	int m_nValueSizeCap;
	bool m_bLeftToRight;
	GDataBase::QueryType m_eQueryType;
	bool m_bNeedCompareValue;
	char m_sCompareValue[GDB_VALUE_SORTING_PART_SIZE];
	int m_nCompareValueSize;

public:
	GDBQueryEnumerator()
	{
		m_pDataBase = NULL;
		m_pTable = NULL;
		m_nField = 0;
		m_nCurrentValuePos = 0;
		m_nBufferedRecordPos = 0;
		m_bLeftToRight = true;
		m_nValueSizeCap = MAX_VALUE_CAP;
	}

	virtual ~GDBQueryEnumerator()
	{
	}

	bool GetNext(GDBRecord* pOutRecord);
};


class GDBObject
{
public:
	enum GDBType
	{
		integer = 0,
		string = 1,
		datetime = 2,
	};

	GDBObject() {}
	virtual ~GDBObject() {}

	virtual GDBType GetType() = 0;
	virtual int GetSerializedSize() = 0;
	virtual void GetSerialized(char* pBuf) = 0;
};


#endif // __GDATABASE_H__
