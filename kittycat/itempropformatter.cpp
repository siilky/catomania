
#include "stdafx.h"

#include "itempropformatter.h"

QString itemPropsToString(const QString & itemName, const ItemProps * props)
{
    if ( ! props)
    {
        return QString();
    }

    switch (props->type)
    {
        case ItemPropTypeFashion:
        {
            const FashionProps *p = static_cast<const FashionProps *>(props);
            QColor color(p->color);
            return QObject::tr("<b>%1</b>"
                "<p>Required level: %2"
                "<br>Color: <font color=%3>%4%4%4</font>"
                )
                .arg(itemName)
                .arg(p->levelReq)
                .arg(color.name())
                .arg(QChar(0x25A0));
        }

        case ItemPropTypeFlyMount:
        {
            const FlyMountProps *p = static_cast<const FlyMountProps *>(props);
            return QObject::tr("<b>%1</b>"
                "<p>Level: %2"
                "<br>Required level: %3"
                "<br>Speed: +%4"
                "<br>Max speed: +%5"
                "<br>Duration: %6/%7"
                )
                .arg(itemName)
                .arg(p->level)
                .arg(p->levelReq)
                .arg(p->speed, 0, 'f', 2, '0')
                .arg(p->maxSpeed, 0, 'f', 2, '0')
                .arg(p->energy).arg(p->maxEnergy)
                ;
        }

        case ItemPropTypeGenie:
        {
            const GeniePropsImpl *p = static_cast<const GeniePropsImpl *>(props);
            return QObject::tr("<b>%1</b>"
                "<p>Level: %2"
                "<br>Points: %3/%4"
                )
                .arg(itemName)
                .arg(p->level)
                .arg(p->totalPoints - p->level)
                .arg(p->level - (p->level % 10));
        }

        default:
            break;
    }
    return QString();
}

