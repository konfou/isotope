/****************************************************************************
** Form interface generated from reading ui file '.\IDE\importcpp.ui'
**
** Created: Tue May 10 05:55:56 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef IMPORTCPPBASE_H
#define IMPORTCPPBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QPushButton;

class ImportCppBase : public QDialog
{ 
    Q_OBJECT

public:
    ImportCppBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ImportCppBase();

    QListBox* ListBox1;
    QPushButton* PushButton10;
    QLabel* TextLabel1_2;
    QLabel* TextLabel1;
    QLineEdit* LineEdit1;
    QPushButton* PushButton9;
    QListBox* ListBox3;
    QLabel* TextLabel2;
    QLabel* TextLabel1_3;
    QListBox* ListBox2;
    QPushButton* PushButton3;
    QPushButton* PushButton4;
    QPushButton* PushButton5;
    QPushButton* PushButton6;

public slots:
    virtual void slot_Button6();
    virtual void slot_Button1();
    virtual void slot_Button2();
    virtual void slot_Button3();
    virtual void slot_Button4();
    virtual void slot_Button5();

};

#endif // IMPORTCPPBASE_H
