
#include "stdafx.h"

#include "serverlist.h"

bool loadServers(std::vector<Server> & servers)
{
    QFile file(QApplication::applicationDirPath() + QDir::separator() + "serverlist.txt");
    if ( ! file.exists()
        || ! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    // file goes in unicode
    QTextStream text(&file);
    text.setCodec("UTF-16");

    while ( ! text.atEnd())
    {
        QString line = text.readLine();
        QStringList parts = line.split(QRegularExpression("\\t+"), QString::SkipEmptyParts);
        if (parts.count() == 1)
        {
            // legacy style
            parts = line.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
        }
        if (parts.count() > 2)
        {
            Server s;
            s.name = parts[0].trimmed().toStdWString();

            QString server = parts[1].trimmed();
            if (server.count(':') == 1)
            {
                int idx = server.indexOf(':');
                s.port = server.mid(0, idx).toStdWString();
                s.host = server.mid(idx + 1).toStdWString();
            }
            else
            {
                s.host = server.toStdWString();
            }

            servers.push_back(s);
        }
    }

    return true;
}
