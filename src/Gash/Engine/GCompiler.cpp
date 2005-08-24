/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GCompiler.h"
#include "TagNames.h"
#include "EInstrArray.h"
#include "InstrTable.h"
#include "InstrSet.h"
#include "Error.h"
#include "EvalExprResult.h"
#include "EMethod.h"
#include "EClass.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN
#include "../Include/GashEngine.h"
#include "../CodeObjects/VarRef.h"
#include "../CodeObjects/InstrArray.h"
#include "../CodeObjects/Project.h"
#include "../CodeObjects/Class.h"
#include "../CodeObjects/Variable.h"
#include "../CodeObjects/File.h"
#include "../CodeObjects/FileSet.h"
#include "../CodeObjects/Call.h"
#include "../CodeObjects/Interface.h"
#include "../CodeObjects/Operator.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GHashTable.h"

#define STR_INTERNAL_VAR "<Internal>"
#define STR_LBP "<Local Base Pointer>"
#define STR_BP "<Base Pointer>"
#define STR_STACK_LAYER_VAR "<Stack layer Var>"
#define STR_IP "<Instruction Pointer>"
#define STR_UNIQUE_ID "___UID__"


GCompiler::GCompiler(COProject* pCOProject, CompileError* pErrorHolder)
{
	GAssert(pCOProject->IsSourceLoaded(), "No Source loaded--todo: handle this case better");
	m_pCOProject = pCOProject;
	m_pErrorHolder = pErrorHolder;
	m_nUniqueID = 0;
	m_pCurrentFile = NULL;

	m_pEInstrArrayBuilder = new EInstrArrayBuilder();
	m_nStackSizeAtCall = 0;
	m_pCompilerInternalClass = new COClass(0, 0, 0, STR_INTERNAL_VAR, m_pCOProject->m_pObject, m_pCOProject->m_pBuiltIn, NULL, pCOProject);
	m_pLocalBasePointer = new COVariable(0, 0, 0, STR_LBP, m_pCompilerInternalClass, true, true, false);
	m_pBasePointer = new COVariable(0, 0, 0, STR_BP, m_pCompilerInternalClass, true, true, false);
	m_pStackLayerVar = new COVariable(0, 0, 0, STR_STACK_LAYER_VAR, m_pCompilerInternalClass, true, true, false);
	m_pInstructionPointer = new COVariable(0, 0, 0, STR_IP, m_pCompilerInternalClass, true, true, false);
	m_pVariables = new GPointerArray(64);
	m_pVarOwned = new GIntArray(64);
	m_pCurrentClassTag = NULL;
	m_pLibraryTag = NULL;
	m_pStringTableTag = NULL;
	m_pExternallyCalledMethods = new GHashTable(37);
	m_pExternallyReferencedTypes = new GHashTable(23);
	m_pAlreadyImportedTypes = NULL;
	m_bSymbolMode = false;
}

GCompiler::~GCompiler()
{
	ClearStack();
	GAssert(m_pVariables->GetSize() == 0 && m_pVarOwned->GetSize() == 0, "Didn't clear the stack");
	delete(m_pVariables);
	delete(m_pVarOwned);
	delete(m_pInstructionPointer);
	delete(m_pBasePointer);
	delete(m_pStackLayerVar);
	delete(m_pLocalBasePointer);
	delete(m_pCompilerInternalClass);
	delete(m_pLibraryTag);
	delete(m_pStringTableTag);
	delete(m_pExternallyCalledMethods);
	delete(m_pExternallyReferencedTypes);
	delete(m_pAlreadyImportedTypes);
	delete(m_pEInstrArrayBuilder);
}

Library* GCompiler::Compile(bool bLibraryOwnsProject)
{
	bool bSuccess = CompileProject();
	if(bSuccess)
	{
		GXMLTag* pTmp = m_pLibraryTag;
		m_pLibraryTag = NULL;
		Library* pLibrary = Library::CreateFromXML(pTmp, m_pCOProject, bLibraryOwnsProject);
		if(!pLibrary)
		{
			m_pErrorHolder->SetError(&Error::INTERNAL_ERROR, NULL);
			return NULL;
		}
		return pLibrary;
	}
	else
		return NULL;
}

EInstrArray* GCompiler::PartialCompileMethod(COMethod* pMethod)
{
	m_bSymbolMode = true;
	return pMethod->Compile2(this);
}

void GCompiler::AddInstr(GVMInstrPointer pMeth, COInstruction* pInstruction)
{
	m_pEInstrArrayBuilder->AddInstr(pMeth, pInstruction);
}

void GCompiler::AddParam(int nParam)
{
	m_pEInstrArrayBuilder->AddParam(nParam);
}

void GCompiler::SetCurrentFile(COFile* pFile)
{
	m_pCurrentFile = pFile;
}

int GCompiler::GetCurrentOffset()
{
	return m_pEInstrArrayBuilder->GetSize();
}

void GCompiler::ClearStack()
{
	COVariable* pTmp;
	while(m_pVariables->GetSize() > 0)
		PopStackVariable(&pTmp, NULL);
	GAssert(m_pVarOwned->GetSize() == 0, "This should follow m_pVariables");
}

bool GCompiler::FindVarOnStack(int* pnOutOffset, COVariable* pVariable, COInstruction* pObjectWithError)
{
	if(m_bSymbolMode)
	{
		*pnOutOffset = 0;
		return true;
	}
	COVariable* pTmp;
	int n;
	int nCount = m_pVariables->GetSize();
	for(n = nCount - 1; n >= 0; n--) // Count backwards because newest vars on top of stack
	{
		pTmp = (COVariable*)m_pVariables->GetPointer(n);
		if(pTmp == pVariable)
		{
			*pnOutOffset = n - m_nBasePointer;
			return true;
		}
	}
	SetError(&Error::VARIABLE_NOT_FOUND, pObjectWithError, pVariable->GetName());
	return false;
}

bool GCompiler::IsVariableDefined(COVariable* pVariable)
{
	COVariable* pTmp;
	int n;
	int nCount = m_pVariables->GetSize();
	for(n = nCount - 1; n >= 0; n--) // Count backwards because newest vars on top of stack
	{
		pTmp = (COVariable*)m_pVariables->GetPointer(n);
		if(pTmp == pVariable)
			return true;
	}
	return false;
}

bool GCompiler::PushStackVariable(COVariable* pVariable, bool bTakeOwnership)
{
	if(pVariable->GetType() != m_pCompilerInternalClass)
	{
		if(IsVariableDefined(pVariable))
		{
			SetError(&Error::VARIABLE_ALREADY_DECLARED, pVariable, pVariable->GetName());
			return false;
		}
	}
	m_pVariables->AddPointer(pVariable);
	m_pVarOwned->AddInt(bTakeOwnership ? 1 : 0);
	return true;
}

// This should destroy the decl and return NULL if we have ownership of it
bool GCompiler::PopStackVariable(COVariable** ppVariable, CodeObject* pObjForError)
{
	int nSize = m_pVariables->GetSize();
	if(m_pVarOwned->GetSize() != nSize)
	{
		SetError(&Error::INTERNAL_ERROR, pObjForError);
		return false;
	}
	if(nSize < 1)
	{
		SetError(&Error::INTERNAL_ERROR, pObjForError);
		return false;
	}
	COVariable* pVariable = (COVariable*)m_pVariables->GetPointer(nSize - 1);
	bool bOwned = ((m_pVarOwned->GetInt(nSize - 1) == 0) ? false : true);
	m_pVariables->DeleteCell(nSize - 1);
	m_pVarOwned->DeleteCell(nSize - 1);
	if(bOwned)
	{
		delete(pVariable);
		*ppVariable = NULL;
	}
	else
		*ppVariable = pVariable;
	return true;
}

