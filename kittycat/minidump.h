#ifndef minidump_h_
#define minidump_h_

#include <windows.h>

long __stdcall crashHandler(EXCEPTION_POINTERS* pep);

#endif