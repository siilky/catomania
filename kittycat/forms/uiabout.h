#ifndef uiabout_h
#define uiabout_h

#include <QDialog>

namespace Ui
{
    class About;
}

class AboutView
    : public QDialog
{
    Q_OBJECT
public:
    AboutView(QWidget *parent = 0);
    ~AboutView();

private slots:
    void on_btnRenew_clicked();

private:
    void updateText();

    QScopedPointer<Ui::About> ui;
};

#endif