bool GCompiler::CompileBegin()
{
	// Init some stuff
	delete(m_pStringTableTag);
	m_pStringTableTag = NULL;
	delete(m_pLibraryTag);
	m_pLibraryTag = new GXMLTag(TAG_NAME_LIBRARY);
	const char* szFilename = m_pCOProject->GetFilename();
	if(szFilename != NULL && strlen(szFilename) > 0)
		m_pLibraryTag->AddAttribute(new GXMLAttribute(ATTR_SOURCE, szFilename));

	// Add built in classes to the library
	m_pLibraryTag->AddChildTag(m_pCOProject->m_pObject->ToXMLForLibrary(this));
	m_pLibraryTag->AddChildTag(m_pCOProject->m_pInteger->ToXMLForLibrary(this));
	m_pLibraryTag->AddChildTag(m_pCOProject->m_pStackLayer->ToXMLForLibrary(this));

	// Add references to internally-referenced methods
	COClass* pExceptionClass = m_pCOProject->FindClass("Exception");
	if(pExceptionClass)
	{
		EMethodSignature sig(SIG_EXCEPTION_NEW);
		COMethod* pMethodNew = pExceptionClass->FindMethod(&sig);
		if(pMethodNew)
			AddImportMethod(pMethodNew);
	}

	// Add references to internally-referenced types
	AddImportType(m_pCOProject->FindClass("NullReferenceException"));
	AddImportType(m_pCOProject->FindClass("EngineException"));
	AddImportType(m_pCOProject->FindClass("CastException"));
	AddImportType(m_pCOProject->FindClass("DeserializationException"));
	AddImportType(m_pCOProject->FindClass("IOException"));
	AddImportType(m_pCOProject->FindClass("SdlException"));
	AddImportType(m_pCOProject->FindClass("XmlException"));
	AddImportType(m_pCOProject->FindClass("CompileException"));

	return true;
}

bool GCompiler::CompileFinish()
{
	// Import the library methods that were called
	if(!DoImporting())
	{
		CheckError();
		return false;
	}

	if(!Library::RenumberIDs(m_pLibraryTag, m_pCOProject))
	{
		SetError(&Error::INTERNAL_ERROR, NULL);
		return false;
	}

	// Add the String Table
	if(m_pStringTableTag)
	{
		m_pLibraryTag->AddChildTag(m_pStringTableTag);
		m_pStringTableTag = NULL;
	}

	GAssert(!m_bSymbolMode, "Shouldn't be called in symbol mode");
	return true;
}

void GCompiler::CompileFailed()
{
	ClearStack();
	m_pEInstrArrayBuilder->Flush();
	delete(m_pLibraryTag);
	m_pLibraryTag = NULL;
	m_pCurrentClassTag = NULL;
	delete(m_pStringTableTag);
	m_pStringTableTag = NULL;
}

bool GCompiler::MakeSureNullIsDefined(COInstruction* pSymbolInstr)
{
	if(IsVariableDefined(m_pCOProject->m_pNull))
		return true;
	return AsmCmdDecl(m_pCOProject->m_pNull, false, pSymbolInstr);
}

void GCompiler::AddImportMethod(COMethodDecl* pMethod)
{
	if(m_bSymbolMode)
		return;
	COType* pType = pMethod->GetType();
	COFile* pFile = pType->GetFile();
	if(pFile->GetFileType() != FT_XLIB)
		return; // it's not an external method, so we don't need to import it
	void* pTmp;
	if(m_pExternallyCalledMethods->Get(pMethod, &pTmp))
		return; // it's already in the table of methods to import
	m_pExternallyCalledMethods->Add(pMethod, NULL);
	AddImportType(pType);
}

void GCompiler::AddImportType(COType* pType)
{
	if(!pType)
		return;
	if(m_bSymbolMode)
		return;
	COFile* pFile = pType->GetFile();
	if(pFile->GetFileType() != FT_XLIB)
		return; // it's not an external type
	void* pTmp;
	if(m_pExternallyReferencedTypes->Get(pType, &pTmp))
		return; // it's already in the table of types to import
	if(m_pAlreadyImportedTypes && m_pAlreadyImportedTypes->Get(pType->GetName(), &pTmp))
		return; // it's already been imported
	m_pExternallyReferencedTypes->Add(pType, NULL);
	if(pType->GetTypeType() == COType::TT_CLASS)
		AddImportType(((COClass*)pType)->GetParent()); // Recursively add parents
}

void GCompiler::ImportMethods()
{
	// Copy the method hash table to an array so we can shrink it more efficiently
	int nMethodCount = m_pExternallyCalledMethods->GetCount();
	GTEMPBUF(pTmpMethods, nMethodCount * sizeof(COMethod*));
	COMethodDecl** pMethods = (COMethodDecl**)pTmpMethods;
	GHashTableEnumerator hteMethods(m_pExternallyCalledMethods);
	int n;
	for(n = 0; n < nMethodCount; n++)
	{
		COMethodDecl* pMethod = (COMethodDecl*)hteMethods.GetNextKey();
		GAssert(pMethod, "hash table has bad count");
		pMethods[n] = pMethod;
	}

	// Import all the externally called methods
	while(nMethodCount > 0)
	{
		// Make a hash table with ID's of all the methods that need to be imported from the same library as the first method
		GHashTable htMethodsToImport(17);
		Library* pExternalLibrary = pMethods[0]->GetLibrary();
		if(!pExternalLibrary)
		{
			GAssert(false, "not a library method--todo: handle this case");
		}
		for(n = 0; n < nMethodCount; n++)
		{
			if(pMethods[n]->GetLibrary() == pExternalLibrary)
			{
				if(pMethods[n]->GetType()->GetTypeType() == COType::TT_CLASS)
				{
					EMethod* pEMethod = ((COMethod*)pMethods[n])->GetEMethod();
					pEMethod->GetDependentMethods(&htMethodsToImport, m_pExternallyReferencedTypes, pExternalLibrary, m_pCOProject);
				}
				pMethods[n] = pMethods[nMethodCount - 1];
				nMethodCount--;
				n--;
			}
		}

		// Import all the methods
		GHashTableEnumerator hte(&htMethodsToImport);
		int nMethodID;
		while(hte.GetNextIntKey(&nMethodID))
		{
			ImportExternalMethod(pExternalLibrary, nMethodID);
		}
	}
}

bool GCompiler::DoImporting()
{
/*
// This is debug code to save a list of the types that were marked for importing
FILE* pFile = fopen("importtypes.txt", "w");
GHashTableEnumerator hte(m_pExternallyReferencedTypes);
while(true)
{
	COType* pType = (COType*)hte.GetNextKey();
	if(!pType)
		break;
	fputs(pType->GetName(), pFile);
	fputs("\n", pFile);
}
fclose(pFile);
*/

	GAssert(!m_pAlreadyImportedTypes, "who allocated m_pAlreadyImportedTypes?");
	delete(m_pAlreadyImportedTypes);
	m_pAlreadyImportedTypes = new GConstStringHashTable(MAX(m_pExternallyReferencedTypes->GetCount() * 2, 23), false);

	// Import the methods
	ImportMethods();

	// Import the remaining externally referenced types
	while(true)
	{
		// todo: this is a horribly inefficient loop because we make a new enumerator
		// for each type in the hash table.  Surely we can write this better
		COType* pType;
		{
			GHashTableEnumerator hteTypes(m_pExternallyReferencedTypes);
			pType = (COType*)hteTypes.GetNextKey();
		}
		if(!pType)
			break;
		ImportExternalType(pType);			
	}

	delete(m_pAlreadyImportedTypes);
	m_pAlreadyImportedTypes = NULL;

	return true;
}

