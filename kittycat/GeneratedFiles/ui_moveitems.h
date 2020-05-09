/********************************************************************************
** Form generated from reading UI file 'moveitems.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MOVEITEMS_H
#define UI_MOVEITEMS_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_MoveItems
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpinBox *spinBox;
    QPushButton *pushButton;

    void setupUi(QDialog *MoveItems)
    {
        if (MoveItems->objectName().isEmpty())
            MoveItems->setObjectName(QString::fromUtf8("MoveItems"));
        MoveItems->resize(225, 44);
        horizontalLayout = new QHBoxLayout(MoveItems);
        horizontalLayout->setSpacing(10);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetFixedSize);
        horizontalLayout->setContentsMargins(10, 10, 10, 10);
        label = new QLabel(MoveItems);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        spinBox = new QSpinBox(MoveItems);
        spinBox->setObjectName(QString::fromUtf8("spinBox"));
        spinBox->setMinimumSize(QSize(70, 0));
        spinBox->setMinimum(1);

        horizontalLayout->addWidget(spinBox);

        pushButton = new QPushButton(MoveItems);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
        pushButton->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/trade/tick.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton->setIcon(icon);

        horizontalLayout->addWidget(pushButton);


        retranslateUi(MoveItems);
        QObject::connect(pushButton, SIGNAL(clicked()), MoveItems, SLOT(accept()));

        pushButton->setDefault(true);


        QMetaObject::connectSlotsByName(MoveItems);
    } // setupUi

    void retranslateUi(QDialog *MoveItems)
    {
        MoveItems->setWindowTitle(QCoreApplication::translate("MoveItems", "Move items", nullptr));
        label->setText(QCoreApplication::translate("MoveItems", "Move / split items:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MoveItems: public Ui_MoveItems {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MOVEITEMS_H
