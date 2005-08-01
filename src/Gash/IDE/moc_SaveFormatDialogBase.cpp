/****************************************************************************
** SaveFormatDialogBase meta object code from reading C++ file 'SaveFormatDialogBase.h'
**
** Created: Tue May 10 05:55:57 2005
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_SaveFormatDialogBase
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "SaveFormatDialogBase.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *SaveFormatDialogBase::className() const
{
    return "SaveFormatDialogBase";
}

QMetaObject *SaveFormatDialogBase::metaObj = 0;

void SaveFormatDialogBase::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("SaveFormatDialogBase","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString SaveFormatDialogBase::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("SaveFormatDialogBase",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* SaveFormatDialogBase::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(SaveFormatDialogBase::*m1_t0)();
    typedef void(SaveFormatDialogBase::*m1_t1)();
    typedef void(SaveFormatDialogBase::*m1_t2)();
    m1_t0 v1_0 = Q_AMPERSAND SaveFormatDialogBase::slot_radio3clicked;
    m1_t1 v1_1 = Q_AMPERSAND SaveFormatDialogBase::slot_radio1clicked;
    m1_t2 v1_2 = Q_AMPERSAND SaveFormatDialogBase::slot_radio2clicked;
    QMetaData *slot_tbl = QMetaObject::new_metadata(3);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(3);
    slot_tbl[0].name = "slot_radio3clicked()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "slot_radio1clicked()";
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl_access[1] = QMetaData::Public;
    slot_tbl[2].name = "slot_radio2clicked()";
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl_access[2] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"SaveFormatDialogBase", "QDialog",
	slot_tbl, 3,
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