bool GCompiler::ImportExternalMethod(Library* pExternalLibrary, int nMethodID)
{
	// Get the new type tag
	EMethod* pExternalMethod = pExternalLibrary->GetEMethod(nMethodID);
	GXMLTag* pExternalTypeTag = pExternalMethod->GetClass()->GetTag();
	GXMLAttribute* pNameAttr = pExternalTypeTag->GetAttribute(ATTR_NAME);
	if(!pNameAttr)
	{
		GAssert(false, "bad xlib"); // todo: handle this case
		return false;
	}
	GXMLTag* pNewTypeTag;
	if(!m_pAlreadyImportedTypes->Get(pNameAttr->GetValue(), (void**)&pNewTypeTag))
	{
		pNewTypeTag = ImportExternalType(pExternalLibrary, pExternalTypeTag);
		GAssert(pNewTypeTag, "todo: handle this case");
	}

	// Copy the method tag
	GXMLTag* pNewMethodTag = pExternalMethod->GetTag()->Copy();

	// Fix up the ID
	GXMLAttribute* pAttrID = pNewMethodTag->GetAttribute(ATTR_ID);
	if(!pAttrID)
	{
		GAssert(false, "todo: handle this case");
	}
	GAssert(atoi(pAttrID->GetValue()) == nMethodID, "inconsistent ID");
	COMethod* pCOMethod = pExternalMethod->GetCOMethod(m_pCOProject);
	int nNewID = pCOMethod->GetID();
	char szTmp[32];
	itoa(nNewID, szTmp, 10);
	pAttrID->SetValue(szTmp);

	// Fix up the Bin tags
	EInstrArray instrArray(pNewMethodTag, NULL);
	int nCount = instrArray.GetInstrCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		InstrBin* pInstr = instrArray.GetInstr(n);
		InstructionStruct* pInstrStruct = pInstr->GetInstrStruct();
		int i;
		for(i = 0; pInstrStruct->szParamTypes[i] != '\0'; i++)
		{
			switch(pInstrStruct->szParamTypes[i])
			{
			case 'm':
				{
					int nOldID = pInstr->GetParam(i);
					EMethod* pOldEMethod = pExternalLibrary->GetEMethod(nOldID);
					pCOMethod = pOldEMethod->GetCOMethod(m_pCOProject);
					int nNewID = pCOMethod->GetID();
					pInstr->SetParam(i, nNewID);
				}
				break;

			case 's':
				{
					int nOldID = pInstr->GetParam(i);
					const char* szString = pExternalLibrary->GetStringFromTable(nOldID);
					int nNewID = AddConstantString(szString);
					pInstr->SetParam(i, nNewID);
				}
				break;

			case 't':
				{
					int nOldID = pInstr->GetParam(i);
					EType* pOldEType = pExternalLibrary->GetEType(nOldID);
					COType* pCOType = pOldEType->GetCOType(m_pCOProject);
					int nNewID = pCOType->GetID();
					pInstr->SetParam(i, nNewID);
				}
				break;
			}
		}
	}
	instrArray.SetBinTags(pNewMethodTag);

	// Add the method to the class
	pNewTypeTag->AddChildTag(pNewMethodTag);
	return true;
}

GXMLTag* GCompiler::ImportExternalType(Library* pExternalLibrary, GXMLTag* pExternalTypeTag)
{
	GXMLAttribute* pAttrID = pExternalTypeTag->GetAttribute(ATTR_ID);
	if(!pAttrID)
	{
		GAssert(false, "expected an ID tag--todo: handle this case");
		return false;
	}
	int nExternalID = atoi(pAttrID->GetValue());
	EType* pEType = pExternalLibrary->GetEType(nExternalID);
	COType* pCOType = pEType->GetCOType(m_pCOProject);
	m_pExternallyReferencedTypes->Remove(pCOType);
	return ImportExternalType(pCOType);
}

GXMLTag* GCompiler::ImportExternalType(COType* pCOType)
{
	const char* szTypeName = pCOType->GetName();
	GXMLTag* pNewTag = NULL;
	if(!m_pAlreadyImportedTypes->Get(szTypeName, (void**)&pNewTag))
	{
		pNewTag = pCOType->ToXMLForLibrary(this);
		m_pLibraryTag->AddChildTag(pNewTag);
		m_pAlreadyImportedTypes->Add(szTypeName, pNewTag);
	}
	m_pExternallyReferencedTypes->Remove(pCOType);
	return pNewTag;
}

void GCompiler::AjustIDInDataAttr(GXMLAttribute* pPrevAttr, GXMLAttribute* pCurrAttr, int nPos, int nNewValue)
{
	char* pNewPrevData = NULL;
	int nPrevLen = 0;
	char* pNewCurrData = pNewCurrData = new char[strlen(pCurrAttr->GetValue()) + 1];
	unsigned char* pValues = (unsigned char*)&nNewValue;
	strcpy(pNewCurrData, pCurrAttr->GetValue());
	if(nPos < 0)
	{
		GAssert(pPrevAttr, "bad value for pPrevAttr");
		nPrevLen = strlen(pPrevAttr->GetValue());
		pNewPrevData = new char[nPrevLen + 1];
		strcpy(pNewPrevData, pPrevAttr->GetValue());
	}

	// Do first byte
	if(nPos < 0)
		ByteToHex(pValues[0], pNewPrevData + nPrevLen + nPos);
	else
		ByteToHex(pValues[0], pNewCurrData + nPos);
	nPos += 2;

	// Do second byte
	if(nPos < 0)
		ByteToHex(pValues[1], pNewPrevData + nPrevLen + nPos);
	else
		ByteToHex(pValues[1], pNewCurrData + nPos);
	nPos += 2;

	// Do third byte
	if(nPos < 0)
		ByteToHex(pValues[2], pNewPrevData + nPrevLen + nPos);
	else
		ByteToHex(pValues[2], pNewCurrData + nPos);
	nPos += 2;

	// Do fourth byte
	GAssert(nPos >= 0, "n should be zero or positive here");
		ByteToHex(pValues[3], pNewCurrData + nPos);

	if(pNewPrevData)
		pPrevAttr->SetValue(pNewPrevData);
	pCurrAttr->SetValue(pNewCurrData);
}

