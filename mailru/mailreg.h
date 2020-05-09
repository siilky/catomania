#ifndef mailreg_h
#define mailreg_h

#include <QObject>
#include <QString>

class MailruRegistrator : public QObject
{
    Q_OBJECT
public:
    MailruRegistrator(QNetworkAccessManager *qnam, QObject *parent);

    void registration(const QString & email = QString(),
                      const QString & name = QString(),
                      const QString & password = QString());

    //

    QString email() const
    {
        return email_;
    }

    QString name() const
    {
        return name_;
    }

    QString password() const
    {
        return password_;
    }

public Q_SLOTS:
    void checkEmail(const QString & email);
    void checkLogin(const QString & login);
    void checkPassword(const QString & login, const QString & password);

Q_SIGNALS:
    void finished(bool success);
    void emailChecked(bool success /*system*/, bool result, QString message);
    void loginChecked(bool success /*system*/, bool result, QString message);
    void passwordChecked(bool success /*system*/, bool result, QString message);

private Q_SLOTS:
    void emailCheckFinished();
    void loginCheckFinished();
    void passwordCheckFinished();

private:
    QNetworkRequest createRequest();
    bool parseCheckReply(const QByteArray & data, bool & valid, QString & message);

    QString email_;
    QString name_;
    QString password_;

    static const QString                accessUrl_;
    static const QString                referer_;
    static const QList<QNetworkCookie>  cookies_;

    QNetworkAccessManager   *qnam_;
    QPointer<QNetworkReply> emailCheckReply_;
    QPointer<QNetworkReply> loginCheckReply_;
    QPointer<QNetworkReply> passwordCheckReply_;
};

#endif