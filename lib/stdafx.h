#pragma once

// Change these values to use different versions
#ifndef WINVER
#define WINVER		    0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x0502
#endif

#ifndef _WIN32_IE
#define _WIN32_IE	    0x0501
#endif

#pragma warning(disable : 4127)     // conditional expression is constant
#pragma warning(disable : 4267)     // conversion from 'size_t' to 'unsigned int', possible loss of data
#pragma warning(disable : 4503)     // decorated name length exceeded, name was truncated
#pragma warning(disable : 4996)     // unsafe function calls

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#pragma warning(push)

#pragma warning(disable : 4512)     // assignment operator could not be generated
#pragma warning(disable : 6246)     // Local declaration of 'pstr' hides declaration of the same name in outer scope

// --- WIN32

#define WIN32_LEAN_AND_MEAN		        // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Tlhelp32.h>
#include <process.h>
#include <shlwapi.h>

#pragma warning(pop)

#include <tchar.h>
#include <stdio.h>
#include <assert.h>

#pragma warning(push)
#pragma warning(disable : 6011)         // Dereferencing null pointer

#include <map>
#include <queue>
#include <string>
#include <stack>
#include <vector>
#include <set>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <locale>
#include <cwchar>
#include <iterator>
#include <memory>

#include <fstream>
#include <functional>
using namespace std::placeholders;

#if defined(_DEBUG)
    #define FRAGMENT_PRINTABLE
#endif

#pragma warning(pop)