int GCompiler::AddConstantString(const char* szValue)
{
	if(m_bSymbolMode)
		return 0;
	GXMLTag* pStringTag = NULL;
	if(m_pStringTableTag)
	{
		// Look for an existing match
		for(pStringTag = m_pStringTableTag->GetFirstChildTag(); pStringTag; pStringTag = m_pStringTableTag->GetNextChildTag(pStringTag))
		{
			GXMLAttribute* pName = pStringTag->GetAttribute(ATTR_VAL);
			if(strcmp(pName->GetValue(), szValue) == 0)
				break;
		}
	}
	else
		m_pStringTableTag = new GXMLTag(TAG_NAME_STRINGTABLE);
	int nID = -1;
	if(pStringTag)
	{
		// Get the ID from the existing match
		GXMLAttribute* pID = pStringTag->GetAttribute(ATTR_ID);
		GAssert(pID, "todo: handle this case");
		nID = atoi(pID->GetValue());
	}
	else
	{
		// Add a new string to the table
		nID = m_pStringTableTag->GetChildTagCount();
		GXMLTag* pNewTag = new GXMLTag(TAG_NAME_STRING);
		pNewTag->AddAttribute(new GXMLAttribute(ATTR_VAL, szValue));
		char szTmp[32];
		itoa(nID, szTmp, 10);
		pNewTag->AddAttribute(new GXMLAttribute(ATTR_ID, szTmp));
		m_pStringTableTag->AddChildTag(pNewTag);
	}
	return nID;
}

COVariable* GCompiler::GetConstIntVar(int nValue, COInstruction* pSymbolInstr)
{
	// Make an integer to hold the result
	char szTmp[64];
	GetUniqueID(szTmp);
	COVariable* pNewVar;
	if(m_bSymbolMode)
		pNewVar = m_pCOProject->m_pNull; // todo: this is an error if this var is used in an "if" statement because "Null" will change the output
	else
	{
		pNewVar = new COVariable(0, 0, 0, szTmp, m_pCOProject->m_pInteger, true, false, false);
		if(!PushStackVariable(pNewVar, true))
		{
			CheckError();
			return NULL;
		}
	}
	int nOffset;
	if(!FindVarOnStack(&nOffset, pNewVar, pSymbolInstr))
	{
		CheckError();
		return NULL;
	}

	// Copy the value into the variable
	AddInstr(Instr_NewVariable, pSymbolInstr);
	AddInstr(Instr_NewInteger, pSymbolInstr);
	AddParam(nOffset);
	AddInstr(Instr_SetIntWithConst, pSymbolInstr);
	AddParam(nOffset);
	AddParam(nValue);

	return pNewVar;
}

COVariable* GCompiler::GetConstFloatVar(double dValue, COInstruction* pSymbolInstr)
{
	// Get the current code indexes
	int nLine = pSymbolInstr->GetLineNumber();
	int nCol, nWid;
	pSymbolInstr->GetColumnAndWidth(&nCol, &nWid);

	// Make a variable to reference the float
	char szTmp[64];
	GetUniqueID(szTmp);
	COMachineClass* pFloatClass = m_pCOProject->GetFloatClass();
	GAssert(pFloatClass, "Problem with built in class");
	COVariable* pNewVar;
	pNewVar = new COVariable(nLine, nCol, nWid, szTmp, pFloatClass, false, false, false);
	if(!PushStackVariable(pNewVar, true))
	{
		CheckError();
		return NULL;
	}
	AddInstr(Instr_NewVariable, pSymbolInstr);

	// Make code to construct the Float object
	COExpressionArray paramList;
	COVarRef* pNewVarRef = new COVarRef(nLine, nCol, nWid, pNewVar);
	const char* szTmpVarName;
	pNewVarRef->SetExpReadOnly(false, false, &szTmpVarName);
	paramList.AddExpression(pNewVarRef);
	EMethodSignature methodSig(SIG_NEW); // todo: cache this
	if(!COCall::CompileImplicitCall(this, pFloatClass, &methodSig, &paramList, pSymbolInstr, (COInstrArray*)pSymbolInstr->GetParent()))
	{
		CheckError();
		return NULL;
	}

	// Make code to set the value
	int nOffset;
	if(!FindVarOnStack(&nOffset, pNewVar, pSymbolInstr))
	{
		CheckError();
		return NULL;
	}
	AddInstr(Instr_SetFloatWithConst, pSymbolInstr);
	AddParam(nOffset);
	GAssert(sizeof(double) == sizeof(unsigned int) + sizeof(unsigned int), "size issue");
	unsigned int* pUInts = (unsigned int*)&dValue;
	AddParam((int)pUInts[0]);
	AddParam((int)pUInts[1]);

	return pNewVar;
}

COVariable* GCompiler::GetConstStringVar(const char* szValue, COInstruction* pSymbolInstr)
{
	// Get the current code indexes
	int nLine = pSymbolInstr->GetLineNumber();
	int nCol, nWid;
	pSymbolInstr->GetColumnAndWidth(&nCol, &nWid);

	// Make a variable to reference the string
	char szTmp[64];
	GetUniqueID(szTmp);
	COMachineClass* pStringClass = m_pCOProject->GetStringClass();
	GAssert(pStringClass, "Problem with built in class");
	COVariable* pNewVar;
	pNewVar = new COVariable(nLine, nCol, nWid, szTmp, pStringClass, false, false, false);
	if(!PushStackVariable(pNewVar, true))
	{
		CheckError();
		return NULL;
	}
	AddInstr(Instr_NewVariable, pSymbolInstr);

	// Add the string to the table
	int nID = AddConstantString(szValue);
	COVariable* pID = GetConstIntVar(nID, pSymbolInstr); // todo: do we leak this object?
	if(!pID)
	{
		CheckError();
		return NULL;
	}

	// Call the GetConstString instruction
	int nOffset;
	if(!FindVarOnStack(&nOffset, pNewVar, pSymbolInstr))
	{
		CheckError();
		return NULL;
	}
	AddInstr(Instr_GetConstString, pSymbolInstr);
	AddParam(nOffset);
	AddParam(nID);

	return pNewVar;
}

bool GCompiler::CompileMethodStart(COMethod* pMethod)
{
	// Check initial conditions
	if(m_pVariables->GetSize() != 0 || m_pEInstrArrayBuilder->GetSize() != 0)
	{
		SetError(&Error::INTERNAL_ERROR, NULL);
		return false;
	}
	return true;
}

bool GCompiler::CompileMethodParameter(COVariable* pVariable)
{
	if(!PushStackVariable(pVariable, false))
	{
		CheckError();
		return false;
	}
	return true;
}

bool GCompiler::CompileMethodPostParameters()
{
	// Recognize internal stuff on the stack
	if(!PushStackVariable(m_pBasePointer, false))
	{
		CheckError();
		return false;
	}
	if(!PushStackVariable(m_pInstructionPointer, false))
	{
		CheckError();
		return false;
	}
	if(!PushStackVariable(m_pStackLayerVar, false))
	{
		CheckError();
		return false;
	}
	m_nBasePointer = m_pVariables->GetSize();
	return true;
}

EInstrArray* GCompiler::CompileMethodFinish(COMethod* pMethod)
{
	// Add return instruction
	AddInstr(Instr_Return, NULL);

	// delete the variables on the stack
	COVariable* pTmp;
	int nCount = m_pVariables->GetSize();
	while(m_pVariables->GetSize() > 0)
	{
		if(!PopStackVariable(&pTmp, pMethod))
		{
			CheckError();
			return NULL;
		}
	}

	// Link the method
	EInstrArray* pMB = m_pEInstrArrayBuilder->MakeEInstrArray();
	if(!pMB)
	{
		SetError(&Error::INTERNAL_ERROR, NULL);
		return NULL;
	}
	return pMB;
}

bool GCompiler::CompileCallStart(COCall* pCall, COInstruction* pSymbolInstr)
{
	// Add start-local if necessary
	if(pCall->GetParamCount() > 0)
	{
		if(!AddStartScope(pSymbolInstr))
		{
			CheckError();
			return false;
		}
	}
	return true;
}

