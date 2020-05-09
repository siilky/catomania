
#include "stdafx.h"

#include "clicker.h"


Clicker::Clicker(QObject *parent /*= 0*/)
    : QObject(parent)
{
}

bool Clicker::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        clicked();
    }

    return QObject::eventFilter(obj, event);
}
