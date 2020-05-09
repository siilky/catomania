/********************************************************************************
** Form generated from reading UI file 'accounts.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ACCOUNTS_H
#define UI_ACCOUNTS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include "treewidgetltr.h"

QT_BEGIN_NAMESPACE

class Ui_Accounts
{
public:
    QVBoxLayout *verticalLayout_2;
    QFormLayout *formLayout;
    QComboBox *cbServers;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *leEmail;
    QLabel *label_3;
    QLineEdit *lePassword;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_3;
    TreeWidgetLtr *treeWidget;
    QLabel *lbStatus;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Accounts)
    {
        if (Accounts->objectName().isEmpty())
            Accounts->setObjectName(QString::fromUtf8("Accounts"));
        Accounts->resize(270, 267);
        verticalLayout_2 = new QVBoxLayout(Accounts);
        verticalLayout_2->setSpacing(5);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(5, 5, 5, 5);
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setHorizontalSpacing(4);
        formLayout->setVerticalSpacing(4);
        formLayout->setContentsMargins(-1, 4, -1, -1);
        cbServers = new QComboBox(Accounts);
        cbServers->setObjectName(QString::fromUtf8("cbServers"));
        cbServers->setMaxVisibleItems(16);

        formLayout->setWidget(2, QFormLayout::FieldRole, cbServers);

        label = new QLabel(Accounts);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label);

        label_2 = new QLabel(Accounts);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_2);

        leEmail = new QLineEdit(Accounts);
        leEmail->setObjectName(QString::fromUtf8("leEmail"));
        leEmail->setInputMethodHints(Qt::ImhEmailCharactersOnly);

        formLayout->setWidget(0, QFormLayout::FieldRole, leEmail);

        label_3 = new QLabel(Accounts);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_3);

        lePassword = new QLineEdit(Accounts);
        lePassword->setObjectName(QString::fromUtf8("lePassword"));
        lePassword->setMaxLength(128);
        lePassword->setEchoMode(QLineEdit::PasswordEchoOnEdit);

        formLayout->setWidget(1, QFormLayout::FieldRole, lePassword);


        verticalLayout_2->addLayout(formLayout);

        groupBox_2 = new QGroupBox(Accounts);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setFlat(false);
        verticalLayout_3 = new QVBoxLayout(groupBox_2);
        verticalLayout_3->setSpacing(2);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(1, 5, 1, 1);
        treeWidget = new TreeWidgetLtr(groupBox_2);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidget->setHeaderItem(__qtreewidgetitem);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
        treeWidget->setStyleSheet(QString::fromUtf8("QTreeWidget QLineEdit {\n"
"	/*border: 1px solid darkgray;*/\n"
"	border: none;\n"
"	qproperty-alignment: AlignRight;\n"
"}"));
        treeWidget->setFrameShape(QFrame::NoFrame);
        treeWidget->setFrameShadow(QFrame::Plain);
        treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeWidget->setAlternatingRowColors(true);
        treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        treeWidget->setIndentation(25);
        treeWidget->setRootIsDecorated(true);
        treeWidget->setItemsExpandable(true);
        treeWidget->header()->setVisible(false);

        verticalLayout_3->addWidget(treeWidget);


        verticalLayout_2->addWidget(groupBox_2);

        lbStatus = new QLabel(Accounts);
        lbStatus->setObjectName(QString::fromUtf8("lbStatus"));
        lbStatus->setAlignment(Qt::AlignCenter);
        lbStatus->setWordWrap(true);

        verticalLayout_2->addWidget(lbStatus);

        buttonBox = new QDialogButtonBox(Accounts);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStyleSheet(QString::fromUtf8(".QDialogButtonBox {\n"
"	background-color: white;\n"
"}"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Close|QDialogButtonBox::Save);

        verticalLayout_2->addWidget(buttonBox);

        QWidget::setTabOrder(leEmail, lePassword);
        QWidget::setTabOrder(lePassword, cbServers);
        QWidget::setTabOrder(cbServers, treeWidget);

        retranslateUi(Accounts);
        QObject::connect(buttonBox, SIGNAL(accepted()), Accounts, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Accounts, SLOT(reject()));

        QMetaObject::connectSlotsByName(Accounts);
    } // setupUi

    void retranslateUi(QDialog *Accounts)
    {
        Accounts->setWindowTitle(QCoreApplication::translate("Accounts", "Account", nullptr));
        label->setText(QCoreApplication::translate("Accounts", "Server", nullptr));
        label_2->setText(QCoreApplication::translate("Accounts", "Login", nullptr));
        label_3->setText(QCoreApplication::translate("Accounts", "Password", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("Accounts", "Accounts", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Accounts: public Ui_Accounts {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ACCOUNTS_H
