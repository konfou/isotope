/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "TagNames.h"
#include "EInstrArray.h"
#include "InstrTable.h"
#include "InstrSet.h"
#include "Disassembler.h"
#include "EMethod.h"
#include "../Include/GashEngine.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GBitTable.h"
#include "../CodeObjects/Project.h"
#include "../CodeObjects/Method.h"
#include "../CodeObjects/Class.h"
#include "EInterface.h"
#include "EClass.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

// Takes ownership of the XML tree
Library::Library(GXMLTag* pLibraryTag, COProject* pProject, bool bTakeProjectOwnership)
{
	m_pProject = pProject;
	m_bOwnProject = bTakeProjectOwnership;
	m_pObject = NULL;
	m_pInteger = NULL;
	m_pStackLayer = NULL;
	m_pLibraryTag = pLibraryTag;
	m_pTypeTableByName = new GConstStringHashTable(23, true); // 23 is a totally arbitrary magic value
	m_pTypeTableByPointer = new GHashTable(23); // 23 is a totally arbitrary magic value
#ifdef _DEBUG
	m_nLoadPhase = 0;
#endif // _DEBUG
}

Library::~Library()
{
	if(m_bOwnProject)
		delete(m_pProject);
	delete(m_pLibraryTag);
	delete [] m_pMethods;

	// Delete the types
	int n;
	for(n = 0; n < m_nTypeCount; n++)
		delete(m_pTypes[n]);
	delete [] m_pTypes;

	delete(m_pStrings);
	delete(m_pTypeTableByName);
	delete(m_pTypeTableByPointer);
}

bool Library::Init()
{
	// Quick-index the Methods, CallBacks, and Strings in the library
	GXMLTag* pStringTableTag;
	if(!CountTypes(&pStringTableTag))
		return false;
	m_pTypes = new EType*[m_nTypeCount];
	memset(m_pTypes, '\0', m_nTypeCount * sizeof(EType*));
	if(!LoadTypesAndCountMethods())
		return false;
	m_pMethods = new EMethod[m_nMethodCount];
	if(!LoadMethods())
		return false;
	if(!PolishTypes())
		return false;
	if(!LoadStrings(pStringTableTag))
		return false;
	return true;
}

/*static*/ Library* Library::LoadFromFile(const char* szFilename, COProject* pProject, bool bTakeProjectOwnership)
{
	const char* szErrorMessage;
	int nErrorOffset;
	int nErrorLine;
	int nErrorColumn;
	GXMLTag* pRootTag = GXMLTag::FromFile(szFilename, &szErrorMessage, &nErrorOffset, &nErrorLine, &nErrorColumn);
	if(!pRootTag)
		return NULL;
	Library* pLibrary = CreateFromXML(pRootTag, pProject, bTakeProjectOwnership); // takes ownership of pRootTag
	return pLibrary;
}

/*static*/ Library* Library::LoadFromBuffer(const char* pBuffer, int nBufferSize, COProject* pProject, bool bTakeProjectOwnership)
{
	const char* szErrorMessage;
	int nErrorOffset;
	int nErrorLine;
	int nErrorColumn;
	GXMLTag* pRootTag = GXMLTag::FromString(pBuffer, nBufferSize, &szErrorMessage, &nErrorOffset, &nErrorLine, &nErrorColumn);
	if(!pRootTag)
		return NULL;
	Library* pLibrary = CreateFromXML(pRootTag, pProject, bTakeProjectOwnership); // takes ownership of pRootTag
	return pLibrary;
}

/*static*/ Library* Library::CreateFromXML(GXMLTag* pLibraryTag, COProject* pProject, bool bTakeProjectOwnership)
{
	Holder<Library*> hLibrary(new Library(pLibraryTag, pProject, bTakeProjectOwnership));
	if(!hLibrary.Get()->Init())
	{
		GAssert(false, "failed to init library!");
		return NULL;
	}
	return hLibrary.Drop();
}

