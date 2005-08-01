/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __ECLASS_H__
#define __ECLASS_H__

#include "EType.h"

class EInterface;

class EClass : public EType
{
friend class Library;
friend class EType;
protected:
	int m_nMemberCount;
	int m_nMemberCountExtendedOnly;
	int m_nParentClassID;
	int m_nFirstMethodID;
	int m_nMethodCount;
	unsigned int m_nFlags;
	int* m_pVirtualTable;

public:
	enum
	{
		CD_INTEGER_TYPE = 0x00000001,
	};

	EClass(GXMLTag* pTag, int nID, int nParentID);
	virtual ~EClass();

	// This method is called when you load the library.  You probably don't need to call it.
	bool CreateVirtualTable(Library* pLibrary);

	// The virtual table is an array of integers.  It can be thought of as containing a sub-table for each
	// implemented interface (including interfaces implemented by ancestor classes) where each sub-table
	// consists of the interface ID, followed by the index of the next sub-table, followed by a list of
	// method IDs for each MethodDecl in the interface.  The value -1 is used to terminate the "linked list"
	// of sub-tables.  Also, the value -1 is used for any abstract methods.  (An abstract methods is any
	// method specified in an implemented interface for which there is no actual method with a matching
	// signature.)  There is no concept of non-abstract virtual methods in this language, so there is no
	// way to overload a method that already exists in a parent class.  (This was an intentional design in
	// order to promote development of clean code, not something that would be difficult to make work.)
	int* GetVirtualTable() { return m_pVirtualTable; }

	virtual TypeType GetTypeType() { return TT_CLASS; }
	GXMLTag* GetMemberTag(int n, Library* pLibrary);
	inline bool IsIntegerType() { return (m_nFlags & CD_INTEGER_TYPE) != 0; }
	inline void SetIntegerType() { m_nFlags |= CD_INTEGER_TYPE; }
	bool DoesImplement(EInterface* pInterface);

	int GetTotalMemberCount();
	int GetExtendedMemberCount() { return m_nMemberCountExtendedOnly; }
	int CountTotalMembers(Library* pLibrary); // Used while loading the library
	int GetMethodCount() { return m_nMethodCount; }
	int GetParentID() { return m_nParentClassID; }
	int GetFirstMethodID() { return m_nFirstMethodID; }

protected:
	int GetInterfaces(EInterface** pInterfaces, Library* pLibrary);
};

#endif // __ECLASS_H__