bool GCompiler::CompileCallParameter(COVariable* pVariable, int nParam, COInstruction* pSymbolInstr, int* pnOutOffset)
{
	if(nParam == 0)
		m_nStackSizeAtCall = m_pVariables->GetSize() - m_nBasePointer;
	int nOffset;
	if(!FindVarOnStack(&nOffset, pVariable, pSymbolInstr))
	{
		CheckError();
		return false;
	}
	AddInstr(Instr_PushParameter, pSymbolInstr);
	AddParam(nOffset);
	return true;
}

bool GCompiler::CompileMakeTheCall(COCall* pCall, COVariable* pCatcher, COInstruction* pSymbolInstr, int nThisOffset)
{
	switch(pCall->GetCallType())
	{
	case COCall::CT_METHOD:
		{
			COMethod* pMethod = ((COMethodCall*)pCall)->GetMethod();
			if(pCatcher)
			{
				AddInstr(Instr_TryCall, pSymbolInstr);
				int nOffset;
				if(!FindVarOnStack(&nOffset, pCatcher, pSymbolInstr))
				{
					CheckError();
					return false;
				}
				AddParam(nOffset);
			}
			else
				AddInstr(Instr_Call, pSymbolInstr);
			AddParam(pMethod->GetID());
			AddImportMethod(pMethod);
		}
		break;

	case COCall::CT_VIRTUAL:
		{
			COVirtualCall* pVCall = (COVirtualCall*)pCall;
			if(pCatcher)
			{
				GAssert(false, "todo: implement catching interface calls");
			}
			else
			{
				// todo: import all methods that could be called virtually here?
				AddInstr(Instr_VCall, pSymbolInstr);
				AddParam(m_nStackSizeAtCall); // the "this" variable
				AddParam(pVCall->GetVirtualTableIndex());
			}			
		}
		break;

	case COCall::CT_INTERFACE:
		{
			COInterfaceCall* pICall = (COInterfaceCall*)pCall;
			if(pCatcher)
			{
				GAssert(false, "todo: implement catching interface calls");
			}
			else
			{
				if(pCall->GetParamCount() < 1)
				{
					SetError(&Error::NOT_ENOUGH_PARAMETERS, pCall);
					return false;
				}
				COMethodDecl* pMethodDecl = pICall->GetMethodDecl();
				AddImportMethod(pMethodDecl);
				COType* pTypeTmp = pCall->GetParam(0)->GetType(m_pCOProject);
				if(pTypeTmp->GetTypeType() == COType::TT_CLASS)
				{
					SetError(&Error::USING_CLASS_TO_CALL_SIGNATURE, pCall);
					return false;
				}
				COInterface* pInterface = (COInterface*)pTypeTmp;
				if(pCall->GetTargetType()->GetTypeType() == COType::TT_MACHINE)
					AddInstr(Instr_MCall, pSymbolInstr);
				else
					AddInstr(Instr_ICall, pSymbolInstr);
				AddParam(m_nStackSizeAtCall); // the "this" variable
				AddParam(pInterface->GetID());
				int nMethodDeclIndex = pInterface->FindMethodDecl(pMethodDecl);
				GAssert(nMethodDeclIndex >= 0, "method decl not found");
				AddParam(nMethodDeclIndex);
			}
		}
		break;

	default:
		GAssert(false, "unexpected case");
		break;
	}

	return true;
}

bool GCompiler::CompileCallFinish(COCall* pCall, COInstruction* pSymbolInstr)
{
	// Add end-local if necessary	
	if(pCall->GetParamCount() > 0)
	{
		if(!AddEndScope(pSymbolInstr))
		{
			CheckError();
			return false;
		}
	}
	return true;
}

bool GCompiler::CompileSetDeclFromParam(COVariable* pDest, int nParam, COInstruction* pSymbolInstr, COType* pCastType)
{
	int nSourceOffset = m_nStackSizeAtCall + nParam;
	int nDestOffset;
	if(!FindVarOnStack(&nDestOffset, pDest, pSymbolInstr))
	{
		CheckError();
		return false;
	}
	if(pCastType != NULL)
	{
		AddInstr(Instr_Cast, pSymbolInstr);
		AddParam(pCastType->GetID());
		AddParam(nSourceOffset);
	}
	AddInstr(Instr_CopyVar, pSymbolInstr);
	AddParam(nDestOffset);
	AddParam(nSourceOffset);
	return true;
}

bool GCompiler::CompileClassStart(COClass* pClass)
{
	GAssert(!m_bSymbolMode, "Shouldn't be called in symbol mode");
	m_pCurrentClassTag = pClass->ToXMLForLibrary(this);
	m_pLibraryTag->AddChildTag(m_pCurrentClassTag);
	AddImportType(pClass->GetParent());
	return true;
}

bool GCompiler::CompileInterface(COInterface* pInterface)
{
	GXMLTag* pInterfaceTag = pInterface->ToXMLForLibrary(this);
	m_pLibraryTag->AddChildTag(pInterfaceTag);
	return true;
}

bool GCompiler::AddStartScope(COInstruction* pSymbolInstr)
{
	AddInstr(Instr_StartScope, pSymbolInstr);
	if(!PushStackVariable(m_pLocalBasePointer, false))
	{
		CheckError();
		return false;
	}
	return true;
}

bool GCompiler::AddEndScope(COInstruction* pSymbolInstr)
{
	AddInstr(Instr_EndScope, pSymbolInstr);

	// Pop variables until we pop the local base pointer
	if(m_bSymbolMode)
		return true;
	while(true)
	{
		COVariable* pVariable;
		if(!PopStackVariable(&pVariable, pSymbolInstr))
		{
			CheckError();
			return false;
		}
		if(pVariable == m_pLocalBasePointer)
			return true;
	}
	GAssert(false, "How'd it get here?");
	return false;
}

bool GCompiler::AsmCmdSetMember(COCall* pCall, COVariable* pDest, COVariable* pSource, COVariable* pMember)
{
	if(pDest->GetType()->GetTypeType() != COType::TT_CLASS)
	{
		SetError(&Error::MEMBER_NOT_FOUND, pCall);
		return false;
	}
	int nSourceOffset;
	if(!FindVarOnStack(&nSourceOffset, pSource, pCall))
	{
		CheckError();
		return false;
	}
	int nDestOffset;
	if(!FindVarOnStack(&nDestOffset, pDest, pCall))
	{
		CheckError();
		return false;
	}
	int nMemberNumber = ((COClass*)pDest->GetType())->FindMember(pMember);
	if(nMemberNumber < 0)
	{
		SetError(&Error::MEMBER_NOT_FOUND, pCall);
		return false;
	}

	// Check for proper inherritance
	bool bNeedCast;
	COType* pDestType = pMember->GetType();
	if(!CheckCast(pSource->GetType(), pDestType, &bNeedCast, pCall))
	{
		CheckError();
		return false;
	}
	if(bNeedCast)
	{
		AddInstr(Instr_Cast, pCall);
		AddParam(pDestType->GetID());
		AddParam(nSourceOffset);
	}
	AddInstr(Instr_SetMember, pCall);
	AddParam(nDestOffset);
	AddParam(nMemberNumber);
	AddParam(nSourceOffset);
	return true;
}


bool GCompiler::AsmCmdDecl(COVariable* pVariable, bool bTakeOwnership, COInstruction* pSymbolInstr)
{
	if(!PushStackVariable(pVariable, bTakeOwnership))
	{
		CheckError();
		return false;
	}
	AddInstr(Instr_NewVariable, pSymbolInstr);
	return true;
}