bool Library::CountTypes(GXMLTag** ppStringTableTag)
{
	GAssert(m_nLoadPhase++ == 0, "Unexpected value for LoadPhase");

	// Count the types and methods
	m_nTypeCount = 1;
	*ppStringTableTag = NULL;
	GXMLTag* pTag;
	for(pTag = m_pLibraryTag->GetFirstChildTag(); pTag; pTag = m_pLibraryTag->GetNextChildTag(pTag))
	{
		bool bType = false;
		bool bClass = false;
		if(stricmp(pTag->GetName(), TAG_NAME_CLASS) == 0)
		{
			bType = true;
			bClass = true;
		}
		else if(stricmp(pTag->GetName(), TAG_NAME_INTERFACE) == 0 || stricmp(pTag->GetName(), TAG_NAME_MACHINE) == 0)
			bType = true;
		if(bType)
		{
			// Check the ID
			GXMLAttribute* pIDAttr = pTag->GetAttribute(ATTR_ID);
			if(!pIDAttr)
			{
				GAssert(false, "Type has no ID attribute"); // todo: make a formal error
				return false;
			}
			if(atoi(pIDAttr->GetValue()) != m_nTypeCount)
			{
				GAssert(false, "Type ID out of order"); // todo: make a formal error
				return false;
			}
			m_nTypeCount++;
		}
		else if(stricmp(pTag->GetName(), TAG_NAME_STRINGTABLE) == 0)
		{
			if(*ppStringTableTag != NULL)
			{
				GAssert(false, "only one string table allowed");
				return false;
			}
			*ppStringTableTag = pTag;
		}
		else
		{
			GAssert(false, "unrecognized tag in library");
			return false;
		}
	}
	return true;
}

bool Library::LoadTypesAndCountMethods()
{
	GAssert(m_nLoadPhase++ == 1, "Unexpected value for LoadPhase");
	m_nMethodCount = 0;
	GXMLTag* pTag;
	for(pTag = m_pLibraryTag->GetFirstChildTag(); pTag; pTag = m_pLibraryTag->GetNextChildTag(pTag))
	{
		EType::TypeType eType;
		if(stricmp(pTag->GetName(), TAG_NAME_CLASS) == 0)
			eType = EType::TT_CLASS;
		else if(stricmp(pTag->GetName(), TAG_NAME_INTERFACE) == 0)
			eType = EType::TT_INTERFACE;
		else if(stricmp(pTag->GetName(), TAG_NAME_MACHINE) == 0)
			eType = EType::TT_MACHINE;
		else
			continue;

		// Get the ID
		GXMLAttribute* pIDTag = pTag->GetAttribute(ATTR_ID);
		int nID = atoi(pIDTag->GetValue());

		// Get the Name
		GXMLAttribute* pNameAttr = pTag->GetAttribute(ATTR_NAME);
		GAssert(pNameAttr, "todo: handle this case");
		GAssert(nID != 0 || stricmp(pNameAttr->GetValue(), "Object") == 0, "Object should have ID 0--todo: handle this case");
		m_pTypeTableByName->Add(pNameAttr->GetValue(), (void*)nID);

		// Instantiate the object
		if(eType == EType::TT_CLASS)
		{
			// Get Parent ID
			GXMLAttribute* pParentIDTag = pTag->GetAttribute(ATTR_PARENTID);
			int nParentID = 0;
			if(pParentIDTag)
				nParentID = atoi(pParentIDTag->GetValue());

			// Instantiate the class object
			m_pTypes[nID] = new EClass(pTag, nID, nParentID);

			// Count Methods
			GXMLTag* pMethodTag;
			for(pMethodTag = pTag->GetFirstChildTag(); pMethodTag; pMethodTag = pTag->GetNextChildTag(pMethodTag))
			{
				if(stricmp(pMethodTag->GetName(), TAG_NAME_METHOD) != 0 && stricmp(pMethodTag->GetName(), TAG_NAME_PROCEDURE) != 0)
					continue;
				GXMLAttribute* pIDAttr = pMethodTag->GetAttribute(ATTR_ID);
				if(!pIDAttr)
				{
					GAssert(false, "Method has no ID"); // todo: make a formal error
					return false;
				}
				if(atoi(pIDAttr->GetValue()) != m_nMethodCount)
				{
					GAssert(false, "Method ID out of order"); // todo: make a formal error
					return false;
				}
				m_nMethodCount++;
			}
		}
		else if(eType == EType::TT_INTERFACE)
		{
			m_pTypes[nID] = new EInterface(pTag, nID);
		}
		else
		{
			GAssert(eType == EType::TT_MACHINE, "unexpected type");
			m_pTypes[nID] = new EMachineClass(pTag, nID);
		}
	}

	// Find the built in types
	int nObjectID;
	if(!m_pTypeTableByName->Get(CLASS_NAME_OBJECT, (void**)&nObjectID))
	{
		GAssert(false, "todo: handle this case");
		return false;
	}
	m_pObject = (EClass*)GetEType(nObjectID);
	int nIntegerID;
	if(!m_pTypeTableByName->Get(CLASS_NAME_INTEGER, (void**)&nIntegerID))
	{
		GAssert(false, "todo: handle this case");
		return false;
	}
	m_pInteger = (EClass*)GetEType(nIntegerID);
	int nStackLayerID;
	if(!m_pTypeTableByName->Get(CLASS_NAME_STACKLAYER, (void**)&nStackLayerID))
	{
		GAssert(false, "todo: handle this case");
		return false;
	}
	m_pStackLayer = (EClass*)GetEType(nStackLayerID);
	return true;
}

