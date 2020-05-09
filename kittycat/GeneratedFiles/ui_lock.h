/********************************************************************************
** Form generated from reading UI file 'lock.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOCK_H
#define UI_LOCK_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTimeEdit>

QT_BEGIN_NAMESPACE

class Ui_Lock
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QSpinBox *sbDays;
    QTimeEdit *sbTime;
    QPushButton *btnApply;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label;
    QLabel *lbMessage;
    QLabel *lbCurrentTime;

    void setupUi(QDialog *Lock)
    {
        if (Lock->objectName().isEmpty())
            Lock->setObjectName(QString::fromUtf8("Lock"));
        Lock->resize(397, 92);
        gridLayout = new QGridLayout(Lock);
        gridLayout->setSpacing(8);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setSizeConstraint(QLayout::SetFixedSize);
        gridLayout->setContentsMargins(10, 10, 10, 10);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        sbDays = new QSpinBox(Lock);
        sbDays->setObjectName(QString::fromUtf8("sbDays"));
        sbDays->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        sbDays->setMaximum(365);

        horizontalLayout->addWidget(sbDays);

        sbTime = new QTimeEdit(Lock);
        sbTime->setObjectName(QString::fromUtf8("sbTime"));
        sbTime->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        sbTime->setMinimumTime(QTime(0, 0, 1));
        sbTime->setCurrentSection(QDateTimeEdit::HourSection);
        sbTime->setTime(QTime(0, 0, 1));

        horizontalLayout->addWidget(sbTime);


        gridLayout->addLayout(horizontalLayout, 2, 3, 1, 1);

        btnApply = new QPushButton(Lock);
        btnApply->setObjectName(QString::fromUtf8("btnApply"));

        gridLayout->addWidget(btnApply, 2, 4, 1, 1);

        label_2 = new QLabel(Lock);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 2, 2, 1, 1);

        label_3 = new QLabel(Lock);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 2, 1, 1);

        label = new QLabel(Lock);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setPixmap(QPixmap(QString::fromUtf8(":/lock/lock_32.png")));

        gridLayout->addWidget(label, 1, 1, 2, 1);

        lbMessage = new QLabel(Lock);
        lbMessage->setObjectName(QString::fromUtf8("lbMessage"));
        lbMessage->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(lbMessage, 3, 2, 1, 2);

        lbCurrentTime = new QLabel(Lock);
        lbCurrentTime->setObjectName(QString::fromUtf8("lbCurrentTime"));
        lbCurrentTime->setMinimumSize(QSize(0, 20));
        lbCurrentTime->setFrameShape(QFrame::StyledPanel);
        lbCurrentTime->setFrameShadow(QFrame::Sunken);
        lbCurrentTime->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbCurrentTime, 1, 3, 1, 1);


        retranslateUi(Lock);

        QMetaObject::connectSlotsByName(Lock);
    } // setupUi

    void retranslateUi(QDialog *Lock)
    {
        sbDays->setSuffix(QCoreApplication::translate("Lock", " days", nullptr));
        sbTime->setDisplayFormat(QCoreApplication::translate("Lock", "HH'h' : mm'm' : ss's'", nullptr));
        btnApply->setText(QCoreApplication::translate("Lock", "Apply", nullptr));
        label_2->setText(QCoreApplication::translate("Lock", "Set time to", nullptr));
        label_3->setText(QCoreApplication::translate("Lock", "Current setting", nullptr));
        label->setText(QString());
        Q_UNUSED(Lock);
    } // retranslateUi

};

namespace Ui {
    class Lock: public Ui_Lock {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOCK_H
