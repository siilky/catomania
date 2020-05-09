#ifndef qchanneltcp_h_
#define qchanneltcp_h_

#include <QTcpSocket>
#include <QNetworkProxy>
#include <QTimer>
#include <QByteArray>

#include "common.h"
#include "netio/netchannel.h"
#include "netio/encoder.h"
#include "netio/connector.h"


// these classes are not thread-safe


class NetChannelTcp : public QObject
                    , public NetChannel
{
    Q_OBJECT
public:
    NetChannelTcp(QObject *parent = 0);
    explicit NetChannelTcp(QTcpSocket *socket);
    virtual ~NetChannelTcp();

    void setProxy(const QNetworkProxy & proxy);
    void setSlice(unsigned sliceSize);

    bool connect(const std::wstring & adddress);
    virtual bool isConnected() const;

    virtual bool peek(barray & data);
    virtual bool read(barray & data);
    virtual bool read(size_t bytes);

    virtual bool write(const barray & data);

    virtual const ErrorState & getError() const
    {
        return error_;
    }

public Q_SLOTS: 
    void write(const QByteArray & data);

Q_SIGNALS:
    void connected();
    void disconnected();
    void dataIn(const QByteArray & data);     // incoming decoded data

protected:
    virtual barray processDataIn(const barray & data);
    virtual barray processDataOut(const barray & data);

    void setError(const std::wstring & msg);

    ErrorState  error_;
    QTcpSocket *socket_;


protected Q_SLOTS:
    virtual void onConnected();
    virtual void onDisconnected();
    virtual void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onConnectTimeout();

    void readFromSocket();

private:
    void setup();

    enum
    {
        ConnectTimeout = 2 * 60 * 1000,
    };

    unsigned        sliceSize_;
    QTimer          connectTimer_;
    QNetworkProxy   proxy_;

    QByteArray      readQ_;
};

//

class QChannelTcp : public NetChannelTcp
                  , private Connector
{
    Q_OBJECT
public:
    QChannelTcp(QObject *parent = 0);
    virtual ~QChannelTcp();

    bool connect( const std::string & login
                , const std::string & password
                , const std::wstring & adddress
                , bool forced = true);

Q_SIGNALS:
    void authDataIn(const QByteArray & data);       // data from remote point before authentication

protected:
    virtual barray processDataIn(const barray & data);
    virtual barray processDataOut(const barray & data);

    virtual void sendReply(const barray & data);
    virtual void fail(const std::wstring & msg);

private Q_SLOTS:
    virtual void onConnected();
    virtual void onDisconnected();

    void onKeepailve();

private:
    QTimer  keepaliveTimer_;
};


class QChannelTcpServerEp : public NetChannelTcp
                          , private ConnectorServerEp
{
    Q_OBJECT
public:
    QChannelTcpServerEp(QTcpSocket *socket);
    virtual ~QChannelTcpServerEp();

public Q_SLOTS:
    void serverAuthData(const QByteArray & data);

protected:
    virtual barray processDataIn(const barray & data);
    virtual barray processDataOut(const barray & data);

    virtual void sendReply(const barray & data);
    virtual void fail(const std::wstring & msg);

private:
};


#endif
