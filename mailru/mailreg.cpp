#include "stdafx.h"

#include "mailreg.h"
#include "mailru.h"


// const QString MailruRegistrator::accessUrl_("http://pwcats.info/ulogin/reg");
const QString MailruRegistrator::accessUrl_("http://pw.mail.ru/ajaxreg.php");
const QString MailruRegistrator::referer_ = "http://pw.mail.ru/account.php";
const QList<QNetworkCookie> MailruRegistrator::cookies_ = QList<QNetworkCookie>()
                                    << QNetworkCookie("__referrer", "http://pw.mail.ru/")
                                    << QNetworkCookie("mc2", "pw.mail.ru");


MailruRegistrator::MailruRegistrator(QNetworkAccessManager *qnam, QObject *parent)
    : QObject(parent)
    , qnam_(qnam)
{
}

void MailruRegistrator::registration(const QString & email /*= QString()*/,
                                     const QString & name /*= QString()*/, 
                                     const QString & password /*= QString()*/)
{
    if ( ! email.isEmpty())
    {
        email_ = email;
    }
    else
    {

    }

    if ( ! name.isEmpty())
    {
        name_ = name;
    }
    else
    {

    }

    if ( ! password.isEmpty())
    {
        password_ = password;
    }
    else
    {

    }

}

void MailruRegistrator::checkEmail(const QString & email)
{
    if (emailCheckReply_)
    {
        delete emailCheckReply_;
    }

    QNetworkRequest req = createRequest();
    QByteArray data(QString("do=verifyEmail&email=%1")
                    .arg(QString(email.toUtf8().toPercentEncoding())).toLatin1());

    emailCheckReply_ = qnam_->post(req, data);
    connect(emailCheckReply_.data(),    &QNetworkReply::finished,
            this,                       &MailruRegistrator::emailCheckFinished);
}

void MailruRegistrator::checkLogin(const QString & login)
{
    if (loginCheckReply_)
    {
        delete loginCheckReply_;
    }

    QNetworkRequest req = createRequest();
    QByteArray data(QString("do=verifyLogin&login=%1")
                    .arg(QString(login.toUtf8().toPercentEncoding())).toLatin1());

    loginCheckReply_ = qnam_->post(req, data);
    connect(loginCheckReply_.data(),    &QNetworkReply::finished, 
            this,                       &MailruRegistrator::emailCheckFinished);
}

void MailruRegistrator::checkPassword(const QString & login, const QString & password)
{
    if (passwordCheckReply_)
    {
        delete passwordCheckReply_;
    }

    QNetworkRequest req = createRequest();
    QByteArray data(QString("do=verifyPassword&login=%1&password=%1")
                    .arg(QString(login.toUtf8().toPercentEncoding()))
                    .arg(QString(password.toUtf8().toPercentEncoding())).toLatin1());

    passwordCheckReply_ = qnam_->post(req, data);
    connect(passwordCheckReply_.data(), &QNetworkReply::finished,
            this,                       &MailruRegistrator::emailCheckFinished);

}

//

void MailruRegistrator::emailCheckFinished()
{
    QString message;
    bool result;
    bool isValid;

    if (emailCheckReply_->error() == QNetworkReply::NoError)
    {
        result = parseCheckReply(emailCheckReply_->readAll(), isValid, message);
    }
    else
    {
        result = false;
        isValid = false;
        message = emailCheckReply_->errorString();
        qDebug() << "Email check failed:" << message;
    }
    emailCheckReply_->deleteLater();

    emailChecked(result, isValid, message);
}

void MailruRegistrator::loginCheckFinished()
{
    QString message;
    bool result;
    bool isValid;

    if (loginCheckReply_->error() == QNetworkReply::NoError)
    {
        result = parseCheckReply(loginCheckReply_->readAll(), isValid, message);
    }
    else
    {
        result = false;
        isValid = false;
        message = loginCheckReply_->errorString();
        qDebug() << "Login check failed:" << message;
    }
    loginCheckReply_->deleteLater();

    loginChecked(result, isValid, message);
}

void MailruRegistrator::passwordCheckFinished()
{
    QString message;
    bool result;
    bool isValid;

    if (passwordCheckReply_->error() == QNetworkReply::NoError)
    {
        result = parseCheckReply(passwordCheckReply_->readAll(), isValid, message);
    }
    else
    {
        result = false;
        isValid = false;
        message = passwordCheckReply_->errorString();
        qDebug() << "Password check failed:" << message;
    }
    passwordCheckReply_->deleteLater();

    emailChecked(result, isValid, message);
}

//

QNetworkRequest MailruRegistrator::createRequest()
{
    QNetworkRequest request(accessUrl_);
    request.setHeader(QNetworkRequest::UserAgentHeader, PwMailRu::userAgent);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    request.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(cookies_));
    request.setRawHeader("Referer", referer_.toUtf8());
    request.setRawHeader("Accept", "application/json, text/javascript, */*");

    return request;
}

bool MailruRegistrator::parseCheckReply(const QByteArray & data, bool & valid, QString & message)
{
    QJsonDocument json(QJsonDocument::fromJson(data));
    if (!data.isEmpty()
        && !json.isNull())
    {
        QJsonObject root(json.object());

        QJsonValue isValid(root["valid"]);
        valid = !isValid.isNull() && isValid.toString() == "1";
        message = root["error"].toString();

        return true;
    }
    else
    {
        qDebug() << "Json error:" << data;
        valid = false;
        message = "Data error";

        return false;
    }
}
