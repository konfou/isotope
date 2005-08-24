/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COCLASS_H__
#define __COCLASS_H__

#include "Type.h"
#include "Method.h"

class COProject;
class COFile;
class COInterface;
class COVariable;
class COConstant;
class COMethodDecl;
class COMethod;
class GQueue;
class GCompilerBase;

class COClass : public COType
{
protected:
	COVariable* m_pVariable;
	COClass* m_pParent;
	GPointerArray* m_pInterfaceRefs;
	GPointerArray* m_pMembers;
	GPointerArray* m_pConstants;
	GPointerArray* m_pMethods;
	GPointerArray* m_pProcedures;
	char* m_szSource;

public:
	COClass(int nLine, int nCol, int nWid, const char* szName, COClass* pParent, COFile* pFile, const char* szSource, COProject* pCOProject);
	virtual ~COClass();

	const char* GetSource() { return m_szSource; }
	virtual TypeType GetTypeType() { return TT_CLASS; }

	COClass* GetParent() { return m_pParent; }
	void SetParent(COClass* pParent) { m_pParent = pParent; }

	int GetInterfaceCount();
	void AddInterfaceRef(COInterface* pInterface);
	COInterface* GetInterface(int nIndex);

	int GetExtendedMemberCount();
	COVariable* GetExtendedMember(int n);
	void AddMember(COVariable* pMember);

	int GetTotalMemberCount()
	{
		if(m_pParent)
			return m_pParent->GetTotalMemberCount() + GetExtendedMemberCount();
		else
			return GetExtendedMemberCount();
	}

	int GetConstantCount();
	COConstant* GetConstant(int n);
	void AddConstant(COConstant* pConst);

	int GetMethodCount();
	COMethod* GetMethod(int n);
	void AddMethod(COMethod* pMethod);

	int GetProcedureCount();
	void AddProcedure(COMethod* pProcedure);
	COMethod* GetProcedure(int n);

	void LoadMembers(GXMLTag* pTag, COProject* pCOProject, bool bPartial);
	void LoadAllMethodDefinitions(GXMLTag* pTag, COProject* pCOProject, bool bPartial);
	void LoadAllInstructions(GXMLTag* pTag, COProject* pCOProject, bool bPartial);
	GXMLTag* SaveToXML();
	virtual GXMLTag* ToXMLForLibrary(GCompiler* pCompiler);
	void SaveToClassicSyntax(GQueue* pQ);
	static void FromClassicSyntax(ClassicSyntax* pParser);

	//int FindImplementingMethod(COMethodDecl* pMethodDecl);
	COMethod* FindMethod(EMethodSignature* pSignature);
	COMethod* FindMethod(const char* szName, GPointerArray* pParams, ErrorStruct** ppError, int* pErrorParam, COProject* pProject);
	COMethodDecl* FindVirtualTableIndex(int* pOutIndex, const char* szName, GPointerArray* pParams, ErrorStruct** ppError, int* pnErrorParam, COProject* pProject);

	bool DoesInherritFrom(COClass* pClass);
	bool DoesImplement(COInterface* pInterface);
	COVariable* FindMember(const char* szMemberName);
	COVariable* FindMember(const char* pMemberName, int nLength);
	int FindMember(COVariable* pMember);
	COConstant* FindConstant(const char* pName, int nLen);
	COConstant* FindConstant(const char* szConstantName);
	int FindConstant(COConstant* pConstant);
	virtual const char* GetFilename();
	COVariable* GetVariable(COProject* pProject);

	bool Compile(GCompiler* pCompiler);
	void SetVariable(COVariable* pVariable); // takes ownership
};

#endif // __COCLASS_H__