bool GCompiler::CompileProject()
{
	// Initialize that we have no errors yet
	m_pErrorHolder->Reset();

	// Compile beginning stuff
	if(!CompileBegin())
	{
		CheckError();
		CompileFailed();
		return false;
	}

	// Compile each file in the Source
	COFileSet* pSource = m_pCOProject->GetSource();
	COFile* pFile;
	int nCount = pSource->GetFileCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pFile = pSource->GetFile(n);
		if(pFile->GetFileType() == FT_CLASSIC || pFile->GetFileType() == FT_XML)
		{
			if(!pFile->Compile(this))
			{
				CheckError();
				CompileFailed();
				return false;
			}
		}
	}

	// Compile finishing stuff
	if(!CompileFinish())
	{
		CheckError();
		CompileFailed();
		return false;
	}

	return true;
}

void GCompiler::SetError(ErrorStruct* pError, CodeObject* pCodeObject, const char* szParam1)
{
	m_pErrorHolder->SetError(pError, pCodeObject);
	if(szParam1)
		m_pErrorHolder->SetParam1(szParam1);
	if(m_pCurrentFile)
		m_pErrorHolder->SetFilename(m_pCurrentFile->GetFilename());
}

void GCompiler::SetError(ErrorStruct* pError, CodeObject* pCodeObject)
{
	SetError(pError, pCodeObject, NULL);
}

void GCompiler::CheckError()
{
	m_pErrorHolder->CheckError();
}

void GCompiler::GetUniqueID(char* szBuff)
{
	strcpy(szBuff, STR_UNIQUE_ID);
	itoa(m_nUniqueID, szBuff + strlen(STR_UNIQUE_ID), 10);
	m_nUniqueID++;
}

bool GCompiler::PrepareForParam(COExpression* pParam, COCall* pCall)
{
	switch(pParam->GetExpressionType())
	{
	case COExpression::ET_VARDECL:
		if(!AsmCmdDecl((COVariable*)pParam, false, pCall))
		{
			CheckError();
			return false;
		}
		break;
	case COExpression::ET_VARREF:
		if(((COVarRef*)pParam)->GetVar() == m_pCOProject->m_pNull)
		{
			if(!MakeSureNullIsDefined(pCall))
			{
				CheckError();
				return false;
			}
		}
		break;
	case COExpression::ET_COMPOUND:
		if(!PrepareForParam(((COOperator*)pParam)->GetLeftExpression(), pCall))
			return false;
		if(!PrepareForParam(((COOperator*)pParam)->GetRightExpression(), pCall))
			return false;
		break;
	}
	return true;
}

bool GCompiler::CompileParams(COCall* pCall, ParamVarArrayHolder* pParamVarHolders)
{
	int nCount = pCall->GetParamCount();
	GAssert(pParamVarHolders->GetCount() == nCount, "parameter count mismatch");
	COMethod* pMeth = pCall->GetMethod();
	int n;
	for(n = 0; n < nCount; n++)
	{
		// Compile the parameter
		COExpression* pParam = pCall->GetParam(n);
		COVariable* pVar = pCall->GetTargetParam(n);
		COType* pType = pParam->GetType(m_pCOProject);
		AddImportType(pType);
		bool bModifyVar = !pVar->IsVarReadOnly();
		bool bModifyObj = !pVar->IsObjReadOnly();
		Holder<EvalExprResult*>* phParamVar = pParamVarHolders->GetParamVarHolder(n);
		if(!pParam->Compile(bModifyVar, bModifyObj, this, phParamVar, pCall))
		{
			CheckError();
			return false;
		}

		// Determine its position on the stack
		COVariable* pParamVar = phParamVar->Get()->GetResult();
		int nStackPos;
		if(!FindVarOnStack(&nStackPos, pParamVar, pCall))
		{
			CheckError();
			return false;
		}
		pParamVarHolders->SetStackPos(n, nStackPos);
	}
	return true;
}

bool GCompiler::CompileIf(COCall* pCall, ParamVarArrayHolder* pParams, COMethod* pSourceMethod)
{
	COVariable* pCondition = pParams->GetParamVar(0);

	// Add the branch instruction
	AddInstr(Instr_BranchIfFalse, pCall);
	AddParam(pParams->GetStackPos(0)); // condition
	bool bDidElse = false;
	int nConditionJumpPos = m_pEInstrArrayBuilder->GetSize();
	AddParam(0); // Place holder for the branch offset

	// Compile all the sub-Instructions
	if(!AddStartScope(pCall))
	{
		CheckError();
		return false;
	}

	// Compile each instruction
	COInstruction* pInstruction;
	GAssert(pCall->CanHaveChildren(), "'if' calls should be able to have children");
	COInstrArray* pChildren = pCall->GetChildInstructions();
	int nInstructionCount = pChildren->GetInstrCount();
	int n;
	for(n = 0; n < nInstructionCount; n++)
	{
		pInstruction = pChildren->GetInstr(n);
		if(pInstruction->GetInstructionType() == COInstruction::IT_CALL &&
			((COCall*)pInstruction)->GetCallType() == COCall::CT_METHOD &&
			((COMethodCall*)pInstruction)->GetMethod() == m_pCOProject->m_pObject_else)
		{
			// Compile Else Instruction
			if(bDidElse)
			{
				SetError(&Error::ELSE_WITHOUT_CORRESPONDING_IF, pInstruction);
				return false;
			}
			if(!AddEndScope(pCall))
			{
				CheckError();
				return false;
			}
			AddInstr(Instr_Jump, pCall);
			int nOldConditionJumpPos = nConditionJumpPos;
			nConditionJumpPos = m_pEInstrArrayBuilder->GetSize();
			AddParam(0); // Place holder
			m_pEInstrArrayBuilder->AddFixUpSpot(nOldConditionJumpPos, (m_pEInstrArrayBuilder->GetSize() - nOldConditionJumpPos) - 4);
			int nOffset = GetCurrentOffset();
			if(!AddStartScope(pCall))
			{
				CheckError();
				return false;
			}
			bDidElse = true;
		}
		else
		{
			// Compile normal instruction
			if(!pInstruction->Compile(this, pSourceMethod, pInstruction))
			{
				CheckError();
				return false;
			}
		}
	}

	if(!AddEndScope(pCall))
	{
		CheckError();
		return false;
	}

	// Add a fix up spot for the branch offset
	m_pEInstrArrayBuilder->AddFixUpSpot(nConditionJumpPos, (m_pEInstrArrayBuilder->GetSize() - nConditionJumpPos) - 4);

	return true;
}

