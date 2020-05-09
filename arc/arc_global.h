#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(ARC_LIB)
#  define ARC_EXPORT Q_DECL_EXPORT
# else
#  define ARC_EXPORT Q_DECL_IMPORT
# endif
#else
# define ARC_EXPORT
#endif
