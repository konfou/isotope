/****************************************************************************
** Form implementation generated from reading ui file '.\ide\GetStringDialogBase.ui'
**
** Created: Tue May 10 05:55:56 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "GetStringDialogBase.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a GetStringDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
GetStringDialogBase::GetStringDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "GetStringDialogBase" );
    resize( 461, 85 ); 
    QFont f( font() );
    f.setFamily( "Courier New" );
    f.setPointSize( 11 );
    setFont( f ); 
    setProperty( "caption", tr( "Enter a string..." ) );

    LineEdit1 = new QLineEdit( this, "LineEdit1" );
    LineEdit1->setGeometry( QRect( 10, 10, 440, 30 ) ); 

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setGeometry( QRect( 93, 50, 120, 26 ) ); 
    PushButton1->setProperty( "text", tr( "Enter- OK" ) );
    PushButton1->setProperty( "default", QVariant( TRUE, 0 ) );

    PushButton2 = new QPushButton( this, "PushButton2" );
    PushButton2->setGeometry( QRect( 230, 50, 120, 26 ) ); 
    PushButton2->setProperty( "text", tr( "Esc- Cancel" ) );

    // signals and slots connections
    connect( PushButton1, SIGNAL( released() ), this, SLOT( accept() ) );
    connect( PushButton2, SIGNAL( released() ), this, SLOT( reject() ) );
    connect( LineEdit1, SIGNAL( returnPressed() ), this, SLOT( accept() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
GetStringDialogBase::~GetStringDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

