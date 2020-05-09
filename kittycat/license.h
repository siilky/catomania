#ifndef license_h_
#define license_h_

#include <QTimer>
#include <QFile>
#include "qlib/vmp.h"

class LicenseStorage
    : public QObject
{
    Q_OBJECT
public:
    static LicenseStorage & instance();

    VMProtectSerialStateFlags setSerial(const QByteArray & serial);

    QByteArray getSerial() const;
    bool getSerialData(Vmp::SerialNumberData & data);

signals:
    void licenseStateChanged(VMProtectSerialStateFlags state);

private slots:
    void onDailyTimer();

private:
    LicenseStorage();
    void setDailyTimer();
    
    QTimer      dailyTimer_;
    QByteArray  serial_;
    VMProtectSerialStateFlags   state_;
};


bool validateLicense();
QByteArray askUserLicense(const QString & licStatus);
QString licenseFilename();
bool saveLicense(const QByteArray &licData, QFile & licFile);


#endif