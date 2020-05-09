#ifndef uigconfig_h
#define uigconfig_h

#include <QDialog>
#include <QScopedPointer>

struct GlobalOptions
{
    bool        minimizeToTray;
    QStringList languages;
    int         currentLanguage;
};

namespace Ui
{
    class GConfig;
}

class GConfigView
    : public QDialog
{
    Q_OBJECT

public:
    GConfigView(QWidget *parent = 0);
    ~GConfigView();

    void setOptions(const GlobalOptions & opts);
    GlobalOptions getOptions() const;

protected:
    virtual void changeEvent(QEvent *event);

private:
    QScopedPointer<Ui::GConfig> ui;

    QTimer         *validatePathTimer_;
    unsigned int    lastElmEdit_;
    unsigned int    lastElmConfEdit_;
};

#endif
