#include "stdafx.h"

#include <QScriptEngine>

#include "mailru.h"

namespace PwMailRu
{

const QString accountUrl("http://pw.mail.ru/account.php");
const QString userAgent("Mozilla/5.0 (Windows NT 6.3; WOW64; rv:31.0) Gecko/20100101 Firefox/31.0");


AccountSession::AccountSession(QNetworkAccessManager * qnam)
    : QObject(qnam)
    , isOk_(false)
{
}

AccountSession::~AccountSession()
{
}

void AccountSession::initialize()
{
    QNetworkRequest request(accountUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);
    request.setRawHeader("Accept", "text/html");
    reply_ = qnam->get(request);
    connect(reply_, &QNetworkReply::finished,
            this,   &AccountSession::requestFinished);
}

void AccountSession::requestFinished()
{
    isOk_ = false;

    if (reply_->error() == QNetworkReply::NoError)
    {
        QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
        QString html = codec->toUnicode(reply_->readAll());
        QDomDocument doc;
        doc.setContent(html);
        if ( ! doc.isNull())
        {
            QDomNodeList nodes = doc.elementsByTagName("script");
            if (nodes.count() >= 1)
            {
                QString script = nodes.at(0).toElement().text();
                QString cookie = evaluateScript(script);
                // n_js=3208091339; max-age=3600; path=/
                cookies_ = QNetworkCookie::parseCookies(cookie.toLatin1());

                isOk_ = ! cookie.isNull();
            }
            else
            {
                qWarning() << "AccountSession failed to get script section:" << html;
            }
        }
        else
        {
            qWarning() << "AccountSession failed to parse html:" << html;
        }
    }
    else
    {
        qWarning() << "AccountSession failed:" << reply_->errorString();
    }

    reply_->deleteLater();
    reply_ = 0;

    initialized(isOk_);
}

static QScriptValue reloadFunction(QScriptContext * /*context*/, QScriptEngine * /*engine*/)
{
    return QScriptValue();
}

QString AccountSession::evaluateScript(const QString & js)
{
    QScriptEngine engine;

    QScriptValue document = engine.newObject();
    engine.globalObject().setProperty("document", document);

    QScriptValue reload = engine.newFunction(reloadFunction);
    QScriptValue location = engine.newObject();
    location.setProperty("reload", reload);
    QScriptValue window = engine.newObject();
    window.setProperty("location", location);
    engine.globalObject().setProperty("window", window);

    QScriptValue result = engine.evaluate(js);

    if (engine.hasUncaughtException())
    {
        QStringList backtrace = engine.uncaughtExceptionBacktrace();
        qDebug() << result.toString()
                << backtrace.join("\n");
        return QString();
    }

    return document.property("cookie").toString();
}

};