#ifndef nettols_h
#define nettols_h

#include "types.h"

void parseHostString(const std::wstring & host, std::wstring & address, quint16 & port, quint16 defPort);

// return false if failed to read file
// return options = -1 if no file exists / no encryption set
bool readDbServer(const std::wstring & filename,
                  int & clientOption, barray & clientKey,
                  int & serverOption, barray & serverKey);

#endif