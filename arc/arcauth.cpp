#include "stdafx.h"

#include "arcauth.h"
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "qlib/vmp.h"

#define HANDLE_S(cookies, connection, class, name)  cookies.push_back(connection.bindServerHandler \
                                                    (std::function<void (const Fragment##name *)> \
                                                    (std::bind(&##class::on##name, this, std::placeholders::_1) )))

namespace ARC
{

struct Init
{
    Init()
    {
        ERR_load_crypto_strings();
    }
    ~Init()
    {
        ERR_free_strings();
    }
};

static const std::wstring ArcAddress = L"login.core.perfectworld.com:9881";
static const std::string ArcVersion = "arcversion:V1.1.0.17595 os:Microsoft Windows 10 Pro\r\n";

ArcAuth::ArcAuth(QString hwId, QObject *parent /*= 0*/)
    : QObject(parent)
    , hwId_(QCryptographicHash::hash(hwId.toLocal8Bit(), QCryptographicHash::Md5).toHex())
{
    static Init init;

    pcName_ = QSysInfo::machineHostName();
    if (pcName_.isEmpty())
    {
        pcName_ = "Cat-pc";
    }

    QObject::connect(&connection_, &Connection::connected, this, &ArcAuth::onConnected);
    QObject::connect(&connection_, &Connection::disconnected, this, &ArcAuth::onDisconnected);

    HANDLE_S(cookies_, connection_, ArcAuth, RsaPublicKey);
    HANDLE_S(cookies_, connection_, ArcAuth, HandshakeOk);
    HANDLE_S(cookies_, connection_, ArcAuth, LoginRe);
    HANDLE_S(cookies_, connection_, ArcAuth, GetTokenRe);
}

ArcAuth::~ArcAuth()
{
    if (rsa_)
    {
        RSA_free(rsa_);
    }
}

void ArcAuth::setProxy(const QNetworkProxy & proxy)
{
    proxy_ = proxy;
}

void ArcAuth::setPin(QString pin)
{
    FragmentSendPin f;
    f.pin = pin.toStdString();
    f.hwid = hwId_.toStdString();
    f.pcName = pcName_.toStdString();

    connection_.send(&f);
}

void ArcAuth::start(const QString & email, const QString & password)
{
    qDebug() << "ArcAuth: starting login" << email;

    error_.clear();
    statusCode_ = 0;
    tokenReq_ = 0;
    progress_ = 0;

    email_ = email;
    password_ = password;

    if (!connection_.open(ArcAddress, proxy_))
    {
        error_ = connection_.getError();
        abort();
    }
}

void ArcAuth::onConnected()
{
    qDebug() << "ArcAuth: connected";
    progress(0, MaxProgress);
}

void ArcAuth::onDisconnected()
{
    error_ = connection_.getError();
    qDebug() << "ArcAuth: disconnected:" << error_.code << QString::fromStdWString(error_.message);
    connection_.close();
    finished(false);
}

//

void ArcAuth::onRsaPublicKey(const FragmentRsaPublicKey *f)
{
    //BIO_new(BIO_s_mem());
    auto pem = BIO_new_mem_buf((void*)f->key.c_str(), -1);
    if (pem)
    {
        rsa_ = PEM_read_bio_RSAPublicKey(pem, nullptr, nullptr, nullptr);
        if (rsa_)
        {
            for (auto & k : teaKey_)
            {
                k = rand();
            }

            byte buffer[1024];
            auto sz = RSA_public_encrypt(16, (byte*)teaKey_.data(), buffer, rsa_, 4);
            if (sz > 0)
            {
                assert(sz == 0x40);
                //buffer[0] = 0xFF; - triggers timeout
                FragmentRsaPublicKeyRe fr;
                std::copy(buffer, buffer + sz, std::back_inserter(fr.key));

                connection_.send(&fr);

                BIO_free(pem);
                return;
            }
        }
    }
    // ERROR
    auto eCode = ERR_get_error();
    error_.set(eCode, 0, strToWStr(ERR_error_string(eCode, NULL)));
    BIO_free(pem);
    abort();
}

void ArcAuth::onHandshakeOk(const FragmentHandshakeOk * /*f*/)
{
    progress(1, MaxProgress);

    connection_.setTeaKey(teaKey_);

    FragmentLogin fr;
    fr.login = email_.toStdString();
    fr.password = password_.toStdString();
    fr.lang = "en";
    fr.useragent = ArcVersion;
    fr.hwid = hwId_.toStdString();
    fr.pcname = pcName_.toStdString();
    fr.option = 0;

    connection_.send(&fr);
}

void ArcAuth::onLoginRe(const FragmentLoginRe *f)
{
    qDebug() << "LoginRe" << f->status << f->code;
    if (f->status != 0)
    {
        qDebug() << f->token.c_str() << f->uid1.c_str() << f->uid2.c_str() << f->password.c_str() << f->email.c_str() << f->extra.c_str() << f->uid3.c_str();
        qDebug() << f->v1<< f->v2l<< f->v2h<< f->v3;

        progress(2, MaxProgress);
        sendGetToken();
    }
    else
    {
        if (f->code == PinRequired)
        {
            needPin();
        }
        else
        {
            statusCode_ = f->code;
            abort();
        }
    }
}

void ArcAuth::onGetTokenRe(const FragmentGetTokenRe *f)
{
    qDebug() << "TokenRe" << f->token.c_str();

    token_ = QString::fromStdString(f->token);
    finish();
}

void ArcAuth::abort()
{
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(bool, false));
}

void ArcAuth::finish()
{
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(bool, true));
}

void ArcAuth::sendGetToken()
{
    FragmentGetToken f;
    f.game = "pwi";
    f.v1 = 1;
    f.v2h = 0;
    f.v2l = tokenReq_++;

    connection_.send(&f);
}

}
