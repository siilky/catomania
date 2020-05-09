
#include "stdafx.h"

#include "auth_errors.h"
#include "mailru\mailruauth.h"
#include "arc\arcauth.h"


#if defined MAILRU_TOKEN_AUTH
QString mailruAuthErrorToString(int errorCode)
{
    switch (errorCode)
    {
        case PwMailRu::IncorrectPassword:
        case PwMailRu::IncorrectPassword2:
            return QObject::tr("Incorrect account email or password");
            break;
        case PwMailRu::NotRegistered:
            return QObject::tr("No account created. Please create game account on http://pw.mail.ru/register");
        case PwMailRu::UserNotFound:
            return QObject::tr("This email is not registered. Please create game account on http://pw.mail.ru/register");
        default:
            return QString("Error code ") + QString::number(errorCode);
    }
}
#elif defined ARC_TOKEN_AUTH
QString arcAuthErrorToString(int errorCode)
{
    switch (errorCode)
    {
        case ARC::ArcAuth::AccountBlocked:
            return QObject::tr("This account was blocked");
        case ARC::ArcAuth::IncorrectPassword:
            return QObject::tr("Incorrect account email or password");
        case ARC::ArcAuth::IncorrectPin:
            return QObject::tr("Incorrect pin");
        default:
            return QString("Error code ") + QString::number(errorCode);
    }
}
#endif
