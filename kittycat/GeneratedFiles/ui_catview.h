/********************************************************************************
** Form generated from reading UI file 'catview.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CATVIEW_H
#define UI_CATVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CatView
{
public:
    QVBoxLayout *verticalLayout_9;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout_10;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_5;
    QComboBox *cbAccounts;
    QToolButton *btnAccountsSetup;
    QCheckBox *cbAutoLogin;
    QCheckBox *cbForcedLogin;
    QCheckBox *cbAutorelogin;
    QCheckBox *cbTropophobia;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QToolButton *btnLogin;
    QStackedWidget *swLoginClient;
    QWidget *page;
    QVBoxLayout *verticalLayout_12;
    QProgressBar *pbAuth;
    QWidget *verticalStackedWidgetPage1;
    QVBoxLayout *verticalLayout_11;
    QSpacerItem *verticalSpacer_2;
    QToolButton *btnShowClientWindow;
    QToolButton *btnLogout;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_6;
    QCheckBox *cbAutoMarket;
    QGridLayout *gridLayout;
    QToolButton *btnCloseMarket;
    QToolButton *btnMarketSetup;
    QToolButton *btnOpenMarket;
    QToolButton *btnHistory;
    QToolButton *btnCommi;
    QSpacerItem *verticalSpacer;
    QTabWidget *tabWidget;
    QWidget *tabHistory;
    QVBoxLayout *verticalLayout_4;
    QToolButton *toolButton;
    QTextBrowser *teLog;
    QWidget *tabMarket;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_2;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_7;
    QLabel *lbSellingItems;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_3;
    QScrollArea *scrollArea_2;
    QWidget *scrollAreaWidgetContents_2;
    QVBoxLayout *verticalLayout_8;
    QLabel *lbBuyingItems;
    QStatusBar *statusBar;

    void setupUi(QWidget *CatView)
    {
        if (CatView->objectName().isEmpty())
            CatView->setObjectName(QString::fromUtf8("CatView"));
        CatView->resize(750, 348);
        verticalLayout_9 = new QVBoxLayout(CatView);
        verticalLayout_9->setSpacing(0);
        verticalLayout_9->setContentsMargins(11, 11, 11, 11);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        verticalLayout_9->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(5);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(5, 0, 0, 0);
        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setSpacing(0);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        groupBox_2 = new QGroupBox(CatView);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy);
        verticalLayout_5 = new QVBoxLayout(groupBox_2);
        verticalLayout_5->setSpacing(0);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(3, 1, 3, 3);
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(2);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(0, 0, 0, 2);
        cbAccounts = new QComboBox(groupBox_2);
        cbAccounts->setObjectName(QString::fromUtf8("cbAccounts"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(cbAccounts->sizePolicy().hasHeightForWidth());
        cbAccounts->setSizePolicy(sizePolicy1);
        cbAccounts->setMaximumSize(QSize(170, 16777215));
        cbAccounts->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        horizontalLayout_5->addWidget(cbAccounts);

        btnAccountsSetup = new QToolButton(groupBox_2);
        btnAccountsSetup->setObjectName(QString::fromUtf8("btnAccountsSetup"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/catView/group.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnAccountsSetup->setIcon(icon);

        horizontalLayout_5->addWidget(btnAccountsSetup);


        verticalLayout_5->addLayout(horizontalLayout_5);

        cbAutoLogin = new QCheckBox(groupBox_2);
        cbAutoLogin->setObjectName(QString::fromUtf8("cbAutoLogin"));
        sizePolicy.setHeightForWidth(cbAutoLogin->sizePolicy().hasHeightForWidth());
        cbAutoLogin->setSizePolicy(sizePolicy);

        verticalLayout_5->addWidget(cbAutoLogin);

        cbForcedLogin = new QCheckBox(groupBox_2);
        cbForcedLogin->setObjectName(QString::fromUtf8("cbForcedLogin"));

        verticalLayout_5->addWidget(cbForcedLogin);

        cbAutorelogin = new QCheckBox(groupBox_2);
        cbAutorelogin->setObjectName(QString::fromUtf8("cbAutorelogin"));

        verticalLayout_5->addWidget(cbAutorelogin);

        cbTropophobia = new QCheckBox(groupBox_2);
        cbTropophobia->setObjectName(QString::fromUtf8("cbTropophobia"));

        verticalLayout_5->addWidget(cbTropophobia);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 4, 0, 0);
        horizontalSpacer_2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        btnLogin = new QToolButton(groupBox_2);
        btnLogin->setObjectName(QString::fromUtf8("btnLogin"));
        btnLogin->setMinimumSize(QSize(66, 0));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/catView/door_open.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnLogin->setIcon(icon1);
        btnLogin->setIconSize(QSize(32, 32));
        btnLogin->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(btnLogin);

        swLoginClient = new QStackedWidget(groupBox_2);
        swLoginClient->setObjectName(QString::fromUtf8("swLoginClient"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        verticalLayout_12 = new QVBoxLayout(page);
        verticalLayout_12->setSpacing(0);
        verticalLayout_12->setContentsMargins(11, 11, 11, 11);
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        verticalLayout_12->setContentsMargins(0, 0, 0, 0);
        pbAuth = new QProgressBar(page);
        pbAuth->setObjectName(QString::fromUtf8("pbAuth"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Ignored);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(pbAuth->sizePolicy().hasHeightForWidth());
        pbAuth->setSizePolicy(sizePolicy2);
        pbAuth->setMaximumSize(QSize(6, 16777215));
        pbAuth->setStyleSheet(QString::fromUtf8("QProgressBar {\n"
"margin: 1 0 1 0;\n"
"border: 0;\n"
"background: transparent;\n"
"}\n"
"QProgressBar::chunk {\n"
"background-color: #0B0;\n"
"border: 1px solid #000;\n"
"/*height: 4;\n"
"padding: 1;\n"
"*/\n"
"}"));
        pbAuth->setMaximum(5);
        pbAuth->setValue(0);
        pbAuth->setTextVisible(false);
        pbAuth->setOrientation(Qt::Vertical);

        verticalLayout_12->addWidget(pbAuth);

        swLoginClient->addWidget(page);
        verticalStackedWidgetPage1 = new QWidget();
        verticalStackedWidgetPage1->setObjectName(QString::fromUtf8("verticalStackedWidgetPage1"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(verticalStackedWidgetPage1->sizePolicy().hasHeightForWidth());
        verticalStackedWidgetPage1->setSizePolicy(sizePolicy3);
        verticalLayout_11 = new QVBoxLayout(verticalStackedWidgetPage1);
        verticalLayout_11->setSpacing(0);
        verticalLayout_11->setContentsMargins(11, 11, 11, 11);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        verticalLayout_11->setContentsMargins(0, 0, 0, 0);
        verticalSpacer_2 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_11->addItem(verticalSpacer_2);

        btnShowClientWindow = new QToolButton(verticalStackedWidgetPage1);
        btnShowClientWindow->setObjectName(QString::fromUtf8("btnShowClientWindow"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/catView/monitor_16.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnShowClientWindow->setIcon(icon2);
        btnShowClientWindow->setCheckable(true);

        verticalLayout_11->addWidget(btnShowClientWindow, 0, Qt::AlignLeft);

        swLoginClient->addWidget(verticalStackedWidgetPage1);

        horizontalLayout->addWidget(swLoginClient);

        btnLogout = new QToolButton(groupBox_2);
        btnLogout->setObjectName(QString::fromUtf8("btnLogout"));
        btnLogout->setMinimumSize(QSize(65, 0));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/catView/door.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnLogout->setIcon(icon3);
        btnLogout->setIconSize(QSize(32, 32));
        btnLogout->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout->addWidget(btnLogout);

        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout_5->addLayout(horizontalLayout);


        verticalLayout_10->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(CatView);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        sizePolicy.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy);
        verticalLayout_6 = new QVBoxLayout(groupBox_3);
        verticalLayout_6->setSpacing(0);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(3, 1, 3, 3);
        cbAutoMarket = new QCheckBox(groupBox_3);
        cbAutoMarket->setObjectName(QString::fromUtf8("cbAutoMarket"));

        verticalLayout_6->addWidget(cbAutoMarket);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(1);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 4, 0, 0);
        btnCloseMarket = new QToolButton(groupBox_3);
        btnCloseMarket->setObjectName(QString::fromUtf8("btnCloseMarket"));
        sizePolicy1.setHeightForWidth(btnCloseMarket->sizePolicy().hasHeightForWidth());
        btnCloseMarket->setSizePolicy(sizePolicy1);
        btnCloseMarket->setMinimumSize(QSize(60, 0));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/catView/shop_closed.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCloseMarket->setIcon(icon4);
        btnCloseMarket->setIconSize(QSize(32, 32));
        btnCloseMarket->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        gridLayout->addWidget(btnCloseMarket, 0, 1, 1, 1);

        btnMarketSetup = new QToolButton(groupBox_3);
        btnMarketSetup->setObjectName(QString::fromUtf8("btnMarketSetup"));
        sizePolicy1.setHeightForWidth(btnMarketSetup->sizePolicy().hasHeightForWidth());
        btnMarketSetup->setSizePolicy(sizePolicy1);
        btnMarketSetup->setMinimumSize(QSize(60, 0));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/catView/widgets.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnMarketSetup->setIcon(icon5);
        btnMarketSetup->setIconSize(QSize(32, 32));
        btnMarketSetup->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        gridLayout->addWidget(btnMarketSetup, 0, 2, 1, 1);

        btnOpenMarket = new QToolButton(groupBox_3);
        btnOpenMarket->setObjectName(QString::fromUtf8("btnOpenMarket"));
        sizePolicy1.setHeightForWidth(btnOpenMarket->sizePolicy().hasHeightForWidth());
        btnOpenMarket->setSizePolicy(sizePolicy1);
        btnOpenMarket->setMinimumSize(QSize(60, 0));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/catView/cat_32.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnOpenMarket->setIcon(icon6);
        btnOpenMarket->setIconSize(QSize(32, 32));
        btnOpenMarket->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        gridLayout->addWidget(btnOpenMarket, 0, 0, 1, 1);

        btnHistory = new QToolButton(groupBox_3);
        btnHistory->setObjectName(QString::fromUtf8("btnHistory"));
        sizePolicy1.setHeightForWidth(btnHistory->sizePolicy().hasHeightForWidth());
        btnHistory->setSizePolicy(sizePolicy1);
        btnHistory->setMinimumSize(QSize(60, 0));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/catView/table_money.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnHistory->setIcon(icon7);
        btnHistory->setIconSize(QSize(32, 32));
        btnHistory->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        gridLayout->addWidget(btnHistory, 1, 2, 1, 1);

        btnCommi = new QToolButton(groupBox_3);
        btnCommi->setObjectName(QString::fromUtf8("btnCommi"));
        sizePolicy1.setHeightForWidth(btnCommi->sizePolicy().hasHeightForWidth());
        btnCommi->setSizePolicy(sizePolicy1);
        btnCommi->setMinimumSize(QSize(60, 0));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/catView/small_business.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCommi->setIcon(icon8);
        btnCommi->setIconSize(QSize(32, 32));
        btnCommi->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        gridLayout->addWidget(btnCommi, 1, 0, 1, 1);


        verticalLayout_6->addLayout(gridLayout);


        verticalLayout_10->addWidget(groupBox_3);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_10->addItem(verticalSpacer);


        horizontalLayout_4->addLayout(verticalLayout_10);

        tabWidget = new QTabWidget(CatView);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setStyleSheet(QString::fromUtf8("QTabWidget::pane\n"
"{\n"
"	border: 1px solid lightgray;\n"
"    background-color: transparent;\n"
"}\n"
""));
        tabHistory = new QWidget();
        tabHistory->setObjectName(QString::fromUtf8("tabHistory"));
        verticalLayout_4 = new QVBoxLayout(tabHistory);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        toolButton = new QToolButton(tabHistory);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));
        sizePolicy2.setHeightForWidth(toolButton->sizePolicy().hasHeightForWidth());
        toolButton->setSizePolicy(sizePolicy2);
        toolButton->setMinimumSize(QSize(17, 16));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/multiView/cross.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButton->setIcon(icon9);
        toolButton->setIconSize(QSize(9, 9));

        verticalLayout_4->addWidget(toolButton, 0, Qt::AlignRight);

        teLog = new QTextBrowser(tabHistory);
        teLog->setObjectName(QString::fromUtf8("teLog"));
        teLog->setMinimumSize(QSize(200, 0));
        teLog->setStyleSheet(QString::fromUtf8("QTextBrowser {\n"
"	background-color: black;\n"
"	color: yellow;\n"
"}"));
        teLog->setFrameShape(QFrame::NoFrame);
        teLog->setTabChangesFocus(true);
        teLog->setReadOnly(true);
        teLog->setOpenExternalLinks(false);
        teLog->setOpenLinks(false);

        verticalLayout_4->addWidget(teLog);

        tabWidget->addTab(tabHistory, QString());
        teLog->raise();
        toolButton->raise();
        tabMarket = new QWidget();
        tabMarket->setObjectName(QString::fromUtf8("tabMarket"));
        verticalLayout = new QVBoxLayout(tabMarket);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        groupBox_4 = new QGroupBox(tabMarket);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setAutoFillBackground(true);
        groupBox_4->setAlignment(Qt::AlignCenter);
        groupBox_4->setFlat(true);
        verticalLayout_2 = new QVBoxLayout(groupBox_4);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(4, 0, 0, 0);
        scrollArea = new QScrollArea(groupBox_4);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 100, 30));
        verticalLayout_7 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_7->setSpacing(0);
        verticalLayout_7->setContentsMargins(11, 11, 11, 11);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(0, 0, 0, 0);
        lbSellingItems = new QLabel(scrollAreaWidgetContents);
        lbSellingItems->setObjectName(QString::fromUtf8("lbSellingItems"));
        lbSellingItems->setTextFormat(Qt::RichText);
        lbSellingItems->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbSellingItems->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);

        verticalLayout_7->addWidget(lbSellingItems);

        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout_2->addWidget(scrollArea);


        verticalLayout->addWidget(groupBox_4);

        groupBox_5 = new QGroupBox(tabMarket);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        groupBox_5->setAutoFillBackground(true);
        groupBox_5->setAlignment(Qt::AlignCenter);
        groupBox_5->setFlat(true);
        verticalLayout_3 = new QVBoxLayout(groupBox_5);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(4, 0, 0, 0);
        scrollArea_2 = new QScrollArea(groupBox_5);
        scrollArea_2->setObjectName(QString::fromUtf8("scrollArea_2"));
        scrollArea_2->setFrameShape(QFrame::NoFrame);
        scrollArea_2->setWidgetResizable(true);
        scrollAreaWidgetContents_2 = new QWidget();
        scrollAreaWidgetContents_2->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_2"));
        scrollAreaWidgetContents_2->setGeometry(QRect(0, 0, 100, 30));
        verticalLayout_8 = new QVBoxLayout(scrollAreaWidgetContents_2);
        verticalLayout_8->setSpacing(0);
        verticalLayout_8->setContentsMargins(11, 11, 11, 11);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        verticalLayout_8->setContentsMargins(0, 0, 0, 0);
        lbBuyingItems = new QLabel(scrollAreaWidgetContents_2);
        lbBuyingItems->setObjectName(QString::fromUtf8("lbBuyingItems"));
        lbBuyingItems->setTextFormat(Qt::RichText);
        lbBuyingItems->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbBuyingItems->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);

        verticalLayout_8->addWidget(lbBuyingItems);

        scrollArea_2->setWidget(scrollAreaWidgetContents_2);

        verticalLayout_3->addWidget(scrollArea_2);


        verticalLayout->addWidget(groupBox_5);

        tabWidget->addTab(tabMarket, QString());

        horizontalLayout_4->addWidget(tabWidget);

        horizontalLayout_4->setStretch(1, 1);

        verticalLayout_9->addLayout(horizontalLayout_4);

        statusBar = new QStatusBar(CatView);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));

        verticalLayout_9->addWidget(statusBar);


        retranslateUi(CatView);
        QObject::connect(toolButton, SIGNAL(clicked()), teLog, SLOT(clear()));

        swLoginClient->setCurrentIndex(0);
        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(CatView);
    } // setupUi

    void retranslateUi(QWidget *CatView)
    {
        CatView->setWindowTitle(QCoreApplication::translate("CatView", "Cat", "WindowTitle"));
        groupBox_2->setTitle(QCoreApplication::translate("CatView", "Login", nullptr));
#if QT_CONFIG(tooltip)
        cbAccounts->setToolTip(QCoreApplication::translate("CatView", "Account or character to use when logging in to game", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        btnAccountsSetup->setToolTip(QCoreApplication::translate("CatView", "Setup account", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        cbAutoLogin->setToolTip(QCoreApplication::translate("CatView", "Automatically logs in to game\n"
"after application is launched", nullptr));
#endif // QT_CONFIG(tooltip)
        cbAutoLogin->setText(QCoreApplication::translate("CatView", "Log in automatically", nullptr));
#if QT_CONFIG(tooltip)
        cbForcedLogin->setToolTip(QCoreApplication::translate("CatView", "Game 'forced login'", nullptr));
#endif // QT_CONFIG(tooltip)
        cbForcedLogin->setText(QCoreApplication::translate("CatView", "Forced login", nullptr));
#if QT_CONFIG(tooltip)
        cbAutorelogin->setToolTip(QCoreApplication::translate("CatView", "Enables automatic reconnect after specified interval of time\n"
"when connection is dropped or any connection error is occured", nullptr));
#endif // QT_CONFIG(tooltip)
        cbAutorelogin->setText(QCoreApplication::translate("CatView", "Relogin on disconnect", nullptr));
#if QT_CONFIG(tooltip)
        cbTropophobia->setToolTip(QCoreApplication::translate("CatView", "Logs out when log in position is different from last recorded", nullptr));
#endif // QT_CONFIG(tooltip)
        cbTropophobia->setText(QCoreApplication::translate("CatView", "Immobilizer", nullptr));
#if QT_CONFIG(tooltip)
        btnLogin->setToolTip(QCoreApplication::translate("CatView", "Log in to game", nullptr));
#endif // QT_CONFIG(tooltip)
        btnLogin->setText(QCoreApplication::translate("CatView", "Log in", nullptr));
#if QT_CONFIG(tooltip)
        btnShowClientWindow->setToolTip(QCoreApplication::translate("CatView", "Show/hide client window", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        btnLogout->setToolTip(QCoreApplication::translate("CatView", "Log out from game", nullptr));
#endif // QT_CONFIG(tooltip)
        btnLogout->setText(QCoreApplication::translate("CatView", "Log out", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("CatView", "Market", nullptr));
#if QT_CONFIG(tooltip)
        cbAutoMarket->setToolTip(QCoreApplication::translate("CatView", "Perform automatic market open\n"
"after logging in to game and lock expiration", nullptr));
#endif // QT_CONFIG(tooltip)
        cbAutoMarket->setText(QCoreApplication::translate("CatView", "Open market after logging in", nullptr));
#if QT_CONFIG(tooltip)
        btnCloseMarket->setToolTip(QCoreApplication::translate("CatView", "Close market", nullptr));
#endif // QT_CONFIG(tooltip)
        btnCloseMarket->setText(QCoreApplication::translate("CatView", "Close", nullptr));
#if QT_CONFIG(tooltip)
        btnMarketSetup->setToolTip(QCoreApplication::translate("CatView", "Market setup and cat parameters", nullptr));
#endif // QT_CONFIG(tooltip)
        btnMarketSetup->setText(QCoreApplication::translate("CatView", "Setup", nullptr));
#if QT_CONFIG(tooltip)
        btnOpenMarket->setToolTip(QCoreApplication::translate("CatView", "Open market", nullptr));
#endif // QT_CONFIG(tooltip)
        btnOpenMarket->setText(QCoreApplication::translate("CatView", "Open", nullptr));
#if QT_CONFIG(tooltip)
        btnHistory->setToolTip(QCoreApplication::translate("CatView", "Trade history", nullptr));
#endif // QT_CONFIG(tooltip)
        btnHistory->setText(QCoreApplication::translate("CatView", "Balance", nullptr));
#if QT_CONFIG(tooltip)
        btnCommi->setToolTip(QCoreApplication::translate("CatView", "Comissioneer information", nullptr));
#endif // QT_CONFIG(tooltip)
        btnCommi->setText(QCoreApplication::translate("CatView", "Commi", nullptr));
#if QT_CONFIG(tooltip)
        toolButton->setToolTip(QCoreApplication::translate("CatView", "Clear history", nullptr));
#endif // QT_CONFIG(tooltip)
        tabWidget->setTabText(tabWidget->indexOf(tabHistory), QCoreApplication::translate("CatView", "History", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("CatView", "Selling", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("CatView", "Buying", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabMarket), QString());
    } // retranslateUi

};

namespace Ui {
    class CatView: public Ui_CatView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CATVIEW_H
