/********************************************************************************
** Form generated from reading UI file 'clientlocation.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENTLOCATION_H
#define UI_CLIENTLOCATION_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ClientLocation
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QLineEdit *tbPath;
    QToolButton *btnOpenPath;
    QLabel *lbExecutable;
    QLineEdit *leExecutable;
    QFrame *line;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ClientLocation)
    {
        if (ClientLocation->objectName().isEmpty())
            ClientLocation->setObjectName(QString::fromUtf8("ClientLocation"));
        ClientLocation->resize(421, 241);
        verticalLayout = new QVBoxLayout(ClientLocation);
        verticalLayout->setSpacing(5);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
        verticalLayout->setContentsMargins(15, 15, 15, 10);
        label = new QLabel(ClientLocation);
        label->setObjectName(QString::fromUtf8("label"));
        label->setWordWrap(true);

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(10);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        tbPath = new QLineEdit(ClientLocation);
        tbPath->setObjectName(QString::fromUtf8("tbPath"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tbPath->sizePolicy().hasHeightForWidth());
        tbPath->setSizePolicy(sizePolicy);
        tbPath->setMinimumSize(QSize(300, 0));

        horizontalLayout->addWidget(tbPath);

        btnOpenPath = new QToolButton(ClientLocation);
        btnOpenPath->setObjectName(QString::fromUtf8("btnOpenPath"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/config/folder_blue.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnOpenPath->setIcon(icon);
        btnOpenPath->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(btnOpenPath);


        verticalLayout->addLayout(horizontalLayout);

        lbExecutable = new QLabel(ClientLocation);
        lbExecutable->setObjectName(QString::fromUtf8("lbExecutable"));

        verticalLayout->addWidget(lbExecutable);

        leExecutable = new QLineEdit(ClientLocation);
        leExecutable->setObjectName(QString::fromUtf8("leExecutable"));
        sizePolicy.setHeightForWidth(leExecutable->sizePolicy().hasHeightForWidth());
        leExecutable->setSizePolicy(sizePolicy);
        leExecutable->setMinimumSize(QSize(300, 0));

        verticalLayout->addWidget(leExecutable);

        line = new QFrame(ClientLocation);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        buttonBox = new QDialogButtonBox(ClientLocation);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ClientLocation);
        QObject::connect(buttonBox, SIGNAL(rejected()), ClientLocation, SLOT(reject()));

        QMetaObject::connectSlotsByName(ClientLocation);
    } // setupUi

    void retranslateUi(QDialog *ClientLocation)
    {
        ClientLocation->setWindowTitle(QCoreApplication::translate("ClientLocation", "Client location", nullptr));
        label->setText(QCoreApplication::translate("ClientLocation", "<b><u>Game installation path</u></b>\n"
"<br>Please specify path to game installation folder\n"
"<br>(for example, E:\\GamesMailRu\\Perfect World).\n"
"<br>If it's already filled in please make sure the path is correct.", nullptr));
#if QT_CONFIG(tooltip)
        tbPath->setToolTip(QCoreApplication::translate("ClientLocation", "Path to game installation folder.", nullptr));
#endif // QT_CONFIG(tooltip)
        btnOpenPath->setText(QCoreApplication::translate("ClientLocation", "Browse folder", nullptr));
        lbExecutable->setText(QCoreApplication::translate("ClientLocation", "<b><u>Executable option</u></b>\n"
"<br>Set this to alternative name of game executable if you need to\n"
"<br>launch different one or leave empty for default name.\n"
"<br>(for example, elementclient_1.exe)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ClientLocation: public Ui_ClientLocation {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENTLOCATION_H