bool Library::LoadMethods()
{
	GAssert(m_nLoadPhase++ == 2, "Unexpected value for LoadPhase");
	int nMethodCount = 0;
	GXMLTag* pTag;
	for(pTag = m_pLibraryTag->GetFirstChildTag(); pTag; pTag = m_pLibraryTag->GetNextChildTag(pTag))
	{
		if(stricmp(pTag->GetName(), TAG_NAME_CLASS) != 0)
			continue;

		// Find the class
		GXMLAttribute* pIDTag = pTag->GetAttribute(ATTR_ID);
		int nID = atoi(pIDTag->GetValue());
		EClass* pClass = (EClass*)m_pTypes[nID];

		// Create the methods
		int nFirstMethod = nMethodCount;
		GXMLTag* pChildTag;
		for(pChildTag = pTag->GetFirstChildTag(); pChildTag; pChildTag = pTag->GetNextChildTag(pChildTag))
		{
			if(stricmp(pChildTag->GetName(), TAG_NAME_METHOD) == 0 ||
				stricmp(pChildTag->GetName(), TAG_NAME_PROCEDURE) == 0)
			{
				m_pMethods[nMethodCount].m_pMethodTag = pChildTag;
				m_pMethods[nMethodCount].m_pClass = pClass;
				GAssert(atoi(pChildTag->GetAttribute(ATTR_ID)->GetValue()) == nMethodCount, "Wrong ID"); // Only Asserting because this was already checked in the first pass
				GXMLAttribute* pNameAttr = pChildTag->GetAttribute(ATTR_NAME);
				if(pNameAttr)
					m_pMethods[nMethodCount].m_szName = pNameAttr->GetValue();
				nMethodCount++;
			}
		}

		// Set the class method counts
		pClass->m_nFirstMethodID = nFirstMethod;
		pClass->m_nMethodCount = nMethodCount - nFirstMethod;
	}
	return true;
}

bool Library::PolishTypes()
{
	GAssert(m_nLoadPhase++ == 3, "Unexpected value for LoadPhase");

	int n;
	for(n = 1; n < m_nTypeCount; n++)
	{
		EType* pEType = GetEType(n);
		if(pEType && pEType->GetTypeType() == EType::TT_CLASS)
		{
			EClass* pEClass = (EClass*)pEType;

			// todo: Make sure the parent type is a valid class

			// Count all the members in the class
			pEClass->CountTotalMembers(this);

			// Set Integer flag if this class inherits from Integer
			EClass* pPar = (EClass*)pEType;
			while(pPar->m_nParentClassID != 0)
			{
				pPar = (EClass*)GetEType(pPar->m_nParentClassID);
				if(pPar->IsIntegerType())
				{
					pEClass->SetIntegerType();
					break;
				}
			}

			// Create the virtual table
			if(!pEClass->CreateVirtualTable(this))
				return false;
		}
	}

	return true;
}

bool Library::LoadStrings(GXMLTag* pStringTableTag)
{
	if(pStringTableTag)
	{
		m_nStringCount = pStringTableTag->GetChildTagCount();
		m_pStrings = new const char*[m_nStringCount];
		GXMLTag* pStringTag;
		GXMLAttribute* pVal;
		int n = 0;
		for(pStringTag = pStringTableTag->GetFirstChildTag(); pStringTag && n < m_nStringCount; pStringTag = pStringTableTag->GetNextChildTag(pStringTag))
		{
			pVal = pStringTag->GetAttribute(ATTR_VAL);
			if(!pVal)
			{
				GAssert(false, "String has no VAL attr");
				return false;
			}
			m_pStrings[n] = pVal->GetValue();
			n++;
		}
	}
	else
	{
		m_nStringCount = 0;
		m_pStrings = NULL;
	}
	return true;
}

EMethod* Library::GetEMethod(int nID)
{
	GAssert(nID >= 0 && nID < m_nMethodCount, "Out of range");
	return &m_pMethods[nID];
}

EType* Library::GetEType(int nID)
{
	GAssert(nID >= 0 && nID < m_nTypeCount, "Out of range");
	return m_pTypes[nID];
}

bool Library::FindMethod(struct MethodRef* pOutMethodRef, COMethod* pMethod)
{
	EClass* pClass = (EClass*)FindType(pMethod->GetClass()->GetName());
	if(!pClass)
		return false;
	return FindMethod(pOutMethodRef, pClass, pMethod->GetSignature());
}

