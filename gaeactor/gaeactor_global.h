#ifndef GAEACTOR_GLOBAL_H
#define GAEACTOR_GLOBAL_H

#ifndef USING_GAEACTOR_EXPORT_LIB
#define GAEACTOR_EXPORT
#else
#include <QtCore/qglobal.h>
#if defined(GAEACTOR_LIBRARY)
#define GAEACTOR_EXPORT Q_DECL_EXPORT
#else
#define GAEACTOR_EXPORT Q_DECL_IMPORT
#endif
#endif
#endif // GAEACTOR_GLOBAL_H
