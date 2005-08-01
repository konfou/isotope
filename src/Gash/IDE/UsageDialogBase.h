/****************************************************************************
** Form interface generated from reading ui file '.\Ide\UsageDialogBase.ui'
**
** Created: Tue May 10 05:55:57 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef USAGEDIALOGBASE_H
#define USAGEDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QMultiLineEdit;

class UsageDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    UsageDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~UsageDialogBase();

    QMultiLineEdit* MultiLineEdit1;

protected:
    bool event( QEvent* );
};

#endif // USAGEDIALOGBASE_H
