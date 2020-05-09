#ifndef auth_errors_h_
#define auth_errors_h_


#if defined MAILRU_TOKEN_AUTH
    QString mailruAuthErrorToString(int errorCode);
#elif defined ARC_TOKEN_AUTH
    QString arcAuthErrorToString(int errorCode);
#endif

#endif