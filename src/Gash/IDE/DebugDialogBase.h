/****************************************************************************
** Form interface generated from reading ui file '.\IDE\DebugDialogBase.ui'
**
** Created: Tue May 10 05:55:57 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef DEBUGDIALOGBASE_H
#define DEBUGDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QListBox;
class QListBoxItem;
class QListView;
class QListViewItem;
class QMultiLineEdit;
class QPushButton;
class QTabWidget;
class QWidget;

class DebugDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    DebugDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~DebugDialogBase();

    QPushButton* PushButton1;
    QPushButton* PushButton2;
    QPushButton* PushButton3;
    QPushButton* PushButton8;
    QPushButton* PushButton9;
    QPushButton* PushButton6;
    QPushButton* PushButton7;
    QPushButton* PushButton5;
    QPushButton* PushButton4;
    QLabel* TextLabel2_2;
    QLabel* TextLabel1_2;
    QTabWidget* TabWidget3_2;
    QWidget* tab;
    QMultiLineEdit* MultiLineEdit1;
    QWidget* tab_2;
    QMultiLineEdit* MultiLineEdit2;
    QTabWidget* TabWidget3;
    QWidget* tab_3;
    QListBox* ListBox1;
    QWidget* tab_4;
    QListView* ListView2_2;

public slots:
    virtual void slot_callStackSelChange();
    virtual void slot_button1();
    virtual void slot_button2();
    virtual void slot_button3();
    virtual void slot_button4();
    virtual void slot_button5();
    virtual void slot_button6();
    virtual void slot_button7();
    virtual void slot_button8();
    virtual void slot_button9();

};

#endif // DEBUGDIALOGBASE_H
