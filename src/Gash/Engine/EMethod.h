/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __EMETHOD_H__
#define __EMETHOD_H__

class EMethodSignature;
class EInstrArray;
class GXMLTag;
class GHashTable;
class Library;
class COProject;
class COMethod;
class EClass;


class EMethod
{
friend class Library;
protected:
	EMethodSignature* m_pSignature;
	EInstrArray* m_pEInstrArray;
	EClass* m_pClass;
	GXMLTag* m_pMethodTag;
	const char* m_szName;

public:
	EMethod();
	~EMethod();

	EInstrArray* GetEInstrArray();
	int CountParams();
	const char* FindParamName(int n);
	EMethodSignature* GetSignature();
	EClass* GetClass() { return m_pClass; }
	GXMLTag* GetTag() { return m_pMethodTag; }
	const char* GetName() { return m_szName; }

	// This extracts the ID from the XML file
	int GetID();

	// This adds itself and the ID of all dependencies (deep) to the hash table
	void GetDependentMethods(GHashTable* pMethodTable, GHashTable* pTypeTable, Library* pLibrary, COProject* pProject);

	COMethod* GetCOMethod(COProject* pProject);
};


#endif // __EMETHOD_H__

