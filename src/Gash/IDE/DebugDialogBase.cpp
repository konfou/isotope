/****************************************************************************
** Form implementation generated from reading ui file '.\IDE\DebugDialogBase.ui'
**
** Created: Tue May 10 05:55:57 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "DebugDialogBase.h"

#include <qheader.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>


/* 
 *  Constructs a DebugDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DebugDialogBase::DebugDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "DebugDialogBase" );
    resize( 783, 559 ); 
    QFont f( font() );
    f.setFamily( "Courier New" );
    f.setPointSize( 11 );
    setFont( f ); 
    setProperty( "caption", tr( "Debug View" ) );

    PushButton1 = new QPushButton( this, "PushButton1" );
    PushButton1->setGeometry( QRect( 500, 0, 80, 20 ) ); 
    PushButton1->setProperty( "text", tr( "I- In" ) );

    PushButton2 = new QPushButton( this, "PushButton2" );
    PushButton2->setGeometry( QRect( 590, 0, 80, 20 ) ); 
    PushButton2->setProperty( "text", tr( "O- Over" ) );

    PushButton3 = new QPushButton( this, "PushButton3" );
    PushButton3->setGeometry( QRect( 680, 0, 80, 20 ) ); 
    PushButton3->setProperty( "text", tr( "P- Out" ) );

    PushButton8 = new QPushButton( this, "PushButton8" );
    PushButton8->setGeometry( QRect( 50, 10, 80, 20 ) ); 
    PushButton8->setProperty( "text", tr( "Unused" ) );

    PushButton9 = new QPushButton( this, "PushButton9" );
    PushButton9->setGeometry( QRect( 140, 10, 80, 20 ) ); 
    PushButton9->setProperty( "text", tr( "Unused" ) );

    PushButton6 = new QPushButton( this, "PushButton6" );
    PushButton6->setGeometry( QRect( 30, 40, 140, 20 ) ); 
    PushButton6->setProperty( "text", tr( "G- Go" ) );

    PushButton7 = new QPushButton( this, "PushButton7" );
    PushButton7->setGeometry( QRect( 180, 40, 90, 20 ) ); 
    PushButton7->setProperty( "text", tr( "Q- Quit" ) );

    PushButton5 = new QPushButton( this, "PushButton5" );
    PushButton5->setGeometry( QRect( 280, 40, 150, 20 ) ); 
    PushButton5->setProperty( "text", tr( "C- Clear All BP" ) );

    PushButton4 = new QPushButton( this, "PushButton4" );
    PushButton4->setGeometry( QRect( 450, 40, 140, 20 ) ); 
    PushButton4->setProperty( "text", tr( "B- BreakPoint" ) );

    TextLabel2_2 = new QLabel( this, "TextLabel2_2" );
    TextLabel2_2->setGeometry( QRect( 290, 0, 90, 20 ) ); 
    TextLabel2_2->setProperty( "text", tr( "VM State:" ) );

    TextLabel1_2 = new QLabel( this, "TextLabel1_2" );
    TextLabel1_2->setGeometry( QRect( 240, 20, 250, 20 ) ); 
    TextLabel1_2->setProperty( "text", tr( "Running" ) );

    TabWidget3_2 = new QTabWidget( this, "TabWidget3_2" );
    TabWidget3_2->setGeometry( QRect( 380, 70, 370, 480 ) ); 

    tab = new QWidget( TabWidget3_2, "tab" );

    MultiLineEdit1 = new QMultiLineEdit( tab, "MultiLineEdit1" );
    MultiLineEdit1->setGeometry( QRect( 10, 10, 350, 430 ) ); 
    TabWidget3_2->insertTab( tab, tr( "Source" ) );

    tab_2 = new QWidget( TabWidget3_2, "tab_2" );

    MultiLineEdit2 = new QMultiLineEdit( tab_2, "MultiLineEdit2" );
    MultiLineEdit2->setGeometry( QRect( 10, 10, 350, 430 ) ); 
    TabWidget3_2->insertTab( tab_2, tr( "Disassembly" ) );

    TabWidget3 = new QTabWidget( this, "TabWidget3" );
    TabWidget3->setGeometry( QRect( 10, 70, 360, 480 ) ); 
    TabWidget3->setProperty( "focusPolicy", (int)QTabWidget::NoFocus );

    tab_3 = new QWidget( TabWidget3, "tab_3" );

    ListBox1 = new QListBox( tab_3, "ListBox1" );
    ListBox1->insertItem( tr( "New Item" ) );
    ListBox1->setGeometry( QRect( 10, 10, 340, 430 ) ); 
    ListBox1->setProperty( "focusPolicy", (int)QListBox::NoFocus );
    TabWidget3->insertTab( tab_3, tr( "Call Stack" ) );

    tab_4 = new QWidget( TabWidget3, "tab_4" );

    ListView2_2 = new QListView( tab_4, "ListView2_2" );
    ListView2_2->addColumn( tr( "Column 1" ) );
    QListViewItem * item = new QListViewItem( ListView2_2, 0 );
    item->setText( 0, tr( "New Item" ) );

    ListView2_2->setGeometry( QRect( 10, 10, 340, 430 ) ); 
    ListView2_2->setProperty( "focusPolicy", (int)QListView::NoFocus );
    ListView2_2->setProperty( "allColumnsShowFocus", QVariant( TRUE, 0 ) );
    ListView2_2->setProperty( "rootIsDecorated", QVariant( TRUE, 0 ) );
    TabWidget3->insertTab( tab_4, tr( "Variable Stack" ) );

    // signals and slots connections
    connect( PushButton1, SIGNAL( released() ), this, SLOT( slot_button1() ) );
    connect( PushButton2, SIGNAL( released() ), this, SLOT( slot_button2() ) );
    connect( PushButton3, SIGNAL( released() ), this, SLOT( slot_button3() ) );
    connect( PushButton4, SIGNAL( released() ), this, SLOT( slot_button4() ) );
    connect( PushButton5, SIGNAL( released() ), this, SLOT( slot_button5() ) );
    connect( PushButton6, SIGNAL( released() ), this, SLOT( slot_button6() ) );
    connect( PushButton7, SIGNAL( released() ), this, SLOT( slot_button7() ) );
    connect( PushButton8, SIGNAL( released() ), this, SLOT( slot_button8() ) );
    connect( PushButton9, SIGNAL( released() ), this, SLOT( slot_button9() ) );
    connect( ListBox1, SIGNAL( selectionChanged() ), this, SLOT( slot_callStackSelChange() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
DebugDialogBase::~DebugDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void DebugDialogBase::slot_callStackSelChange()
{
    qWarning( "DebugDialogBase::slot_callStackSelChange(): Not implemented yet!" );
}

void DebugDialogBase::slot_button1()
{
    qWarning( "DebugDialogBase::slot_button1(): Not implemented yet!" );
}

void DebugDialogBase::slot_button2()
{
    qWarning( "DebugDialogBase::slot_button2(): Not implemented yet!" );
}

void DebugDialogBase::slot_button3()
{
    qWarning( "DebugDialogBase::slot_button3(): Not implemented yet!" );
}

void DebugDialogBase::slot_button4()
{
    qWarning( "DebugDialogBase::slot_button4(): Not implemented yet!" );
}

void DebugDialogBase::slot_button5()
{
    qWarning( "DebugDialogBase::slot_button5(): Not implemented yet!" );
}

void DebugDialogBase::slot_button6()
{
    qWarning( "DebugDialogBase::slot_button6(): Not implemented yet!" );
}

void DebugDialogBase::slot_button7()
{
    qWarning( "DebugDialogBase::slot_button7(): Not implemented yet!" );
}

void DebugDialogBase::slot_button8()
{
    qWarning( "DebugDialogBase::slot_button8(): Not implemented yet!" );
}

void DebugDialogBase::slot_button9()
{
    qWarning( "DebugDialogBase::slot_button9(): Not implemented yet!" );
}

