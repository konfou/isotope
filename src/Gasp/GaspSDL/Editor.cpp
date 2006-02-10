/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Editor.h"
#include "../../GClasses/GTime.h"
#include "../../GClasses/GWidgets.h"
#include "../CodeObjects/Project.h"
#include "../CodeObjects/FileSet.h"
#include "../CodeObjects/File.h"
#include "../CodeObjects/Class.h"
#include "../CodeObjects/Variable.h"
#include "../CodeObjects/Constant.h"
#include "../CodeObjects/Interface.h"
#include "../CodeObjects/InstrArray.h"
#include "../CodeObjects/Call.h"
#include "../CodeObjects/Block.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GFile.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GThread.h"
#include <wchar.h>

void EditFile(const char* szFilename, const char* szAppPath, ErrorHandler* pErrorHandler)
{
	// Parse the script into a project
	COProject proj("*bogus*");
	if(!proj.LoadLibraries(szAppPath, pErrorHandler))
		return;
	int nSize;
	const char* szScript = GFile::LoadFileToBuffer(szFilename, &nSize);
	if(!szScript)
	{
		GlobalError eh;
		eh.SetError(&Error::FILE_NOT_FOUND);
		eh.SetParam1(szFilename);
		pErrorHandler->OnError(&eh);
		return;
	}
	if(!proj.LoadSources(&szScript, &szFilename, 1, pErrorHandler))
		return;

	// Show the editor
	EditorController ec(&proj);
	ec.Run();
}

// ---------------------------------------------------------------------


class EditorList
{
protected:
	GWidgetListBox* m_pListBox;
	int m_nStoredSelection;
	EditorController* m_pController;

public:
	EditorList(EditorController* pController)
	{
		m_pController = pController;
		m_pListBox = new GWidgetListBox(pController->GetDialog(), 0, 0, 0, 0);
		m_nStoredSelection = 0;
	}

	virtual ~EditorList() {}

	virtual void Update() = 0;
	virtual EditorList* MakeChildList(int n) = 0;
	virtual bool OnChar(char c) { return false; }
	virtual int OnSelect() { return 0; }
	virtual int OnAccept() { return 0; }
	GWidgetListBox* GetListBox() { return m_pListBox; }
	void SetStoredSelection(int n) { m_nStoredSelection = n; }
	int GetStoredSelection() { return m_nStoredSelection; }
};







class EditorListConfirm : public EditorList
{
protected:
	EditorList* m_pList;

public:
	EditorListConfirm(EditorController* pController, EditorList* pList)
		: EditorList(pController)
	{
		m_pList = pList;
		m_pListBox->SetBaseColor(GWidgetListBox::red);
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		new GWidgetListBoxItem(m_pListBox, L"Confirm");
	}

	virtual EditorList* MakeChildList(int n)
	{
		return NULL;
	}

	virtual int OnSelect()
	{
		return m_pList->OnAccept() + 1;
	}
};





#define MAX_NAME_LEN 128

class EditorListEnterName : public EditorList
{
protected:
	EditorList* m_pList;
	wchar_t m_wszName[MAX_NAME_LEN];
	int m_nLen;

public:
	EditorListEnterName(EditorController* pController, EditorList* pList)
		: EditorList(pController)
	{
		m_pList = pList;
		m_pListBox->SetBaseColor(GWidgetListBox::yellow);
		wcscpy(m_wszName, L"");
		m_nLen = 0;
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		new GWidgetListBoxItem(m_pListBox, m_wszName);
	}

	virtual EditorList* MakeChildList(int n)
	{
		return new EditorListConfirm(m_pController, this);
	}

	virtual bool OnChar(char c)
	{
		if(m_nLen >= MAX_NAME_LEN - 1)
			return false;
		m_wszName[m_nLen++] = (wchar_t)c;
		m_wszName[m_nLen] = L'\0';
		Update();
		return true;
	}

	virtual int OnAccept()
	{
		return m_pList->OnAccept() + 1;
	}
};







class EditorListArguments : public EditorList
{
protected:
	COCall* m_pCall;

public:
	EditorListArguments(EditorController* pController, COCall* pCall)
		: EditorList(pController)
	{
		m_pCall = pCall;
		Update();
	}