bool Library::FindMethod(struct MethodRef* pOutMethodRef, EClass* pClass, EMethodSignature* pSignature)
{
	GAssert(((EType*)pClass)->GetTypeType() == EType::TT_CLASS, "not really a class type");

	// Search non-virtual methods
	EMethod* pEMethod;
	int n;
	int nMethodID;
	EClass* pCurrentClass = pClass;
	while(pCurrentClass != NULL)
	{
		for(n = 0; n < pCurrentClass->m_nMethodCount; n++)
		{
			nMethodID = pCurrentClass->m_nFirstMethodID + n;
			pEMethod = GetEMethod(nMethodID);
			EMethodSignature* pMethodSig = pEMethod->GetSignature();
			if(pSignature->Compare(pMethodSig) == 0)
			{
				pOutMethodRef->bVirtual = false;
				pOutMethodRef->nIndex = nMethodID;
				return true;
			}
		}
		pCurrentClass = (EClass*)GetEType(pCurrentClass->GetParentID());
	}

	// Search virtual methods
	int* pVirtualTable = pClass->GetVirtualTable();
	if(pVirtualTable)
	{
		int nPos = 0;
		while(nPos >= 0)
		{
			EInterface* pInterface = (EInterface*)GetEType(pVirtualTable[nPos]);
			GAssert(((EType*)pInterface)->GetTypeType() == EType::TT_INTERFACE, "not an interface type");
			int nMethodDeclCount = pInterface->GetMethodDeclCount();
			int n;
			for(n = 0; n < nMethodDeclCount; n++)
			{
				EMethodSignature sig(pInterface->GetChildTag(n)); // todo: this is unnecessarily expensive.  Cache an EMethodDecl object instead.
				if(sig.Compare(pSignature) == 0)
				{
					pOutMethodRef->bVirtual = true;
					pOutMethodRef->nIndex = nPos + 2 + n;
					return true;
				}
			}
			nPos = pVirtualTable[nPos + 1];
		}
	}

	return false;
}

const char* Library::GetStringFromTable(int nID)
{
	GAssert(nID >= 0 && nID < m_nStringCount, "Out of range");
	return m_pStrings[nID];
}

bool Library::FindMainProc(MethodRef* pMethodRef)
{
	EMethod* pEMethod;
	int n;
	for(n = 0; n < m_nMethodCount; n++)
	{
		pEMethod = &m_pMethods[n];
		if(pEMethod->CountParams() == 0 && pEMethod->m_szName && stricmp(pEMethod->m_szName, VAL_MAIN) == 0) // todo: support command-line args
		{
			pMethodRef->bVirtual = false;
			pMethodRef->nIndex = n;
			return true;
		}
	}
	return false;
}

EType* Library::FindTypeConst(const char* szTypeName)
{
	// First try the really fast way (look up by pointer)
	int nTypeID;
	if(m_pTypeTableByPointer->Get((void*)szTypeName, (void**)&nTypeID))
		return GetEType(nTypeID);

	// If we don't find it by string name, give up
	if(!m_pTypeTableByName->Get(szTypeName, (void**)&nTypeID))
		return NULL;

	// Add results by pointer so future queries won't have to do the string lookup
	if(m_pTypeTableByPointer->GetCount() < m_pTypeTableByName->GetCount() * 7) // Just to make sure this hash table doesn't get out of hand
		m_pTypeTableByPointer->Add((void*)szTypeName, (void*)nTypeID);

	return GetEType(nTypeID);
}

EType* Library::FindType(const char* szTypeName)
{
	int nTypeID;
	if(!m_pTypeTableByName->Get(szTypeName, (void**)&nTypeID))
		return NULL;
	return GetEType(nTypeID);
}

void AjustIDAndAddMapEntry(GXMLTag* pTag, int* pnCounter, int* map, int nMaxID)
{
	GXMLAttribute* pIDAttr = pTag->GetAttribute(ATTR_ID);
	GAssert(pIDAttr, "No ID tag");
	int nID = atoi(pIDAttr->GetValue());
	GAssert(nID >= 0 && nID < nMaxID, "ID out of range");
	GAssert(map[nID] == -1, "ID used more than once");
	map[nID] = *pnCounter;
	char szTmp[32];
	itoa(*pnCounter, szTmp, 10);
	pIDAttr->SetValue(szTmp);
	(*pnCounter)++;
}

