#include "stdafx.h"

#include "game/iconmanager.h"


#pragma warning(disable : 4714)		// function 'QString QString::toLower(void) &&' marked as __forceinline not inlined


IconManager::IconManager()
{
}

IconManager * IconManager::instance()
{
    static IconManager iconManager;
    return & iconManager;
}

void IconManager::load(const QByteArray & descriptionFile, const QByteArray & ddsFile)
{
    QTextStream txtStream(descriptionFile);
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    txtStream.setCodec(codec);

    iconHeight = txtStream.readLine().toInt();
    iconWidth = txtStream.readLine().toInt();
    countIconHeight = txtStream.readLine().toInt();
    countIconWidth = txtStream.readLine().toInt();

    quint32 x = 0, y = 0;
    while (!txtStream.atEnd())
    {
        IconProperty ip;
        ip.name = txtStream.readLine().toLower();
        assert(!ip.name.isEmpty());
        ip.x = iconWidth * x;
        ip.y = iconHeight * y;
        icons_.insert(ip.name, ip);

        if (++x >= countIconWidth)
        {
            x = 0;
            y++;
        }
    }

    ddsImage_.loadFromData(ddsFile);
}

QImage IconManager::getIconByPath(const QString & iconPath) const
{
	QString name = iconPath.section('\\', -1, -1).toLower();
    QHash<QString, IconProperty>::const_iterator it = icons_.find(name);
    if (it != icons_.end())
    {
        return ddsImage_.copy(QRect(it->x, it->y, iconWidth, iconHeight));
    }
    return ddsImage_.copy(QRect(0, 0, iconWidth, iconHeight));
}