	virtual ~EditorListArguments()
	{
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int nCount = m_pCall->GetParamCount();
		GQueue q;
		GString s;
		int n;
		for(n = 0; n < nCount; n++)
		{
			COExpression* pParam = m_pCall->GetParam(n);
			pParam->SaveToClassicSyntax(&q);
			Holder<char*> hParam(q.DumpToString());
			s.Copy(hParam.Get());
			new GWidgetListBoxItem(m_pListBox, s.GetString());
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		return NULL; // todo: fix me
	}
};







class EditorListAddProcCallTypeRef : public EditorList
{
public:
	EditorListAddProcCallTypeRef(EditorController* pController)
		: EditorList(pController)
	{
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		COProject* pProject = m_pController->GetProject();
		int nCount = pProject->CountTypes();
		int n;
		GString s;
		for(n = 0; n < nCount; n++)
		{
			COType* pType = pProject->GetType(n);
			s.Copy(pType->GetName());
			new GWidgetListBoxItem(m_pListBox, s.GetString());
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		return NULL; // todo: fix me
	}
};






const wchar_t* g_editorListInstructionItems[] =
{
	L"Arguments",
	L"Add Method Call",
	L"Add Proc Call",
	L"Add Block",
	L"Delete",
};

class EditorListInstruction : public EditorList
{
protected:
	COInstrArray* m_pInstrSet;
	int m_nInstr;
	bool m_bArgs;
	bool m_bDelete;

public:
	EditorListInstruction(EditorController* pController, COInstrArray* pInstrSet, int nInstr, bool bArgs, bool bDelete)
		: EditorList(pController)
	{
		m_pInstrSet = pInstrSet;
		m_nInstr = nInstr;
		m_pListBox->SetBaseColor(GWidgetListBox::green);
		m_bArgs = bArgs;
		m_bDelete = bDelete;
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int n;
		for(n = 0; n < sizeof(g_editorListInstructionItems) / sizeof(const char*); n++)
		{
			if(n == 0 && !m_bArgs)
				continue;
			if(n == 4 && !m_bDelete)
				continue;
			new GWidgetListBoxItem(m_pListBox, g_editorListInstructionItems[n]);
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		if(!m_bArgs)
			n++;
		switch(n)
		{
			case 0: //"Arguments",
				return new EditorListArguments(m_pController, (COCall*)m_pInstrSet->GetInstr(m_nInstr));
			case 1: //"Add Method Call",
				return NULL;
			case 2: //"Add Proc Call",
				return new EditorListAddProcCallTypeRef(m_pController);
			case 3: //"Add Block",
				return NULL;
			case 4: //"Delete",
				return new EditorListConfirm(m_pController, this);
			default:
				GAssert(false, "unexpected value");
		}
		return NULL;
	}

	virtual int OnAccept()
	{
/*		switch(n)
		{
			case 0: //"Arguments",
				return 0;
			case 1: //"Add Method Call",
				return 0;
			case 2: //"Add Proc Call",
				return 0;
			case 3: //"Add Block",
				return 0;
			case 4: //"Delete",
				{
					return 0;
				}
				break;
			default:
				GAssert(false, "unexpected value");
		}*/
		return 0;
	}
};







const wchar_t* g_editorListVariableItems[] =
{
	L"Change Type",
	L"Change Name",
	L"Jump to type",
	L"Add",
};

class EditorListVariable : public EditorList
{
protected:
	COVariable* m_pVariable;

public:
	EditorListVariable(EditorController* pController, COVariable* pVariable)
		: EditorList(pController)
	{
		m_pVariable = pVariable;
		m_pListBox->SetBaseColor(GWidgetListBox::green);
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int n;
		for(n = 0; n < sizeof(g_editorListVariableItems) / sizeof(const char*); n++)
		{
			if(!m_pVariable && n < 3)
				continue;
			new GWidgetListBoxItem(m_pListBox, g_editorListVariableItems[n]);
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		if(!m_pVariable)
			n += 3;
		switch(n)
		{
			case 0: // Change Type
				return NULL; // todo: fix me
			case 1: // Change Name
				return NULL; // todo: fix me
			case 2: // Jump to type
				return NULL; // todo: fix me
			case 3: // Add
				return NULL; // todo: fix me
			default:
				GAssert(false, "unexpected value");
		}
		return NULL;
	}
};







class EditorListParameters : public EditorList
{
protected:
	COMethod* m_pMethod;

public:
	EditorListParameters(EditorController* pController, COMethod* pMethod)
		: EditorList(pController)
	{
		m_pMethod = pMethod;
		Update();
	}

	virtual ~EditorListParameters()
	{
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int nCount = m_pMethod->GetParameterCount();
		GQueue q;
		int n;
		GString s;
		for(n = 0; n < nCount; n++)
		{
			COVariable* pVar = m_pMethod->GetParameter(n);
			pVar->SaveToClassicSyntax(&q);
			Holder<char*> hVar(q.DumpToString());
			s.Copy(hVar.Get());
			new GWidgetListBoxItem(m_pListBox, s.GetString());
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		return NULL; // todo: fix me
	}
};







class EditorListInstructions : public EditorList
{
protected:
	COInstrArray* m_pInstructions;
	int* m_pInstrMap;

public:
	EditorListInstructions(EditorController* pController, COInstrArray* pInstructions)
		: EditorList(pController)
	{
		m_pInstructions = pInstructions;
		int nMapSize = m_pInstructions->GetInstrCount() * 2 + 2;
		m_pInstrMap = new int[nMapSize];
		memset(m_pInstrMap, -1, sizeof(int) * nMapSize);
		GAssert(m_pInstrMap[0] == -1, "didn't set mem as expected");
		Update();
	}

