#include "stdafx.h"

#include "qchanneltcp.h"
#include "connectiontcp.h"
#include "nettools.h"



NetChannelTcp::NetChannelTcp(QObject *parent)
    : QObject(parent)
    , sliceSize_()
    , socket_(new QTcpSocket())
{
    setup();
}

NetChannelTcp::NetChannelTcp(QTcpSocket *socket)
    : QObject(socket)
    , sliceSize_()
    , socket_(socket)
{
    setup();
}

NetChannelTcp::~NetChannelTcp()
{
    // when closing on disconnected signal should defer deletion
    if (socket_->parent() == nullptr)
    {
        socket_->close();
        socket_->deleteLater();
        socket_ = nullptr;
    }
}

void NetChannelTcp::setup()
{
    QObject::connect(socket_, &QTcpSocket::connected,       this, &NetChannelTcp::onConnected);
    QObject::connect(socket_, &QTcpSocket::connected,       this, &NetChannelTcp::connected);
    QObject::connect(socket_, &QTcpSocket::disconnected,    this, &NetChannelTcp::onDisconnected);
    QObject::connect(socket_, &QTcpSocket::disconnected,    this, &NetChannelTcp::disconnected);
    QObject::connect(socket_, &QTcpSocket::readyRead,       this, &NetChannelTcp::onReadyRead);
    QObject::connect(socket_, &QAbstractSocket::errorOccurred, this, &NetChannelTcp::onError);

    QObject::connect(&connectTimer_, &QTimer::timeout,      this, &NetChannelTcp::onConnectTimeout);

    socket_->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    socket_->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
}

void NetChannelTcp::setSlice(unsigned sliceSize)
{
    sliceSize_ = sliceSize;
}

void NetChannelTcp::setProxy(const QNetworkProxy & proxy)
{
    proxy_ = proxy;
    // disable name resolution through proxy
    proxy_.setCapabilities(QNetworkProxy::TunnelingCapability);
}

bool NetChannelTcp::connect(const std::wstring & adddress)
{
    if (adddress.empty())
    {
        error_.set(ERR_INVALID_ARGUMENT, 0, L"Server parameter is empty");
        return false;
    }

    quint16 port;
    std::wstring host;
    parseHostString(adddress, host, port, 29000);

    if (host.empty())
    {
        error_.set(ERR_INVALID_ARGUMENT, 0, L"Invalid server address");
        return false;
    }

    Log("Server:%ls:%i", host.c_str(), port);
    readQ_.clear();

    socket_->setProxy(proxy_);
    socket_->connectToHost(QString::fromStdWString(host), port);

    connectTimer_.start(ConnectTimeout);

    return true;
}

bool NetChannelTcp::isConnected() const
{
    return socket_->state() == QAbstractSocket::ConnectedState;
}

bool NetChannelTcp::peek(barray & data)
{
    data = barray(readQ_.begin(), readQ_.end());
    return true;
}

bool NetChannelTcp::read(barray & data)
{
    data = barray(readQ_.begin(), readQ_.end());
    readQ_.clear();
    return true;
}

bool NetChannelTcp::read(size_t bytes)
{
    readQ_.remove(0, bytes);
    return true;
}

bool NetChannelTcp::write(const barray & data)
{
    if (!isConnected())
    {
        return false;
    }

    assert(QThread::currentThread() == thread());

    qint64 res = 0;
    size_t bytesLeft = data.size();
    auto iPos = data.begin();

    do 
    {
        size_t bytesToSend = sliceSize_ > 0 ? std::min(bytesLeft, sliceSize_) : bytesLeft;
        bytesLeft -= bytesToSend;

        barray out = processDataOut(barray(iPos, iPos + bytesToSend));
        if (!out.empty())
        {
            res = socket_->write((char*) &out[0], out.size());
            if (bytesLeft > 0
                || socket_->thread() != QThread::currentThread())
            {
                // otherwise socket write (notifiers) will be broken
                socket_->flush();
            }
        }
        iPos += bytesToSend;

    } while (bytesLeft > 0);

    return res != -1;
}

void NetChannelTcp::write(const QByteArray & data)
{
    write(barray(data.constBegin(), data.constEnd()));
}

void NetChannelTcp::setError(const std::wstring & msg)
{
    error_.set(ERR_NETWORK, 0, msg);
    if (socket_->state() == QAbstractSocket::UnconnectedState
        || socket_->state() == QAbstractSocket::HostLookupState
        || socket_->state() == QAbstractSocket::ConnectingState)
    {
        // close will not emit disconnected these cases
        disconnected();
    }
    socket_->close();
}

//

barray NetChannelTcp::processDataIn(const barray & data)
{
    return data;
}

barray NetChannelTcp::processDataOut(const barray & data)
{
    return data;
}

//

