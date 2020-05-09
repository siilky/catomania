#ifndef log_h
#define log_h

#ifdef _DEBUG
    void debugInit(const wchar_t *filename);
    void debugDestroy();
    void debugPrint(const char *format, ...);
#else
    inline void debugInit(const wchar_t *filename) {}
    inline void debugDestroy() {}
    inline void debugPrint(const char *format, ...) {}
#endif

#ifdef _DEBUG
    #define Log(...)	debugPrint(__FUNCTION__": "__VA_ARGS__)
#else
    #define Log(...)	
#endif


#endif
 