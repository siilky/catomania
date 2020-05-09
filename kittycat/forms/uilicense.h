#ifndef uilicense_h
#define uilicense_h

#include <QDialog>
#include <QScopedPointer>

namespace Ui
{
    class License;
}

class LicenseView
    : public QDialog
{
    Q_OBJECT

public:
    LicenseView(const QString & status, const QString & id, QWidget *parent = 0);

    QString license() const;

private:
    QScopedPointer<Ui::License> ui;
};

#endif
