/****************************************************************************
** ImportCppBase meta object code from reading C++ file 'importcpp.h'
**
** Created: Tue May 10 05:55:56 2005
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_ImportCppBase
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "importcpp.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *ImportCppBase::className() const
{
    return "ImportCppBase";
}

QMetaObject *ImportCppBase::metaObj = 0;

void ImportCppBase::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("ImportCppBase","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString ImportCppBase::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("ImportCppBase",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* ImportCppBase::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(ImportCppBase::*m1_t0)();
    typedef void(ImportCppBase::*m1_t1)();
    typedef void(ImportCppBase::*m1_t2)();
    typedef void(ImportCppBase::*m1_t3)();
    typedef void(ImportCppBase::*m1_t4)();
    typedef void(ImportCppBase::*m1_t5)();
    m1_t0 v1_0 = Q_AMPERSAND ImportCppBase::slot_Button6;
    m1_t1 v1_1 = Q_AMPERSAND ImportCppBase::slot_Button1;
    m1_t2 v1_2 = Q_AMPERSAND ImportCppBase::slot_Button2;
    m1_t3 v1_3 = Q_AMPERSAND ImportCppBase::slot_Button3;
    m1_t4 v1_4 = Q_AMPERSAND ImportCppBase::slot_Button4;
    m1_t5 v1_5 = Q_AMPERSAND ImportCppBase::slot_Button5;
    QMetaData *slot_tbl = QMetaObject::new_metadata(6);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(6);
    slot_tbl[0].name = "slot_Button6()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "slot_Button1()";
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl_access[1] = QMetaData::Public;
    slot_tbl[2].name = "slot_Button2()";
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl_access[2] = QMetaData::Public;
    slot_tbl[3].name = "slot_Button3()";
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl_access[3] = QMetaData::Public;
    slot_tbl[4].name = "slot_Button4()";
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    slot_tbl_access[4] = QMetaData::Public;
    slot_tbl[5].name = "slot_Button5()";
    slot_tbl[5].ptr = *((QMember*)&v1_5);
    slot_tbl_access[5] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"ImportCppBase", "QDialog",
	slot_tbl, 6,
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