bool GCompiler::CompileWhile(COCall* pCall, ParamVarArrayHolder* pParams, COMethod* pSourceMethod)
{
	// Make a temporary variable to hold the condition
	int nLine = pCall->GetLineNumber();
	int nCol, nWid;
	pCall->GetColumnAndWidth(&nCol, &nWid);
	char szTmp[64];
	GetUniqueID(szTmp);
	COVariable* pCondition = new COVariable(nLine, nCol, nWid, szTmp, m_pCOProject->m_pInteger, false, false, false);
	if(!PushStackVariable(pCondition, true))
	{
		CheckError();
		return false;
	}
	int nOffsetCondition;
	if(!FindVarOnStack(&nOffsetCondition, pCondition, pCall))
	{
		CheckError();
		return false;
	}
	AddInstr(Instr_NewVariable, pCall);
	AddInstr(Instr_NewInteger, pCall);
	AddParam(nOffsetCondition);

	// Evaluate the condition and copy the results into our temp var
	int nLoopTop = m_pEInstrArrayBuilder->GetSize();
	if(!AddStartScope(pCall))
	{
		CheckError();
		return false;
	}
	if(!CompileParams(pCall, pParams))
	{
		CheckError();
		return false;
	}
	AddInstr(Instr_CopyInt, pCall);
	AddParam(nOffsetCondition);
	AddParam(pParams->GetStackPos(0));
	if(!AddEndScope(pCall))
	{
		CheckError();
		return false;
	}

	// Add the branch instruction
	AddInstr(Instr_BranchIfFalse, pCall);
	AddParam(nOffsetCondition); // the condition
	int nConditionJumpPos = m_pEInstrArrayBuilder->GetSize();
	AddParam(0); // Place holder for the branch offset

	// Compile all the sub-Instructions
	if(!AddStartScope(pCall))
	{
		CheckError();
		return false;
	}
	COInstruction* pInstruction;
	GAssert(pCall->CanHaveChildren(), "'while' calls should be able to have children");
	COInstrArray* pChildren = pCall->GetChildInstructions();
	int nInstructionCount = pChildren->GetInstrCount();
	int n;
	for(n = 0; n < nInstructionCount; n++)
	{
		pInstruction = pChildren->GetInstr(n);
		if(!pInstruction->Compile(this, pSourceMethod, pInstruction))
		{
			CheckError();
			return false;
		}
	}
	if(!AddEndScope(pCall))
	{
		CheckError();
		return false;
	}

	// add a jump back to the start of the loop
	AddInstr(Instr_Jump, pCall);
	AddParam((nLoopTop - m_pEInstrArrayBuilder->GetSize()) - 4);

	// Add a fix up spot for the branch offset
	m_pEInstrArrayBuilder->AddFixUpSpot(nConditionJumpPos, (m_pEInstrArrayBuilder->GetSize() - nConditionJumpPos) - 4);

	return true;
}

bool GCompiler::CompileAllocate(COCall* pCall, ParamVarArrayHolder* pParams, COMethod* pMethod, COInstruction* pSymbolInstr)
{
	// Make sure it's not an interface
	COVariable* pDest = pParams->GetParamVar(0);
	if(pDest->GetType()->GetTypeType() != COType::TT_CLASS)
	{
		SetError(&Error::CANT_MAKE_NEW_INTERFACE, pCall);
		return false;
	}

	// Write the allocation code for the right type of data
	COClass* pClass = (COClass*)pDest->GetType();
	if(pClass->DoesInherritFrom(m_pCOProject->m_pInteger))
	{
		AddInstr(Instr_NewInteger, pSymbolInstr);
		AddParam(pParams->GetStackPos(0));
	}
	else
	{
		if(pDest->GetType() != pMethod->GetClass())
		{
			SetError(&Error::ALLOCATE_MUST_BE_CALLED_IN_SAME_CLASS_PROC, pCall);
			return false;
		}
		AddInstr(Instr_NewObject, pCall);
		AddParam(pParams->GetStackPos(0));
		AddParam(pClass->GetID());
	}

	return true;
}

bool GCompiler::CompileSet(COCall* pCall, ParamVarArrayHolder* pParams)
{
	// See if a cast is needed
	COVariable* pDest = pParams->GetParamVar(0);
	COVariable* pSource = pParams->GetParamVar(1);
	COType* pDestType = pDest->GetType();
	bool bNeedCast;
	if(!CheckCast(pSource->GetType(), pDestType, &bNeedCast, pCall))
	{
		CheckError();
		return false;
	}

	// Write the instructions
	if(bNeedCast)
	{
		AddInstr(Instr_Cast, pCall);
		AddParam(pDestType->GetID());
		AddParam(pParams->GetStackPos(1));
	}
	AddInstr(Instr_CopyVar, pCall);
	AddParam(pParams->GetStackPos(0));
	AddParam(pParams->GetStackPos(1));

	return true;
}

bool GCompiler::CompileCopy(COCall* pCall, ParamVarArrayHolder* pParams)
{
	// Check Cast
	COVariable* pDest = pParams->GetParamVar(0);
	COVariable* pSource = pParams->GetParamVar(1);
	COType* pType = pSource->GetType();
	if(!m_bSymbolMode)
	{
		if(pType->GetTypeType() != COType::TT_CLASS || !((COClass*)pType)->DoesInherritFrom(m_pCOProject->m_pInteger))
		{
			SetError(&Error::BAD_CAST, pCall);
			return false;
		}
	}
	pType = pDest->GetType();
	if(!m_bSymbolMode)
	{
		if(pType->GetTypeType() != COType::TT_CLASS || !((COClass*)pType)->DoesInherritFrom(m_pCOProject->m_pInteger))
		{
			SetError(&Error::BAD_CAST, pCall);
			return false;
		}
	}

	// Write instructions
	AddInstr(Instr_CopyInt, pCall);
	AddParam(pParams->GetStackPos(0));
	AddParam(pParams->GetStackPos(1));

	return true;
}

bool GCompiler::CompileInvert(COCall* pCall, ParamVarArrayHolder* pParams)
{
	// Write instructions
	AddInstr(Instr_InvertInteger, pCall);
	AddParam(pParams->GetStackPos(0));
	return true;
}

bool GCompiler::CompileThrow(COCall* pCall, ParamVarArrayHolder* pParams)
{
	// Write Instructions
	AddInstr(Instr_Throw, pCall);
	AddParam(pParams->GetStackPos(0));
	return true;
}

bool GCompiler::CompileIncrement(COCall* pCall, ParamVarArrayHolder* pParams, bool bDecrement)
{
	// Write Instructions
	if(bDecrement)
		AddInstr(Instr_DecrementInteger, pCall);
	else
		AddInstr(Instr_IncrementInteger, pCall);
	AddParam(pParams->GetStackPos(0));
	return true;
}

bool GCompiler::CompileArithInstruction(COCall* pCall, ParamVarArrayHolder* pParams, TwoVarArithmetic eOp)
{
	// Check cast
	COVariable* pDest = pParams->GetParamVar(0);
	COVariable* pSource = pParams->GetParamVar(1);
	COType* pType = pSource->GetType();
	if(!m_bSymbolMode)
	{
		if(pType->GetTypeType() != COType::TT_CLASS || !((COClass*)pType)->DoesInherritFrom(m_pCOProject->m_pInteger))
		{
			SetError(&Error::BAD_CAST, pCall);
			return false;
		}
	}
	pType = pDest->GetType();
	if(!m_bSymbolMode)
	{
		if(pType->GetTypeType() != COType::TT_CLASS || !((COClass*)pType)->DoesInherritFrom(m_pCOProject->m_pInteger))
		{
			SetError(&Error::BAD_CAST, pCall);
			return false;
		}
	}

	// Write instructions
	switch(eOp)
	{
		case TVA_ADD:			AddInstr(Instr_AddInteger, pCall); break;
		case TVA_SUBTRACT:		AddInstr(Instr_SubtractInteger, pCall); break;
		case TVA_MULTIPLY:		AddInstr(Instr_MultiplyInteger, pCall); break;
		case TVA_DIVIDE:		AddInstr(Instr_DivideInteger, pCall); break;
		case TVA_MODULUS:		AddInstr(Instr_ModulusInteger, pCall); break;
		case TVA_AND:			AddInstr(Instr_AndInteger, pCall); break;
		case TVA_OR:			AddInstr(Instr_OrInteger, pCall); break;
		case TVA_XOR:			AddInstr(Instr_XorInteger, pCall); break;
		case TVA_SHIFTLEFT:		AddInstr(Instr_ShiftLeft, pCall); break;
		case TVA_SHIFTRIGHT:	AddInstr(Instr_ShiftRight, pCall); break;
		default:
			SetError(&Error::UNKNOWN_OPERATOR, pCall);
			return false;
	}

	AddParam(pParams->GetStackPos(0));
	AddParam(pParams->GetStackPos(1));

	return true;
}

