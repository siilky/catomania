#pragma once

#pragma warning(disable : 4503)     // decorated name length exceeded, name was truncated
#pragma warning(disable : 4505)     // 'QwtArraySeriesData<T>::size' : unreferenced local function has been removed
#pragma warning(disable : 4510)     // default constructor could not be generated
#pragma warning(disable : 4512)     // assignment operator could not be generated
#pragma warning(disable : 4610)     // struct '*' can never be instantiated - user defined constructor required
#pragma warning(disable : 4714)     // QTBUG-55042 \ function 'QString QString::toLower(void) const &' marked as __forceinline not inlined

#pragma warning(push)

#pragma warning(disable : 4127)     // conditional expression is constant
#pragma warning(disable : 4251)     // class 'QSharedDataPointer<T>' needs to have dll-interface to be used by clients of class 'QNetworkProxyQuery'
#pragma warning(disable : 4800)     // 'uint' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable : 4996)     // unsafe function calls
#pragma warning(disable : 6011)     // Dereferencing null pointer
#pragma warning(disable : 6326)     // Potential comparison of a constant with another constant.
#pragma warning(disable : 6385)     // Invalid data
#pragma warning(disable : 6386)     // Buffer overrun

#pragma warning(disable : 26495)
#pragma warning(disable : 26451)

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>
#include <QtCharts>

// #define _SECURE_SCL 1
// #define _SECURE_SCL_THROWS 1
#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>
#include <assert.h>
#include <stdexcept>
#include <algorithm>


#pragma warning(pop)

#include <set>
#include <vector>
#include <array>
#include <memory>

#include "config.h"
