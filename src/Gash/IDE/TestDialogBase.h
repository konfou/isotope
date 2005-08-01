/****************************************************************************
** Form interface generated from reading ui file '.\IDE\TestDialogBase.ui'
**
** Created: Tue May 10 05:55:57 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef TESTDIALOGBASE_H
#define TESTDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QGroupBox;
class QLabel;
class QLineEdit;
class QMultiLineEdit;
class QProgressBar;
class QPushButton;

class TestDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    TestDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~TestDialogBase();

    QProgressBar* ProgressBar1;
    QMultiLineEdit* MultiLineEdit1;
    QGroupBox* GroupBox1;
    QPushButton* PushButton4;
    QPushButton* PushButton5;
    QLineEdit* LineEdit1;
    QLabel* TextLabel1;
    QPushButton* PushButton2;
    QPushButton* PushButton1;
    QPushButton* PushButton3;

public slots:
    virtual void slot_button_debug();
    virtual void slot_button_clear();
    virtual void slot_button_run();
    virtual void slot_button_start();
    virtual void slot_slider_value_changed();

};

#endif // TESTDIALOGBASE_H
