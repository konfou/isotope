#ifndef IMPORTCPP_H
#define IMPORTCPP_H
#include "importcpp.h"

class GCppScope;
class GPointerArray;
class GStringHeap;

class ImportCpp : public ImportCppBase
{ 
    Q_OBJECT

protected:
	QString m_sFile;
	GCppScope* m_pGlobalScope;
	GStringHeap* m_pStringHeap;
	GPointerArray* m_pDefines;
	GPointerArray* m_pIncludePaths;
	GPointerArray* m_pTypes;

public:
    ImportCpp(QWidget* parent, const char* name);
    ~ImportCpp();

    virtual void slot_Button1();
    virtual void slot_Button2();
    virtual void slot_Button3();
    virtual void slot_Button4();
    virtual void slot_Button5();
    virtual void slot_Button6();

protected:
	void RefreshLists();
};

#endif // IMPORTCPP_H
