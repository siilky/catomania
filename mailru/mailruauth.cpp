#include "stdafx.h"

#include "mailruauth.h"

namespace PwMailRu
{

    /* Если мыло отбивает при заведомо правильных логине и пароле с ошибкой "неверный пароль"
       то нужно проверить испльзование перекомпиленных Qt dll чтобы они не мусорили в хедеры 
       лишними параметрами ("accept-encoding").
       (track https://bugreports.qt.io/browse/QTBUG-9463)
    */

static const QString gc_userAgent("Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.181 Downloader/15460 MailRuGameCenter/1546 Safari/537.36");
static const QString gc_firstLink("_1lp=0&amp;_1ld=2046937_0");
static const QUrl g_tokenUrl("https://o2.mail.ru/token");
static const QUrl g_authLoginUrl("https://authdl.mail.ru/sz.php?hint=AutoLogin");
static const QUrl g_authPersListUrl("https://authdl.mail.ru/sz.php?hint=PersList");
static const QUrl g_getPage2Url("https://authdl.mail.ru/ec.php?hint=MrPage2");


MailruAuth::MailruAuth(QObject *parent)
    : QObject(parent)
    , progress_(0)
    , progressMax_(0)
    , statusCode_(0)
    , qnam_(new QNetworkAccessManager(this))
{
    // 7209859237883549019
    while (userId1_.size() < 19)
    {
        userId1_.append(QString::number(qrand() & 0x0FF).right(1));
    }
    // 15309381820031324198
    while (userId2_.size() < 20)
    {
        userId2_.append(QString::number(qrand() & 0x0FF).right(1));
    }
}

MailruAuth::~MailruAuth()
{
}

void MailruAuth::setProxy(const QNetworkProxy & proxy)
{
    qnam_->setProxy(proxy);
}

QList<GameAccount> MailruAuth::accounts() const
{
    return accounts_;
}

QString MailruAuth::token() const
{
    return token_;
}

QString MailruAuth::mracToken() const
{
    return mracToken_;
}

QString MailruAuth::accId() const
{
    return accId_;
}

int MailruAuth::statusCode() const
{
    return statusCode_;
}

//

void MailruAuth::login_finished()
{
    progress(++progress_, progressMax_);

    QNetworkReply *reply = loginReply_;
    reply->deleteLater();
    
    // bad args replies may have http error code with xml payload

    // Code="402" Reason="user has no such project"

    QDomDocument xml;
    QString errorMsg;
    int errorLine = 0;
    QString text(QString::fromUtf8(reply->readAll()));
    xml.setContent(text, &errorMsg, &errorLine);
    if (!xml.isNull())
    {
        QDomElement login = xml.firstChildElement("AutoLogin");
        if (!login.isNull())
        {
            token_ = login.attribute("Key");
            mracToken_ = login.attribute("MRACToken");
            accId_ = login.attribute("PersId");

            sendPerslist(authParams_);
            return;
        }

        // report XML error if any
        if (handleXmlError(xml))
        {
            return;
        }
    }

    if (!handleHttpError(reply)) // http error and cannot read xml 
    {
        // http ok but no xml
        error(tr("Failed to read auth data"));
        finished(false);
    }
}

void MailruAuth::persList_finished()
{
    progress(++progress_, progressMax_);

    QNetworkReply *reply = perslistReply_;
    reply->deleteLater();

    QDomDocument xml;
    QString errorMsg;
    int errorLine = 0;
    QString text(QString::fromUtf8(reply->readAll()));
    xml.setContent(text, &errorMsg, &errorLine);
    if (!xml.isNull())
    {
        QDomElement persList = xml.firstChildElement("PersList");
        if (!persList.isNull())
        {
            QDomElement pers = persList.firstChildElement("Pers");
            while (!pers.isNull())
            {
                GameAccount account;
                account.id = pers.attribute("Id").toInt();
                account.name = pers.attribute("Title");

                accounts_ << account;

                pers = pers.nextSiblingElement("Pers");
            }

            finished(true);
            return;
        }

        // report XML error if any
        if (handleXmlError(xml))
        {
            return;
        }
    }

    if (!handleHttpError(reply)) // http error and cannot read xml 
    {
        // http ok but no xml
        error(tr("Failed to read auth data"));
        finished(false);
    }
}

//

void MailruAuth::init()
{
    accounts_.clear();
    authParams_.clear();
    token_.clear();
    progress_ = 0;
    statusCode_ = 0;
    progress(progress_, progressMax_);
}

void MailruAuth::sendLogin(const QList< QPair<QString, QString> > & params)
{
    if (loginReply_)
    {
        delete loginReply_;
    }

    QNetworkRequest request(createRequest(g_authLoginUrl));
    QString xml = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                          "<AutoLogin ProjectId=\"61\" SubProjectId=\"0\" ShardId=\"0\""
                          " UserId=\"%1\" UserId2=\"%2\""
                          " FirstLink=\"%3\""
    ).arg(userId1_).arg(userId2_).arg(gc_firstLink);

