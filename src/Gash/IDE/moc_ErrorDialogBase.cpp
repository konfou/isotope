/****************************************************************************
** ErrorDialogBase meta object code from reading C++ file 'ErrorDialogBase.h'
**
** Created: Sat Oct 30 13:28:54 2004
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_ErrorDialogBase
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "ErrorDialogBase.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *ErrorDialogBase::className() const
{
    return "ErrorDialogBase";
}

QMetaObject *ErrorDialogBase::metaObj = 0;

void ErrorDialogBase::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("ErrorDialogBase","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString ErrorDialogBase::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("ErrorDialogBase",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* ErrorDialogBase::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(ErrorDialogBase::*m1_t0)();
    typedef void(ErrorDialogBase::*m1_t1)();
    m1_t0 v1_0 = Q_AMPERSAND ErrorDialogBase::slot_button2;
    m1_t1 v1_1 = Q_AMPERSAND ErrorDialogBase::slot_button1;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(2);
    slot_tbl[0].name = "slot_button2()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "slot_button1()";
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl_access[1] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"ErrorDialogBase", "QDialog",
	slot_tbl, 2,
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
