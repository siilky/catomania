
#include "stdafx.h"

#include "qlib/vmp.h"
#include "forms/uilicense.h"
#include "license.h"
#include "log.h"


LicenseStorage::LicenseStorage()
    : state_(SERIAL_STATE_FLAG_INVALID)
{
    dailyTimer_.setSingleShot(true);
    connect(&dailyTimer_, &QTimer::timeout, this, &LicenseStorage::onDailyTimer);
    setDailyTimer();
}

LicenseStorage & LicenseStorage::instance()
{
    static LicenseStorage theSingleInstance;
    return theSingleInstance;
}

VMProtectSerialStateFlags LicenseStorage::setSerial(const QByteArray & serial)
{
    serial_ = serial;
    state_ = Vmp::setSerial(serial);
    licenseStateChanged(state_);    // always notify to update expiry while state does not actually changes
    return state_;
}

QByteArray LicenseStorage::getSerial() const
{
    return serial_;
}

bool LicenseStorage::getSerialData(Vmp::SerialNumberData & data)
{
    bool res = Vmp::getSerialData(data);
    if (res && data.state != state_)
    {
        state_ = data.state;
        licenseStateChanged(state_);
    }
    return res;
}

void LicenseStorage::onDailyTimer()
{
    Vmp::SerialNumberData data;
    if (Vmp::getSerialData(data))
    {
        state_ = data.state;
        licenseStateChanged(state_);
    }
    setDailyTimer();
}

void LicenseStorage::setDailyTimer()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime next = now.addDays(1);
    next.setTime(QTime(0, 0, 30));
    int seconds = int(now.secsTo(next));
    Log("Next license state update in %i", seconds);
    dailyTimer_.start(seconds * 1000);
}

//

bool validateLicense()
{
    QString licName = licenseFilename();
    if (licName.isEmpty())
    {
        QMessageBox::critical( 0
                             , QApplication::applicationName()
                             , QApplication::tr("Failed to get license file name"));
        return false;
    }

    QByteArray licData;

    QFile licFile(licName);
    if (licFile.exists())
    {
        if ( ! licFile.open(QIODevice::ReadWrite))
        {
            QMessageBox::critical( 0
                , QApplication::applicationName()
                , QApplication::tr("Error opening license file\n%1\n%2").arg(licName).arg(licFile.errorString()));
            return false;
        }

        licData = licFile.readAll();
    }

    VMProtectSerialStateFlags licStatus = SERIAL_STATE_FLAG_INVALID;
    bool needSave = false;
    for(;;)
    {
        QString status;

        if ( ! licData.isEmpty())
        {
            licStatus = LicenseStorage::instance().setSerial(licData);
            if (licStatus == 0)
            {
                if (needSave && ! saveLicense(licData, licFile))
                {
                    QMessageBox::warning( 0
                                        , QApplication::applicationName()
                                        , QApplication::tr("Error creating license file\n%1\n%2").arg(licFile.fileName()).arg(licFile.errorString()));
                }
                break;
            }
            else
            {
                status = Vmp::statusText(licStatus);
            }
        }
        else
        {
            if ( ! licFile.exists())
            {
                status = QApplication::tr("No license found.");
            }
            else
            {
                status = QApplication::tr("License file is empty.");
            }
        }

        QByteArray newData = askUserLicense(status);
        if (newData.isEmpty())
        {
            break;
        }

        needSave = newData != licData;
        licData = newData;
    };

    return licStatus == 0;
}

QByteArray askUserLicense(const QString & licStatus)
{
    QScopedPointer<LicenseView> licView(new LicenseView(licStatus, Vmp::getHwId()));
    if (licView->exec() != QDialog::Accepted)
    {
        return QByteArray();
    }

    return licView->license().toLatin1();
}

QString licenseFilename()
{
    return QDir(QApplication::applicationDirPath()).filePath("cat.key");
}

bool saveLicense(const QByteArray & licData, QFile & licFile)
{
    if (licData.isEmpty())
    {
        return true;
    }

    if ( ! licFile.isOpen())
    {
        // was missing
        if ( ! licFile.open(QIODevice::ReadWrite))
        {
            return false;
        }
    }
    else
    {
        licFile.resize(0);
    }

    // write if created/opened
    if (licFile.isOpen())
    {
        licFile.write(licData);
    }

    return true;
}

