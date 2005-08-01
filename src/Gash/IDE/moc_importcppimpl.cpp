/****************************************************************************
** ImportCpp meta object code from reading C++ file 'importcppimpl.h'
**
** Created: Tue May 10 05:55:58 2005
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_ImportCpp
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "importcppimpl.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *ImportCpp::className() const
{
    return "ImportCpp";
}

QMetaObject *ImportCpp::metaObj = 0;

void ImportCpp::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(ImportCppBase::className(), "ImportCppBase") != 0 )
	badSuperclassWarning("ImportCpp","ImportCppBase");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString ImportCpp::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("ImportCpp",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* ImportCpp::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) ImportCppBase::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    QMetaData::Access *slot_tbl_access = 0;
    metaObj = QMetaObject::new_metaobject(
	"ImportCpp", "ImportCppBase",
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
