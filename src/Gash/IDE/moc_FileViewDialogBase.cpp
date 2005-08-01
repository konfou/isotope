/****************************************************************************
** FileViewDialogBase meta object code from reading C++ file 'FileViewDialogBase.h'
**
** Created: Tue May 10 05:55:57 2005
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_FileViewDialogBase
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "FileViewDialogBase.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *FileViewDialogBase::className() const
{
    return "FileViewDialogBase";
}

QMetaObject *FileViewDialogBase::metaObj = 0;

void FileViewDialogBase::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("FileViewDialogBase","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString FileViewDialogBase::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("FileViewDialogBase",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* FileViewDialogBase::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(FileViewDialogBase::*m1_t0)();
    typedef void(FileViewDialogBase::*m1_t1)();
    m1_t0 v1_0 = Q_AMPERSAND FileViewDialogBase::slot_button1;
    m1_t1 v1_1 = Q_AMPERSAND FileViewDialogBase::slot_button2;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(2);
    slot_tbl[0].name = "slot_button1()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "slot_button2()";
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl_access[1] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"FileViewDialogBase", "QDialog",
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
