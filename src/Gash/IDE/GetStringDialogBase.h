/****************************************************************************
** Form interface generated from reading ui file '.\ide\GetStringDialogBase.ui'
**
** Created: Tue May 10 05:55:55 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef GETSTRINGDIALOGBASE_H
#define GETSTRINGDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLineEdit;
class QPushButton;

class GetStringDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    GetStringDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GetStringDialogBase();

    QLineEdit* LineEdit1;
    QPushButton* PushButton1;
    QPushButton* PushButton2;

};

#endif // GETSTRINGDIALOGBASE_H
