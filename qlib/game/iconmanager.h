#pragma once

class IconManager
{
public:
    static IconManager * instance();

    void load(const QByteArray & descriptionFile, const QByteArray & ddsFile);

    QImage getIconByPath(const QString & iconPath) const;

private:
    IconManager();

    struct IconProperty
    {
        QString name;
        quint32 x;
        quint32 y;
    };

    QImage  ddsImage_;
    quint32 iconHeight;
    quint32 iconWidth;
    quint32 countIconHeight;
    quint32 countIconWidth;
    QHash<QString, IconProperty> icons_;
};

