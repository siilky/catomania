#include "stdafx.h"

#include "connection.h"
#include "tea.h"
#include "fragments.h"

namespace ARC
{

void FragmentProcessor::process(FragmentBase *fragment)
{
    if (doEncode_)
    {
        auto f = static_cast<ARC::Fragment*>(fragment);
        f->encode(teaKey_);
    }

    ::FragmentProcessor::process(fragment);
}

//

Connection::Connection()
{
    serverProcessor_ = new FragmentProcessor();
    clientProcessor_ = new FragmentProcessor();

    serverFactory_ = new ARC::FragmentFactory(ARC::fragmentFactory);    // copy!
}

Connection::~Connection()
{
    delete serverFactory_;
    serverFactory_ = nullptr;
}

bool Connection::open(const std::wstring & adddress, const QNetworkProxy &proxy)
{
    error_.clear();
    close();

    auto channel = new NetChannelTcp();
    channel->setProxy(proxy);

    QObject::connect(channel, &NetChannelTcp::connected,    this, &Connection::connected);
    QObject::connect(channel, &NetChannelTcp::disconnected, this, &Connection::disconnected, Qt::QueuedConnection);
    QObject::connect(channel, &NetChannelTcp::dataIn,       this, &Connection::onDataIn);

    server_ = channel;
    if (!channel->connect(adddress))
    {
        error_ = server_->getError();
        close();
        return false;
    }

    return true;
}

void Connection::close()
{
    delete server_;
    server_ = nullptr;
}

void Connection::setTeaKey(const std::array<uint32_t, 4> & key)
{
    static_cast<ARC::FragmentFactory*>(serverFactory_)->setTeaDecoder(key);
    static_cast<ARC::FragmentProcessor*>(clientProcessor_)->setTeaEncoder(key);
}

void Connection::onDataIn(const QByteArray & /*data*/)
{
    ConnectionBase::receive();
}

}
