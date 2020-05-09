#include "stdafx.h"

#include "uimoveitems.h"
#include "ui_moveitems.h"

MoveItems::MoveItems(QWidget *parent)
    : QDialog(parent, Qt::Tool)
{
    ui_ = new Ui::MoveItems();
    ui_->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
}

MoveItems::~MoveItems()
{
    delete ui_;
}

void MoveItems::setMaximum(unsigned maximum)
{
    assert(maximum > 1);
    ui_->spinBox->setMaximum(maximum);
}

unsigned MoveItems::value() const
{
    return ui_->spinBox->value();
}
