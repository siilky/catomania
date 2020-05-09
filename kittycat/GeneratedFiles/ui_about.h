/********************************************************************************
** Form generated from reading UI file 'about.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

class Ui_About
{
public:
    QGridLayout *gridLayout;
    QLabel *lbAbout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *lbRegId;
    QLabel *lbRegInfo;
    QToolButton *btnRenew;
    QFrame *line;
    QLabel *label;
    QLabel *lbVersion;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;

    void setupUi(QDialog *About)
    {
        if (About->objectName().isEmpty())
            About->setObjectName(QString::fromUtf8("About"));
        About->resize(405, 276);
        gridLayout = new QGridLayout(About);
        gridLayout->setSpacing(5);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setSizeConstraint(QLayout::SetFixedSize);
        gridLayout->setContentsMargins(5, 5, 5, 5);
        lbAbout = new QLabel(About);
        lbAbout->setObjectName(QString::fromUtf8("lbAbout"));
        lbAbout->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbAbout->setOpenExternalLinks(true);

        gridLayout->addWidget(lbAbout, 0, 1, 1, 1);

        groupBox = new QGroupBox(About);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setHorizontalSpacing(6);
        gridLayout_2->setVerticalSpacing(5);
        gridLayout_2->setContentsMargins(3, 2, 3, 5);
        lbRegId = new QLabel(groupBox);
        lbRegId->setObjectName(QString::fromUtf8("lbRegId"));
        lbRegId->setMinimumSize(QSize(220, 0));
        lbRegId->setFrameShape(QFrame::Panel);
        lbRegId->setFrameShadow(QFrame::Sunken);
        lbRegId->setMargin(2);
        lbRegId->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);

        gridLayout_2->addWidget(lbRegId, 1, 0, 1, 1);

        lbRegInfo = new QLabel(groupBox);
        lbRegInfo->setObjectName(QString::fromUtf8("lbRegInfo"));
        lbRegInfo->setWordWrap(true);
        lbRegInfo->setIndent(5);

        gridLayout_2->addWidget(lbRegInfo, 0, 0, 1, 1);

        btnRenew = new QToolButton(groupBox);
        btnRenew->setObjectName(QString::fromUtf8("btnRenew"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(btnRenew->sizePolicy().hasHeightForWidth());
        btnRenew->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/license/license_key.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnRenew->setIcon(icon);
        btnRenew->setIconSize(QSize(24, 24));
        btnRenew->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        gridLayout_2->addWidget(btnRenew, 0, 1, 2, 1);


        gridLayout->addWidget(groupBox, 1, 1, 1, 1);

        line = new QFrame(About);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 2, 0, 1, 2);

        label = new QLabel(About);
        label->setObjectName(QString::fromUtf8("label"));
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setFrameShape(QFrame::Box);
        label->setPixmap(QPixmap(QString::fromUtf8(":/about/antikotin.png")));

        gridLayout->addWidget(label, 0, 0, 2, 1);

        lbVersion = new QLabel(About);
        lbVersion->setObjectName(QString::fromUtf8("lbVersion"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        lbVersion->setFont(font);
        lbVersion->setStyleSheet(QString::fromUtf8("color: #808080;"));

        gridLayout->addWidget(lbVersion, 3, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(About);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);


        gridLayout->addLayout(horizontalLayout, 3, 1, 1, 1);

        gridLayout->setRowStretch(0, 1);

        retranslateUi(About);
        QObject::connect(pushButton, SIGNAL(clicked()), About, SLOT(accept()));

        QMetaObject::connectSlotsByName(About);
    } // setupUi

    void retranslateUi(QDialog *About)
    {
        About->setWindowTitle(QCoreApplication::translate("About", "About", nullptr));
        lbAbout->setText(QCoreApplication::translate("About", "<html><head/><body>\n"
"<p align=\"center\"><span style=\" font-size:14pt; font-weight:600;\">Cat'o'mania</span></p>\n"
"<p align=\"center\">\302\251 Jerry,Mousehouse Lab, 2011-2018<br/>\n"
"<a href=\"http://client.pwcats.info\"><span style=\" text-decoration: underline; color:#0000ff;\">http://client.pwcats.info<br/></span></a>\n"
"<span style=\" font-size:10pt; \"/><img src=\":/about/gmail.png\" /> \n"
"<a href=\"mailto:silkytail@gmail.com\">silkytail@gmail.com</a><br/>\n"
"<img src=\":/about/telegram.png\" /> <a href=\"https://t.me/siilky\"><span style=\"text-decoration: underline; color:#0000ff;\">@siilky</span></a></p>\n"
"<p align=\"center\">Please visit our Market Search Service<br/>\n"
"<a href=\"http://pwcats.info\"><span style=\" text-decoration: underline; color:#0000ff;\">http://pwcats.info</span></a></p>\n"
"</body></html>", nullptr));
        groupBox->setTitle(QCoreApplication::translate("About", "Registration info", nullptr));
        btnRenew->setText(QCoreApplication::translate("About", "Renew", nullptr));
        pushButton->setText(QCoreApplication::translate("About", "Close", nullptr));
    } // retranslateUi

};

namespace Ui {
    class About: public Ui_About {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_H
