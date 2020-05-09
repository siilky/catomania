#ifndef types_h
#define types_h

#include <wtypes.h>
#include <vector>
#include <fstream>

#define NAKED           __declspec(naked)
#define sizeof_array(z)	(sizeof(z)/sizeof(z[0]))
#undef min
#undef max

enum coreExceptionCode
{
    eNoError = 0,
    eHandleError,
    eResourceError,
    eCrtError,
    eWinapiError,
    eMemoryReadError,
    eArgumentError,
    eStateError,
    eCannotOpenProcess,
    eCannotSetProtect,
    eCannotCreateThread,
};

typedef unsigned char	                byte;
typedef unsigned long long              Timestamp;      // ms

typedef std::basic_ofstream<TCHAR>      tofstream;
typedef std::basic_ostream<TCHAR>       tostream;
typedef std::basic_ios<TCHAR>           tios;
typedef std::basic_ostringstream<TCHAR> tostringstream;
typedef std::basic_stringstream<TCHAR>  tstringstream;

typedef std::basic_string<TCHAR>	    tstring;
typedef std::vector<byte>               barray;

// Classes

template<typename T>
class Singleton
{
public:
    static T & Instance()
    {
        static T theSingleInstance;
        return theSingleInstance;
    }
};


#endif