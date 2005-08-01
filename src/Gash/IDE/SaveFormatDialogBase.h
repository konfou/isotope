/****************************************************************************
** Form interface generated from reading ui file '.\IDE\SaveFormatDialogBase.ui'
**
** Created: Tue May 10 05:55:57 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef SAVEFORMATDIALOGBASE_H
#define SAVEFORMATDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QPushButton;
class QRadioButton;

class SaveFormatDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    SaveFormatDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SaveFormatDialogBase();

    QLabel* TextLabel1;
    QRadioButton* RadioButton2;
    QRadioButton* RadioButton1;
    QRadioButton* RadioButton3;
    QPushButton* PushButton2;
    QPushButton* PushButton1;

public slots:
    virtual void slot_radio3clicked();
    virtual void slot_radio1clicked();
    virtual void slot_radio2clicked();

};

#endif // SAVEFORMATDIALOGBASE_H
