
#include "stdafx.h"

#include "ui_about.h"
#include "uiabout.h"

#include "license.h"


AboutView::AboutView(QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::About)
{
    ui->setupUi(this);
    setModal(true);

    if (Vmp::hasOption(Vmp::OptionNoUpdate))
    {
        ui->btnRenew->setDisabled(true);
    }

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(6);
    effect->setColor(QColor("#808080"));
    effect->setOffset(2,2);
    ui->lbAbout->setGraphicsEffect(effect);

    updateText();
}

AboutView::~AboutView()
{
}

void AboutView::on_btnRenew_clicked()
{
    QString licFileName = licenseFilename();
    if (licFileName.isEmpty())
    {
        QMessageBox::critical( this
                             , QApplication::applicationName()
                             , QApplication::tr("Failed to get license file name"));
        return;
    }

    QByteArray oldLicense = LicenseStorage::instance().getSerial();
    QByteArray license = askUserLicense(QApplication::tr("Please enter new license code"));
    if (license.isEmpty())
    {
        return;
    }

    VMProtectSerialStateFlags licStatus = LicenseStorage::instance().setSerial(license);
    if (licStatus == 0)
    {
        QFile licFile(licFileName);
        if ( ! saveLicense(license, licFile))
        {
            QMessageBox::warning( this
                                , QApplication::applicationName()
                                , QApplication::tr("Error creating license file\n%1\n%2").arg(licFile.fileName()).arg(licFile.errorString()));
        }
        else
        {
            updateText();
        }
    }
    else
    {
        QMessageBox::warning( this
                            , QApplication::applicationName()
                            , QApplication::tr("%1\nPrevious license still in use.")
                                .arg(Vmp::statusText(licStatus)));
        LicenseStorage::instance().setSerial(oldLicense);
    }
}

void AboutView::updateText()
{
    QString formatted;

    Vmp::SerialNumberData serial;
    if (LicenseStorage::instance().getSerialData(serial))
    {
        formatted = serial.name + "\n";
        if (serial.state == 0)
        {
            if ( ! serial.expiry.isNull()
                && serial.expiry.isValid())
            {
                formatted += tr("Expires at: %1").arg(serial.expiry.toString(Qt::DefaultLocaleShortDate));
            }
            else
            {
                formatted += tr("No expiration");
            }
        }
        else 
        {
            formatted += Vmp::statusText(serial.state);
        }
    }
    if (formatted.isEmpty())
    {
        formatted = "[---]";
    }

    ui->lbRegInfo->setText(formatted);
    ui->lbRegId->setText(Vmp::getHwId());
    ui->lbVersion->setText(tr("version %1").arg(QApplication::applicationVersion()));
}