	virtual ~EditorListInstructions()
	{
		delete(m_pInstrMap);
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int nCount = m_pInstructions->GetInstrCount();
		int n;
		GQueue q;
		GString s;
		for(n = 0; n < nCount; n++)
		{
			m_pInstrMap[m_pListBox->GetSize()] = n;
			COInstruction* pInstr = m_pInstructions->GetInstr(n);
			switch(pInstr->GetInstructionType())
			{
				case COInstruction::IT_CALL:
					{
						COCall* pCall = (COCall*)pInstr;
						pCall->SaveToClassicSyntax(&q, 0, true);
						Holder<char*> hInstr(q.DumpToString());
						s.Copy(hInstr.Get());
						new GWidgetListBoxItem(m_pListBox, s.GetString());
						if(pCall->CanHaveChildren())
							new GWidgetListBoxItem(m_pListBox, L"    { ... }");
					}
					break;

				case COInstruction::IT_BLOCK:
					s.Copy(((COBlock*)pInstr)->GetComment());
					new GWidgetListBoxItem(m_pListBox, s.GetString());
					new GWidgetListBoxItem(m_pListBox, L"    { ... }");
					break;
			}
		}
		m_pInstrMap[m_pListBox->GetSize()] = n;
		new GWidgetListBoxItem(m_pListBox, L"");
	}

	virtual EditorList* MakeChildList(int n)
	{
		int nInstr = m_pInstrMap[n];
		if(nInstr >= 0)
		{
			bool bArgs = false;
			bool bDelete = (nInstr < m_pInstructions->GetInstrCount());
			if(bDelete && m_pInstructions->GetInstr(nInstr)->GetInstructionType() == COInstruction::IT_CALL)
				bArgs = true;
			return new EditorListInstruction(m_pController, m_pInstructions, n, bArgs, bDelete);
		}
		else
		{
			GAssert(n > 0 && m_pInstrMap[n - 1] >= 0, "Problem with instruction map--only two lines per instruction are allowed here");
			COInstruction* pInstr = m_pInstructions->GetInstr(m_pInstrMap[n - 1]);
			if(pInstr->GetInstructionType() == COInstruction::IT_BLOCK)
                return new EditorListInstructions(m_pController, ((COBlock*)pInstr)->GetChildInstructions());
			else
			{
				GAssert(pInstr->GetInstructionType() == COInstruction::IT_CALL, "unexpected instruction type");
                return new EditorListInstructions(m_pController, ((COCall*)pInstr)->GetChildInstructions());
			}
		}
	}
};








const wchar_t* g_editorListMethodItems[] =
{
	L"Instructions",
	L"Parameters",
	L"Name",
	L"Add",
};

class EditorListMethod : public EditorList
{
protected:
	COMethod* m_pMethod;

public:
	EditorListMethod(EditorController* pController, COMethod* pMethod)
		: EditorList(pController)
	{
		m_pMethod = pMethod;
		m_pListBox->SetBaseColor(GWidgetListBox::green);
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int n;
		for(n = 0; n < sizeof(g_editorListMethodItems) / sizeof(const char*); n++)
		{
			if(!m_pMethod && n < 3)
				continue;
			new GWidgetListBoxItem(m_pListBox, g_editorListMethodItems[n]);
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		if(!m_pMethod)
			n += 3;
		switch(n)
		{
			case 0: // Instructions
				return new EditorListInstructions(m_pController, m_pMethod->GetInstructions());
			case 1: // Parameters
				return new EditorListParameters(m_pController, m_pMethod);
			case 2: // Name
				return NULL; // todo: fix me
			case 3: // Add
				return NULL; // todo: fix me
			default:
				GAssert(false, "unexpected value");
		}
		return NULL;
	}
};




class EditorListClassPart : public EditorList
{
protected:
	COClass* m_pClass;
	int m_nPart;

public:
	EditorListClassPart(EditorController* pController, COClass* pClass, int nPart)
		: EditorList(pController)
	{
		m_pClass = pClass;
		m_nPart = nPart;
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int nCount = GetCount();
		GString s;
		int n;
		for(n = 0; n < nCount; n++)
		{
			s.Copy(GetPartName(n));
			new GWidgetListBoxItem(m_pListBox, s.GetString());
		}
		new GWidgetListBoxItem(m_pListBox, L"");
	}

	virtual EditorList* MakeChildList(int n)
	{
		switch(m_nPart)
		{
			case 0: return new EditorListMethod(m_pController, (n < m_pClass->GetMethodCount() ? m_pClass->GetMethod(n) : NULL));
			case 1: return new EditorListMethod(m_pController, (n < m_pClass->GetProcedureCount() ? m_pClass->GetProcedure(n) : NULL));
			case 2: return new EditorListVariable(m_pController, (n < m_pClass->GetExtendedMemberCount() ? m_pClass->GetExtendedMember(n) : NULL));
/*			case 3: return m_pClass->GetInterfaceCount();
			case 4: return m_pClass->GetConstantCount();
			case 5: return m_pClass->GetConstantCount(); // todo: fix me
			case 6: return m_pClass->GetConstantCount(); // todo: fix me*/
		}
		GAssert(false, "unexpected case");
		return NULL;
	}

protected:
	int GetCount()
	{
		switch(m_nPart)
		{
			case 0: return m_pClass->GetMethodCount();
			case 1: return m_pClass->GetProcedureCount();
			case 2: return m_pClass->GetExtendedMemberCount();
			case 3: return m_pClass->GetInterfaceCount();
			case 4: return m_pClass->GetConstantCount();
			case 5: return m_pClass->GetConstantCount(); // todo: fix me
			case 6: return m_pClass->GetConstantCount(); // todo: fix me
		}
		GAssert(false, "unexpected case");
		return 0;
	}