void NetChannelTcp::onConnected()
{
    connectTimer_.stop();
}

void NetChannelTcp::onDisconnected()
{
    connectTimer_.stop();
}

void NetChannelTcp::onError(QAbstractSocket::SocketError socketError)
{
    connectTimer_.stop();
    setError((socket_->errorString() + QString(" (%1)").arg(socketError)).toStdWString());
}

void NetChannelTcp::onReadyRead()
{
    readFromSocket();
}

void NetChannelTcp::onConnectTimeout()
{
    setError(tr("Connection timed out").toStdWString());
}

//

void NetChannelTcp::readFromSocket()
{
    if (socket_->bytesAvailable() == 0)
    {
        Log("RFS BA 0");
        return;
    }

    QByteArray data = socket_->readAll();
    if (data.isEmpty())
    {
        Log("RFS Empty read");
        return;
    }
    //else
    //{
    //    Log("RFS %i", data.size());
    //}

    barray out = processDataIn(barray(data.begin(), data.end()));
    if (!out.empty())
    {
        QByteArray bOut((char*)out.data(), out.size());
        readQ_.append(bOut);
        dataIn(bOut);
    }
}

//

QChannelTcp::QChannelTcp(QObject *parent)
    : NetChannelTcp(parent)
{
    QObject::connect(&keepaliveTimer_, &QTimer::timeout, this, &QChannelTcp::onKeepailve);
    keepaliveTimer_.setInterval(15000);
}

QChannelTcp::~QChannelTcp()
{
}

bool QChannelTcp::connect( const std::string  & login
                         , const std::string  & password
                         , const std::wstring & adddress
                         , bool                 forced /*= true*/)
{
    if (login.empty() || password.empty())
    {
        error_.set(ERR_INVALID_ARGUMENT, 0, L"Invalid login parameters");
        return false;
    }

    if ( ! setAuth(login, password, forced))
    {
        return false;
    }

    int clientOption, serverOption;
    barray clientKey, serverKey;

    if (!readDbServer(L"dbserver.conf",
                      clientOption, clientKey,
                      serverOption, serverKey))
    {
        error_.set(ERR_CONFIG, 0, L"Failed to read dbserver.conf");
        return false;
    }

    if (clientOption >= 0 && serverOption >= 0
        && !setPreEncoding(clientOption, clientKey, serverOption, serverKey))
    {
        return false;
    }

    return NetChannelTcp::connect(adddress);
}

//

barray QChannelTcp::processDataIn(const barray & data)
{
    if (!authenticated() && !data.empty())
    {
        authDataIn(QByteArray((char*)&data[0], data.size()));
    }
    return Connector::processDataIn(data);
}

barray QChannelTcp::processDataOut(const barray & data)
{
    return Connector::processDataOut(data);
}

void QChannelTcp::sendReply(const barray & data)
{
    if (!data.empty())
    {
        socket_->write((char*)&data[0], data.size());
    }
}

void QChannelTcp::fail(const std::wstring & msg)
{
    setError(msg);
}

//

void QChannelTcp::onConnected()
{
    keepaliveTimer_.start();
    NetChannelTcp::onConnected();
}

void QChannelTcp::onDisconnected()
{
    keepaliveTimer_.stop();
    NetChannelTcp::onDisconnected();
}

void QChannelTcp::onKeepailve()
{
    if (!authenticated())
    {
        return;
    }
    if (socket_->state() != QAbstractSocket::ConnectedState)
    {
        keepaliveTimer_.stop();
        return;
    }

    processTimer();
}

//

QChannelTcpServerEp::QChannelTcpServerEp(QTcpSocket *socket)
    : NetChannelTcp(socket)
{
//     setSlice(1000);
}

QChannelTcpServerEp::~QChannelTcpServerEp()
{
}

//

void QChannelTcpServerEp::serverAuthData(const QByteArray & data)
{
    if (data.size() < 2)
    {
        Log("Short packet");
        return;
    }

    switch (data[0])
    {
        case 0x01:
        {
            // пересылаем S01 клиенту как есть 
            barray b(data.constBegin(), data.constEnd());
            logBytes("<==", b);
            sendReply(b);
            break;
        }

        case 0x02:
        case 0x05:
            break;

        default:
            fail(L"Unexpected client auth data");
            break;
    }
}

barray QChannelTcpServerEp::processDataIn(const barray & data)
{
    // from client
    return ConnectorServerEp::processDataIn(data);
}

barray QChannelTcpServerEp::processDataOut(const barray & data)
{
    // to client
    return ConnectorServerEp::processDataOut(data);
}

//

void QChannelTcpServerEp::sendReply(const barray & data)
{
    if (!data.empty())
    {
        socket_->write((char*) &data[0], data.size());
    }
}

void QChannelTcpServerEp::fail(const std::wstring & msg)
{
    setError(msg);
}
