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

// --- WTL stuff

#pragma warning(disable : 6386)
// #pragma warning(disable : 6387)     // 'argument 1' might be '0': this does not adhere to the specification for the function 'lstrlenW': Lines: 3914, 3915, 3916, 3918, 3919, 3920, 3922
// #pragma warning(disable : 6269)
// #pragma warning(disable : 6400)     // Using 'lstrcmpiW' to perform a case-insensitive compare to constant string 'static'. Yields unexpected results in non-English locales
// #pragma warning(disable : 6401)     // Using 'CompareStringW' in a default locale to perform a case-insensitive compare to constant string '<A>'. Yields unexpected results in non-English locales
// #pragma warning(disable : 6209)     // Using 'sizeof scanInterval_s' as parameter '3' in call to 'WTL::CWinDataExchange<CMainDlg>::DDX_Text' where 'scanInterval_s' might be an array of wide characters, did you intend to use character count rather than byte count ?
// #pragma warning(disable : 6211)     // Leaking memory 'pParam' due to an exception. Consider using a local catch block to clean up memory
// #pragma warning(disable : 6309)     // Argument '1' is null: this does not adhere to function specification of 
// #pragma warning(disable : 6001)     // Using uninitialized memory 
// 
//     #include <atlbase.h>
//     #include <atlapp.h>
//     #include <atlwin.h>
//     #include <atlframe.h>
//     #include <atlctrls.h>
//     #include <atlctrlx.h>
//     #include <atlcrack.h>
//     #include <atlmisc.h>
//     #include <atlddx.h>
// 

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

#pragma warning(pop)