	const char* GetPartName(int n)
	{
		switch(m_nPart)
		{
			case 0: return m_pClass->GetMethod(n)->GetName();
			case 1: return m_pClass->GetProcedure(n)->GetName();
			case 2: return m_pClass->GetExtendedMember(n)->GetName();
			case 3: return m_pClass->GetInterface(n)->GetName();
			case 4: return m_pClass->GetConstant(n)->GetName();
			case 5: return m_pClass->GetConstant(n)->GetName(); // todo: fix me
			case 6: return m_pClass->GetConstant(n)->GetName(); // todo: fix me
			default:
				GAssert(false, "unexpected case");
		}
		return NULL;
	}
};







const wchar_t* g_editorListClassItems[] =
{
	L"Methods",
	L"Procs",
	L"Variables (Extended)",
	L"Interfaces",
	L"Constants",
	L"Name",
	L"Parent",
	L"Add",
};

class EditorListClass : public EditorList
{
protected:
	COClass* m_pClass;
	COFile* m_pFile;

public:
	EditorListClass(EditorController* pController, COClass* pClass, COFile* pFile)
		: EditorList(pController)
	{
		m_pClass = pClass;
		m_pFile = pFile;
		m_pListBox->SetBaseColor(GWidgetListBox::green);
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int n;
		for(n = 0; n < sizeof(g_editorListClassItems) / sizeof(const char*); n++)
		{
			if(!m_pClass && n < 7)
				continue;
			new GWidgetListBoxItem(m_pListBox, g_editorListClassItems[n]);
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		if(!m_pClass)
			n += 7;
		switch(n)
		{
			case 0: // Methods
				return new EditorListClassPart(m_pController, m_pClass, n);
			case 1: // Procs
				return new EditorListClassPart(m_pController, m_pClass, n);
			case 2: // Variables (Extended)
				return new EditorListClassPart(m_pController, m_pClass, n);
			case 3: // Interfaces
				return new EditorListClassPart(m_pController, m_pClass, n);
			case 4: // Constants
				return new EditorListClassPart(m_pController, m_pClass, n);
			case 5: // Name
				return new EditorListClassPart(m_pController, m_pClass, n);
			case 6: // Parent
				return new EditorListClassPart(m_pController, m_pClass, n);
			case 7: // Add
				return new EditorListEnterName(m_pController, this);
			default:
				GAssert(false, "unexpected value");
		}
		return NULL;
	}

	virtual int OnAccept()
	{
		int n = m_nStoredSelection;
		if(!m_pClass)
			n += 7;
		switch(n)
		{
			case 0: // Methods
			case 1: // Procs
			case 2: // Variables (Extended)
			case 3: // Interfaces
			case 4: // Constants
			case 5: // Name
			case 6: // Parent
				break;
			case 7: // Add
				{
					COProject* pProject = m_pController->GetProject();
					COClass* pNewClass = new COClass(0, 0, 0, "newclass", pProject->m_pObject, m_pFile, NULL, pProject);
					m_pFile->AddClass(pNewClass);
				}
				return 1;
			default:
				GAssert(false, "unexpected value");
		}
		return 0;
	}
};



const wchar_t* g_editorListInterfaceItems[] =
{
	L"Methods",
	L"Add",
	L"Delete",
};

class EditorListInterface : public EditorList
{
protected:
	COInterface* m_pInterface;

public:
	EditorListInterface(EditorController* pController, COInterface* pInterface)
		: EditorList(pController)
	{
		m_pInterface = pInterface;
		m_pListBox->SetBaseColor(GWidgetListBox::green);
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int n;
		for(n = 0; n < sizeof(g_editorListInterfaceItems) / sizeof(const char*); n++)
			new GWidgetListBoxItem(m_pListBox, g_editorListInterfaceItems[n]);
	}

