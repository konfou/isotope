/****************************************************************************
** TestDialogBase meta object code from reading C++ file 'TestDialogBase.h'
**
** Created: Tue May 10 05:55:57 2005
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_TestDialogBase
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "TestDialogBase.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *TestDialogBase::className() const
{
    return "TestDialogBase";
}

QMetaObject *TestDialogBase::metaObj = 0;

void TestDialogBase::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("TestDialogBase","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString TestDialogBase::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("TestDialogBase",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* TestDialogBase::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(TestDialogBase::*m1_t0)();
    typedef void(TestDialogBase::*m1_t1)();
    typedef void(TestDialogBase::*m1_t2)();
    typedef void(TestDialogBase::*m1_t3)();
    typedef void(TestDialogBase::*m1_t4)();
    m1_t0 v1_0 = Q_AMPERSAND TestDialogBase::slot_button_debug;
    m1_t1 v1_1 = Q_AMPERSAND TestDialogBase::slot_button_clear;
    m1_t2 v1_2 = Q_AMPERSAND TestDialogBase::slot_button_run;
    m1_t3 v1_3 = Q_AMPERSAND TestDialogBase::slot_button_start;
    m1_t4 v1_4 = Q_AMPERSAND TestDialogBase::slot_slider_value_changed;
    QMetaData *slot_tbl = QMetaObject::new_metadata(5);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(5);
    slot_tbl[0].name = "slot_button_debug()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "slot_button_clear()";
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl_access[1] = QMetaData::Public;
    slot_tbl[2].name = "slot_button_run()";
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl_access[2] = QMetaData::Public;
    slot_tbl[3].name = "slot_button_start()";
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl_access[3] = QMetaData::Public;
    slot_tbl[4].name = "slot_slider_value_changed()";
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    slot_tbl_access[4] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"TestDialogBase", "QDialog",
	slot_tbl, 5,
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