/*static*/ void Library::CheckForReferencesToUnimportedTypes(GXMLTag* pLibraryTag, COProject* pCOProject)
{
	// Flag all the imported types
	int nMaxTypeID = pCOProject->GetUniqueTypeID();
	GBitTable bt(nMaxTypeID);
	GXMLAttribute* pAttrID;
	GXMLTag* pClassTag;
	for(pClassTag = pLibraryTag->GetFirstChildTag(); pClassTag; pClassTag = pLibraryTag->GetNextChildTag(pClassTag))
	{
		pAttrID = pClassTag->GetAttribute("ID");
		GAssert(pAttrID, "Expected an ID tag");
		int id = atoi(pAttrID->GetValue());
		GAssert(id < nMaxTypeID, "ID out of range");
		bt.SetBit(id, true);
	}

	// Check that all the types referenced by methods have been imported
	GXMLTag* pMethodTag;
	int nOldID;
	for(pClassTag = pLibraryTag->GetFirstChildTag(); pClassTag; pClassTag = pLibraryTag->GetNextChildTag(pClassTag))
	{
		if(stricmp(pClassTag->GetName(), TAG_NAME_CLASS) != 0)
			continue;
		for(pMethodTag = pClassTag->GetFirstChildTag(); pMethodTag; pMethodTag = pClassTag->GetNextChildTag(pMethodTag))
		{
			if(stricmp(pMethodTag->GetName(), TAG_NAME_METHOD) != 0 && stricmp(pMethodTag->GetName(), TAG_NAME_PROCEDURE) != 0)
				continue;
			EInstrArray mb(pMethodTag, NULL);
			int nCount = mb.GetInstrCount();
			int n;
			for(n = 0; n < nCount; n++)
			{
				struct InstrBin* pInstr = mb.GetInstr(n);
				InstructionStruct* pInstrStruct = pInstr->GetInstrStruct();
				const char* szParamTypes = pInstrStruct->szParamTypes;
				int nParam;
				for(nParam = 0; szParamTypes[nParam] != '\0'; nParam++)
				{
					switch(szParamTypes[nParam])
					{
					case 't':
						nOldID = pInstr->GetParam(nParam);
						GAssert(nOldID >= 0 && nOldID < nMaxTypeID, "out of range");
						if(!bt.GetBit(nOldID))
						{
							pLibraryTag->ToFile("bug.xlib");
							GXMLAttribute* pAttrName = pMethodTag->GetAttribute("Name");
							const char* szMethodName = pAttrName->GetValue();
							COType* pType = pCOProject->FindType(nOldID);
							GAssert(false, "a method references a type that was never imported!");
						}
						break;
					}
				}
				GAssert(nParam == pInstrStruct->nParamCount, "inconsistency in Instr Table");
			}
		}
	}
}

