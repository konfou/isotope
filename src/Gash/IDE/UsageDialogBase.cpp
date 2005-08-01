/****************************************************************************
** Form implementation generated from reading ui file '.\Ide\UsageDialogBase.ui'
**
** Created: Tue May 10 05:55:57 2005
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "UsageDialogBase.h"

#include <qmultilineedit.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a UsageDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
UsageDialogBase::UsageDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "UsageDialogBase" );
    resize( 528, 482 ); 
    setProperty( "caption", tr( "Usage" ) );

    MultiLineEdit1 = new QMultiLineEdit( this, "MultiLineEdit1" );
    MultiLineEdit1->setGeometry( QRect( 0, 0, 530, 480 ) ); 
    QFont MultiLineEdit1_font(  MultiLineEdit1->font() );
    MultiLineEdit1_font.setFamily( "@Arial Unicode MS" );
    MultiLineEdit1_font.setPointSize( 11 );
    MultiLineEdit1->setFont( MultiLineEdit1_font ); 

    // tab order
}

/*  
 *  Destroys the object and frees any allocated resources
 */
UsageDialogBase::~UsageDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool UsageDialogBase::event( QEvent* ev )
{
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont MultiLineEdit1_font(  MultiLineEdit1->font() );
	MultiLineEdit1_font.setFamily( "@Arial Unicode MS" );
	MultiLineEdit1_font.setPointSize( 11 );
	MultiLineEdit1->setFont( MultiLineEdit1_font ); 
    }
    return ret;
}

