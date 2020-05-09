#ifndef uilock_h
#define uilock_h

#include <QDialog>
#include <QScopedPointer>

#include "catctl.h"

namespace Ui
{
    class Lock;
}

class LockView
    : public QDialog
{
    Q_OBJECT

public:
    LockView(CatCtl * ctl, QWidget *parent = 0);

signals:
    void logMessage(const QString & message);

protected:
    virtual void changeEvent(QEvent *event);

private slots:
    void on_btnApply_clicked();

    void onGameEvent(const qlib::GameEvent &);

private:
    void updateUi();

    QScopedPointer<Ui::Lock> ui_;

    CatCtl  *ctl_;
};

#endif
