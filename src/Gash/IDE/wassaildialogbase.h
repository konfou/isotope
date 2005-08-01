/****************************************************************************
** Form interface generated from reading ui file '.\IDE\wassaildialogbase.ui'
**
** Created: Tue May 10 05:55:56 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef WASSAILDIALOGBASE_H
#define WASSAILDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QPushButton;

class WassailDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    WassailDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~WassailDialogBase();

    QLabel* TextLabel1;
    QLabel* TextLabel2;
    QPushButton* PushButton5_2;
    QPushButton* PushButton5;
    QPushButton* PushButton8;
    QPushButton* PushButton7;
    QPushButton* PushButton6;
    QPushButton* PushButton6_2;
    QPushButton* PushButton7_2;

public slots:
    virtual void slot_button6();
    virtual void slot_button1();
    virtual void slot_button2();
    virtual void slot_button3();
    virtual void slot_button4();
    virtual void slot_button5();
    virtual void slot_go();

protected:
    bool event( QEvent* );
};

#endif // WASSAILDIALOGBASE_H
