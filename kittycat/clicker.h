#ifndef clicker_h_
#define clicker_h_

#include <QObject>

class Clicker : public QObject
{
    Q_OBJECT
public:
    Clicker(QObject *parent = 0);

signals:
    void clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif