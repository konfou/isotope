/****************************************************************************
** WassailDialogBase meta object code from reading C++ file 'wassaildialogbase.h'
**
** Created: Tue May 10 05:55:56 2005
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_WassailDialogBase
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "wassaildialogbase.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *WassailDialogBase::className() const
{
    return "WassailDialogBase";
}

QMetaObject *WassailDialogBase::metaObj = 0;

void WassailDialogBase::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("WassailDialogBase","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString WassailDialogBase::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("WassailDialogBase",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* WassailDialogBase::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(WassailDialogBase::*m1_t0)();
    typedef void(WassailDialogBase::*m1_t1)();
    typedef void(WassailDialogBase::*m1_t2)();
    typedef void(WassailDialogBase::*m1_t3)();
    typedef void(WassailDialogBase::*m1_t4)();
    typedef void(WassailDialogBase::*m1_t5)();
    typedef void(WassailDialogBase::*m1_t6)();
    m1_t0 v1_0 = Q_AMPERSAND WassailDialogBase::slot_button6;
    m1_t1 v1_1 = Q_AMPERSAND WassailDialogBase::slot_button1;
    m1_t2 v1_2 = Q_AMPERSAND WassailDialogBase::slot_button2;
    m1_t3 v1_3 = Q_AMPERSAND WassailDialogBase::slot_button3;
    m1_t4 v1_4 = Q_AMPERSAND WassailDialogBase::slot_button4;
    m1_t5 v1_5 = Q_AMPERSAND WassailDialogBase::slot_button5;
    m1_t6 v1_6 = Q_AMPERSAND WassailDialogBase::slot_go;
    QMetaData *slot_tbl = QMetaObject::new_metadata(7);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(7);
    slot_tbl[0].name = "slot_button6()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "slot_button1()";
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl_access[1] = QMetaData::Public;
    slot_tbl[2].name = "slot_button2()";
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl_access[2] = QMetaData::Public;
    slot_tbl[3].name = "slot_button3()";
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl_access[3] = QMetaData::Public;
    slot_tbl[4].name = "slot_button4()";
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    slot_tbl_access[4] = QMetaData::Public;
    slot_tbl[5].name = "slot_button5()";
    slot_tbl[5].ptr = *((QMember*)&v1_5);
    slot_tbl_access[5] = QMetaData::Public;
    slot_tbl[6].name = "slot_go()";
    slot_tbl[6].ptr = *((QMember*)&v1_6);
    slot_tbl_access[6] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"WassailDialogBase", "QDialog",
	slot_tbl, 7,
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
