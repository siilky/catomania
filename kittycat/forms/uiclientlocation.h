#ifndef uiclientlocation_h
#define uiclientlocation_h

#include <QDialog>
#include <QScopedPointer>

namespace Ui
{
    class ClientLocation;
}

class ClientLocationView
    : public QDialog
{
    Q_OBJECT

public:
    ClientLocationView(QWidget *parent = 0);
    ~ClientLocationView();

    void setPath(const QString & path);
    QString getPath() const;

    void setExeName(const QString & name);
    QString getExeName() const;

protected:
    virtual void changeEvent(QEvent *event);

private slots:
    void on_tbPath_textChanged(const QString &);
    void on_btnOpenPath_clicked();
    void on_buttonBox_accepted();

private:
    QScopedPointer<Ui::ClientLocation> ui;
};

#endif
