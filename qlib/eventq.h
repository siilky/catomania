#ifndef eventq_h_
#define eventq_h_

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QSharedPointer>
#include <QMetatype>

#include "event.h"


class EventQ : public QObject
{
    Q_OBJECT
public:

    void push(const BaseEvent *e);
    BaseEvent *pop();

Q_SIGNALS:
    void eventAdded();

private:
    QMutex              lock_;
    QQueue<BaseEvent *> queue_;
};

#endif