bool GCompiler::CompileReturn(COCall* pCall, ParamVarArrayHolder* pParams)
{
	AddInstr(Instr_Return, pCall);
	return true;
}

bool GCompiler::CheckCast(COType* pSourceType, COType* pDestType, bool* pbNeedCast, CodeObject* pCodeObject)
{
	ErrorStruct* pError;
	bool bRetVal = pSourceType->CanCastTo(pDestType, pbNeedCast, &pError);
	if(!bRetVal)
	{
		SetError(pError, pCodeObject);
		return false;
	}
	return true;
}

bool GCompiler::CheckParameters(COCall* pCall)
{
	GAssert(!m_bSymbolMode, "shouldn't be called in symbol mode");
	COExpression* pCatcher = pCall->GetCatcher();
	if(pCatcher)
		PrepareForParam(pCatcher, pCall);
	int nCount = pCall->GetParamCount();
	int n;
	COExpression* pParam;
	for(n = 0; n < nCount; n++)
	{
		pParam = pCall->GetParam(n);
		PrepareForParam(pParam, pCall);
	}

	// Check for correct number of parameters
	int nParamCount = pCall->GetTargetParamCount();
	if(pCall->GetParamCount() < nParamCount)
	{
		SetError(&Error::NOT_ENOUGH_PARAMETERS, pCall);
		return false;
	}
	if(pCall->GetParamCount() > nParamCount)
	{
		SetError(&Error::TOO_MANY_PARAMETERS, pCall);
		return false;
	}

	// Check type of each parameter
	COVariable* pVariable;
	for(n = 0; n < nCount; n++)
	{
		pParam = pCall->GetParam(n);
		pVariable = pCall->GetTargetParam(n);
		bool bNeedCast;
		if(!CheckCast(pParam->GetType(m_pCOProject), pVariable->GetType(), &bNeedCast, pCall))
		{
			CheckError();
			return false;
		}
		// todo: do something if we need to cast

	}

	// read-only status is checked when we evaluate the parameters to
	// make a value to push on the stack
	return true;
}

bool GCompiler::CompileCallToBuiltInMethod(COMethodCall* pCall, COMethod* pSourceMethod, COInstruction* pSymbolInstr)
{
	COMethod* pMethod = pCall->GetMethod();
	COClass* pClass = pMethod->GetClass();
	GAssert(pClass->GetFile() == m_pCOProject->m_pBuiltIn, "Not a built in method");

	// Compile the params
	ParamVarArrayHolder pvah(pCall->GetParamCount());
	if(pMethod != m_pCOProject->m_pObject_while) // "while" will compile the parameters itself
	{
		if(!CompileParams(pCall, &pvah))
		{
			CheckError();
			return false;
		}
	}

	if(pClass == m_pCOProject->m_pObject)
	{
		if(pMethod == m_pCOProject->m_pObject_return)
			return CompileReturn(pCall, &pvah);
		else if(pMethod == m_pCOProject->m_pObject_if)
			return CompileIf(pCall, &pvah, pSourceMethod);
		else if(pMethod == m_pCOProject->m_pObject_while)
			return CompileWhile(pCall, &pvah, pSourceMethod);
		else if(pMethod == m_pCOProject->m_pObject_allocate)
			return CompileAllocate(pCall, &pvah, pSourceMethod, pSymbolInstr);
		else if(pMethod == m_pCOProject->m_pObject_set)
			return CompileSet(pCall, &pvah);
		else if(pMethod == m_pCOProject->m_pObject_copy)
			return CompileCopy(pCall, &pvah);
		else if(pMethod == m_pCOProject->m_pObject_else)
		{
			SetError(&Error::ELSE_MUST_BE_CHILD_OF_IF, pMethod);
			return false;
		}
		else if(pMethod == m_pCOProject->m_pObject_throw)
			return CompileThrow(pCall, &pvah);
		else
		{
			char* szError = (char*)alloca(strlen(pCall->GetMethod()->GetName()) + 10);
			strcpy(szError, "Object.");
			strcat(szError, pCall->GetMethod()->GetName());
			SetError(&Error::METHOD_NOT_FOUND, pCall, szError);
			return false;
		}
	}
	else if(pClass == m_pCOProject->m_pInteger)
	{
		const char* pName = pMethod->GetName();
		if(pMethod == m_pCOProject->m_pInteger_increment)
			return CompileIncrement(pCall, &pvah, false);
		else if(pMethod == m_pCOProject->m_pInteger_decrement)
			return CompileIncrement(pCall, &pvah, true);
		else if(pMethod == m_pCOProject->m_pInteger_new)
			return CompileAllocate(pCall, &pvah, pSourceMethod, pSymbolInstr);
		else if(pMethod == m_pCOProject->m_pInteger_add)
			return CompileArithInstruction(pCall, &pvah, TVA_ADD);
		else if(pMethod == m_pCOProject->m_pInteger_subtract)
			return CompileArithInstruction(pCall, &pvah, TVA_SUBTRACT);
		else if(pMethod == m_pCOProject->m_pInteger_multiply)
			return CompileArithInstruction(pCall, &pvah, TVA_MULTIPLY);
		else if(pMethod == m_pCOProject->m_pInteger_divide)
			return CompileArithInstruction(pCall, &pvah, TVA_DIVIDE);
		else if(pMethod == m_pCOProject->m_pInteger_modulus)
			return CompileArithInstruction(pCall, &pvah, TVA_MODULUS);
		else if(pMethod == m_pCOProject->m_pInteger_and)
			return CompileArithInstruction(pCall, &pvah, TVA_AND);
		else if(pMethod == m_pCOProject->m_pInteger_or)
			return CompileArithInstruction(pCall, &pvah, TVA_OR);
		else if(pMethod == m_pCOProject->m_pInteger_xor)
			return CompileArithInstruction(pCall, &pvah, TVA_XOR);
		else if(pMethod == m_pCOProject->m_pInteger_shiftLeft)
			return CompileArithInstruction(pCall, &pvah, TVA_SHIFTLEFT);
		else if(pMethod == m_pCOProject->m_pInteger_shiftRight)
			return CompileArithInstruction(pCall, &pvah, TVA_SHIFTRIGHT);
		else if(pMethod == m_pCOProject->m_pInteger_invert)
			return CompileInvert(pCall, &pvah);
		else
		{
			char* szError = (char*)alloca(strlen(pCall->GetMethod()->GetName()) + 10);
			strcpy(szError, "Integer.");
			strcat(szError, pCall->GetMethod()->GetName());
			SetError(&Error::METHOD_NOT_FOUND, pCall);
			return false;
		}
	}
	else
	{
		GAssert(false, "This built in type has no methods");
		return false;
	}
}

