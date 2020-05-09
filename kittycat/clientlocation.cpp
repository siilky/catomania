
#include "stdafx.h"


#include "clientlocation.h"
#include "log.h"


QString getClientLocation()
{
    QSettings reg( "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Perfect World"
                 , QSettings::NativeFormat);

    QString clientPath;
    if (reg.contains("InstallLocation"))
    {
        return reg.value("InstallLocation").toString();
    }

    return QString();

    //return isClientPathValid(clientPath) ? clientPath : QString();
}


