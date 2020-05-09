#include "StdAfx.h"

#include "PortManager.h"


PortManager::PortManager()
{
}

PortManager::~PortManager()
{
}

PortManager * PortManager::instance()
{
    static PortManager manager_;
    return &manager_;
}

quint16 PortManager::acquire()
{
    for (quint16 port = FirstAvailable; port < LastAvailable; ++port)
    {
        if (ports_.find(port) == ports_.end())
        {
            ports_.insert(port);
            return port;
        }
    }
    return 0;
}

void PortManager::release(quint16 port)
{
    if (port == 0)
    {
        return;
    }

    auto it = ports_.find(port);
    assert(it != ports_.end());
    ports_.erase(it);
}

//

UniquePort::UniquePort()
{
    value_ = PortManager::instance()->acquire();
}

UniquePort::~UniquePort()
{
    PortManager::instance()->release(value_);
}
