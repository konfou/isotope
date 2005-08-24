/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COMETHOD_H__
#define __COMETHOD_H__

#include "Scope.h"
#include "../Engine/TagNames.h"
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GArray.h"

class COProject;
class COClass;
class COType;
class COInstruction;
class GQueue;
class COVariable;
class COInstrArray;
class EInstrArray;
class GCompilerBase;
class ClassicSyntax;

class COMethodDecl : public COScope
{
protected:
	int m_nID;
	char* m_szName;
	GPointerArray* m_pParamArray;
	COType* m_pType;
	EMethodSignature* m_pSignature;
	bool m_bStatic;

public:
	COMethodDecl(int nLine, int nCol, int nWid, const char* szName, COType* pType, bool bStatic, COProject* pProject);
	virtual ~COMethodDecl();

	const char* GetName() { return m_szName; }
	void SetName(const char* szName) { GAssert(szName, "Must have a valid name"); delete(m_szName); m_szName = new char[strlen(szName) + 1]; strcpy(m_szName, szName); }
	int GetID() { return m_nID; }

	// Methods that manage Parameters
	int GetParameterCount() { if(!m_pParamArray) return 0; return m_pParamArray->GetSize(); }
	COVariable* GetParameter(int n) { GAssert(m_pParamArray, "No params to get"); return (COVariable*)m_pParamArray->GetPointer(n); }
	void AddParameter(COVariable* pParam) { if(!m_pParamArray) m_pParamArray = new GPointerArray(8); m_pParamArray->AddPointer(pParam); }
	void SetParameter(int n, COVariable* pParam) { m_pParamArray->SetPointer(n, pParam); }

	bool IsStatic() { return m_bStatic; }
	virtual ScopeType GetScopeType() { return ST_METHOD; }
	COType* GetType() { return m_pType; }
	void LoadAllParams(COType* pThisType, GXMLTag* pTag, char cModifier, COProject* pCOProject, bool bPartial);
	virtual GXMLTag* SaveToXML(GCompiler* pCompiler); // pCompiler is an optional parameter used only when compiling to an XLIB
	void SaveToClassicSyntax(GQueue* pQ);
	COVariable* FindVariable(const char* pName, int nLength);
	bool CheckParams(GPointerArray* pParams, ErrorStruct** ppError, int* pnErrorParam, COProject* pProject);
	EMethodSignature* GetSignature();
	Library* GetLibrary();
	bool Compile(GCompiler* pCompiler);

protected:
	void SetSignature(EMethodSignature* pSig); // takes ownership of pSig
	virtual bool CompileInstructions(GXMLTag* pTag, GCompiler* pCompiler);
};






class COMethod : public COMethodDecl
{
protected:
	COInstrArray* m_pInstrArray;

public:
	COMethod(int nLine, int nCol, int nWid, const char* szName, COClass* pClass, bool bStatic, COProject* pCOProject);
	virtual ~COMethod();

	// Methods that manage the properties of the method
	COClass* GetClass() { return (COClass*)m_pType; }
	virtual const char* GetFilename();

	// Methods that manage Instructions
	COInstrArray* GetInstructions() { return m_pInstrArray; }

	// Methods for loading/saving
	virtual GXMLTag* SaveToXML(GCompiler* pCompiler); // pCompiler is an optional parameter used only when compiling to an XLIB
	void SaveToClassicSyntax(GQueue* pQ);
	static void FromClassicSyntax(ClassicSyntax* pParser, bool bStatic);

	// For methods in xlib files, this returns the EMethod that corresponds to it.  This doesn't make sense for methods in source files
	EMethod* GetEMethod();

	EInstrArray* Compile2(GCompiler* pCompiler);

	COInstruction* FindInstruction(int index);

protected:
	bool ProduceSymbols(EInstrArray* pEInstrArray, COProject* pProject);
	virtual bool CompileInstructions(GXMLTag* pTag, GCompiler* pCompiler);
};


#endif // __COMETHOD_H__
