#include "stdafx.h"

#include "PWCatsApi.h"
#include "version.h"
#include "util.h"


void PWCatsRequest::reset(QNetworkReply *r)
{
    if (reply_)
    {
        delete reply_;
    }
    reply_ = r;
    isParsed_ = false;
    error_.clear();
}

//

PWCatsPriceHistoryRequest::PriceElement::PriceElement(const QJsonObject & o)
    : price(toUInt(o["price"]))
    , count(toUInt(o["count"]))
    , name(o["name"].toString())
{
    if (o.contains("lastupdate"))
    {
        ts = o["lastupdate"].toInt();
        //QDateTime dt = QDateTime::fromTime_t(t);    // was: "yyyy-MM-dd HH:mm:ss" "2016-04-22 09:08:16"
    }
    else
    {
        qWarning() << "pwcats api: Missing lastupdate field";
        ts = 0;
    }
}

PWCatsPriceHistoryRequest::PriceHistoryElement::PriceHistoryElement(const QJsonObject & o)
    : sell(toUInt(o["sell"]))
    , buy(toUInt(o["buy"]))
    , time(toUInt(o["time"]))
{
}

template< template<typename, typename ...> class Array, typename Value, typename ... A>
static bool getArray(const QJsonObject & o, const QString & name, typename Array<Value, A ...> & array)
{
    array.clear();

    QJsonObject::const_iterator it = o.find(name);
    if (it != o.end() && it->isArray())
    {
        const QJsonArray arr = it->toArray();
        for (int i = 0; i < arr.size(); i++)
        {
            array.push_back(Value(arr[i].toObject()));
        }
        return true;
    }

    return false;
}

//

PWCatsPriceHistoryRequest::PWCatsPriceHistoryRequest(int serverId, unsigned itemId)
    : serverId_(serverId)
    , itemId_(itemId)
{
    query.addQueryItem("s", QString::number(serverId));
    query.addQueryItem("i", QString::number(itemId));
    query.addQueryItem("h", getHashString(serverId, itemId));
}

void PWCatsPriceHistoryRequest::onRequestFinished()
{
    if (reply_->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply_->readAll();
        QJsonDocument json = QJsonDocument::fromJson(data);
        if (!json.isNull() && json.isObject())
        {
            QJsonObject o = json.object();
            bool success = o["success"].toBool(false);
            if (success)
            {
                getArray(o, "catsell", sellList);
                getArray(o, "catbuy", buyList);
                getArray(o, "komiss_sell", comSellList);
                getArray(o, "komiss_buy", comBuyList);
                getArray(o, "static", history);

                isParsed_ = true;
            }
            else
            {
                error_ = o["message"].toString();
            }
        }
        else
        {
            error_ = "Invalid format";
        }

        // don't need reply anymore
        reply_->deleteLater();
        reply_ = 0;
    }

    emit finished();
}

QByteArray PWCatsPriceHistoryRequest::getHashString(int serverId, unsigned itemId)
{
    QDate now = QDateTime::currentDateTimeUtc().date();
    QString str = QString("%1%2%3%4%5")
        .arg(serverId)
        .arg(now.month(), 2, 10, QLatin1Char('0'))
        .arg(now.day(), 2, 10, QLatin1Char('0'))
        .arg(itemId)
        .arg(now.year());
    QByteArray data = str.toLatin1().toBase64();
    return QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();
}

PWCatsApi::PWCatsApi(QObject *parent)
    : QObject(parent)
    , qnam_(new QNetworkAccessManager(this))
{
}

PWCatsApi::~PWCatsApi()
{
}

void PWCatsApi::get(PWCatsRequest *request)
{
    QUrl url("https://pwcats.info/api");
    url.setQuery(request->query);
    qDebug() << "Requesting" << url.toString();

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", (QString("CatOmania 2.%0/Win")
                                        .arg(QString::fromWCharArray(g_revision))).toLatin1());
    request->reset(qnam_->get(req));
    connect(request->reply_, &QNetworkReply::finished, request, &PWCatsRequest::onRequestFinished);
}
