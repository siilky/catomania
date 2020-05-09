#ifndef PWCATSAPI_H
#define PWCATSAPI_H

#include <QObject>
#include <QNetworkReply>
#include <QUrlQuery>


class PWCatsRequest : public QObject
{
    Q_OBJECT
public:
    PWCatsRequest()
        : reply_(0), isParsed_(false)
    { }
    virtual ~PWCatsRequest()
    {
        reset(0);
    }

    bool isOk() const
    {
        return isParsed_;
    }

    bool isFinished() const
    {
        return reply_ ? reply_->isFinished() : true;
    }

    QNetworkReply::NetworkError networkError() const
    {
        return reply_ ? reply_->error() : QNetworkReply::NoError;
    }

    QString errorString() const
    {
        return !error_.isNull() ? error_
            : (reply_ ? reply_->errorString() : QString());
    }

signals:
    void finished();

private slots:
    virtual void onRequestFinished() = 0;

protected:
    friend class PWCatsApi;
    void reset(QNetworkReply *r);

    QUrlQuery       query;

    QNetworkReply   *reply_;
    bool            isParsed_;
    QString         error_;
};


class PWCatsPriceHistoryRequest : public PWCatsRequest
{
    Q_OBJECT
public:
    struct PriceElement
    {
        #if defined(_DEBUG)
        PriceElement(unsigned p, unsigned c)
            : price(p), count(c), ts(0)
        { }
        #endif
        PriceElement()
            : price(), count(), ts()
        {}

        PriceElement(const QJsonObject & o);

        unsigned    price;
        unsigned    count;
        QString     name;
        unsigned    ts;     // unix, 0 if not set
    };

    struct PriceHistoryElement
    {
        PriceHistoryElement(const QJsonObject & o);

        unsigned    sell;
        unsigned    buy;
        unsigned    time;
    };

    //

    PWCatsPriceHistoryRequest(int serverId, unsigned itemId);

    int serverId() const
    {
        return serverId_;
    }
    unsigned itemId() const
    {
        return itemId_;
    }

    //

    std::vector<PriceElement>           sellList, buyList, comSellList, comBuyList;
    std::vector<PriceHistoryElement>    history;

private slots:
    virtual void onRequestFinished();

private:
    int         serverId_;
    unsigned    itemId_;
    static QByteArray getHashString(int serverId, unsigned itemId);
};


class PWCatsApi : public QObject
{
    Q_OBJECT
public:
    PWCatsApi(QObject *parent);
    ~PWCatsApi();

    void get(PWCatsRequest *request);

private:
    QNetworkAccessManager   *qnam_;
};

#endif
