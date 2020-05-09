#ifndef MAILRUAUTH_H
#define MAILRUAUTH_H


class QDomDocument;

namespace PwMailRu
{

    struct GameAccount
    {
        int     id;
        QString name;
        #pragma warning(suppress : 26495)
    };

    enum StatusCode
    {
        None                = 0,
        IncorrectPassword   = 3,        // invalid username or password (mail auth)
        NotRegistered       = 402,      // user has no such project
        IncorrectPassword2  = 411,      // wrong password (non-mail)
        UserNotFound        = 418,      // user not found
    };

    class MailruAuth : public QObject
    {
        Q_OBJECT
    public:
        MailruAuth(QObject *parent = 0);
        virtual ~MailruAuth();

        void setProxy(const QNetworkProxy & proxy);

        QList<GameAccount> accounts() const;
        QString token() const;
        QString mracToken() const;
        QString accId() const;
        int statusCode() const;

    public Q_SLOTS:
        virtual void start(const QString & email, const QString & password) = 0;

    Q_SIGNALS:
        void progress(int value, int maxValue);
        void finished(bool isOk);               // emits always when completes
        void error(const QString & message);    // indicates error message only

    protected:
        int                 progress_;
        int                 progressMax_;
        int                 statusCode_;
        QList<GameAccount>  accounts_;
        QString             token_;
        QString             mracToken_;
        QString             accId_;

        QNetworkAccessManager   *qnam_;
        QList< QPair<QString, QString> > authParams_;

        void init();
        QNetworkRequest createRequest(const QUrl & url) const;

        void sendLogin(const QList< QPair<QString, QString> > & params);
        void sendPerslist(const QList< QPair<QString, QString> > & params);

        bool handleHttpError(QNetworkReply *);
        bool handleXmlError(const QDomDocument & xml);

    private Q_SLOTS:
        void login_finished();
        void persList_finished();

    private:
        QString escapeXml(const QString & string);

        QString userId1_;
        QString userId2_;

        QPointer<QNetworkReply> loginReply_;
        QPointer<QNetworkReply> perslistReply_;
    };


    class MailruAuthMail : public MailruAuth
    {
        Q_OBJECT
    public:
        MailruAuthMail(QObject *parent = 0);
        virtual ~MailruAuthMail();

    public Q_SLOTS:
        virtual void start(const QString & email, const QString & password);

    private Q_SLOTS:
        void token_finished();
        void mrpage2_finished();
        void mrpage2Auth_finished();

    private:
        void sendToken(const QString & email, const QString & password);
        void sendMrpage2(const QString & accessToken);
        void sendMrpage2Auth(const QString & authLink);

        QPointer<QNetworkReply> tokenReply_;
        QPointer<QNetworkReply> mrpage2Reply_;
        QPointer<QNetworkReply> mrpage2AuthReply_;
    };


    class MailruAuthOther : public MailruAuth
    {
        Q_OBJECT
    public:
        MailruAuthOther(QObject *parent = 0);
        virtual ~MailruAuthOther();

    public Q_SLOTS:
        virtual void start(const QString & email, const QString & password);
    };


    MailruAuth * getAuth(const QString & email, QObject * parent = 0);


}

#endif
