/********************************************************************************
** Form generated from reading UI file 'charsel.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHARSEL_H
#define UI_CHARSEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Charsel
{
public:
    QGridLayout *gridLayout;
    QListWidget *listChars;
    QHBoxLayout *horizontalLayout;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *label;

    void setupUi(QDialog *Charsel)
    {
        if (Charsel->objectName().isEmpty())
            Charsel->setObjectName(QString::fromUtf8("Charsel"));
        Charsel->resize(262, 149);
        gridLayout = new QGridLayout(Charsel);
        gridLayout->setSpacing(5);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(5, 5, 5, 5);
        listChars = new QListWidget(Charsel);
        listChars->setObjectName(QString::fromUtf8("listChars"));
        listChars->setEditTriggers(QAbstractItemView::NoEditTriggers);
        listChars->setTabKeyNavigation(true);

        gridLayout->addWidget(listChars, 1, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        okButton = new QPushButton(Charsel);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout->addWidget(okButton);

        cancelButton = new QPushButton(Charsel);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout->addWidget(cancelButton);


        gridLayout->addLayout(horizontalLayout, 2, 0, 1, 1);

        label = new QLabel(Charsel);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);


        retranslateUi(Charsel);
        QObject::connect(okButton, SIGNAL(clicked()), Charsel, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), Charsel, SLOT(reject()));
        QObject::connect(listChars, SIGNAL(itemDoubleClicked(QListWidgetItem*)), Charsel, SLOT(accept()));

        QMetaObject::connectSlotsByName(Charsel);
    } // setupUi

    void retranslateUi(QDialog *Charsel)
    {
        Charsel->setWindowTitle(QCoreApplication::translate("Charsel", "Charselect", nullptr));
        okButton->setText(QCoreApplication::translate("Charsel", "OK", nullptr));
        cancelButton->setText(QCoreApplication::translate("Charsel", "Cancel", nullptr));
        label->setText(QCoreApplication::translate("Charsel", "Select character to log in", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Charsel: public Ui_Charsel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHARSEL_H
