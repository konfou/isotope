/****************************************************************************
** Form implementation generated from reading ui file '.\IDE\FileViewDialogBase.ui'
**
** Created: Tue May 10 05:55:56 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "FileViewDialogBase.h"

#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a FileViewDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FileViewDialogBase::FileViewDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "FileViewDialogBase" );
    resize( 660, 507 ); 
    QFont f( font() );
    f.setFamily( "Courier New" );
    f.setPointSize( 11 );
    setFont( f ); 
    setProperty( "caption", tr( "Form1" ) );

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setGeometry( QRect( 230, 470, 93, 26 ) ); 
    PushButton1->setProperty( "text", tr( "OK" ) );

    PushButton2 = new QPushButton( this, "PushButton2" );
    PushButton2->setGeometry( QRect( 340, 470, 93, 26 ) ); 
    PushButton2->setProperty( "text", tr( "Cancel" ) );

    MultiLineEdit1 = new QMultiLineEdit( this, "MultiLineEdit1" );
    MultiLineEdit1->setGeometry( QRect( 10, 10, 631, 451 ) ); 
    MultiLineEdit1->setProperty( "undoDepth", 1024 );

    // signals and slots connections
    connect( PushButton2, SIGNAL( released() ), this, SLOT( slot_button2() ) );
    connect( PushButton1, SIGNAL( released() ), this, SLOT( slot_button1() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
FileViewDialogBase::~FileViewDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void FileViewDialogBase::slot_button1()
{
    qWarning( "FileViewDialogBase::slot_button1(): Not implemented yet!" );
}

void FileViewDialogBase::slot_button2()
{
    qWarning( "FileViewDialogBase::slot_button2(): Not implemented yet!" );
}

