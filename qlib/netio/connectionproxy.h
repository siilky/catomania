#ifndef connectionproxy_h
#define connectionproxy_h

#include "netdata/connection.h"


namespace qlib
{
    class ConnectionProxy : public QObject
                          , public Connection
    {
        Q_OBJECT
    public:
        ConnectionProxy(QObject *parent = 0);
        virtual ~ConnectionProxy();

        bool open( const std::string   &login
                 , const std::string   &password
                 , bool                 forced
                 , const std::wstring  &adddress
                 , const QNetworkProxy &proxy
                 , quint16              localPort
                 , const QHostAddress  &localId);
        void close();

        virtual const ErrorState & getError();

        quint16 localPort() const
        {
            return localServer_.serverPort();
        }

    private Q_SLOTS:
        void onAcceptError(QAbstractSocket::SocketError socketError);
        void onNewConnection();

    private:
        // server
        std::string     login_;
        std::string     password_;
        std::wstring    adddress_;
        bool            forced_ = false;
        QNetworkProxy   proxy_;

        QTcpServer      localServer_;
    };

}


#endif