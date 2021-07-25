#ifndef vmp_h_
#define vmp_h_

#include <QDate>
#include <QString>

#include "wtypes.h"
#include "VMProtectSDK.h"

namespace Vmp
{
    struct SerialNumberData
    {
        VMProtectSerialStateFlags   state;
        QString     name;
        QDate       expiry;
    };

    enum Option
    {
        OptionHistory,
        OptionAutoprice,
        OptionNoUpdate,

        OptionEnd
    };

    VMProtectSerialStateFlags setSerial(const QByteArray & serial);
    QByteArray getSerial();

    QString getHwId();
    bool getSerialData(SerialNumberData & data);
    bool hasOption(Option opt);

    QString statusText(VMProtectSerialStateFlags status);   // return QString if valid, or error text if not
};

#endif