	virtual EditorList* MakeChildList(int n)
	{
		switch(n)
		{
			case 0: // Methods
				return NULL; // todo: fix me
			case 1: // Add
				return NULL; // todo: fix me
			case 2: // Delete
				return NULL; // todo: fix me
			default:
				GAssert(false, "unexpected value");
		}
		return NULL;
	}
};



class EditorListClasses : public EditorList
{
protected:
	COFileSet* m_pFileSet;

public:
	EditorListClasses(EditorController* pController, COFileSet* pFileSet)
		: EditorList(pController)
	{
		m_pFileSet = pFileSet;
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int nFileCount = m_pFileSet->GetFileCount();
		int n;
		GString s;
		for(n = 0; n < nFileCount; n++)
		{
			COFile* pFile = m_pFileSet->GetFile(n);
			if(pFile->GetFileType() == FT_XLIB)
				continue;
			int nClassCount = pFile->GetClassCount();
			int i;
			for(i = 0; i < nClassCount; i++)
			{
				COClass* pClass = pFile->GetClass(i);
				s.Copy(pClass->GetName());
				new GWidgetListBoxItem(m_pListBox, s.GetString());
			}
			new GWidgetListBoxItem(m_pListBox, L"");
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		COClass* pClass = GetClass(n);
		COFile* pFile = pClass ? pClass->GetFile() : NULL;
		if(!pFile || pFile->GetFileType() == FT_XLIB)
			pFile = GetSourceFile();
		return new EditorListClass(m_pController, pClass, pFile);
	}

protected:
	COClass* GetClass(int n)
	{
		int i;
		int nFileCount = m_pFileSet->GetFileCount();
		for(i = 0; i < nFileCount; i++)
		{
			COFile* pFile = m_pFileSet->GetFile(i);
			if(pFile->GetFileType() == FT_XLIB)
				continue;
			if(n >= pFile->GetClassCount())
				n -= pFile->GetClassCount();
			else
				return pFile->GetClass(n);
		}
		return NULL;
	}

	COFile* GetSourceFile()
	{
		int nFileCount = m_pFileSet->GetFileCount();
		int i;
		for(i = 0; i < nFileCount; i++)
		{
			COFile* pFile = m_pFileSet->GetFile(i);
			if(pFile->GetFileType() != FT_XLIB)
				return pFile;
		}
		return NULL;
	}
};






class EditorListInterfaces : public EditorList
{
protected:
	COFileSet* m_pFileSet;

public:
	EditorListInterfaces(EditorController* pController, COFileSet* pFileSet)
		: EditorList(pController)
	{
		m_pFileSet = pFileSet;
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int nFileCount = m_pFileSet->GetFileCount();
		int n;
		GString s;
		for(n = 0; n < nFileCount; n++)
		{
			COFile* pFile = m_pFileSet->GetFile(n);
			if(pFile->GetFileType() == FT_XLIB)
				continue;
			int nInterfaceCount = pFile->GetInterfaceCount();
			int i;
			for(i = 0; i < nInterfaceCount; i++)
			{
				COInterface* pInterface = pFile->GetInterface(i);
				s.Copy(pInterface->GetName());
				new GWidgetListBoxItem(m_pListBox, s.GetString());
			}			
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		return new EditorListInterface(m_pController, GetInterface(n));
	}

protected:
	COInterface* GetInterface(int n)
	{
		int i;
		int nFileCount = m_pFileSet->GetFileCount();
		for(i = 0; i < nFileCount; i++)
		{
			COFile* pFile = m_pFileSet->GetFile(i);
			if(pFile->GetFileType() == FT_XLIB)
				continue;
			if(n >= pFile->GetInterfaceCount())
				n -= pFile->GetInterfaceCount();
			else
				return pFile->GetInterface(n);
		}
		GAssert(false, "failed to find interface");
		return NULL;
	}
};






class EditorListMachineClasses : public EditorList
{
protected:
	COFileSet* m_pFileSet;

public:
	EditorListMachineClasses(EditorController* pController, COFileSet* pFileSet)
		: EditorList(pController)
	{
		m_pFileSet = pFileSet;
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int nFileCount = m_pFileSet->GetFileCount();
		int n;
		GString s;
		for(n = 0; n < nFileCount; n++)
		{
			COFile* pFile = m_pFileSet->GetFile(n);
			if(pFile->GetFileType() == FT_XLIB)
				continue;
			int nInterfaceCount = pFile->GetMachineClassCount();
			int i;
			for(i = 0; i < nInterfaceCount; i++)
			{
				COMachineClass* pInterface = pFile->GetMachineClass(i);
				s.Copy(pInterface->GetName());
				new GWidgetListBoxItem(m_pListBox, s.GetString());
			}			
		}
	}

	virtual EditorList* MakeChildList(int n)
	{
		return new EditorListInterface(m_pController, GetMachineClass(n));
	}

protected:
	COMachineClass* GetMachineClass(int n)
	{
		int i;
		int nFileCount = m_pFileSet->GetFileCount();
		for(i = 0; i < nFileCount; i++)
		{
			COFile* pFile = m_pFileSet->GetFile(i);
			if(pFile->GetFileType() == FT_XLIB)
				continue;
			if(n >= pFile->GetMachineClassCount())
				n -= pFile->GetMachineClassCount();
			else
				return pFile->GetMachineClass(n);
		}
		GAssert(false, "failed to find machine class");
		return NULL;
	}
};






const wchar_t* g_editorListProjectItems[] =
{
	L"Classes",
	L"Interfaces",
	L"MachineClasses",
};

class EditorListProject : public EditorList
{
protected:
	COProject* m_pProject;

public:
	EditorListProject(EditorController* pController, COProject* pProject)
		: EditorList(pController)
	{
		m_pProject = pProject;
		m_pListBox->SetBaseColor(GWidgetListBox::green);
		Update();
	}

	virtual void Update()
	{
		m_pListBox->Clear();
		int n;
		for(n = 0; n < sizeof(g_editorListProjectItems) / sizeof(const char*); n++)
			new GWidgetListBoxItem(m_pListBox, g_editorListProjectItems[n]);
	}

