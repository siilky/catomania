
#include "stdafx.h"

#include "options.h"
#include "config.h"


Options::Options(const JsonValue & config)
{
    config.get(L"ForcedLogin", forcedLogin, Tristate<bool>(false));
    config.get(L"AutoLogin", autoLogin, Tristate<bool>(false));
    config.get(L"AutoShop", autoShop, Tristate<bool>(false));
    config.get(L"AutoReconnect", autoReconnect, Tristate<bool>(false));

    config.get(L"ReconnectDelayOnStartup", reconnectDelayOnStartup, Tristate<bool>(false));
    config.get(L"AutoReconnectInterval", reconnectDelayFixed, Tristate<int>(300));
    config.get(L"AutoReconnectIntervalRandom", reconnectDelayRandom, Tristate<int>(60));

    config.get(L"AutoReopen", useReopen, Tristate<bool>(true));
    config.get(L"AutoReopenSelector", reopenModeSelector, Tristate<int>(10));
    config.get(L"AutoReopenByPercentage", useReopenPercentage, Tristate<bool>(true));
    config.get(L"AutoReopenByValue", useReopenValue, Tristate<bool>(true));
    config.get(L"AutoReopenPercent", reopenPercentage, Tristate<int>(50));
    config.get(L"AutoReopenValue", reopenValue, Tristate<int>(1));
    config.get(L"AutoReopenCloseDelayFixed", closeDelayFixed, Tristate<int>(60));
    config.get(L"AutoReopenCloseDelayRandom", closeDelayRandom, Tristate<int>(60));
    config.get(L"AutoReopenOpenDelayFixed", openDelayFixed, Tristate<int>(60));
    config.get(L"AutoReopenOpenDelayRandom", openDelayRandom, Tristate<int>(60));
    config.get(L"AutoReopenExternal", useReopenExternal, Tristate<bool>(false));
    config.get(L"AutoReopenExternalInterval", reopenExternalInterval, Tristate<int>(60));
    config.get(L"AutoReopenSaveExternal", saveExternal, Tristate<bool>(false));

    config.get(L"MaxLogLines", maxLogLines, Tristate<int>(0));
    config.get(L"SaveLogToFile", saveLogToFile, Tristate<bool>(true));
    config.get(L"SplitLogFilesPerDay", splitLogFilesPerDay, Tristate<bool>(false));
    config.get(L"LogPath", logPath);

    config.get(L"Tropophobia", tropophobia, Tristate<bool>(false));
    config.get(L"OnlineRegistration", onlineReg, Tristate<bool>(true));

    config.get(L"ProxyEnable", useProxy, Tristate<bool>(false));

    int pType;
    config.get(L"ProxyType", pType, (int)ProxySocks);
    proxyType = ProxyType(pType);
    config.get(L"ProxyAddr", proxyAddr);
    config.get(L"ProxyUser", proxyUser);
    config.get(L"ProxyPassword", proxyPass);

    config.get(L"LowConverter", lowConverter, Tristate<bool>(false));
    config.get(L"LowConverterValue", lowConverterValue, Tristate<unsigned>(10));
    config.get(L"HighConverter", highConverter, Tristate<bool>(false));
    config.get(L"HighConverterValue", highConverterValue, Tristate<unsigned>(10));

    config.get(L"ÑhatBufferSize", chatBufferSize, Tristate<int>(2));

    validate();
}

void Options::writeTo(JsonValue & config)
{
    config.set(L"ForcedLogin",                  forcedLogin.value());
    config.set(L"AutoLogin",                    autoLogin.value());
    config.set(L"AutoShop",                     autoShop.value());
    config.set(L"AutoReconnect",                autoReconnect.value());

    config.set(L"ReconnectDelayOnStartup",      reconnectDelayOnStartup.value());
    config.set(L"AutoReconnectInterval",        reconnectDelayFixed.value());
    config.set(L"AutoReconnectIntervalRandom",  reconnectDelayRandom.value());

    config.set(L"AutoReopen",                   useReopen.value());
    config.set(L"AutoReopenSelector",           reopenModeSelector.value());
    config.set(L"AutoReopenByPercentage",       useReopenPercentage.value());
    config.set(L"AutoReopenByValue",            useReopenValue.value());
    config.set(L"AutoReopenPercent",            reopenPercentage.value());
    config.set(L"AutoReopenValue",              reopenValue.value());
    config.set(L"AutoReopenCloseDelayFixed",    closeDelayFixed.value());
    config.set(L"AutoReopenCloseDelayRandom",   closeDelayRandom.value());
    config.set(L"AutoReopenOpenDelayFixed",     openDelayFixed.value());
    config.set(L"AutoReopenOpenDelayRandom",    openDelayRandom.value());
    config.set(L"AutoReopenExternal",           useReopenExternal.value());
    config.set(L"AutoReopenExternalInterval",   reopenExternalInterval.value());
    config.set(L"AutoReopenSaveExternal",       saveExternal.value());

    config.set(L"MaxLogLines",                  maxLogLines.value());
    config.set(L"SaveLogToFile",                saveLogToFile.value());
    config.set(L"SplitLogFilesPerDay",          splitLogFilesPerDay.value());
    config.set(L"LogPath",                      logPath.value());

    config.set(L"Tropophobia",                  tropophobia.value());
    config.set(L"OnlineRegistration",           onlineReg.value());

    config.set(L"ProxyEnable",                  useProxy.value());
    config.set(L"ProxyType",                    proxyType.value());
    config.set(L"ProxyAddr",                    proxyAddr.value());
    config.set(L"ProxyUser",                    proxyUser.value());
    config.set(L"ProxyPassword",                proxyPass.value());

    config.set(L"LowConverter",                 lowConverter.value());
    config.set(L"LowConverterValue",            lowConverterValue.value());
    config.set(L"HighConverter",                highConverter.value());
    config.set(L"HighConverterValue",           highConverterValue.value());

    config.set(L"ÑhatBufferSize",               chatBufferSize.value());
}

