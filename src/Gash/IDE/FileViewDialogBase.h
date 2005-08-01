/****************************************************************************
** Form interface generated from reading ui file '.\IDE\FileViewDialogBase.ui'
**
** Created: Tue May 10 05:55:56 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef FILEVIEWDIALOGBASE_H
#define FILEVIEWDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QMultiLineEdit;
class QPushButton;

class FileViewDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    FileViewDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FileViewDialogBase();

    QPushButton* PushButton1;
    QPushButton* PushButton2;
    QMultiLineEdit* MultiLineEdit1;

public slots:
    virtual void slot_button1();
    virtual void slot_button2();

};

#endif // FILEVIEWDIALOGBASE_H
