/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GCOMPILER_H__
#define __GCOMPILER_H__

#include "../CodeObjects/Expression.h"

class CodeObject;
class COProject;
class COVariable;
class COClass;
class Library;
class GCompiler;
class GXMLTag;
class GXMLAttribute;
class GPointerArray;
class GIntArray;
class EInstrArrayBuilder;
class COInstruction;
class COCall;
class COFile;
class COInterface;
class COMethodCall;
class COExpression;
class COType;
class COOperator;
class EInstrArray;
class ParamVarArrayHolder;
class COMethodDecl;

class GCompiler
{
friend class EvalExprResult;
friend class COCall;
public:
	COProject* m_pCOProject;
	GXMLTag* m_pCurrentClassTag;

protected:
	// Simulated Stack
	COVariable* m_pLocalBasePointer;
	COVariable* m_pBasePointer;
	COVariable* m_pInstructionPointer;
	COVariable* m_pStackLayerVar;

	// The XML we're producing
	GXMLTag* m_pLibraryTag;
	GXMLTag* m_pStringTableTag;

	// Import stuff
	GHashTable* m_pExternallyCalledMethods;
	GHashTable* m_pExternallyReferencedTypes;
	GConstStringHashTable* m_pAlreadyImportedTypes;

	// Misc Members
	EInstrArrayBuilder* m_pEInstrArrayBuilder;
	int m_nBasePointer;
	int m_nStackSizeAtCall;
	GPointerArray* m_pVariables;
	GIntArray* m_pVarOwned;
	COClass* m_pCompilerInternalClass;
	bool m_bSymbolMode;
	int m_nUniqueID;
	CodeObject* m_pObjectWithError;
	CompileError* m_pErrorHolder;
	COFile* m_pCurrentFile;

	enum TwoVarArithmetic
	{
		TVA_ADD,
		TVA_SUBTRACT,
		TVA_MULTIPLY,
		TVA_DIVIDE,
		TVA_MODULUS,
		TVA_AND,
		TVA_OR,
		TVA_XOR,
		TVA_INVERT,
		TVA_SHIFTLEFT,
		TVA_SHIFTRIGHT,
	};

public:
	GCompiler(COProject* pCOProject, CompileError* pErrorHolder);
	virtual ~GCompiler();

	// This takes a COProject and returns a Library.  You must delete
	// the library when you're done with it.  On error it returns NULL.
	Library* Compile(bool bLibraryOwnsProject);
	EInstrArray* PartialCompileMethod(COMethod* pMethod);
 


	bool CompileMethodStart(COMethod* pMethod);
	bool CompileMethodParameter(COVariable* pVariable);
	bool CompileMethodPostParameters();
	EInstrArray* CompileMethodFinish(COMethod* pMethod);
	bool CompileCallStart(COCall* pCall, COInstruction* pSymbolInstr);
	bool CompileCallParameter(COVariable* pVariable, int nParam, COInstruction* pSymbolInstr, int* pnOutOffset);
	bool CompileMakeTheCall(COCall* pCall, COVariable* pCatcher, COInstruction* pSymbolInstr, int nThisOffset);
	bool CompileCallFinish(COCall* pCall, COInstruction* pSymbolInstr);
	bool CompileSetDeclFromParam(COVariable* pDest, int nParam, COInstruction* pSymbolInstr, COType* pCastType);
	bool CompileClassStart(COClass* pClass);
	bool CompileInterface(COInterface* pInterface);
	COVariable* GetConstIntVar(int nValue, COInstruction* pSymbolInstr);
	COVariable* GetConstFloatVar(double dValue, COInstruction* pSymbolInstr);
	COVariable* GetConstStringVar(const char* szValue, COInstruction* pSymbolInstr);
	bool AsmCmdDecl(COVariable* pVariable, bool bTakeOwnership, COInstruction* pSymbolInstr);
	void GetUniqueID(char* szBuff); // pBuff should point to a buffer of size 64
	bool CompileCallToBuiltInMethod(COMethodCall* pCall, COMethod* pMethod, COInstruction* pSymbolInstr);
	bool CheckParameters(COCall* pCall);
	void SetError(ErrorStruct* pError, CodeObject* pCodeObject);
	void SetError(ErrorStruct* pError, CodeObject* pCodeObject, const char* szParam1);
	void CheckError();
	bool CompileParams(COCall* pCall, ParamVarArrayHolder* pParamVarHolders);
	void SetCurrentFile(COFile* pFile);
	bool FindVarOnStack(int* pnOutOffset, COVariable* pVariable, COInstruction* pObjectWithError);
	bool PushStackVariable(COVariable* pVariable, bool bTakeOwnership);
	void AddInstr(GVMInstrPointer pMeth, COInstruction* pInstruction);
	void AddParam(int nParam);
	void AddImportMethod(COMethodDecl* pMethod);
	void AddImportType(COType* pType);

protected:
	bool AsmCmdSetMember(COCall* pCall, COVariable* pDest, COVariable* pSource, COVariable* pMember);
	bool MakeSureNullIsDefined(COInstruction* pSymbolInstr);
	bool CompileArithInstruction(COCall* pCall, ParamVarArrayHolder* pParams, TwoVarArithmetic eOp);
	bool CompileIncrement(COCall* pCall, ParamVarArrayHolder* pParams, bool bDecrement);
	bool CompileInvert(COCall* pCall, ParamVarArrayHolder* pParams);
	bool CompileAllocate(COCall* pCall, ParamVarArrayHolder* pParams, COMethod* pMethod, COInstruction* pSymbolInstr);
	bool CompileIf(COCall* pCall, ParamVarArrayHolder* pParams, COMethod* pSourceMethod);
	bool CompileWhile(COCall* pCall, ParamVarArrayHolder* pParams, COMethod* pSourceMethod);
	bool CompileSet(COCall* pCall, ParamVarArrayHolder* pParams);
	bool CompileCopy(COCall* pCall, ParamVarArrayHolder* pParams);
	bool CompileThrow(COCall* pCall, ParamVarArrayHolder* pParams);
	bool CompileReturn(COCall* pCall, ParamVarArrayHolder* pParams);
	bool GetZeroParams(COCall* pCall);
	bool PrepareForParam(COExpression* pParam, COCall* pCall);
	bool CheckCast(COType* pSourceType, COType* pDestType, bool* pbNeedCast, CodeObject* pCodeObject);
	bool IsVariableDefined(COVariable* pVariable);
	bool PopStackVariable(COVariable** ppVariable, CodeObject* pObjForError);
	void ClearStack();
	bool LinkMethod(GXMLTag* pMethodTag);
	bool AddStartScope(COInstruction* pSymbolInstr);
	bool AddEndScope(COInstruction* pSymbolInstr);
	bool DoImporting();
	int AddConstantString(const char* szValue);
	void AjustIDInDataAttr(GXMLAttribute* pPrevAttr, GXMLAttribute* pCurrAttr, int nPos, int nNewValue);
	COMethod* FindMethodByID(GXMLTag* pLibraryTag, int nID);
	const char* FindCallBackByID(COFile* pFile, GXMLTag* pLibraryTag, int nID);
	bool ImportExternalMethod(Library* pExternalLibrary, int nMethodID);
	GXMLTag* ImportExternalType(Library* pExternalLibrary, GXMLTag* pExternalTypeTag);
	GXMLTag* ImportExternalType(COType* pCOType);
	bool CompileBegin();
	bool CompileFinish();
	void CompileFailed();
	int GetCurrentOffset();
	bool CompileProject();
	void ImportMethods();
};



#endif // __GCOMPILER_H__