Options & Options::operator<<(const Options & r)
{
    forcedLogin             << r.forcedLogin;
    autoLogin               << r.autoLogin;
    autoShop                << r.autoShop;
    autoReconnect           << r.autoReconnect;

    reconnectDelayOnStartup << r.reconnectDelayOnStartup;
    reconnectDelayFixed     << r.reconnectDelayFixed;
    reconnectDelayRandom    << r.reconnectDelayRandom;

    useReopen               << r.useReopen;
    reopenModeSelector      << r.reopenModeSelector;
    useReopenPercentage     << r.useReopenPercentage;
    useReopenValue          << r.useReopenValue;
    reopenPercentage        << r.reopenPercentage;
    reopenValue             << r.reopenValue;
    closeDelayFixed         << r.closeDelayFixed;
    closeDelayRandom        << r.closeDelayRandom;
    openDelayFixed          << r.openDelayFixed;
    openDelayRandom         << r.openDelayRandom;
    useReopenExternal       << r.useReopenExternal;
    reopenExternalInterval  << r.reopenExternalInterval;
    saveExternal            << r.saveExternal;

    tropophobia             << r.tropophobia;
    onlineReg               << r.onlineReg;

    useProxy                << r.useProxy;
    proxyAddr               << r.proxyAddr;
    proxyType               << r.proxyType;
    proxyUser               << r.proxyUser;
    proxyPass               << r.proxyPass;

    lowConverter            << r.lowConverter;
    lowConverterValue       << r.lowConverterValue;
    highConverter           << r.highConverter;
    highConverterValue      << r.highConverterValue;

    chatBufferSize          << r.chatBufferSize;

    return *this;
}

const Options & Options::operator>>(Options & r) const
{
    forcedLogin             >> r.forcedLogin;
    autoLogin               >> r.autoLogin;
    autoShop                >> r.autoShop;
    autoReconnect           >> r.autoReconnect;

    reconnectDelayOnStartup >> r.reconnectDelayOnStartup;
    reconnectDelayFixed     >> r.reconnectDelayFixed;
    reconnectDelayRandom    >> r.reconnectDelayRandom;

    useReopen               >> r.useReopen;
    reopenModeSelector      >> r.reopenModeSelector;
    useReopenPercentage     >> r.useReopenPercentage;
    useReopenValue          >> r.useReopenValue;
    reopenPercentage        >> r.reopenPercentage;
    reopenValue             >> r.reopenValue;
    closeDelayFixed         >> r.closeDelayFixed;
    closeDelayRandom        >> r.closeDelayRandom;
    openDelayFixed          >> r.openDelayFixed;
    openDelayRandom         >> r.openDelayRandom;
    useReopenExternal       >> r.useReopenExternal;
    reopenExternalInterval  >> r.reopenExternalInterval;
    saveExternal            >> r.saveExternal;

    tropophobia             >> r.tropophobia;
    onlineReg               >> r.onlineReg;

    useProxy                >> r.useProxy;
    proxyAddr               >> r.proxyAddr;
    proxyType               >> r.proxyType;
    proxyUser               >> r.proxyUser;
    proxyPass               >> r.proxyPass;

    lowConverter            >> r.lowConverter;
    lowConverterValue       >> r.lowConverterValue;
    highConverter           >> r.highConverter;
    highConverterValue      >> r.highConverterValue;

    chatBufferSize          >> r.chatBufferSize;

    return *this;
}

void Options::validate()
{
    if (reopenPercentage < 0 || reopenPercentage > 100)
    {
        reopenPercentage = 50;
    }

    if (reopenValue < 1)
    {
        reopenValue = 1;
    }

    if (proxyAddr.value().empty())
    {
        useProxy = false;
    }

    if (maxLogLines < 0)
    {
        maxLogLines = 0;
    }

    if (proxyType != ProxySocks && proxyType != ProxyHttp)
    {
        proxyType = ProxySocks;
    }

    if (lowConverterValue <= 0)
    {
        lowConverter = false;
    }
    if (highConverterValue <= 0)
    {
        highConverter = false;
    }
    if (highConverterValue < 2)
    {
        if (lowConverter)
        {
            highConverterValue = 2;
        }
        else
        {
            highConverterValue = 1;
        }
    }
    if (lowConverter && highConverter
        && highConverterValue <= lowConverterValue)
    {
        highConverterValue = lowConverterValue + 1;
    }
}
