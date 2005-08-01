/****************************************************************************
** Form implementation generated from reading ui file '.\IDE\importcpp.ui'
**
** Created: Tue May 10 05:55:56 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "importcpp.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a ImportCppBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ImportCppBase::ImportCppBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ImportCppBase" );
    resize( 507, 375 ); 
    setProperty( "caption", tr( "ImportCpp" ) );
    setProperty( "sizeGripEnabled", QVariant( TRUE, 0 ) );

    ListBox1 = new QListBox( this, "ListBox1" );
    ListBox1->insertItem( tr( "New Item" ) );
    ListBox1->setGeometry( QRect( 120, 260, 370, 100 ) ); 

    PushButton10 = new QPushButton( this, "PushButton10" );
    PushButton10->setGeometry( QRect( 20, 260, 90, 26 ) ); 
    PushButton10->setProperty( "text", tr( "Import" ) );

    TextLabel1_2 = new QLabel( this, "TextLabel1_2" );
    TextLabel1_2->setGeometry( QRect( 120, 240, 53, 20 ) ); 
    TextLabel1_2->setProperty( "text", tr( "Classes:" ) );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 120, 190, 60, 20 ) ); 
    TextLabel1->setProperty( "text", tr( "Header File:" ) );

    LineEdit1 = new QLineEdit( this, "LineEdit1" );
    LineEdit1->setGeometry( QRect( 120, 210, 370, 22 ) ); 

    PushButton9 = new QPushButton( this, "PushButton9" );
    PushButton9->setGeometry( QRect( 20, 210, 93, 26 ) ); 
    PushButton9->setProperty( "text", tr( "Browse" ) );

    ListBox3 = new QListBox( this, "ListBox3" );
    ListBox3->insertItem( tr( "New Item" ) );
    ListBox3->setGeometry( QRect( 120, 120, 370, 60 ) ); 

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setGeometry( QRect( 120, 100, 80, 20 ) ); 
    TextLabel2->setProperty( "text", tr( "Include Paths:" ) );

    TextLabel1_3 = new QLabel( this, "TextLabel1_3" );
    TextLabel1_3->setGeometry( QRect( 120, 10, 53, 20 ) ); 
    TextLabel1_3->setProperty( "text", tr( "Defines:" ) );

    ListBox2 = new QListBox( this, "ListBox2" );
    ListBox2->insertItem( tr( "New Item" ) );
    ListBox2->setGeometry( QRect( 120, 30, 370, 60 ) ); 

    PushButton3 = new QPushButton( this, "PushButton3" );
    PushButton3->setGeometry( QRect( 20, 30, 90, 26 ) ); 
    PushButton3->setProperty( "text", tr( "Add" ) );

    PushButton4 = new QPushButton( this, "PushButton4" );
    PushButton4->setGeometry( QRect( 20, 60, 90, 26 ) ); 
    PushButton4->setProperty( "text", tr( "Delete" ) );

    PushButton5 = new QPushButton( this, "PushButton5" );
    PushButton5->setGeometry( QRect( 20, 120, 90, 26 ) ); 
    PushButton5->setProperty( "text", tr( "Add" ) );

    PushButton6 = new QPushButton( this, "PushButton6" );
    PushButton6->setGeometry( QRect( 20, 150, 90, 26 ) ); 
    PushButton6->setProperty( "text", tr( "Delete" ) );

    // signals and slots connections
    connect( PushButton9, SIGNAL( released() ), this, SLOT( slot_Button1() ) );
    connect( PushButton3, SIGNAL( released() ), this, SLOT( slot_Button3() ) );
    connect( PushButton4, SIGNAL( released() ), this, SLOT( slot_Button4() ) );
    connect( PushButton5, SIGNAL( released() ), this, SLOT( slot_Button5() ) );
    connect( PushButton6, SIGNAL( released() ), this, SLOT( slot_Button6() ) );
    connect( PushButton10, SIGNAL( released() ), this, SLOT( slot_Button2() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ImportCppBase::~ImportCppBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void ImportCppBase::slot_Button6()
{
    qWarning( "ImportCppBase::slot_Button6(): Not implemented yet!" );
}

void ImportCppBase::slot_Button1()
{
    qWarning( "ImportCppBase::slot_Button1(): Not implemented yet!" );
}

void ImportCppBase::slot_Button2()
{
    qWarning( "ImportCppBase::slot_Button2(): Not implemented yet!" );
}

void ImportCppBase::slot_Button3()
{
    qWarning( "ImportCppBase::slot_Button3(): Not implemented yet!" );
}

void ImportCppBase::slot_Button4()
{
    qWarning( "ImportCppBase::slot_Button4(): Not implemented yet!" );
}

void ImportCppBase::slot_Button5()
{
    qWarning( "ImportCppBase::slot_Button5(): Not implemented yet!" );
}