	virtual EditorList* MakeChildList(int n)
	{
		switch(n)
		{
			case 0:
				return new EditorListClasses(m_pController, m_pProject->GetSource());
			case 1:
				return new EditorListInterfaces(m_pController, m_pProject->GetSource());
			case 2:
				return new EditorListMachineClasses(m_pController, m_pProject->GetSource());
			default:
				GAssert(false, "unexpected value");
		}
		return NULL;
	}
};



// ---------------------------------------------------------------------

EditorView::EditorView(COProject* pModel)
{
	m_nScreenWidth = 800;
	m_nScreenHeight = 600;
	SetScreenSize(m_nScreenWidth, m_nScreenHeight);
	m_pDialog = new GWidgetDialog(800, 600, 0x00000044);
	m_nLists = 0;
	m_nCursorCol = 0;
	m_nCursorRow = 0;
}

EditorView::~EditorView()
{
	delete(m_pLists[0]);
	delete(m_pDialog);
}

GWidgetStyle* EditorView::GetWidgetStyle()
{
	return m_pDialog->GetStyle();
}

void EditorView::MakeRootList(EditorController* pController)
{
	GAssert(m_nLists == 0, "There's already a root list");
	m_pLists[0] = new EditorListProject(pController, pController->GetProject());
	m_pLists[0]->GetListBox()->SetSelection(0);
	m_nLists++;
	m_nCursorCol = 0;
	m_nCursorRow = 0;
	MakeNextListBox();
}

/*static*/ void EditorView::BlitImage(SDL_Surface* pScreen, int x, int y, GImage* pImage)
{
	if(pScreen->format->BytesPerPixel == 4)
	{
		// 32 bits per pixel
		GColor* pRGB = pImage->GetRGBQuads();
		int w = pImage->GetWidth();
		int h = pImage->GetHeight();
		int yy;
		Uint32* pPix;
		for(yy = 0; yy < h; yy++)
		{
			pPix = getPixMem32(pScreen, x, y);
			memcpy(pPix, &pRGB[yy * w], w * sizeof(GColor));
			y++;
		}
	}
	else
	{
		// 16 bits per pixel
		GAssert(pScreen->format->BytesPerPixel == 2, "Only 16 and 32 bit video modes are supported");
		int w = pImage->GetWidth();
		int h = pImage->GetHeight();
		int xx, yy, xxx;
		GColor colIn;
		Uint16* pPix;
		for(yy = 0; yy < h; yy++)
		{
			xxx = x;
			pPix = (Uint16*)pScreen->pixels + y * pScreen->pitch / 2 + x;
			for(xx = 0; xx < w; xx++)
			{
				colIn = pImage->GetPixel(xx, yy);
				*pPix = SDL_MapRGB(pScreen->format, gRed(colIn), gGreen(colIn), gBlue(colIn));
				xxx++;
				pPix++;
			}
			y++;
		}
	}
}

/*static*/ void EditorView::StretchClipAndBlitImage(SDL_Surface* pScreen, GRect* pDestRect, GRect* pClipRect, GImage* pImage)
{
	float fSourceDX =  (float)(pImage->GetWidth() - 1) / (float)(pDestRect->w - 1);
	GAssert((int)((pDestRect->w - 1) * fSourceDX) < (int)pImage->GetWidth(), "Extends past source image width");
	float fSourceDY = (float)(pImage->GetHeight() - 1) / (float)(pDestRect->h - 1);
	GAssert((int)((pDestRect->h - 1) * fSourceDY) < (int)pImage->GetHeight(), "Extends past source image height");
	float fSourceX = 0;
	float fSourceY = 0;
	int xStart = pDestRect->x;
	int yStart = pDestRect->y;
	if(pClipRect->x > xStart)
	{
		xStart = pClipRect->x;
		fSourceX = (pClipRect->x * pDestRect->x) * fSourceDX;
	}
	if(pClipRect->y > yStart)
	{
		yStart = pClipRect->y;
		fSourceY = (pClipRect->y * pDestRect->y) * fSourceDY;
	}
	int xEnd = MIN(pDestRect->x + pDestRect->w, pClipRect->x + pClipRect->w);
	int yEnd = MIN(pDestRect->y + pDestRect->h, pClipRect->y + pClipRect->h);
	int x, y;
	float fSX;
	if(pScreen->format->BytesPerPixel == 4)
	{
		// 32 bits per pixel
		for(y = yStart; y < yEnd; y++)
		{
			fSX = fSourceX;
			for(x = xStart; x < xEnd; x++)
			{
				*getPixMem32(pScreen, x, y) = pImage->GetPixel((int)fSX, (int)fSourceY);
				fSX += fSourceDX;
			}
			fSourceY += fSourceDY;
		}
	}
	else
	{
		// 16 bits per pixel
		GAssert(pScreen->format->BytesPerPixel == 2, "Only 16 and 32 bit video modes are supported");
		GColor colIn;
		for(y = yStart; y < yEnd; y++)
		{
			fSX = fSourceX;
			for(x = xStart; x < xEnd; x++)
			{
				colIn = pImage->GetPixel((int)fSX, (int)fSourceY);
				*getPixMem16(pScreen, x, y) = SDL_MapRGB(pScreen->format, gRed(colIn), gGreen(colIn), gBlue(colIn));
				fSX += fSourceDX;
			}
			fSourceY += fSourceDY;
		}
	}
}

void EditorView::AjustListPositions()
{
	GRect* pOldRect;
	GRect rNew;
	int n;
	int nRunningHeight = 0;
	for(n = 0; n < m_nLists; n++)
	{
		// Calculate new rect
		if(!m_pLists[n])
			continue;
		GWidgetListBox* pListBox = m_pLists[n]->GetListBox();
		rNew.w = EDITOR_VIEW_LIST_WIDTH;
		rNew.x = EDITOR_VIEW_LIST_WIDTH * n;
		if(m_nLists * EDITOR_VIEW_LIST_WIDTH > m_screenRect.w)
		{
			int nSquishedListCount = m_nLists - EDITOR_VIEW_MIN_FULL_SIZE_LISTS;
			int nSquishedWidth = (m_screenRect.w - (EDITOR_VIEW_MIN_FULL_SIZE_LISTS * EDITOR_VIEW_LIST_WIDTH)) / nSquishedListCount;
			if(m_nLists - n > EDITOR_VIEW_MIN_FULL_SIZE_LISTS)
			{
				rNew.w = nSquishedWidth;
				rNew.x = nSquishedWidth * n;
			}
			else
				rNew.x = nSquishedListCount * nSquishedWidth + (n - nSquishedListCount) * EDITOR_VIEW_LIST_WIDTH;
		}
		rNew.h = m_pDialog->GetStyle()->GetListBoxLineHeight() * pListBox->GetSize() + 2;
		if(rNew.h > m_screenRect.h)
			rNew.h = m_screenRect.h;
		rNew.y = 0;
		if(n > 0)
		{
			nRunningHeight += m_pLists[n - 1]->GetStoredSelection() * m_pDialog->GetStyle()->GetListBoxLineHeight();
			if(nRunningHeight + rNew.h > m_screenRect.h)
				nRunningHeight = 0;
			rNew.y = nRunningHeight;
		}

		// Resize if different
		pOldRect = pListBox->GetRect();
		if(memcmp(&rNew, pOldRect, sizeof(GRect)) == 0)
			continue;
		pListBox->SetSize(rNew.w, rNew.h);
		pListBox->SetPos(rNew.x, rNew.y);
	}
}

void EditorView::Update()
{
	AjustListPositions();

	// Lock the screen for direct access to the pixels
    SDL_Surface *pScreen = m_pScreen;
	if ( SDL_MUSTLOCK(pScreen) )
	{
		if ( SDL_LockSurface(pScreen) < 0 )
		{
			GAssert(false, SDL_GetError()); // failed to lock the surface
			return;
		}
	}

	// Clear the screen
	SDL_FillRect(pScreen, NULL/*&r*/, 0x000000);

	// Draw the lists
	GRect* pPosRect;
	GWidgetListBox* pListBox;
	GRect r;
	GImage* pImage;
	int n;
	for(n = 0; n < m_nLists; n++)
	{
		if(!m_pLists[n])
			continue;
		pListBox = m_pLists[n]->GetListBox();
		pPosRect = pListBox->GetRect();
		if(pPosRect->x >= 0)
		{
			pImage = pListBox->GetImage(&r);
			BlitImage(pScreen, m_screenRect.x + pPosRect->x, m_screenRect.y + pPosRect->y, pImage); // note that this ignores the clipping rect, but that's okay for now because GWidgetListbox doesn't use it
		}		
	}

	// Unlock the screen
	if ( SDL_MUSTLOCK(pScreen) )
		SDL_UnlockSurface(pScreen);

	// Update the whole screen
	SDL_UpdateRect(pScreen, m_screenRect.x, m_screenRect.y, m_screenRect.w, m_screenRect.h);
}

void EditorView::SetScreenSize(int x, int y)
{
	unsigned int flags = 
		SDL_HWSURFACE |
		SDL_ANYFORMAT |
		SDL_DOUBLEBUF;
	m_pScreen = SDL_SetVideoMode(x, y, 32, flags);
	if(!m_pScreen)
	{
		GAssert(false, SDL_GetError());
		throw "failed to create SDL screen";
	}
	m_screenRect.x = 5;
	m_screenRect.y = 5;
	m_screenRect.w = m_pScreen->w - 10;
	m_screenRect.h = m_pScreen->h - 10;
}

void EditorView::MakeScreenSmaller()
{
	m_nScreenWidth = (int)(m_nScreenWidth / 1.25);
	m_nScreenHeight = (int)(m_nScreenHeight / 1.25);
	SetScreenSize(m_nScreenWidth, m_nScreenHeight);
}

void EditorView::MakeScreenBigger()
{
	m_nScreenWidth = (int)(m_nScreenWidth * 1.25);
	m_nScreenHeight = (int)(m_nScreenHeight* 1.25);
	SetScreenSize(m_nScreenWidth, m_nScreenHeight);
}

void EditorView::MakeNextListBox()
{
	while(m_nLists > m_nCursorCol + 1)
		delete(m_pLists[--m_nLists]);
	m_pLists[m_nLists++] = m_pLists[m_nCursorCol]->MakeChildList(m_nCursorRow);
}

void EditorView::OnSelectionChange()
{
	EditorList* pList = m_pLists[m_nCursorCol];
	int nMoveLeft = pList->OnSelect();
	if(nMoveLeft > 0)
	{
		MoveCol(-nMoveLeft);
		return;
	}
	GWidgetListBox* pListBox = pList->GetListBox();
	pListBox->SetSelection(m_nCursorRow);
	MakeNextListBox();
}

void EditorView::MoveCol(int dx)
{
	int nNewCol = m_nCursorCol + dx;
	if(nNewCol < 0)
		nNewCol = 0;
	if(nNewCol >= m_nLists)
		nNewCol = m_nLists - 1;
	if(nNewCol == m_nCursorCol)
		return;
	GWidgetListBox* pOldListBox = m_pLists[m_nCursorCol]->GetListBox();
	pOldListBox->SetSelection(-1);
	m_nCursorCol = nNewCol;
	m_nCursorRow = m_pLists[nNewCol]->GetStoredSelection();
	OnSelectionChange();
}

void EditorView::MoveRow(int dy)
{
	EditorList* pList = m_pLists[m_nCursorCol];
	GWidgetListBox* pListBox = pList->GetListBox();
	int nListBoxSize = pListBox->GetSize();
	int nNewRow = m_nCursorRow + dy;
	if(nNewRow < 0)
		nNewRow = 0;
	if(nNewRow >= nListBoxSize)
		nNewRow = nListBoxSize - 1;
	if(nNewRow == m_nCursorRow)
		return;
	m_nCursorRow = nNewRow;
	pList->SetStoredSelection(m_nCursorRow);
	OnSelectionChange();
}

bool EditorView::OnChar(char c)
{
	EditorList* pList = m_pLists[m_nCursorCol];
	return pList->OnChar(c);
}

// ---------------------------------------------------------------------

EditorController::EditorController(COProject* pProject)
{
	m_bKeepRunning = true;
	m_pModel = pProject;
	m_pView = new EditorView(pProject);
	m_pView->MakeRootList(this);

	// Init the keyboard
	int n;
	for(n = 0; n < SDLK_LAST; n++)
		m_keyboard[n] = 0;

	m_mouse[1] = 0;
	m_mouse[2] = 0;
	m_mouse[3] = 0;
	m_mouseX = -1;
	m_mouseY = -1;
	m_eKeyState = Normal;
	m_lastPressedKey = SDLK_UNKNOWN;
}

EditorController::~EditorController()
{
	delete(m_pView);
}

void EditorController::Run()
{
	double timeOld = GTime::GetTime();
	double time;
	m_pView->Update();
	while(m_bKeepRunning)
	{
		time = GTime::GetTime();
		if(Update(time - timeOld))
			m_pView->Update();
		else
			GThread::sleep(0);
		timeOld = time;
	}
}

bool EditorController::Update(double dTimeDelta)
{
	// Check for events
	SDL_Event event;
	SDL_PollEvent(&event);
	bool bMouse = false;
	bool bKey = false;
	switch(event.type)
	{
		case SDL_KEYDOWN:
			m_keyboard[event.key.keysym.sym] = 1;
			bKey = true;
			break;

		case SDL_KEYUP:
			m_keyboard[event.key.keysym.sym] = 0;
			m_eKeyState = Normal;
			break;

		case SDL_MOUSEBUTTONDOWN:
			m_mouse[event.button.button] = 1;
			m_mouseX = event.button.x;
			m_mouseY = event.button.y;
			bMouse = true;
			break;

		case SDL_MOUSEBUTTONUP:
			m_mouse[event.button.button] = 0;
			bMouse = true;
			break;

		case SDL_QUIT:
			m_bKeepRunning = false;
			break;

		default:
			break;
	}

	if(bKey)
		HandleKeyPress(event.key.keysym.sym);
	else if(bMouse)
		HandleMouseClick();
	else if(m_keyboard[m_lastPressedKey])
	{
		switch(m_eKeyState)
		{
			case Normal:
				m_eKeyState = Holding;
				m_dKeyRepeatTimer = 0;
				return false; // don't bother updating the display
			case Holding:
				m_dKeyRepeatTimer += dTimeDelta;
				if(m_dKeyRepeatTimer >= KEY_REPEAT_DELAY)
				{
					m_dKeyRepeatTimer = 0;
					m_eKeyState = Repeating;
				}
				return false; // don't bother updating the display
			case Repeating:
				m_dKeyRepeatTimer += dTimeDelta;
				if(m_dKeyRepeatTimer > KEY_REPEAT_RATE)
				{
					m_dKeyRepeatTimer -= KEY_REPEAT_RATE;
					HandleKeyPress(m_lastPressedKey);
				}
				break;
			default:
				GAssert(false, "unexpected case");
		}
	}
	else
	{
		m_eKeyState = Normal;
		return false; // false = don't bother updating the view
	}
	return true; // true = need to update the view
}

void EditorController::HandleKeyPress(SDLKey eKey)
{
	if(eKey >= 'a' && eKey <= 'z')
	{
		if(m_pView->OnChar((char)eKey))
			m_pView->Update();
	}
	else
	{
		int dx = m_keyboard[SDLK_RIGHT] - m_keyboard[SDLK_LEFT];
		int dy = m_keyboard[SDLK_DOWN] - m_keyboard[SDLK_UP];
		if(dy != 0)
			m_pView->MoveRow(dy);
		if(dx != 0)
			m_pView->MoveCol(dx);
	}
	m_lastPressedKey = eKey;
}

void EditorController::HandleMouseClick()
{
}