    QPair<QString, QString> param;
    foreach(param, params)
    {
        xml.append(QString(" %1=\"%2\"").arg(param.first).arg(escapeXml(param.second)));
    }
    xml.append("/>");

    loginReply_ = qnam_->post(request, xml.toUtf8());
    connect(loginReply_.data(), &QNetworkReply::finished,
            this,               &MailruAuth::login_finished);
}

void MailruAuth::sendPerslist(const QList< QPair<QString, QString> > & params)
{
    if (perslistReply_)
    {
        delete perslistReply_;
    }

    QNetworkRequest request(createRequest(g_authPersListUrl));
    QString xml = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                          "<PersList ProjectId=\"61\" SubProjectId=\"0\" ShardId=\"0\""
                          " UserId=\"%1\" UserId2=\"%2\""
    ).arg(userId1_).arg(userId2_);

    QPair<QString, QString> param;
    foreach(param, params)
    {
        xml.append(QString(" %1=\"%2\"").arg(param.first).arg(escapeXml(param.second)));
    }
    xml.append("/>");

    perslistReply_ = qnam_->post(request, xml.toUtf8());
    connect(perslistReply_.data(),  &QNetworkReply::finished,
            this,                   &MailruAuth::persList_finished);
}

QNetworkRequest MailruAuth::createRequest(const QUrl & url) const
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, gc_userAgent);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "*/*");
    return request;
}

bool MailruAuth::handleHttpError(QNetworkReply * reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        return false;
    }

    statusCode_ = reply->error();

    error(tr("Failed to communicate with server (%1) %2")
            .arg(reply->error())
            .arg(reply->errorString()));
    finished(false);
    return true;
}

bool MailruAuth::handleXmlError(const QDomDocument & xml)
{
    // <?xml version="1.0" encoding="UTF-8"?><SZError Code="411" Reason="wrong password" />
    QDomElement err = xml.firstChildElement("SZError");
    if (!err.isNull())
    {
        QString errorCode = err.attribute("Code");
        bool succes;
        int code = errorCode.toInt(&succes);
        if (succes)
        {
            statusCode_ = code;
        }

        QString errorMsg = err.attribute("Reason");
        if (!errorMsg.isEmpty())
        {
            error(errorMsg);
            finished(false);
            return true;
        }
    }
    return false;
}

QString MailruAuth::escapeXml(const QString & string)
{
    QString result(string);
    // Escaping characters is different for tags and attributes.
    // For tags:
    //  < &lt;
    //  > &gt; (only for compatibility, read below)
    //  & &amp;
    // For attributes:
    // " &quot;
    // ' &apos;
    result.replace('"', "&quot;");
    result.replace('\'', "&apos;");
    return result;
}

//

MailruAuthMail::MailruAuthMail(QObject *parent /*= 0*/)
    : MailruAuth(parent)
{
    progressMax_ = 5;
}

MailruAuthMail::~MailruAuthMail()
{
}

void MailruAuthMail::start(const QString & email, const QString & password)
{
    init();
    sendToken(email, password);
}

void MailruAuthMail::token_finished()
{
    progress(++progress_, progressMax_);

    QNetworkReply *reply = tokenReply_;
    reply->deleteLater();

    // it is 200 on wrong pass etc
    if (handleHttpError(reply))
    {
        return;
    }

    QByteArray text(reply->readAll());
    QJsonDocument json(QJsonDocument::fromJson(text));
    if (!json.isNull() && !json.object().isEmpty())
    {
        QJsonObject root = json.object();
        if (root.contains("error"))
        {
            statusCode_ = root["error_code"].toInt();;

            error(root["error"].toString());
            finished(false);
            return;
        }
        else if (root.contains("access_token"))
        {
            QString accessToken_ = root["access_token"].toString();
            if (!accessToken_.isEmpty())
            {
                sendMrpage2(accessToken_);
                return;
            }
        }
    }

    // failed to do anytging
    error(tr("Failed to read token data"));
    finished(false);
}

