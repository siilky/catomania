/********************************************************************************
** Form generated from reading UI file 'license.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LICENSE_H
#define UI_LICENSE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_License
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLabel *label_2;
    QFrame *line;
    QLabel *lbStatus;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QLineEdit *edIdentifier;
    QPlainTextEdit *edLicense;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *License)
    {
        if (License->objectName().isEmpty())
            License->setObjectName(QString::fromUtf8("License"));
        License->setWindowModality(Qt::ApplicationModal);
        License->resize(555, 232);
        License->setModal(true);
        verticalLayout = new QVBoxLayout(License);
        verticalLayout->setSpacing(5);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(5, 5, 5, 5);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(15);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(5, 2, 0, 2);
        label = new QLabel(License);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setPixmap(QPixmap(QString::fromUtf8(":/license/license_key.png")));

        horizontalLayout->addWidget(label);

        label_2 = new QLabel(License);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);


        verticalLayout->addLayout(horizontalLayout);

        line = new QFrame(License);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        lbStatus = new QLabel(License);
        lbStatus->setObjectName(QString::fromUtf8("lbStatus"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        lbStatus->setFont(font);

        verticalLayout->addWidget(lbStatus);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(5);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 5, 0, 0);
        label_3 = new QLabel(License);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_2->addWidget(label_3);

        edIdentifier = new QLineEdit(License);
        edIdentifier->setObjectName(QString::fromUtf8("edIdentifier"));
        edIdentifier->setCursor(QCursor(Qt::ArrowCursor));
        edIdentifier->setMaxLength(255);
        edIdentifier->setReadOnly(true);

        horizontalLayout_2->addWidget(edIdentifier);


        verticalLayout->addLayout(horizontalLayout_2);

        edLicense = new QPlainTextEdit(License);
        edLicense->setObjectName(QString::fromUtf8("edLicense"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Courier New"));
        font1.setPointSize(8);
        edLicense->setFont(font1);
        edLicense->setFrameShape(QFrame::Box);
        edLicense->setTabChangesFocus(true);
        edLicense->setLineWrapMode(QPlainTextEdit::NoWrap);
        edLicense->setCursorWidth(4);

        verticalLayout->addWidget(edLicense);

        buttonBox = new QDialogButtonBox(License);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(License);
        QObject::connect(buttonBox, SIGNAL(accepted()), License, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), License, SLOT(reject()));

        QMetaObject::connectSlotsByName(License);
    } // setupUi

    void retranslateUi(QDialog *License)
    {
        License->setWindowTitle(QCoreApplication::translate("License", "Cats2 - License", nullptr));
        label->setText(QString());
        label_2->setText(QCoreApplication::translate("License", "This application requires valid license to run.\n"
"If you have a license please paste it in text box below.\n"
"If you don't have a license please purchase it using following serial identifier.", nullptr));
        label_3->setText(QCoreApplication::translate("License", "Identifier", nullptr));
    } // retranslateUi

};

namespace Ui {
    class License: public Ui_License {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LICENSE_H
