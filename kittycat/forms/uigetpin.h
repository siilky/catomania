#pragma once

#include <QDialog>

namespace Ui
{
    class GetPin;
}

class GetPinView : public QDialog
{
    Q_OBJECT
public:
    GetPinView(QString email, QString pcName, QWidget *parent);
    ~GetPinView();

    QString pin() const;

private slots:
    void on_lePin_textChanged(const QString &text);

private:
    QScopedPointer<Ui::GetPin> ui;
};
