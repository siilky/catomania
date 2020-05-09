#include "stdafx.h"

#include "eventq.h"


void EventQ::push(const BaseEvent *e)
{
    QMutexLocker lock(&lock_);
    queue_.enqueue(e->clone());
    eventAdded();
}

BaseEvent * EventQ::pop()
{
    QMutexLocker lock(&lock_);

    return queue_.isEmpty() ? NULL : queue_.dequeue();
}
