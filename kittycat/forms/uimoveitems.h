#ifndef UIMOVEITEMS_H
#define UIMOVEITEMS_H

#include <QDialog>
#include <QWidget>

namespace Ui {class MoveItems;};

class MoveItems : public QDialog
{
    Q_OBJECT
public:
    MoveItems(QWidget *parent = 0);
    ~MoveItems();

    void setMaximum(unsigned maximum);
    unsigned value() const;

private:
    Ui::MoveItems *ui_;
};

#endif