void MailruAuthMail::mrpage2_finished()
{
    progress(++progress_, progressMax_);

    QNetworkReply *reply = mrpage2Reply_;
    reply->deleteLater();

    if (handleHttpError(reply))
    {
        return;
    }

    // <?xml version="1.0" encoding="UTF-8"?><MrPage2 Location="https://auth.mail.ru/cgi-bin/auth?Login=rmjau@mail.ru&amp;agent=AG_qpEUGYfbVvaeyAKT7Wtgm&amp;page=http%3A%2F%2Fdl.mail.ru%2Frobots.txt" ErrorCode="0" />
    // <?xml version="1.0" encoding="UTF-8"?><MrPage2 Location="" ErrorCode="401" />
    QDomDocument xml;
    QString errorMsg;
    int errorLine = 0;
    QString text(QString::fromUtf8(reply->readAll()));
    xml.setContent(text, &errorMsg, &errorLine);
    if (!xml.isNull())
    {
        QDomElement page2 = xml.firstChildElement("MrPage2");
        if (!page2.isNull())
        {
            QString location = page2.attribute("Location");
            QString errorCode = page2.attribute("ErrorCode");
            int code = 0;
            if (!errorCode.isEmpty())
            {
                bool isOk;
                code = errorCode.toInt(&isOk);
                if (isOk)
                {
                    statusCode_ = code;
                }
            }

            if (code == 0 && !location.isEmpty())
            {
                // ok
                sendMrpage2Auth(location);
                return;
            }
            else
            {
                // fail
                error(tr("Server error code %1").arg(code));
                finished(false);
                return;
            }
        }
    }

    // general error
    error(tr("Failed to read auth location"));
    finished(false);
}

void MailruAuthMail::mrpage2Auth_finished()
{
    progress(++progress_, progressMax_);

    QNetworkReply *reply = mrpage2AuthReply_;
    reply->deleteLater();

    if (handleHttpError(reply))
    {
        return;
    }

    // we got an 302 if auth failed and 200 if ok -- earlier versions

    // we got an 302 even with success auth -- later versions
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 200
        || statusCode == 302)
    {
        QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie>>(reply->header(QNetworkRequest::SetCookieHeader));
        foreach(const QNetworkCookie & cookie, cookies)
        {
            if (cookie.name() == "Mpop")
            {
                authParams_ << qMakePair(QString("Mpop"), cookie.value());
                sendLogin(authParams_);
                return;
            }
        }
    }
    
    // QByteArray b(reply->readAll());
    // QList<QByteArray> hdrs(reply->rawHeaderList());

    error(tr("Authentication failed"));
    finished(false);
}

void MailruAuthMail::sendToken(const QString & email, const QString & password)
{
    if (tokenReply_)
    {
        delete tokenReply_;
    }

    QNetworkRequest request(createRequest(g_tokenUrl));
    QByteArray xml("client_id=gamecenter.mail.ru&grant_type=password&username=");
    xml += QUrl::toPercentEncoding(email) 
        + "&password="
        + QUrl::toPercentEncoding(password);

    tokenReply_ = qnam_->post(request, xml);
    connect(tokenReply_.data(), &QNetworkReply::finished,
            this,               &MailruAuthMail::token_finished);
}

void MailruAuthMail::sendMrpage2(const QString & accessToken)
{
    if (mrpage2Reply_)
    {
        delete mrpage2Reply_;
    }

    QNetworkRequest request(createRequest(g_getPage2Url));
    QString xml(
        QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><MrPage2 SessionKey=\"%1\" Page=\"https://games.mail.ru/gamecenter/sdc/\"/>")
        .arg(accessToken)
        );
    mrpage2Reply_ = qnam_->post(request, xml.toUtf8());
    connect(mrpage2Reply_.data(),   &QNetworkReply::finished,
            this,                   &MailruAuthMail::mrpage2_finished);
}

void MailruAuthMail::sendMrpage2Auth(const QString & authLink)
{
    if (mrpage2AuthReply_)
    {
        delete mrpage2AuthReply_;
    }

    QNetworkRequest request(createRequest(authLink));
    mrpage2AuthReply_ = qnam_->get(request);
    connect(mrpage2AuthReply_.data(),   &QNetworkReply::finished,
            this,                       &MailruAuthMail::mrpage2Auth_finished);
}

//

MailruAuthOther::MailruAuthOther(QObject *parent /*= 0*/)
    : MailruAuth(parent)
{
    progressMax_ = 2;
}

MailruAuthOther::~MailruAuthOther()
{
}

void MailruAuthOther::start(const QString & email, const QString & password)
{
    init();

    authParams_ << qMakePair(QString("Username"), email);
    authParams_ << qMakePair(QString("Password"), password);

    sendLogin(authParams_);
}

//

MailruAuth * getAuth(const QString & email, QObject * parent)
{
    if (email.isEmpty()
        || email.count('@') != 1
        || email.startsWith('@')
        || email.endsWith('@'))
    {
        //error("Invalid email or password");
        return 0;
    }

    static QStringList mailruDomains = QStringList() << "mail.ru" << "inbox.ru" << "list.ru" << "bk.ru" << "mail.ua";

    QString domain(email.section('@', 1));
    if (mailruDomains.contains(domain))
    {
        return new MailruAuthMail(parent);
    }
    else
    {
        return new MailruAuthOther(parent);
    }
}


}