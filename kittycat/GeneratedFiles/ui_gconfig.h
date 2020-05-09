/********************************************************************************
** Form generated from reading UI file 'gconfig.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GCONFIG_H
#define UI_GCONFIG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_GConfig
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_3;
    QCheckBox *cbMinimizeToTray;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *cbLanguages;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *GConfig)
    {
        if (GConfig->objectName().isEmpty())
            GConfig->setObjectName(QString::fromUtf8("GConfig"));
        GConfig->resize(300, 111);
        GConfig->setMinimumSize(QSize(300, 0));
        GConfig->setModal(true);
        verticalLayout = new QVBoxLayout(GConfig);
        verticalLayout->setSpacing(5);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(5, 5, 5, 6);
        groupBox_2 = new QGroupBox(GConfig);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy);
        verticalLayout_3 = new QVBoxLayout(groupBox_2);
        verticalLayout_3->setSpacing(5);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(8, 5, 8, 8);
        cbMinimizeToTray = new QCheckBox(groupBox_2);
        cbMinimizeToTray->setObjectName(QString::fromUtf8("cbMinimizeToTray"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(cbMinimizeToTray->sizePolicy().hasHeightForWidth());
        cbMinimizeToTray->setSizePolicy(sizePolicy1);
        cbMinimizeToTray->setMinimumSize(QSize(140, 0));

        verticalLayout_3->addWidget(cbMinimizeToTray);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        cbLanguages = new QComboBox(groupBox_2);
        cbLanguages->setObjectName(QString::fromUtf8("cbLanguages"));

        horizontalLayout->addWidget(cbLanguages);


        verticalLayout_3->addLayout(horizontalLayout);


        verticalLayout->addWidget(groupBox_2);

        buttonBox = new QDialogButtonBox(GConfig);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(GConfig);
        QObject::connect(buttonBox, SIGNAL(accepted()), GConfig, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), GConfig, SLOT(reject()));

        QMetaObject::connectSlotsByName(GConfig);
    } // setupUi

    void retranslateUi(QDialog *GConfig)
    {
        GConfig->setWindowTitle(QCoreApplication::translate("GConfig", "Global settings", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("GConfig", "Application", nullptr));
        cbMinimizeToTray->setText(QCoreApplication::translate("GConfig", "Minimize to tray", nullptr));
        label->setText(QCoreApplication::translate("GConfig", "Language", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GConfig: public Ui_GConfig {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GCONFIG_H
