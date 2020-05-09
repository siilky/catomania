#ifndef options_h_
#define options_h_

#include "persistence2.h"
#include "tristate.h"


class Options
{
public:
    Options()
    {}

    Options(const JsonValue & config);
    void writeTo(JsonValue & config);

    void validate();

    Options & operator<<(const Options & r);            // merge to multioptions
    const Options & operator>>(Options & r) const;      // extract (apply) to single


    Tristate<bool>      forcedLogin;
    Tristate<bool>      autoLogin;
    Tristate<bool>      autoShop;
    Tristate<bool>      autoReconnect;

    Tristate<bool>      reconnectDelayOnStartup;
    Tristate<int>       reconnectDelayFixed, reconnectDelayRandom;

    Tristate<bool>      useReopen;
    Tristate<int>       reopenModeSelector;
    Tristate<bool>      useReopenPercentage, useReopenValue;
    Tristate<int>       reopenPercentage, reopenValue;
    Tristate<int>       closeDelayFixed, closeDelayRandom;
    Tristate<int>       openDelayFixed, openDelayRandom;
    Tristate<bool>      useReopenExternal;
    Tristate<int>       reopenExternalInterval;
    Tristate<bool>      saveExternal;

    Tristate<int>           maxLogLines;
    Tristate<bool>          saveLogToFile;
    Tristate<bool>          splitLogFilesPerDay;
    Tristate<std::wstring>  logPath;

    Tristate<bool>      tropophobia;
    Tristate<bool>      onlineReg;

    enum ProxyType {
        ProxySocks = 0,
        ProxyHttp,
    };

    Tristate<bool>          useProxy;
    Tristate<ProxyType>     proxyType;
    Tristate<std::wstring>  proxyAddr;
    Tristate<std::wstring>  proxyUser;
    Tristate<std::wstring>  proxyPass;

    Tristate<bool>          lowConverter;       // convert to money if less
    Tristate<unsigned>      lowConverterValue;
    Tristate<bool>          highConverter;      // convert to notes if greater
    Tristate<unsigned>      highConverterValue;

    Tristate<int>           chatBufferSize;
};


#endif