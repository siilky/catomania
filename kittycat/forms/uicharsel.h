#ifndef uicharsel_h
#define uicharsel_h

#include <QDialog>
#include <QScopedPointer>

#include "catctl.h"

namespace Ui
{
    class Charsel;
}

class CatCtl;

class CharselView
    : public QDialog
{
    Q_OBJECT

public:
    CharselView(const CatCtl::Account & account, QWidget *parent = 0);
    ~CharselView();

    int getSelected() const;

protected:
    virtual void changeEvent(QEvent *event);

private:
    QScopedPointer<Ui::Charsel> ui;
};

#endif
