/********************************************************************************
** Form generated from reading UI file 'getPin.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GETPIN_H
#define UI_GETPIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_GetPin
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLabel *lbEmail;
    QLabel *lbPcName;
    QLabel *label_4;
    QLineEdit *lePin;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *GetPin)
    {
        if (GetPin->objectName().isEmpty())
            GetPin->setObjectName(QString::fromUtf8("GetPin"));
        GetPin->resize(200, 195);
        verticalLayout = new QVBoxLayout(GetPin);
        verticalLayout->setSpacing(5);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
        verticalLayout->setContentsMargins(10, 10, 10, 10);
        label = new QLabel(GetPin);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(180, 0));
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        lbEmail = new QLabel(GetPin);
        lbEmail->setObjectName(QString::fromUtf8("lbEmail"));
        lbEmail->setFrameShape(QFrame::Box);
        lbEmail->setFrameShadow(QFrame::Sunken);
        lbEmail->setMargin(3);

        verticalLayout->addWidget(lbEmail);

        lbPcName = new QLabel(GetPin);
        lbPcName->setObjectName(QString::fromUtf8("lbPcName"));
        lbPcName->setFrameShape(QFrame::Box);
        lbPcName->setFrameShadow(QFrame::Sunken);
        lbPcName->setMargin(3);

        verticalLayout->addWidget(lbPcName);

        label_4 = new QLabel(GetPin);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        verticalLayout->addWidget(label_4);

        lePin = new QLineEdit(GetPin);
        lePin->setObjectName(QString::fromUtf8("lePin"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lePin->sizePolicy().hasHeightForWidth());
        lePin->setSizePolicy(sizePolicy);
        QFont font;
        font.setPointSize(10);
        lePin->setFont(font);
        lePin->setMaxLength(5);
        lePin->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(lePin, 0, Qt::AlignHCenter);

        verticalSpacer = new QSpacerItem(0, 15, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(GetPin);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);
        buttonBox->setCenterButtons(true);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(GetPin);
        QObject::connect(buttonBox, SIGNAL(accepted()), GetPin, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), GetPin, SLOT(reject()));

        QMetaObject::connectSlotsByName(GetPin);
    } // setupUi

    void retranslateUi(QDialog *GetPin)
    {
        GetPin->setWindowTitle(QCoreApplication::translate("GetPin", "Arc defender", nullptr));
        label->setText(QCoreApplication::translate("GetPin", "Verify your account", nullptr));
        lbPcName->setText(QString());
        label_4->setText(QCoreApplication::translate("GetPin", "Please enter the PIN emailed you", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GetPin: public Ui_GetPin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GETPIN_H
