#include "stdafx.h"

#include "nettools.h"

void parseHostString(const std::wstring & host, std::wstring & address, quint16 & port, quint16 defPort)
{
    size_t delim = host.find(':');
    if (delim == host.npos)
    {
        port = defPort;
        address = host;
    }
    else
    {
        std::wstring sPort = host.substr(delim + 1);
        port = quint16(QString::fromStdWString(sPort).toInt());
        if (port == 0)
        {
            port = defPort;
        }

        address = host.substr(0, delim);
    }
}

bool readDbServer(const std::wstring & filename,
                  int & clientOption, barray & clientKey,
                  int & serverOption, barray & serverKey)
{
    auto name = QString::fromStdWString(filename);
    if (!QFile::exists(name))
    {
        clientOption = serverOption = -1;
        return true;
    }

    QSettings file(name, QSettings::IniFormat);

    if (file.status() != QSettings::NoError)
    {
        qDebug() << "Error reading dbserver file" << file.status();
        return false;
    }

    file.beginGroup("GameClient");
    auto isec = file.value("isec");
    auto osec = file.value("osec");
    if (!isec.isNull() && !osec.isNull())
    {
        serverOption = isec.toInt();
        clientOption = osec.toInt();

        auto iKey = file.value("iseckey").toByteArray();
        serverKey.assign(iKey.data(), iKey.data() + iKey.size());

        auto oKey = file.value("oseckey").toByteArray();
        clientKey.assign(oKey.data(), oKey.data() + oKey.size());

        return true;
    }

    //
    clientOption = serverOption = -1;
    return true;
}
