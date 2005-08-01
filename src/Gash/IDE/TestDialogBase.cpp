/****************************************************************************
** Form implementation generated from reading ui file '.\IDE\TestDialogBase.ui'
**
** Created: Tue May 10 05:55:57 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "TestDialogBase.h"

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a TestDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
TestDialogBase::TestDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "TestDialogBase" );
    resize( 793, 522 ); 
    QFont f( font() );
    f.setFamily( "Courier New" );
    f.setPointSize( 11 );
    setFont( f ); 
    setProperty( "caption", tr( "Test View" ) );

    ProgressBar1 = new QProgressBar( this, "ProgressBar1" );
    ProgressBar1->setGeometry( QRect( 20, 10, 740, 25 ) ); 

    MultiLineEdit1 = new QMultiLineEdit( this, "MultiLineEdit1" );
    MultiLineEdit1->setGeometry( QRect( 20, 40, 750, 410 ) ); 

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setGeometry( QRect( 410, 460, 360, 51 ) ); 
    GroupBox1->setProperty( "title", tr( "Single Test" ) );

    PushButton4 = new QPushButton( GroupBox1, "PushButton4" );
    PushButton4->setGeometry( QRect( 140, 20, 93, 26 ) ); 
    PushButton4->setProperty( "text", tr( "R- Run" ) );

    PushButton5 = new QPushButton( GroupBox1, "PushButton5" );
    PushButton5->setGeometry( QRect( 250, 20, 93, 26 ) ); 
    PushButton5->setProperty( "text", tr( "D- Debug" ) );

    LineEdit1 = new QLineEdit( GroupBox1, "LineEdit1" );
    LineEdit1->setGeometry( QRect( 80, 20, 50, 22 ) ); 

    TextLabel1 = new QLabel( GroupBox1, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 20, 20, 60, 20 ) ); 
    TextLabel1->setProperty( "text", tr( "Test #" ) );

    PushButton2 = new QPushButton( this, "PushButton2" );
    PushButton2->setGeometry( QRect( 30, 480, 93, 26 ) ); 
    PushButton2->setProperty( "text", tr( "S- Start" ) );

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setGeometry( QRect( 250, 480, 93, 26 ) ); 
    PushButton1->setProperty( "text", tr( "Q- Close" ) );

    PushButton3 = new QPushButton( this, "PushButton3" );
    PushButton3->setGeometry( QRect( 140, 480, 93, 26 ) ); 
    PushButton3->setProperty( "text", tr( "C- Clear" ) );

    // signals and slots connections
    connect( PushButton1, SIGNAL( released() ), this, SLOT( reject() ) );
    connect( PushButton2, SIGNAL( released() ), this, SLOT( slot_button_start() ) );
    connect( PushButton3, SIGNAL( released() ), this, SLOT( slot_button_clear() ) );
    connect( PushButton4, SIGNAL( released() ), this, SLOT( slot_button_run() ) );
    connect( PushButton5, SIGNAL( released() ), this, SLOT( slot_button_debug() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
TestDialogBase::~TestDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void TestDialogBase::slot_button_debug()
{
    qWarning( "TestDialogBase::slot_button_debug(): Not implemented yet!" );
}

void TestDialogBase::slot_button_clear()
{
    qWarning( "TestDialogBase::slot_button_clear(): Not implemented yet!" );
}

void TestDialogBase::slot_button_run()
{
    qWarning( "TestDialogBase::slot_button_run(): Not implemented yet!" );
}

void TestDialogBase::slot_button_start()
{
    qWarning( "TestDialogBase::slot_button_start(): Not implemented yet!" );
}

void TestDialogBase::slot_slider_value_changed()
{
    qWarning( "TestDialogBase::slot_slider_value_changed(): Not implemented yet!" );
}