/*static*/ bool Library::RenumberIDs(GXMLTag* pLibraryTag, COProject* pCOProject)
{
#ifdef _DEBUG
	CheckForReferencesToUnimportedTypes(pLibraryTag, pCOProject);
#endif // _DEBUG
	GXMLTag* pTypeTag;
	GXMLTag* pMethodTag;
	if(stricmp(pLibraryTag->GetName(), TAG_NAME_LIBRARY) != 0)
	{
		GAssert(false, "expected a library tag");
		return false;
	}

	// Change the IDs and make maps from old to new IDs
	int nMaxMethodID = pCOProject->GetUniqueMethodID();
	int nMaxTypeID = pCOProject->GetUniqueTypeID();
	int* pnMethodIDMap = new int[nMaxMethodID];
	int* pnTypeIDMap = new int[nMaxTypeID];
	memset(pnMethodIDMap, 0xff, nMaxMethodID * sizeof(int));
	memset(pnTypeIDMap, 0xff, nMaxTypeID * sizeof(int));
	int nMethodCount = 0;
	int nTypeCount = 1; // because zero is reserved for "Object".
	for(pTypeTag = pLibraryTag->GetFirstChildTag(); pTypeTag; pTypeTag = pLibraryTag->GetNextChildTag(pTypeTag))
	{
		if(stricmp(pTypeTag->GetName(), TAG_NAME_CLASS) == 0)
		{
			AjustIDAndAddMapEntry(pTypeTag, &nTypeCount, pnTypeIDMap, nMaxTypeID);
			for(pMethodTag = pTypeTag->GetFirstChildTag(); pMethodTag; pMethodTag = pTypeTag->GetNextChildTag(pMethodTag))
			{
				if(stricmp(pMethodTag->GetName(), TAG_NAME_METHOD) == 0 || stricmp(pMethodTag->GetName(), TAG_NAME_PROCEDURE) == 0)
					AjustIDAndAddMapEntry(pMethodTag, &nMethodCount, pnMethodIDMap, nMaxMethodID);
			}
		}
		else if(stricmp(pTypeTag->GetName(), TAG_NAME_INTERFACE) == 0)
			AjustIDAndAddMapEntry(pTypeTag, &nTypeCount, pnTypeIDMap, nMaxTypeID);
		else if(stricmp(pTypeTag->GetName(), TAG_NAME_MACHINE) == 0)
			AjustIDAndAddMapEntry(pTypeTag, &nTypeCount, pnTypeIDMap, nMaxTypeID);
	}

	// Fix up the code
	int nOldID;
	GXMLTag* pClassTag;
	for(pClassTag = pLibraryTag->GetFirstChildTag(); pClassTag; pClassTag = pLibraryTag->GetNextChildTag(pClassTag))
	{
		if(stricmp(pClassTag->GetName(), TAG_NAME_CLASS) == 0)
		{
			// Fix up the parent ID
			GXMLAttribute* pAttrParent = pClassTag->GetAttribute(ATTR_PARENTID);
			if(pAttrParent)
			{
				nOldID = atoi(pAttrParent->GetValue());
				if(nOldID > 0)
				{
					GAssert(nOldID < nMaxTypeID, "out of range");
					char szTmp[32];
					itoa(pnTypeIDMap[nOldID], szTmp, 10);
					pAttrParent->SetValue(szTmp);
				}
			}
			for(pMethodTag = pClassTag->GetFirstChildTag(); pMethodTag; pMethodTag = pClassTag->GetNextChildTag(pMethodTag))
			{
				// Fix up the methods
				if(stricmp(pMethodTag->GetName(), TAG_NAME_METHOD) == 0 || stricmp(pMethodTag->GetName(), TAG_NAME_PROCEDURE) == 0)
				{
					EInstrArray mb(pMethodTag, NULL);
					int nCount = mb.GetInstrCount();
					int n;
					for(n = 0; n < nCount; n++)
					{
						struct InstrBin* pInstr = mb.GetInstr(n);
						InstructionStruct* pInstrStruct = pInstr->GetInstrStruct();
						const char* szParamTypes = pInstrStruct->szParamTypes;
						int nParam;
						for(nParam = 0; szParamTypes[nParam] != '\0'; nParam++)
						{
							switch(szParamTypes[nParam])
							{
							case 'm':
								nOldID = pInstr->GetParam(nParam);
								GAssert(nOldID >= 0 && nOldID < nMaxMethodID, "out of range");
								GAssert(pnMethodIDMap[nOldID] != -1, "bad mapping");
								pInstr->SetParam(nParam, pnMethodIDMap[nOldID]);
								break;
							case 't':
								nOldID = pInstr->GetParam(nParam);
								GAssert(nOldID >= 0 && nOldID < nMaxTypeID, "out of range");
								GAssert(pnTypeIDMap[nOldID] != -1, "bad mapping");
								pInstr->SetParam(nParam, pnTypeIDMap[nOldID]);
								break;
							}
						}
						GAssert(nParam == pInstrStruct->nParamCount, "inconsistency in Instr Table");
					}
					mb.SetBinTags(pMethodTag);
				}
				else if(stricmp(pMethodTag->GetName(), TAG_NAME_INTERFACE) == 0)
				{
					GXMLAttribute* pAttrID = pMethodTag->GetAttribute(ATTR_ID);
					if(pAttrID)
					{
						nOldID = atoi(pAttrID->GetValue());
						if(nOldID > 0)
						{
							GAssert(nOldID < nMaxTypeID, "out of range");
							char szTmp[32];
							itoa(pnTypeIDMap[nOldID], szTmp, 10);
							pAttrID->SetValue(szTmp);
						}
					}
				}
			}
		}
	}

	delete [] pnMethodIDMap;
	delete [] pnTypeIDMap;
	return true;
}

bool Library::SaveAsCppString(const char* szFilename)
{
	Holder<char*> hDump(GetXmlAsCppString());
	if(!hDump.Get())
		return false;
	int nDumpSize = strlen(hDump.Get());
	char szFilePart[256];
	_splitpath(szFilename, NULL, NULL, szFilePart, NULL);
	FILE* pFile = fopen(szFilename, "w");
	if(!pFile)
		return false;
	fputs("// --------------------------------------------------------------\n", pFile);
	fputs("// |                                                            |\n", pFile);
	fputs("// |   This file was generated by Gash.  If you edit it, your   |\n", pFile);
	fputs("// |   changes will be lost when it is regenerated.  If you     |\n", pFile);
	fputs("// |   want to modify this file, you should make your changes   |\n", pFile);
	fputs("// |   to the origional Gash source and regenerate this file.   |\n", pFile);
	fputs("// |                                                            |\n", pFile);
	fputs("// --------------------------------------------------------------\n", pFile);
	fputs("\n#include <gash.h>\n\n", pFile);
	fputs("const char* g_xlib_string_", pFile);
	fputs(szFilePart, pFile);
	fputs(" = \"\\\n", pFile);

	// Dump the body
	fputs(hDump.Get(), pFile);
	
	// global Library object
	fputs("\";\n\nLibrary g_xlib_", pFile);
	fputs(szFilePart, pFile);
	fputs("(g_xlib_string_", pFile);
	fputs(szFilePart, pFile);
	fputs(", ", pFile);
	char szTmp[64];
	itoa(nDumpSize, szTmp, 10);
	fputs(szTmp, pFile);
	fputs(");\n", pFile);

	fclose(pFile);

	// Make the header file

	return true;
}

