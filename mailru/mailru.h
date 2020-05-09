#ifndef MAILRU_H
#define MAILRU_H


namespace PwMailRu
{
    extern const QString accountUrl;
    extern const QString userAgent;


    class AccountSession : public QObject
    {
        Q_OBJECT
    public:
        AccountSession(QNetworkAccessManager * qnam);
        virtual ~AccountSession();

        void initialize();

        QList<QNetworkCookie> cookies() const
        {
            return cookies_;
        }

    signals:
        void initialized(bool success);

    private slots:
        void requestFinished();

    private:
        QString evaluateScript(const QString & js);

        QNetworkReply           *reply_;
        QList<QNetworkCookie>   cookies_;

        bool isOk_;
    };
};

#endif
