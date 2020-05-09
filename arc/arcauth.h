#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include "connection.h"
#include "error.h"
#include "fragments.h"

class NetChannelTcp;

struct rsa_st;
typedef struct rsa_st RSA;

namespace ARC
{

class ArcAuth : public QObject
{
    Q_OBJECT
public:
    enum StatusCode
    {
        None = 0,
        IncorrectPassword = 3,
        AccountBlocked = 10,
        PinRequired = 15,
        IncorrectPin = 20,
    };

    ArcAuth(QObject *parent = 0);
    virtual ~ArcAuth();


    void setProxy(const QNetworkProxy & proxy);

    void setPin(QString pin);

    QString token() const
    {
        return token_;
    }

    ErrorState getError() const
    {
        return error_;
    }
    unsigned statusCode() const
    {
        return statusCode_;
    }

    QString pcName() const
    {
        return pcName_;
    }

public Q_SLOTS:
    virtual void start(const QString & email, const QString & password);

Q_SIGNALS:
    void progress(int value, int maxValue);
    void finished(bool success);
    void needPin();
    void error(const QString & message);    // indicates error message only

private Q_SLOTS:
    void onConnected();
    void onDisconnected();

private:
    void onRsaPublicKey(const FragmentRsaPublicKey *f);
    void onHandshakeOk(const FragmentHandshakeOk *f);
    void onLoginRe(const FragmentLoginRe *f);
    void onGetTokenRe(const FragmentGetTokenRe *f);

    void sendGetToken();
    void abort();
    void finish();

    static const int MaxProgress = 3;

    QString email_;
    QString password_;
    QString hwId_;
    QString pcName_;

    int     progress_;
    int     progressMax_;

    ErrorState  error_;
    unsigned    statusCode_;

    unsigned tokenReq_;
    QString token_;

    QNetworkProxy   proxy_;

    QTimer  keepaliveTimer_;

    ARC::Connection  connection_;
    std::vector<Connection::Cookie> cookies_;

    RSA *rsa_ = nullptr;
    std::array<unsigned, 4> teaKey_;
};

}