char* Library::GetXmlAsCppString()
{
	int nSize = DumpXmlTagToCppString(m_pLibraryTag, NULL, 0);
	char* pBuffer = new char[nSize + 1];
	int nSize2 = DumpXmlTagToCppString(m_pLibraryTag, pBuffer, 0);
	GAssert(nSize2 == nSize, "Different sizes");
	pBuffer[nSize] = '\0';
	return pBuffer;
}

inline void inline_append_to_buffer(char* pBuffer, int* pnPos, const char* szString)
{
    if(pBuffer)
        strcpy(&pBuffer[*pnPos], szString);
    *pnPos += strlen(szString);
}

#define add(s) inline_append_to_buffer(pBuffer, &nPos, s)

int Library::DumpXmlTagToCppString(GXMLTag* pTag, char* pBuffer, int nIndent)
{
	int nPos = 0;
	int n;
	for(n = 0; n < nIndent; n++)
		add("\t");
	add("<");
	add(pTag->GetName());
	GXMLAttribute* pAttr;
	for(pAttr = pTag->GetFirstAttribute(); pAttr; pAttr = pTag->GetNextAttribute(pAttr))
		nPos += DumpXmlAttrToCppString(pAttr, pBuffer ? &pBuffer[nPos] : NULL);
	int nChildCount = pTag->GetChildTagCount();
	if(nChildCount > 0)
	{
		add(">\\\n");
		GXMLTag* pChildTag;
		for(pChildTag = pTag->GetFirstChildTag(); pChildTag; pChildTag = pTag->GetNextChildTag(pChildTag))
			nPos += DumpXmlTagToCppString(pChildTag, pBuffer ? &pBuffer[nPos] : NULL, nIndent + 1);
		for(n = 0; n < nIndent; n++)
			add("\t");
		add("</");
		add(pTag->GetName());
		add(">\\\n");
	}
	else
		add(" />\\\n");
	return nPos;
}

int Library::DumpXmlAttrToCppString(GXMLAttribute* pAttr, char* pBuffer)
{
	int nPos = 0;
	add(" ");
	add(pAttr->GetName());
	add("=\\\"");
	add(pAttr->GetValue());
	add("\\\"");
	return nPos;
}

/*static*/ int Library::DumpMethodParameters(GXMLTag* pMethodTag, char* pBuffer)
{
    int nPos = 0;
    GXMLTag* pVarTag;
    bool bFirst = true;
    for(pVarTag = pMethodTag->GetFirstChildTag(); pVarTag; pVarTag = pMethodTag->GetNextChildTag(pVarTag))
    {
        if(stricmp(pVarTag->GetName(), TAG_NAME_VAR) != 0)
            continue;
        GXMLAttribute* pVarTypeAttr = pVarTag->GetAttribute(ATTR_TYPE);
        if(!pVarTypeAttr)
            continue;
        GXMLAttribute* pVarNameAttr = pVarTag->GetAttribute(ATTR_NAME);
        if(!pVarNameAttr)
            continue;
        if(bFirst)
            bFirst = false;
        else
            add(", ");
        add(pVarTypeAttr->GetValue());
        add(" ");
        add(pVarNameAttr->GetValue());
    }
	return nPos;
}

