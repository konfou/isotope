/****************************************************************************
** WassailDialog meta object code from reading C++ file 'WassailDialog.h'
**
** Created: Sat Mar 30 14:57:51 2002
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_WassailDialog
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "WassailDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *WassailDialog::className() const
{
    return "WassailDialog";
}

QMetaObject *WassailDialog::metaObj = 0;

void WassailDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(WassailDialogBase::className(), "WassailDialogBase") != 0 )
	badSuperclassWarning("WassailDialog","WassailDialogBase");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString WassailDialog::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("WassailDialog",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* WassailDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) WassailDialogBase::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    QMetaData::Access *slot_tbl_access = 0;
    metaObj = QMetaObject::new_metaobject(
	"WassailDialog", "WassailDialogBase",
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    metaObj->set_slot_access( slot_tbl_access );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    return metaObj;
}
