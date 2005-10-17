/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COINTERFACE_H__
#define __COINTERFACE_H__

#include "CodeObject.h"
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GQueue.h"
#include "Type.h"

class COMethodDecl;
class GParser;
class COVariable;
class GPointerArray;
class COProject;
class GQueue;
class EMethodSignature;

class COInterface : public COType
{
protected:
	GPointerArray* m_pMethodDecls;

public:
	COInterface(int nLine, int nCol, int nWid, const char* szName, COFile* pFile, COProject* pCOProject);
	virtual ~COInterface();

	virtual TypeType GetTypeType() { return TT_INTERFACE; }

	int GetMethodDeclCount();
	void AddMethodDecl(COMethodDecl* pMethodDecl);
	COMethodDecl* GetMethodDecl(int n);
	int FindMethodDecl(COMethodDecl* pMethodSig);
	int FindMethodDecl(EMethodSignature* pSignature);
	COMethodDecl* FindMethodDecl(int* pnOutIndex, const char* szName, GPointerArray* pParams, ErrorStruct** ppError, int* pErrorParam, COProject* pProject);

	void LoadMethodDecls(GXMLTag* pTag, COProject* pCOProject, bool bPartial);
	GXMLTag* SaveToXML();
	virtual GXMLTag* ToXMLForLibrary(GCompiler* pCompiler, bool bImport);
	GXMLTag* ToXMLForImplementationInLibrary();
	void SaveToClassicSyntax(GQueue* pQ);
	void ReplaceType(COType* pOld, COType* pNew);
};




class COMachineClass : public COInterface
{
public:
	COMachineClass(int nLine, int nCol, int nWid, const char* szName, COFile* pFile, COProject* pCOProject)
		: COInterface(nLine, nCol, nWid, szName, pFile, pCOProject)
	{
	}

	virtual ~COMachineClass()
	{
	}

	virtual TypeType GetTypeType() { return TT_MACHINE; }
};

#endif // __COINTERFACE_H__
