/********************************************************************************
** Form generated from reading UI file 'chat.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Chat
{
public:
    QVBoxLayout *verticalLayout;
    QTextBrowser *tbText;
    QHBoxLayout *horizontalLayout;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *tbPublic;
    QToolButton *tbWorld;
    QToolButton *tbClan;
    QToolButton *tbGroup;
    QToolButton *tbPrivate;
    QComboBox *cbType;
    QLineEdit *leInput;

    void setupUi(QWidget *Chat)
    {
        if (Chat->objectName().isEmpty())
            Chat->setObjectName(QString::fromUtf8("Chat"));
        Chat->resize(480, 300);
        Chat->setStyleSheet(QString::fromUtf8("QToolButton\n"
"{\n"
"	margin:0;\n"
"	padding:0;\n"
"	border: 1px solid;\n"
"	border-radius: 6px;\n"
"	background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:1, fx:0.25, fy:0.273, stop:0.0738636 rgba(192, 192, 192, 255), stop:0.357955 rgba(129, 129, 129, 255), stop:0.903409 rgba(0, 0, 0, 0));\n"
"}\n"
"QToolButton:pressed {\n"
"    background-color: lightgray;\n"
"}\n"
"QToolButton:pressed:checked {\n"
"    background-color: lightgray;\n"
"}\n"
"QToolButton:checked {\n"
"background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:1, fx:0.25, fy:0.273, stop:0.0738636 rgba(255, 255, 255, 255), stop:0.352273 rgba(255, 255, 127, 234), stop:0.903409 rgba(0, 0, 0, 0));\n"
"}"));
        verticalLayout = new QVBoxLayout(Chat);
        verticalLayout->setSpacing(2);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 2);
        tbText = new QTextBrowser(Chat);
        tbText->setObjectName(QString::fromUtf8("tbText"));
        tbText->setMinimumSize(QSize(200, 0));
        tbText->setFocusPolicy(Qt::ClickFocus);
        tbText->setStyleSheet(QString::fromUtf8("QTextBrowser {\n"
"	background-color: black;\n"
"	color: yellow;\n"
"}"));
        tbText->setFrameShape(QFrame::NoFrame);
        tbText->setTabChangesFocus(true);
        tbText->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse|Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
        tbText->setOpenExternalLinks(false);
        tbText->setOpenLinks(false);

        verticalLayout->addWidget(tbText);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(4);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        tbPublic = new QToolButton(Chat);
        tbPublic->setObjectName(QString::fromUtf8("tbPublic"));
        tbPublic->setCheckable(true);
        tbPublic->setChecked(false);

        horizontalLayout_2->addWidget(tbPublic);

        tbWorld = new QToolButton(Chat);
        tbWorld->setObjectName(QString::fromUtf8("tbWorld"));
        tbWorld->setCheckable(true);
        tbWorld->setToolButtonStyle(Qt::ToolButtonTextOnly);

        horizontalLayout_2->addWidget(tbWorld);

        tbClan = new QToolButton(Chat);
        tbClan->setObjectName(QString::fromUtf8("tbClan"));
        tbClan->setCheckable(true);

        horizontalLayout_2->addWidget(tbClan);

        tbGroup = new QToolButton(Chat);
        tbGroup->setObjectName(QString::fromUtf8("tbGroup"));
        tbGroup->setCheckable(true);
        tbGroup->setToolButtonStyle(Qt::ToolButtonTextOnly);

        horizontalLayout_2->addWidget(tbGroup);

        tbPrivate = new QToolButton(Chat);
        tbPrivate->setObjectName(QString::fromUtf8("tbPrivate"));
        tbPrivate->setCheckable(true);
        tbPrivate->setToolButtonStyle(Qt::ToolButtonTextOnly);

        horizontalLayout_2->addWidget(tbPrivate);


        horizontalLayout->addLayout(horizontalLayout_2);

        cbType = new QComboBox(Chat);
        cbType->setObjectName(QString::fromUtf8("cbType"));

        horizontalLayout->addWidget(cbType);

        leInput = new QLineEdit(Chat);
        leInput->setObjectName(QString::fromUtf8("leInput"));
        leInput->setFocusPolicy(Qt::WheelFocus);
        leInput->setMaxLength(80);

        horizontalLayout->addWidget(leInput);


        verticalLayout->addLayout(horizontalLayout);

        QWidget::setTabOrder(leInput, cbType);
        QWidget::setTabOrder(cbType, tbText);

        retranslateUi(Chat);

        QMetaObject::connectSlotsByName(Chat);
    } // setupUi

    void retranslateUi(QWidget *Chat)
    {
        tbText->setHtml(QCoreApplication::translate("Chat", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"><br /></p></body></html>", nullptr));
#if QT_CONFIG(tooltip)
        tbPublic->setToolTip(QCoreApplication::translate("Chat", "Public (nearby)", nullptr));
#endif // QT_CONFIG(tooltip)
        tbPublic->setText(QCoreApplication::translate("Chat", "A", nullptr));
#if QT_CONFIG(tooltip)
        tbWorld->setToolTip(QCoreApplication::translate("Chat", "World", nullptr));
#endif // QT_CONFIG(tooltip)
        tbWorld->setText(QCoreApplication::translate("Chat", "W", nullptr));
#if QT_CONFIG(tooltip)
        tbClan->setToolTip(QCoreApplication::translate("Chat", "Clan", nullptr));
#endif // QT_CONFIG(tooltip)
        tbClan->setText(QCoreApplication::translate("Chat", "C", nullptr));
#if QT_CONFIG(tooltip)
        tbGroup->setToolTip(QCoreApplication::translate("Chat", "Group", nullptr));
#endif // QT_CONFIG(tooltip)
        tbGroup->setText(QCoreApplication::translate("Chat", "G", nullptr));
#if QT_CONFIG(tooltip)
        tbPrivate->setToolTip(QCoreApplication::translate("Chat", "Private", nullptr));
#endif // QT_CONFIG(tooltip)
        tbPrivate->setText(QCoreApplication::translate("Chat", "P", nullptr));
#if QT_CONFIG(tooltip)
        cbType->setToolTip(QCoreApplication::translate("Chat", "Target chat", nullptr));
#endif // QT_CONFIG(tooltip)
        Q_UNUSED(Chat);
    } // retranslateUi

};

namespace Ui {
    class Chat: public Ui_Chat {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHAT_H
