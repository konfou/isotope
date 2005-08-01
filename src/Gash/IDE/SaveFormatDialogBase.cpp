/****************************************************************************
** Form implementation generated from reading ui file '.\IDE\SaveFormatDialogBase.ui'
**
** Created: Tue May 10 05:55:57 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "SaveFormatDialogBase.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a SaveFormatDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SaveFormatDialogBase::SaveFormatDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "SaveFormatDialogBase" );
    resize( 438, 191 ); 
    QFont f( font() );
    f.setFamily( "Courier New" );
    f.setPointSize( 11 );
    setFont( f ); 
    setProperty( "caption", tr( "Choose a format" ) );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 10, 10, 141, 21 ) ); 
    TextLabel1->setProperty( "text", tr( "Save As..." ) );

    RadioButton2 = new QRadioButton( this, "RadioButton2" );
    RadioButton2->setGeometry( QRect( 30, 70, 400, 21 ) ); 
    RadioButton2->setProperty( "text", tr( "xml in a .cpp file (for calling from C++)" ) );

    RadioButton1 = new QRadioButton( this, "RadioButton1" );
    RadioButton1->setGeometry( QRect( 30, 40, 400, 20 ) ); 
    RadioButton1->setProperty( "text", tr( ".xlib file (the Gash executable format)" ) );

    RadioButton3 = new QRadioButton( this, "RadioButton3" );
    RadioButton3->setGeometry( QRect( 30, 100, 400, 21 ) ); 
    RadioButton3->setProperty( "text", tr( ".cpp file (to use your C++ optimizer)" ) );

    PushButton2 = new QPushButton( this, "PushButton2" );
    PushButton2->setGeometry( QRect( 260, 140, 90, 30 ) ); 
    PushButton2->setProperty( "text", tr( "Cancel" ) );

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setGeometry( QRect( 80, 140, 90, 30 ) ); 
    PushButton1->setProperty( "text", tr( "OK" ) );

    // signals and slots connections
    connect( PushButton2, SIGNAL( released() ), this, SLOT( reject() ) );
    connect( PushButton1, SIGNAL( released() ), this, SLOT( accept() ) );
    connect( RadioButton1, SIGNAL( toggled(bool) ), this, SLOT( slot_radio1clicked() ) );
    connect( RadioButton2, SIGNAL( toggled(bool) ), this, SLOT( slot_radio2clicked() ) );
    connect( RadioButton3, SIGNAL( toggled(bool) ), this, SLOT( slot_radio3clicked() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SaveFormatDialogBase::~SaveFormatDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void SaveFormatDialogBase::slot_radio3clicked()
{
    qWarning( "SaveFormatDialogBase::slot_radio3clicked(): Not implemented yet!" );
}

void SaveFormatDialogBase::slot_radio1clicked()
{
    qWarning( "SaveFormatDialogBase::slot_radio1clicked(): Not implemented yet!" );
}

void SaveFormatDialogBase::slot_radio2clicked()
{
    qWarning( "SaveFormatDialogBase::slot_radio2clicked(): Not implemented yet!" );
}

