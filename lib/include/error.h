#ifndef error_h
#define error_h

#include <map>

#include "types.h"

enum
{
    ERR_NO_ERROR    = 0,
    ERR_EXCEPTION,
    ERR_INVALID_ARGUMENT,
    ERR_INVALID_DATA,
    ERR_CONFIG,
    ERR_DATABASE,
    ERR_SYSTEM,
    ERR_NETWORK,                // error related to network data
    ERR_SERVER
};

//------------------------------------------------------------------------------
class ErrorState
{
public:
    ErrorState()
    {
        clear();
    }
    virtual ~ErrorState()
    {}

    void clear()
    {
        code = ERR_NO_ERROR;
        codeExt = 0;
        message = L"No error(s).";
    }

    void set(int code__)
    {
        code = code__;
        codeExt = 0;
        message = L"";
    }

//     void set(int code, int codeExt, const std::string & what)
//     {
//         set(code, codeExt, strToWStr(what));
//     }
// 
    void set(int code__, int codeExt__, const std::wstring & what__)
    {
        code = code__;
        codeExt = codeExt__;
        message = what__;
    }

    void setFromGLE(int code__ = ERR_SYSTEM, DWORD gleCode = GetLastError())
    {
        code = code__;
        codeExt = gleCode;

        void *msgBuf;
        if ( FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                           , NULL
                           , codeExt
                           , MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
                           , (LPWSTR) &msgBuf
                           , 0, NULL
                           ) != 0 )
        {
            message = std::wstring((WCHAR*)msgBuf);
            LocalFree(msgBuf);
        }
    }

    int             code;          // basic error code
    int             codeExt;       // extended error code
    std::wstring    message;       // Text message explaining the error occurred
};

//------------------------------------------------------------------------------
class ServerError : private Singleton<ServerError>
{
public:
    ServerError();

    static std::wstring getString(int code);

private:

    std::map<int, std::wstring>     errors_;
};


#endif