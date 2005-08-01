#include "importcppimpl.h"
#include <qfiledialog.h>
#include "../../GClasses/GCppParser.h"
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GArray.h"
#include "GetStringDialog.h"

ImportCpp::ImportCpp(QWidget* parent,  const char* name)
    : ImportCppBase(parent, name, true, 0)
{
	m_sFile = "";
	m_pGlobalScope = NULL;
	m_pStringHeap = new GStringHeap(1024);
	m_pDefines = new GPointerArray(8);
	m_pIncludePaths = new GPointerArray(8);
	m_pTypes = new GPointerArray(8);

	// Add initial defines and include paths
	m_pDefines->AddPointer("WIN32");
	m_pIncludePaths->AddPointer("C:\\Program Files\\Microsoft Visual Studio\\VC98\\Include");
	RefreshLists();
}

ImportCpp::~ImportCpp()
{
	delete(m_pDefines);
	delete(m_pIncludePaths);
	delete(m_pTypes);
	delete(m_pGlobalScope);
	delete(m_pStringHeap);
}

void ImportCpp::RefreshLists()
{
	// Refresh the defines
	ListBox2->clear();
	int n;
	int nCount = m_pDefines->GetSize();
	for(n = 0; n < nCount; n++)
		ListBox2->insertItem(QString((const char*)m_pDefines->GetPointer(n)));

	// Refresh the includes
	ListBox3->clear();
	nCount = m_pIncludePaths->GetSize();
	for(n = 0; n < nCount; n++)
		ListBox3->insertItem(QString((const char*)m_pIncludePaths->GetPointer(n)));

	// Refresh the defines
	ListBox1->clear();
	nCount = m_pTypes->GetSize();
	for(n = 0; n < nCount; n++)
		ListBox1->insertItem(QString(((GCppType*)m_pTypes->GetPointer(n))->GetName()));
}

// Browse for source file
void ImportCpp::slot_Button1()
{
	// Get the filename
	m_sFile = QFileDialog::getOpenFileName("", "*.h;*.hpp", NULL, "GetOpenFilename", "Select a header file to import");
	LineEdit1->setText(m_sFile);
	if(m_sFile.length() <= 0)
		return;

	// Parse the file
	GCppParser parser;
	int nCount = m_pDefines->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		parser.AddDefine((const char*)m_pDefines->GetPointer(n), NULL);
	nCount = m_pIncludePaths->GetSize();
	for(n = 0; n < nCount; n++)
		parser.AddIncludePath((const char*)m_pIncludePaths->GetPointer(n));
	m_pGlobalScope = parser.ParseCppFile(m_sFile.latin1());

	// Populate the list of types
	m_pTypes->Clear();
	if(m_pGlobalScope)
	{
		GConstStringHashTable* pTypes = m_pGlobalScope->GetTypes();
		GHashTableEnumerator e(pTypes);
		while(true)
		{
			const char* szTypeName = e.GetNextKey();
			if(!szTypeName)
				break;
			GCppType* pType;
			pTypes->Get(szTypeName, (void**)&pType);
			if(!pType->IsDeclaredInProjectFile())
				continue;
			m_pTypes->AddPointer(pType);
		}
	}

	RefreshLists();
}

// Import button
void ImportCpp::slot_Button2()
{

}

// Add Define button
void ImportCpp::slot_Button3()
{
	GetStringDialog dialog(NULL, "Add Define", "");
	if(dialog.exec() != dialog.Accepted)
		return;
    char* szString = m_pStringHeap->Add(dialog.LineEdit1->text().latin1());
	m_pDefines->AddPointer(szString);
	RefreshLists();
}

// Delete Define button
void ImportCpp::slot_Button4()
{
	int n = ListBox2->currentItem();
	if(n < 0)
		return;
	m_pDefines->DeleteCell(n);
	RefreshLists();
}

// Add Incude Path button
void ImportCpp::slot_Button5()
{
	GetStringDialog dialog(NULL, "Add Include Path", "");
	if(dialog.exec() != dialog.Accepted)
		return;
    char* szString = m_pStringHeap->Add(dialog.LineEdit1->text().latin1());
	m_pIncludePaths->AddPointer(szString);
	RefreshLists();
}

// Delete Include Path button
void ImportCpp::slot_Button6()
{
	int n = ListBox3->currentItem();
	if(n < 0)
		return;
	m_pIncludePaths->DeleteCell(n);
	RefreshLists();
}