/*static*/ int Library::DumpXmlToHFile(GXMLTag* pLibrary, char* pBuffer, const char* szTitle)
{
    // Header file
    int nPos = 0;
    add("#ifndef __");
    add(szTitle);
    add("_h__\n");
    add("#define __");
    add(szTitle);
    add("_h__\n\n");

    // Add class declarations
    GXMLTag* pTag;
    for(pTag = pLibrary->GetFirstChildTag(); pTag; pTag = pLibrary->GetNextChildTag(pTag))
    {
        if(stricmp(pTag->GetName(), TAG_NAME_CLASS) != 0)
            continue;
        GXMLAttribute* pNameAttr = pTag->GetAttribute(ATTR_NAME);
        if(!pNameAttr)
            continue;
        add("class ");
        add(pNameAttr->GetValue());
        add(";\n");
    }
    add("\n");

    // Add class definitions
    for(pTag = pLibrary->GetFirstChildTag(); pTag; pTag = pLibrary->GetNextChildTag(pTag))
    {
        if(stricmp(pTag->GetName(), TAG_NAME_CLASS) != 0)
            continue;
        GXMLAttribute* pNameAttr = pTag->GetAttribute(ATTR_NAME);
        if(!pNameAttr)
            continue;
        add("class ");
        add(pNameAttr->GetValue());
        add("\n{\nprotected:\n");
        
        // Add members
        GXMLTag* pMemberTag;
        for(pMemberTag = pTag->GetFirstChildTag(); pMemberTag; pMemberTag = pTag->GetNextChildTag(pMemberTag))
        {
            if(stricmp(pMemberTag->GetName(), TAG_NAME_VAR) != 0)
                continue;
            GXMLAttribute* pVarTypeAttr = pMemberTag->GetAttribute(ATTR_TYPE);
            if(!pVarTypeAttr)
                continue;
            GXMLAttribute* pVarNameAttr = pMemberTag->GetAttribute(ATTR_NAME);
            if(!pVarNameAttr)
                continue;
            add("\t");
            add(pVarTypeAttr->GetValue());
            add(" ");
            add(pVarNameAttr->GetValue());
            add(";\n");
        }

        // Add procedures
        add("\npublic:\n");
        GXMLTag* pMethodTag;
        for(pMethodTag = pTag->GetFirstChildTag(); pMethodTag; pMethodTag = pTag->GetNextChildTag(pMethodTag))
        {
            if(stricmp(pMethodTag->GetName(), TAG_NAME_PROCEDURE) != 0)
                continue;
            GXMLAttribute* pProcNameAttr = pMethodTag->GetAttribute(ATTR_NAME);
            if(!pProcNameAttr)
                continue;
            add("\tstatic void ");
            add(pProcNameAttr->GetValue());
            add("(");
            nPos += DumpMethodParameters(pMethodTag, &pBuffer[nPos]);
            add(");\n");
        }
        add("\n");

        // Add methods
        for(pMethodTag = pTag->GetFirstChildTag(); pMethodTag; pMethodTag = pTag->GetNextChildTag(pMethodTag))
        {
            if(stricmp(pMethodTag->GetName(), TAG_NAME_METHOD) != 0)
                continue;
            GXMLAttribute* pMethNameAttr = pMethodTag->GetAttribute(ATTR_NAME);
            if(!pMethNameAttr)
                continue;
            add("\tvoid ");
            add(pMethNameAttr->GetValue());
            add("(");
            nPos += DumpMethodParameters(pMethodTag, &pBuffer[nPos]);
            add(");\n");
        }

    }

    add("#endif // __");
    add(szTitle);
    add("_h__\n");
	return nPos;
}

/*static*/ int Library::DumpXmlToCppFile(GXMLTag* pLibrary, char* pBuffer, const char* szTitle)
{
    int nPos = 0;
    add("#include \"");
    add(szTitle);
    add("\"\n\n");
    GXMLTag* pTag;
    for(pTag = pLibrary->GetFirstChildTag(); pTag; pTag = pLibrary->GetNextChildTag(pTag))
    {
        if(stricmp(pTag->GetName(), TAG_NAME_CLASS) != 0)
            continue;
        GXMLAttribute* pClassNameAttr = pTag->GetAttribute(ATTR_NAME);
        if(!pClassNameAttr)
            continue;

        // Add procedures
        GXMLTag* pMethodTag;
        for(pMethodTag = pTag->GetFirstChildTag(); pMethodTag; pMethodTag = pTag->GetNextChildTag(pMethodTag))
        {
            if(stricmp(pMethodTag->GetName(), TAG_NAME_PROCEDURE) != 0)
                continue;
            GXMLAttribute* pProcNameAttr = pMethodTag->GetAttribute(ATTR_NAME);
            if(!pProcNameAttr)
                continue;
            add("/*static*/ void ");
            add(pClassNameAttr->GetValue());
            add("::");
            add(pProcNameAttr->GetValue());
            add("(");
            nPos += DumpMethodParameters(pMethodTag, &pBuffer[nPos]);
            add(")\n{\n");
            add("// todo: dump the asm instructions here");
            add("}\n\n\n");
        }
        add("\n\n\n");
    }
	return nPos;
}

#undef add
