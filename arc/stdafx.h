#pragma once


#pragma warning(push)
//#pragma warning(disable : 4996)     // unsafe function calls
#pragma warning(disable : 4127)     // conditional expression is constant
#pragma warning(disable : 6011)     // Dereferencing null pointer
#pragma warning(disable : 4251)     // class 'QSharedDataPointer<T>' needs to have dll-interface to be used by clients of class 'QNetworkProxyQuery'
#pragma warning(disable : 4996)     // 'std::copy::_Unchecked_iterators::_Deprecate': Call to 'std::copy' with parameters that may be unsafe

#pragma warning(disable : 26451)
#pragma warning(disable : 26495)
#pragma warning(disable : 28182)    // Dereferencing null pointer

#include <QtCore>
#include <QtNetwork>

#pragma warning(pop)

#include <assert.h>

#include <memory>
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <set>

