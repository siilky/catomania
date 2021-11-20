#include "stdafx.h"

#include "types.h"
#include "connectionproxy.h"
#include "qchanneltcp.h"


qlib::ConnectionProxy::ConnectionProxy(bool safeMode, QObject *parent)
    : QObject(parent)
    , Connection(safeMode)
{
    connect(&localServer_, &QTcpServer::acceptError,    this, &ConnectionProxy::onAcceptError);
    connect(&localServer_, &QTcpServer::newConnection,  this, &ConnectionProxy::onNewConnection);
}

qlib::ConnectionProxy::~ConnectionProxy()
{
    close();
}

bool qlib::ConnectionProxy::open( const std::string    &login
                                , const std::string    &password
                                , bool                  forced
                                , const std::wstring   &adddress
                                , const QNetworkProxy  &proxy
                                , quint16               localPort
                                , const QHostAddress   &localIp)
{
    close();

    if (login.empty() || password.empty() || adddress.empty())
    {
        return false;
    }

    login_ = login;
    password_ = password;
    adddress_ = adddress;
    forced_ = forced;
    proxy_ = proxy;

#if PW_SERVER_VERSION >= 1720
    serverDcWorkaroundDone_ = false;
#endif

    localServer_.setMaxPendingConnections(1);
    localServer_.listen(localIp, localPort);

    return true;
}

void qlib::ConnectionProxy::close()
{
    delete client_;
    client_ = 0;
    delete server_;
    server_ = 0;
}

const ErrorState & qlib::ConnectionProxy::getError()
{
    if (server_ != NULL)
    {
        const ErrorState & sErr = server_->getError();
        if (sErr.code != ERR_NO_ERROR)
        {
            error_.set(sErr.code, sErr.codeExt, L"Server side: " + sErr.message);
            return error_;
        }
    }

    if (client_ != NULL)
    {
        const ErrorState & clErr = client_->getError();
        if (clErr.code != ERR_NO_ERROR)
        {
            error_.set(clErr.code, clErr.codeExt, L"Client side: " + clErr.message);
            return error_;
        }
    }

    return error_;

}

//

void qlib::ConnectionProxy::onAcceptError(QAbstractSocket::SocketError socketError)
{
    Log("Proxy accept error");
    error_.set(ERR_NETWORK, 0, (localServer_.errorString() + QString(" (%1)").arg(socketError)).toStdWString());
}

void qlib::ConnectionProxy::onNewConnection()
{
    QTcpSocket *newSocket = localServer_.nextPendingConnection();
    if (newSocket == 0)
    {
        return;
    }

    if (server_ != 0 || client_ != 0)
    {
        Log("Client already connected, dropping new connection");
        newSocket->close();
        delete newSocket;
        return;
    }

    QChannelTcp *server = new QChannelTcp();
    server->setProxy(proxy_);
    server_ = server;

    QChannelTcpServerEp *client = new QChannelTcpServerEp(newSocket);
    client_ = client;

    QObject::connect(server, &QChannelTcp::authDataIn,  client, &QChannelTcpServerEp::serverAuthData);
    QObject::connect(server, &QChannelTcp::dataIn
                    , client, static_cast<void(NetChannelTcp::*)(const QByteArray &)>(&NetChannelTcp::write));
#if PW_SERVER_VERSION >= 1720
    QObject::connect(client, &QChannelTcp::dataIn,      this, &ConnectionProxy::onClientDataIn);
#else
    QObject::connect(client, &QChannelTcp::dataIn
                    , server, static_cast<void(QChannelTcp::*)(const QByteArray &)>(&NetChannelTcp::write));
#endif

    if (!server->connect(login_, password_, adddress_, forced_))
    {
        close();
        return;
    }
}

void qlib::ConnectionProxy::onClientDataIn(const QByteArray& data)
{
#if PW_SERVER_VERSION >= 1720
    // server disconnects when two consecutive initial fragments are sent
    if (!serverDcWorkaroundDone_
        && data.size() == 49 && data[0] == 0x48)
    {
        static_cast<QChannelTcp*>(server_)->write(data.left(26));
        qApp->processEvents();  // yes, both calls are required
        QThread::msleep(200);
        qApp->processEvents();
        static_cast<QChannelTcp*>(server_)->write(data.right(data.size() - 26));

        serverDcWorkaroundDone_ = true;
    }
    else
#endif
    {
        static_cast<QChannelTcp *>(server_)->write(data);
    }
}
