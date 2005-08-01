/****************************************************************************
** Form implementation generated from reading ui file '.\IDE\wassaildialogbase.ui'
**
** Created: Tue May 10 05:55:56 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "wassaildialogbase.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a WassailDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
WassailDialogBase::WassailDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "WassailDialogBase" );
    resize( 432, 353 ); 
    QFont f( font() );
    f.setFamily( "Lucida Bright" );
    f.setPointSize( 10 );
    setFont( f ); 
    setProperty( "caption", tr( "Gash" ) );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 110, 0, 260, 21 ) ); 
    TextLabel1->setProperty( "text", tr( "Welcome to" ) );
    TextLabel1->setProperty( "alignment", int( QLabel::AlignCenter ) );

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setGeometry( QRect( 110, 20, 260, 50 ) ); 
    QFont TextLabel2_font(  TextLabel2->font() );
    TextLabel2_font.setFamily( "Lucida Calligraphy" );
    TextLabel2_font.setPointSize( 36 );
    TextLabel2->setFont( TextLabel2_font ); 
    TextLabel2->setProperty( "text", tr( "Gash" ) );
    TextLabel2->setProperty( "alignment", int( QLabel::AlignCenter ) );

    PushButton5_2 = new QPushButton( this, "PushButton5_2" );
    PushButton5_2->setGeometry( QRect( 140, 310, 190, 26 ) ); 
    PushButton5_2->setProperty( "text", tr( "Q- Quit" ) );

    PushButton5 = new QPushButton( this, "PushButton5" );
    PushButton5->setGeometry( QRect( 140, 120, 190, 26 ) ); 
    PushButton5->setProperty( "text", tr( "R- Run" ) );

    PushButton8 = new QPushButton( this, "PushButton8" );
    PushButton8->setGeometry( QRect( 140, 150, 190, 26 ) ); 
    PushButton8->setProperty( "text", tr( "D- Debug" ) );

    PushButton7 = new QPushButton( this, "PushButton7" );
    PushButton7->setGeometry( QRect( 140, 210, 190, 26 ) ); 
    PushButton7->setProperty( "text", tr( "A- Disassembler" ) );

    PushButton6 = new QPushButton( this, "PushButton6" );
    PushButton6->setGeometry( QRect( 140, 240, 190, 26 ) ); 
    PushButton6->setProperty( "text", tr( "S- Documentation" ) );

    PushButton6_2 = new QPushButton( this, "PushButton6_2" );
    PushButton6_2->setGeometry( QRect( 140, 270, 190, 28 ) ); 
    PushButton6_2->setProperty( "text", tr( "I- Import Cpp" ) );

    PushButton7_2 = new QPushButton( this, "PushButton7_2" );
    PushButton7_2->setGeometry( QRect( 140, 180, 190, 28 ) ); 
    PushButton7_2->setProperty( "text", tr( "B- Build" ) );

    // signals and slots connections
    connect( PushButton5, SIGNAL( released() ), this, SLOT( slot_button1() ) );
    connect( PushButton6, SIGNAL( released() ), this, SLOT( slot_button2() ) );
    connect( PushButton7, SIGNAL( released() ), this, SLOT( slot_button3() ) );
    connect( PushButton8, SIGNAL( released() ), this, SLOT( slot_button4() ) );
    connect( PushButton5_2, SIGNAL( released() ), this, SLOT( accept() ) );
    connect( PushButton6_2, SIGNAL( released() ), this, SLOT( slot_button5() ) );
    connect( PushButton7_2, SIGNAL( released() ), this, SLOT( slot_button6() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
WassailDialogBase::~WassailDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool WassailDialogBase::event( QEvent* ev )
{
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont TextLabel2_font(  TextLabel2->font() );
	TextLabel2_font.setFamily( "Lucida Calligraphy" );
	TextLabel2_font.setPointSize( 36 );
	TextLabel2->setFont( TextLabel2_font ); 
    }
    return ret;
}

void WassailDialogBase::slot_button6()
{
    qWarning( "WassailDialogBase::slot_button6(): Not implemented yet!" );
}

void WassailDialogBase::slot_button1()
{
    qWarning( "WassailDialogBase::slot_button1(): Not implemented yet!" );
}

void WassailDialogBase::slot_button2()
{
    qWarning( "WassailDialogBase::slot_button2(): Not implemented yet!" );
}

void WassailDialogBase::slot_button3()
{
    qWarning( "WassailDialogBase::slot_button3(): Not implemented yet!" );
}

void WassailDialogBase::slot_button4()
{
    qWarning( "WassailDialogBase::slot_button4(): Not implemented yet!" );
}

void WassailDialogBase::slot_button5()
{
    qWarning( "WassailDialogBase::slot_button5(): Not implemented yet!" );
}

void WassailDialogBase::slot_go()
{
    qWarning( "WassailDialogBase::slot_go(): Not implemented yet!" );
}

