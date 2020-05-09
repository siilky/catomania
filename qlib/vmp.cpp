#include "stdafx.h"

#include "vmp.h"

QByteArray s_Serial;

VMProtectSerialStateFlags Vmp::setSerial(const QByteArray & serial)
{
    if (serial.isEmpty())
    {
        return SERIAL_STATE_FLAG_INVALID;
    }
    s_Serial = serial;
    return (VMProtectSerialStateFlags)VMProtectSetSerialNumber(serial.data());
}

QByteArray Vmp::getSerial()
{
    return s_Serial;
}

QString Vmp::getHwId()
{
    VMProtectBeginVirtualization(__FUNCTION__);

    int size = VMProtectGetCurrentHWID(NULL, 0);
    QScopedArrayPointer<char> buf(new char[size]);
    VMProtectGetCurrentHWID(buf.data(), size);

    VMProtectEnd();

    return QString::fromLocal8Bit(buf.data(), size);
}

bool Vmp::getSerialData(SerialNumberData & data)
{
    VMProtectSerialNumberData pData;

    if (VMProtectGetSerialNumberData(&pData, sizeof(pData)))
    {
        data.state = (VMProtectSerialStateFlags)pData.nState;
        data.name = QString::fromWCharArray(pData.wUserName);
        data.expiry = QDate(pData.dtExpire.wYear, pData.dtExpire.bMonth, pData.dtExpire.bDay);

        return true;
    }
    return false;
}

bool Vmp::hasOption(Option opt)
{
    VMProtectSerialNumberData pData;

    if (VMProtectGetSerialNumberData(&pData, sizeof(pData)))
    {
        unsigned char *o = nullptr;
        switch (opt)
        {
            case OptionHistory: o = (unsigned char *)VMProtectDecryptStringA("history");
                break;
            case OptionAutoprice: o = (unsigned char *) VMProtectDecryptStringA("autoprice");
                break;
        }
        if (o == nullptr)
        {
            return false;
        }

        unsigned char *pStart = pData.bUserData;
        unsigned char *p = pStart;
        while (p < pData.bUserData + pData.nUserDataLength)
        {
            if (*p != ';')
            {
                ++p;
                continue;
            }

            if (pStart < p)
            {
                unsigned char *po = o;
                while (*po != 0
                       && pStart < p
                       && *pStart == *po)
                {
                    ++pStart;
                    ++po;
                }
                if (*po == 0 && pStart == p)
                {
                    return true;
                }
            }

            ++p;
            pStart = p;
        }
    }
    return false;
}

QString Vmp::statusText(VMProtectSerialStateFlags status)
{
    if (status == 0)
    {
        return QString();
    }
    switch (status)
    {
        case SERIAL_STATE_FLAG_CORRUPTED:
            return QCoreApplication::tr("Your license data may be corrupted.");
        case SERIAL_STATE_FLAG_INVALID:
            return QCoreApplication::tr("Your license is invalid or not compatible with application.");
        case SERIAL_STATE_FLAG_BLACKLISTED:
            return QCoreApplication::tr("Your license is disabled.");
        case SERIAL_STATE_FLAG_DATE_EXPIRED:
            return QCoreApplication::tr("Your license has expired, please renew your license.");
        case SERIAL_STATE_FLAG_BAD_HWID:
            return QCoreApplication::tr("Your license is not compatible with current hardware.");
        default:
            return QCoreApplication::tr("Your license is not currently applicable.");
    }
}
