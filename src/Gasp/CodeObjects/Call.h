/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COCALL_H__
#define __COCALL_H__

#include "Instruction.h"

class COProject;
class COMethod;
class COExpression;
class COExpressionArray;
class COMethodDecl;
class COInterface;
class COType;
class COInstrArray;
class COClass;


// A Call is an instruction that calls another method, procedure, or interface
class COCall : public COInstruction
{
public:
	enum CallType
	{
		CT_METHOD,
		CT_INTERFACE,
		CT_VIRTUAL,
	};

protected:
	COExpression* m_pCatcher;
	COExpressionArray* m_pParameters;

public:
	COCall(int nLine, int nCol, int nWid, COInstrArray* pParent, COExpressionArray* pParameters); // takes ownership of pParameters
	virtual ~COCall();

	void SetParameters(COExpressionArray* pParameters); // takes ownership of pParameters
	COExpressionArray* DropParameters();
	virtual InstructionType GetInstructionType() { return IT_CALL; }
	virtual CallType GetCallType() = 0;
	virtual int GetTargetParamCount() = 0;
	virtual COVariable* GetTargetParam(int n) = 0;
	virtual void SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay) = 0;
	virtual COType* GetTargetType() = 0;
	int GetParamCount();
	COExpression* GetParam(int n);
	COExpression* GetCatcher() { return m_pCatcher; }

	// takes ownership of pCatcher
	void SetCatcher(COExpression* pCatcher);

	virtual bool CanHaveChildren() = 0;
	virtual COVariable* FindVariable(const char* pName, int nLength);
	static COCall* FromXML(GXMLTag* pTag, COInstrArray* pParent, COProject* pCOProject, bool bPartial, int* pnInstructionIndex);
	virtual bool Compile(GCompiler* pCompiler, COMethod* pMethod, COInstruction* pSymbolInstr); // todo: this shouldn't be virtual (or the child classes should override it and it should be abstract)

	// WARNING: this method takes ownership of pParamList, so after you pass it in don't delete it!  todo: don't do this
	static bool CompileImplicitCall(GCompiler* pCompiler, COType* pType, EMethodSignature* pSig, COExpressionArray* pParamList, COInstruction* pSymbolInstr, COInstrArray* pParent);

protected:
	static bool LoadParameters(GXMLTag* pTag, Holder<COExpressionArray*>* pParamListHolder, bool bCanHaveChildren, COInstrArray* pParent, GXMLTag** ppInstructionsTag, COProject* pCOProject, bool bPartial);
	bool SymbolModeCompile(GCompiler* pCompiler, COMethod* pMethod, COInstruction* pSymbolInstr);
};






// for calling methods or procedures
class COMethodCall : public COCall
{
friend class COCall;
protected:
	COMethod* m_pMethod;

public:
	COMethodCall(int nLine, int nCol, int nWid, COMethod* pTarget, COInstrArray* pParent, COExpressionArray* pParameters);
	virtual ~COMethodCall();

	virtual CallType GetCallType() { return CT_METHOD; }
	virtual int GetTargetParamCount();
	virtual COVariable* GetTargetParam(int n);
	virtual GXMLTag* SaveToXML(COInstrArray* pParent);
	virtual void SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay = false);
	COMethod* GetMethod() { return m_pMethod; }
	virtual bool CanHaveChildren() { return false; }
	virtual COType* GetTargetType();

protected:
	// This takes ownership of pParameters and pChildInstructions
	bool SetMethod(COMethod* pMethod, COExpressionArray* pParameters);
};





// This is a special class for "If" and "While" instructions
class COMethodCallThatCanHaveChildren : public COMethodCall
{
public:
	COInstrArray* m_pInstrArray;

	COMethodCallThatCanHaveChildren(int nLine, int nCol, int nWid, COMethod* pTarget, COInstrArray* pParent, COExpressionArray* pParameters);
	virtual ~COMethodCallThatCanHaveChildren();

	virtual bool CanHaveChildren() { return true; }
	virtual COInstrArray* GetChildInstructions() { return m_pInstrArray; }
	virtual GXMLTag* SaveToXML(COInstrArray* pParent);
	virtual void SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay = false);
	virtual COInstruction* FindInstruction(int nIndex);
};


class COVirtualCall : public COCall
{
protected:
	COClass* m_pClass;
	int m_nVirtualTableIndex;
	COMethodDecl* m_pMethodDecl;

public:
	COVirtualCall(int nLine, int nCol, int nWid, COClass* pClass, int nVirtualTableIndex, COMethodDecl* pMethodDecl, COInstrArray* pParent, COExpressionArray* pParameters);
	virtual ~COVirtualCall();

	virtual CallType GetCallType() { return CT_VIRTUAL; }
	virtual bool CanHaveChildren() { return false; }
	virtual int GetTargetParamCount();
	virtual COVariable* GetTargetParam(int n);
	virtual GXMLTag* SaveToXML(COInstrArray* pParent);
	virtual void SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay = false);
	virtual COType* GetTargetType();

	int GetVirtualTableIndex() { return m_nVirtualTableIndex; }
};


class COInterfaceCall : public COCall
{
protected:
	COInterface* m_pInterface;
	int m_nMethodDeclIndex;

public:
	COInterfaceCall(int nLine, int nCol, int nWid, COInterface* pInterface, int nMethodDeclIndex, COInstrArray* pParent, COExpressionArray* pParameters);
	virtual ~COInterfaceCall();

	virtual CallType GetCallType() { return CT_INTERFACE; }
	virtual bool CanHaveChildren() { return false; }
	COMethodDecl* GetMethodDecl();
	virtual int GetTargetParamCount();
	virtual COVariable* GetTargetParam(int n);
	virtual GXMLTag* SaveToXML(COInstrArray* pParent);
	virtual void SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay = false);
	virtual COType* GetTargetType();
};

#endif // __COCALL_H__
