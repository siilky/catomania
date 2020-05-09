#pragma once

#pragma warning(push)
//#pragma warning(disable : 4996)     // unsafe function calls
#pragma warning(disable : 4127)     // conditional expression is constant
#pragma warning(disable : 4251)     // class 'QSharedDataPointer<T>' needs to have dll-interface to be used by clients of class 'QNetworkProxyQuery'
#pragma warning(disable : 4481)     // nonstandard extension used: override specifier 'override'
#pragma warning(disable : 4503)     // decorated name was truncated
#pragma warning(disable : 4512)     // assignment operator could not be generated
//#pragma warning(disable : 4714)		// function 'QString QString::toLower(void) &&' marked as __forceinline not inlined
#pragma warning(disable : 6011)     // Dereferencing null pointer
#pragma warning(disable : 6326)     // Potential comparison of a constant with another constant.
#pragma warning(disable : 6385)     // Invalid data
#pragma warning(disable : 6386)     // Buffer overrun

#pragma warning(disable : 26495)
#pragma warning(disable : 26451)
#pragma warning(disable : 28182)    // Dereferencing null pointer

#include <QtCore>
#include <QtGui>
// #include <QtWidgets>
#include <QtNetwork>

#include <assert.h>

#pragma warning(pop)
