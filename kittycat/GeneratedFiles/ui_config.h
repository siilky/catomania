/********************************************************************************
** Form generated from reading UI file 'config.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIG_H
#define UI_CONFIG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Config
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *tbShopTitle;
    QLabel *lbTitleLength;
    QSpacerItem *horizontalSpacer;
    QToolButton *btnBuyCatshop;
    QLabel *lbCatshop;
    QLabel *lbCatshopTimer;
    QTableView *tableView;
    QHBoxLayout *horizontalLayout_4;
    QLabel *lbMoney;
    QLabel *label_13;
    QLineEdit *lbToSell;
    QLabel *label_14;
    QLineEdit *lbToBuy;
    QWidget *wProfit;
    QHBoxLayout *horizontalLayout_8;
    QSpacerItem *horizontalSpacer_7;
    QLabel *label_6;
    QLineEdit *lbProfit;
    QToolButton *btnProfitDetails;
    QWidget *wConvert;
    QHBoxLayout *horizontalLayout_7;
    QCheckBox *cbLowConverter;
    QSpinBox *sbLowConverter;
    QSpacerItem *horizontalSpacer_5;
    QCheckBox *cbHighConverter;
    QSpinBox *sbHighConverter;
    QSpacerItem *horizontalSpacer_6;
    QWidget *tab_2;
    QGridLayout *gridLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_3;
    QCheckBox *cbEnableAutoreopenValue;
    QSpinBox *udAutoreopenModeSelector;
    QSpinBox *udAutoreopenValue;
    QSpacerItem *horizontalSpacer_3;
    QSpinBox *udAutoreopenPercentage;
    QLabel *label_5;
    QCheckBox *cbEnableAutoreopenPercentage;
    QCheckBox *cbEnableAutoReopen;
    QSpinBox *udAutoReopenExternalInterval;
    QCheckBox *cbEnableAutoReopenExternal;
    QGridLayout *gridLayout_4;
    QLabel *label_7;
    QLabel *label_8;
    QSpinBox *udAutoreopenDelayClose;
    QSpinBox *udAutoreopenDelayCloseRnd;
    QLabel *label_9;
    QSpinBox *udAutoreopenDelayOpen;
    QLabel *label_10;
    QSpinBox *udAutoreopenDelayOpenRnd;
    QCheckBox *cbSavePartialConfig;
    QFrame *frame;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_8;
    QCheckBox *cbOnlineRegistration;
    QSpacerItem *verticalSpacer_2;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QSpinBox *udConnectDelayFixed;
    QLabel *label_11;
    QSpinBox *udConnectDelayRandom;
    QCheckBox *cbDelayConnectStart;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_7;
    QCheckBox *cbEnableProxy;
    QGridLayout *gridLayout_2;
    QLabel *label_3;
    QLabel *label_15;
    QLineEdit *leProxyHost;
    QLabel *label_17;
    QVBoxLayout *verticalLayout_6;
    QRadioButton *rbProxyTypeSocks;
    QRadioButton *rbProxyTypeHttp;
    QLabel *label_16;
    QLineEdit *leProxyUser;
    QLabel *label_19;
    QLineEdit *leProxyPassword;
    QLabel *label_18;
    QSpacerItem *verticalSpacer;
    QWidget *tab_4;
    QVBoxLayout *verticalLayout_9;
    QGroupBox *groupBox_6;
    QHBoxLayout *horizontalLayout_9;
    QLabel *label_12;
    QSpacerItem *horizontalSpacer_4;
    QSlider *slChatBufferSize;
    QLabel *lbChatBufferSize;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *cbLimitHistory;
    QSpinBox *udLimitHistory;
    QCheckBox *cbSaveLog;
    QCheckBox *cbSplitLog;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_4;
    QLineEdit *tbLogPath;
    QToolButton *btnOpenLogPath;
    QSpacerItem *verticalSpacer_3;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *btnGlobalSettings;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Config)
    {
        if (Config->objectName().isEmpty())
            Config->setObjectName(QString::fromUtf8("Config"));
        Config->resize(685, 453);
        Config->setModal(true);
        verticalLayout = new QVBoxLayout(Config);
        verticalLayout->setSpacing(5);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(5, 5, 5, 5);
        tabWidget = new QTabWidget(Config);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout_2 = new QVBoxLayout(tab);
        verticalLayout_2->setSpacing(5);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(5, 8, 5, 5);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(8);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 10, 0);
        label = new QLabel(tab);
        label->setObjectName(QString::fromUtf8("label"));
        label->setIndent(2);

        horizontalLayout->addWidget(label);

        tbShopTitle = new QLineEdit(tab);
        tbShopTitle->setObjectName(QString::fromUtf8("tbShopTitle"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tbShopTitle->sizePolicy().hasHeightForWidth());
        tbShopTitle->setSizePolicy(sizePolicy);
        tbShopTitle->setMinimumSize(QSize(190, 0));
        tbShopTitle->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(tbShopTitle);

        lbTitleLength = new QLabel(tab);
        lbTitleLength->setObjectName(QString::fromUtf8("lbTitleLength"));
        lbTitleLength->setStyleSheet(QString::fromUtf8("color: gray;"));

        horizontalLayout->addWidget(lbTitleLength);

        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        btnBuyCatshop = new QToolButton(tab);
        btnBuyCatshop->setObjectName(QString::fromUtf8("btnBuyCatshop"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/config/card_money_32.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnBuyCatshop->setIcon(icon);
        btnBuyCatshop->setIconSize(QSize(20, 20));
        btnBuyCatshop->setPopupMode(QToolButton::InstantPopup);
        btnBuyCatshop->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout->addWidget(btnBuyCatshop);

        lbCatshop = new QLabel(tab);
        lbCatshop->setObjectName(QString::fromUtf8("lbCatshop"));
        lbCatshop->setEnabled(false);
        lbCatshop->setMaximumSize(QSize(24, 24));
        lbCatshop->setPixmap(QPixmap(QString::fromUtf8(":/config/24717.png")));
        lbCatshop->setScaledContents(true);

        horizontalLayout->addWidget(lbCatshop);

        lbCatshopTimer = new QLabel(tab);
        lbCatshopTimer->setObjectName(QString::fromUtf8("lbCatshopTimer"));
        lbCatshopTimer->setMinimumSize(QSize(130, 0));

        horizontalLayout->addWidget(lbCatshopTimer);


        verticalLayout_2->addLayout(horizontalLayout);

        tableView = new QTableView(tab);
        tableView->setObjectName(QString::fromUtf8("tableView"));
        QFont font;
        font.setFamily(QString::fromUtf8("Microsoft Sans Serif"));
        font.setPointSize(8);
        tableView->setFont(font);
        tableView->setMouseTracking(true);
        tableView->setFocusPolicy(Qt::ClickFocus);
        tableView->setContextMenuPolicy(Qt::CustomContextMenu);
        tableView->setStyleSheet(QString::fromUtf8("QTableView QLineEdit {\n"
"	/*border: 1px solid darkgray;*/\n"
"	border: none;\n"
"	qproperty-alignment: AlignRight;\n"
"}"));
        tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
        tableView->setDragEnabled(true);
        tableView->setDragDropOverwriteMode(false);
        tableView->setDragDropMode(QAbstractItemView::DragDrop);
        tableView->setDefaultDropAction(Qt::MoveAction);
        tableView->setAlternatingRowColors(true);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->setIconSize(QSize(20, 20));
        tableView->setTextElideMode(Qt::ElideNone);
        tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableView->setShowGrid(false);
        tableView->setSortingEnabled(true);
        tableView->setWordWrap(false);
        tableView->horizontalHeader()->setMinimumSectionSize(16);
        tableView->horizontalHeader()->setHighlightSections(false);
        tableView->verticalHeader()->setVisible(false);
        tableView->verticalHeader()->setMinimumSectionSize(21);
        tableView->verticalHeader()->setDefaultSectionSize(21);
        tableView->verticalHeader()->setHighlightSections(false);

        verticalLayout_2->addWidget(tableView);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(10);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        lbMoney = new QLabel(tab);
        lbMoney->setObjectName(QString::fromUtf8("lbMoney"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lbMoney->sizePolicy().hasHeightForWidth());
        lbMoney->setSizePolicy(sizePolicy1);
        lbMoney->setAlignment(Qt::AlignCenter);

        horizontalLayout_4->addWidget(lbMoney);

        label_13 = new QLabel(tab);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        horizontalLayout_4->addWidget(label_13);

        lbToSell = new QLineEdit(tab);
        lbToSell->setObjectName(QString::fromUtf8("lbToSell"));
        sizePolicy.setHeightForWidth(lbToSell->sizePolicy().hasHeightForWidth());
        lbToSell->setSizePolicy(sizePolicy);
        lbToSell->setReadOnly(true);

        horizontalLayout_4->addWidget(lbToSell);

        label_14 = new QLabel(tab);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        horizontalLayout_4->addWidget(label_14);

        lbToBuy = new QLineEdit(tab);
        lbToBuy->setObjectName(QString::fromUtf8("lbToBuy"));
        sizePolicy.setHeightForWidth(lbToBuy->sizePolicy().hasHeightForWidth());
        lbToBuy->setSizePolicy(sizePolicy);
        lbToBuy->setReadOnly(true);

        horizontalLayout_4->addWidget(lbToBuy);

        horizontalLayout_4->setStretch(0, 1);

        verticalLayout_2->addLayout(horizontalLayout_4);

        wProfit = new QWidget(tab);
        wProfit->setObjectName(QString::fromUtf8("wProfit"));
        horizontalLayout_8 = new QHBoxLayout(wProfit);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(1, 1, 1, 1);
        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_7);

        label_6 = new QLabel(wProfit);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_8->addWidget(label_6);

        lbProfit = new QLineEdit(wProfit);
        lbProfit->setObjectName(QString::fromUtf8("lbProfit"));
        sizePolicy.setHeightForWidth(lbProfit->sizePolicy().hasHeightForWidth());
        lbProfit->setSizePolicy(sizePolicy);
        lbProfit->setReadOnly(true);

        horizontalLayout_8->addWidget(lbProfit);

        btnProfitDetails = new QToolButton(wProfit);
        btnProfitDetails->setObjectName(QString::fromUtf8("btnProfitDetails"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/config/show_comment.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnProfitDetails->setIcon(icon1);
        btnProfitDetails->setCheckable(true);
        btnProfitDetails->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_8->addWidget(btnProfitDetails);


        verticalLayout_2->addWidget(wProfit);

        wConvert = new QWidget(tab);
        wConvert->setObjectName(QString::fromUtf8("wConvert"));
        wConvert->setEnabled(false);
        horizontalLayout_7 = new QHBoxLayout(wConvert);
        horizontalLayout_7->setSpacing(0);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalLayout_7->setContentsMargins(5, 1, 0, 1);
        cbLowConverter = new QCheckBox(wConvert);
        cbLowConverter->setObjectName(QString::fromUtf8("cbLowConverter"));

        horizontalLayout_7->addWidget(cbLowConverter);

        sbLowConverter = new QSpinBox(wConvert);
        sbLowConverter->setObjectName(QString::fromUtf8("sbLowConverter"));
        sbLowConverter->setEnabled(false);
        sbLowConverter->setMinimumSize(QSize(70, 0));
        sbLowConverter->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        sbLowConverter->setMinimum(1);
        sbLowConverter->setMaximum(99);

        horizontalLayout_7->addWidget(sbLowConverter);

        horizontalSpacer_5 = new QSpacerItem(30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_5);

        cbHighConverter = new QCheckBox(wConvert);
        cbHighConverter->setObjectName(QString::fromUtf8("cbHighConverter"));

        horizontalLayout_7->addWidget(cbHighConverter);

        sbHighConverter = new QSpinBox(wConvert);
        sbHighConverter->setObjectName(QString::fromUtf8("sbHighConverter"));
        sbHighConverter->setEnabled(false);
        sbHighConverter->setMinimumSize(QSize(70, 0));
        sbHighConverter->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        sbHighConverter->setMinimum(1);
        sbHighConverter->setMaximum(100);

        horizontalLayout_7->addWidget(sbHighConverter);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_6);


        verticalLayout_2->addWidget(wConvert);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        gridLayout = new QGridLayout(tab_2);
        gridLayout->setSpacing(5);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(5, 5, 5, 5);
        groupBox = new QGroupBox(tab_2);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy2);
        gridLayout_3 = new QGridLayout(groupBox);
        gridLayout_3->setSpacing(5);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setContentsMargins(8, 5, 10, 8);
        cbEnableAutoreopenValue = new QCheckBox(groupBox);
        cbEnableAutoreopenValue->setObjectName(QString::fromUtf8("cbEnableAutoreopenValue"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(cbEnableAutoreopenValue->sizePolicy().hasHeightForWidth());
        cbEnableAutoreopenValue->setSizePolicy(sizePolicy3);

        gridLayout_3->addWidget(cbEnableAutoreopenValue, 3, 1, 1, 2);

        udAutoreopenModeSelector = new QSpinBox(groupBox);
        udAutoreopenModeSelector->setObjectName(QString::fromUtf8("udAutoreopenModeSelector"));
        sizePolicy2.setHeightForWidth(udAutoreopenModeSelector->sizePolicy().hasHeightForWidth());
        udAutoreopenModeSelector->setSizePolicy(sizePolicy2);
        udAutoreopenModeSelector->setMinimumSize(QSize(70, 0));
        udAutoreopenModeSelector->setMaximum(50000);

        gridLayout_3->addWidget(udAutoreopenModeSelector, 1, 3, 1, 1);

        udAutoreopenValue = new QSpinBox(groupBox);
        udAutoreopenValue->setObjectName(QString::fromUtf8("udAutoreopenValue"));
        sizePolicy2.setHeightForWidth(udAutoreopenValue->sizePolicy().hasHeightForWidth());
        udAutoreopenValue->setSizePolicy(sizePolicy2);
        udAutoreopenValue->setMaximum(50000);

        gridLayout_3->addWidget(udAutoreopenValue, 3, 3, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(20, 1, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_3, 1, 0, 1, 1);

        udAutoreopenPercentage = new QSpinBox(groupBox);
        udAutoreopenPercentage->setObjectName(QString::fromUtf8("udAutoreopenPercentage"));
        sizePolicy2.setHeightForWidth(udAutoreopenPercentage->sizePolicy().hasHeightForWidth());
        udAutoreopenPercentage->setSizePolicy(sizePolicy2);
        udAutoreopenPercentage->setMaximum(100);

        gridLayout_3->addWidget(udAutoreopenPercentage, 2, 3, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        sizePolicy1.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy1);
        label_5->setWordWrap(true);

        gridLayout_3->addWidget(label_5, 1, 1, 1, 2);

        cbEnableAutoreopenPercentage = new QCheckBox(groupBox);
        cbEnableAutoreopenPercentage->setObjectName(QString::fromUtf8("cbEnableAutoreopenPercentage"));
        sizePolicy3.setHeightForWidth(cbEnableAutoreopenPercentage->sizePolicy().hasHeightForWidth());
        cbEnableAutoreopenPercentage->setSizePolicy(sizePolicy3);

        gridLayout_3->addWidget(cbEnableAutoreopenPercentage, 2, 1, 1, 2);

        cbEnableAutoReopen = new QCheckBox(groupBox);
        cbEnableAutoReopen->setObjectName(QString::fromUtf8("cbEnableAutoReopen"));

        gridLayout_3->addWidget(cbEnableAutoReopen, 0, 0, 1, 4);

        udAutoReopenExternalInterval = new QSpinBox(groupBox);
        udAutoReopenExternalInterval->setObjectName(QString::fromUtf8("udAutoReopenExternalInterval"));
        udAutoReopenExternalInterval->setMaximum(86400);

        gridLayout_3->addWidget(udAutoReopenExternalInterval, 6, 3, 1, 1);

        cbEnableAutoReopenExternal = new QCheckBox(groupBox);
        cbEnableAutoReopenExternal->setObjectName(QString::fromUtf8("cbEnableAutoReopenExternal"));

        gridLayout_3->addWidget(cbEnableAutoReopenExternal, 6, 0, 1, 2);

        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setHorizontalSpacing(5);
        gridLayout_4->setVerticalSpacing(3);
        gridLayout_4->setContentsMargins(0, 0, 0, 0);
        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        sizePolicy1.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy1);

        gridLayout_4->addWidget(label_7, 0, 0, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout_4->addWidget(label_8, 0, 2, 1, 1);

        udAutoreopenDelayClose = new QSpinBox(groupBox);
        udAutoreopenDelayClose->setObjectName(QString::fromUtf8("udAutoreopenDelayClose"));
        udAutoreopenDelayClose->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        udAutoreopenDelayClose->setMaximum(2592000);

        gridLayout_4->addWidget(udAutoreopenDelayClose, 0, 1, 1, 1);

        udAutoreopenDelayCloseRnd = new QSpinBox(groupBox);
        udAutoreopenDelayCloseRnd->setObjectName(QString::fromUtf8("udAutoreopenDelayCloseRnd"));
        udAutoreopenDelayCloseRnd->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        udAutoreopenDelayCloseRnd->setMaximum(2592000);

        gridLayout_4->addWidget(udAutoreopenDelayCloseRnd, 0, 3, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        sizePolicy1.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy1);

        gridLayout_4->addWidget(label_9, 1, 0, 1, 1);

        udAutoreopenDelayOpen = new QSpinBox(groupBox);
        udAutoreopenDelayOpen->setObjectName(QString::fromUtf8("udAutoreopenDelayOpen"));
        udAutoreopenDelayOpen->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        udAutoreopenDelayOpen->setMaximum(2592000);

        gridLayout_4->addWidget(udAutoreopenDelayOpen, 1, 1, 1, 1);

        label_10 = new QLabel(groupBox);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout_4->addWidget(label_10, 1, 2, 1, 1);

        udAutoreopenDelayOpenRnd = new QSpinBox(groupBox);
        udAutoreopenDelayOpenRnd->setObjectName(QString::fromUtf8("udAutoreopenDelayOpenRnd"));
        udAutoreopenDelayOpenRnd->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        udAutoreopenDelayOpenRnd->setMaximum(2592000);

        gridLayout_4->addWidget(udAutoreopenDelayOpenRnd, 1, 3, 1, 1);


        gridLayout_3->addLayout(gridLayout_4, 9, 0, 1, 4);

        cbSavePartialConfig = new QCheckBox(groupBox);
        cbSavePartialConfig->setObjectName(QString::fromUtf8("cbSavePartialConfig"));

        gridLayout_3->addWidget(cbSavePartialConfig, 5, 0, 1, 2);

        frame = new QFrame(groupBox);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::HLine);
        frame->setFrameShadow(QFrame::Raised);

        gridLayout_3->addWidget(frame, 7, 0, 1, 4);

        gridLayout_3->setColumnStretch(1, 1);

        gridLayout->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_5 = new QGroupBox(tab_2);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        verticalLayout_8 = new QVBoxLayout(groupBox_5);
        verticalLayout_8->setSpacing(5);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        verticalLayout_8->setContentsMargins(8, 5, 8, 8);
        cbOnlineRegistration = new QCheckBox(groupBox_5);
        cbOnlineRegistration->setObjectName(QString::fromUtf8("cbOnlineRegistration"));

        verticalLayout_8->addWidget(cbOnlineRegistration);


        gridLayout->addWidget(groupBox_5, 1, 0, 1, 1);

        verticalSpacer_2 = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer_2, 2, 0, 1, 1);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        verticalLayout_5 = new QVBoxLayout(tab_3);
        verticalLayout_5->setSpacing(3);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(5, 5, 5, 5);
        groupBox_4 = new QGroupBox(tab_3);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        verticalLayout_4 = new QVBoxLayout(groupBox_4);
        verticalLayout_4->setSpacing(5);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(8, 5, 10, 8);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(5);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(groupBox_4);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(label_2);

        udConnectDelayFixed = new QSpinBox(groupBox_4);
        udConnectDelayFixed->setObjectName(QString::fromUtf8("udConnectDelayFixed"));
        udConnectDelayFixed->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        udConnectDelayFixed->setMinimum(0);
        udConnectDelayFixed->setMaximum(2592000);

        horizontalLayout_2->addWidget(udConnectDelayFixed);

        label_11 = new QLabel(groupBox_4);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_2->addWidget(label_11);

        udConnectDelayRandom = new QSpinBox(groupBox_4);
        udConnectDelayRandom->setObjectName(QString::fromUtf8("udConnectDelayRandom"));
        udConnectDelayRandom->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        udConnectDelayRandom->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        udConnectDelayRandom->setMinimum(0);
        udConnectDelayRandom->setMaximum(2592000);

        horizontalLayout_2->addWidget(udConnectDelayRandom);


        verticalLayout_4->addLayout(horizontalLayout_2);

        cbDelayConnectStart = new QCheckBox(groupBox_4);
        cbDelayConnectStart->setObjectName(QString::fromUtf8("cbDelayConnectStart"));

        verticalLayout_4->addWidget(cbDelayConnectStart);


        verticalLayout_5->addWidget(groupBox_4);

        groupBox_3 = new QGroupBox(tab_3);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_7 = new QVBoxLayout(groupBox_3);
        verticalLayout_7->setSpacing(5);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(8, 5, 10, 8);
        cbEnableProxy = new QCheckBox(groupBox_3);
        cbEnableProxy->setObjectName(QString::fromUtf8("cbEnableProxy"));

        verticalLayout_7->addWidget(cbEnableProxy);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setHorizontalSpacing(10);
        gridLayout_2->setVerticalSpacing(5);
        gridLayout_2->setContentsMargins(15, -1, -1, -1);
        label_3 = new QLabel(groupBox_3);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy4(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy4);

        gridLayout_2->addWidget(label_3, 0, 0, 1, 1);

        label_15 = new QLabel(groupBox_3);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout_2->addWidget(label_15, 3, 0, 1, 1);

        leProxyHost = new QLineEdit(groupBox_3);
        leProxyHost->setObjectName(QString::fromUtf8("leProxyHost"));
        sizePolicy2.setHeightForWidth(leProxyHost->sizePolicy().hasHeightForWidth());
        leProxyHost->setSizePolicy(sizePolicy2);

        gridLayout_2->addWidget(leProxyHost, 0, 1, 1, 2);

        label_17 = new QLabel(groupBox_3);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        gridLayout_2->addWidget(label_17, 1, 0, 2, 1);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setSpacing(2);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        rbProxyTypeSocks = new QRadioButton(groupBox_3);
        rbProxyTypeSocks->setObjectName(QString::fromUtf8("rbProxyTypeSocks"));

        verticalLayout_6->addWidget(rbProxyTypeSocks);

        rbProxyTypeHttp = new QRadioButton(groupBox_3);
        rbProxyTypeHttp->setObjectName(QString::fromUtf8("rbProxyTypeHttp"));

        verticalLayout_6->addWidget(rbProxyTypeHttp);


        gridLayout_2->addLayout(verticalLayout_6, 3, 1, 1, 2);

        label_16 = new QLabel(groupBox_3);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout_2->addWidget(label_16, 0, 3, 4, 1);

        leProxyUser = new QLineEdit(groupBox_3);
        leProxyUser->setObjectName(QString::fromUtf8("leProxyUser"));

        gridLayout_2->addWidget(leProxyUser, 2, 1, 1, 1);

        label_19 = new QLabel(groupBox_3);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        gridLayout_2->addWidget(label_19, 1, 2, 1, 1);

        leProxyPassword = new QLineEdit(groupBox_3);
        leProxyPassword->setObjectName(QString::fromUtf8("leProxyPassword"));

        gridLayout_2->addWidget(leProxyPassword, 2, 2, 1, 1);

        label_18 = new QLabel(groupBox_3);
        label_18->setObjectName(QString::fromUtf8("label_18"));

        gridLayout_2->addWidget(label_18, 1, 1, 1, 1);


        verticalLayout_7->addLayout(gridLayout_2);


        verticalLayout_5->addWidget(groupBox_3);

        verticalSpacer = new QSpacerItem(20, 215, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer);

        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        verticalLayout_9 = new QVBoxLayout(tab_4);
        verticalLayout_9->setSpacing(5);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        verticalLayout_9->setContentsMargins(5, 5, 5, 5);
        groupBox_6 = new QGroupBox(tab_4);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        horizontalLayout_9 = new QHBoxLayout(groupBox_6);
        horizontalLayout_9->setSpacing(10);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        horizontalLayout_9->setContentsMargins(8, 5, 10, 8);
        label_12 = new QLabel(groupBox_6);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        horizontalLayout_9->addWidget(label_12);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_4);

        slChatBufferSize = new QSlider(groupBox_6);
        slChatBufferSize->setObjectName(QString::fromUtf8("slChatBufferSize"));
        slChatBufferSize->setMaximum(4);
        slChatBufferSize->setPageStep(1);
        slChatBufferSize->setSliderPosition(1);
        slChatBufferSize->setOrientation(Qt::Horizontal);
        slChatBufferSize->setTickPosition(QSlider::TicksBothSides);

        horizontalLayout_9->addWidget(slChatBufferSize);

        lbChatBufferSize = new QLabel(groupBox_6);
        lbChatBufferSize->setObjectName(QString::fromUtf8("lbChatBufferSize"));
        sizePolicy4.setHeightForWidth(lbChatBufferSize->sizePolicy().hasHeightForWidth());
        lbChatBufferSize->setSizePolicy(sizePolicy4);
        lbChatBufferSize->setMinimumSize(QSize(60, 0));

        horizontalLayout_9->addWidget(lbChatBufferSize);


        verticalLayout_9->addWidget(groupBox_6);

        groupBox_2 = new QGroupBox(tab_4);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_3 = new QVBoxLayout(groupBox_2);
        verticalLayout_3->setSpacing(5);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(8, 5, 10, 8);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(5);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        cbLimitHistory = new QCheckBox(groupBox_2);
        cbLimitHistory->setObjectName(QString::fromUtf8("cbLimitHistory"));
        sizePolicy3.setHeightForWidth(cbLimitHistory->sizePolicy().hasHeightForWidth());
        cbLimitHistory->setSizePolicy(sizePolicy3);

        horizontalLayout_3->addWidget(cbLimitHistory);

        udLimitHistory = new QSpinBox(groupBox_2);
        udLimitHistory->setObjectName(QString::fromUtf8("udLimitHistory"));
        udLimitHistory->setMinimumSize(QSize(70, 0));
        udLimitHistory->setMinimum(10);
        udLimitHistory->setMaximum(100000);

        horizontalLayout_3->addWidget(udLimitHistory);


        verticalLayout_3->addLayout(horizontalLayout_3);

        cbSaveLog = new QCheckBox(groupBox_2);
        cbSaveLog->setObjectName(QString::fromUtf8("cbSaveLog"));

        verticalLayout_3->addWidget(cbSaveLog);

        cbSplitLog = new QCheckBox(groupBox_2);
        cbSplitLog->setObjectName(QString::fromUtf8("cbSplitLog"));
        cbSplitLog->setStyleSheet(QString::fromUtf8("margin-left: 20px;"));

        verticalLayout_3->addWidget(cbSplitLog);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_5->addWidget(label_4);

        tbLogPath = new QLineEdit(groupBox_2);
        tbLogPath->setObjectName(QString::fromUtf8("tbLogPath"));
        tbLogPath->setReadOnly(true);

        horizontalLayout_5->addWidget(tbLogPath);

        btnOpenLogPath = new QToolButton(groupBox_2);
        btnOpenLogPath->setObjectName(QString::fromUtf8("btnOpenLogPath"));
        btnOpenLogPath->setMinimumSize(QSize(26, 0));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/config/folder_blue.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnOpenLogPath->setIcon(icon2);

        horizontalLayout_5->addWidget(btnOpenLogPath);


        verticalLayout_3->addLayout(horizontalLayout_5);


        verticalLayout_9->addWidget(groupBox_2);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_3);

        tabWidget->addTab(tab_4, QString());

        verticalLayout->addWidget(tabWidget);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_2);

        btnGlobalSettings = new QPushButton(Config);
        btnGlobalSettings->setObjectName(QString::fromUtf8("btnGlobalSettings"));
        sizePolicy2.setHeightForWidth(btnGlobalSettings->sizePolicy().hasHeightForWidth());
        btnGlobalSettings->setSizePolicy(sizePolicy2);
        btnGlobalSettings->setMinimumSize(QSize(115, 0));
        btnGlobalSettings->setAutoDefault(false);

        horizontalLayout_6->addWidget(btnGlobalSettings);

        buttonBox = new QDialogButtonBox(Config);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        horizontalLayout_6->addWidget(buttonBox);


        verticalLayout->addLayout(horizontalLayout_6);


        retranslateUi(Config);
        QObject::connect(cbEnableAutoReopen, SIGNAL(toggled(bool)), udAutoreopenModeSelector, SLOT(setEnabled(bool)));
        QObject::connect(cbEnableAutoReopen, SIGNAL(toggled(bool)), cbEnableAutoreopenPercentage, SLOT(setEnabled(bool)));
        QObject::connect(cbEnableAutoReopen, SIGNAL(toggled(bool)), cbEnableAutoreopenValue, SLOT(setEnabled(bool)));
        QObject::connect(buttonBox, SIGNAL(accepted()), Config, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Config, SLOT(reject()));
        QObject::connect(cbEnableProxy, SIGNAL(toggled(bool)), leProxyHost, SLOT(setEnabled(bool)));
        QObject::connect(cbEnableProxy, SIGNAL(toggled(bool)), rbProxyTypeSocks, SLOT(setEnabled(bool)));
        QObject::connect(cbEnableProxy, SIGNAL(toggled(bool)), rbProxyTypeHttp, SLOT(setEnabled(bool)));
        QObject::connect(cbEnableProxy, SIGNAL(toggled(bool)), leProxyUser, SLOT(setEnabled(bool)));
        QObject::connect(cbEnableProxy, SIGNAL(toggled(bool)), leProxyPassword, SLOT(setEnabled(bool)));
        QObject::connect(cbLowConverter, SIGNAL(toggled(bool)), sbLowConverter, SLOT(setEnabled(bool)));
        QObject::connect(cbHighConverter, SIGNAL(toggled(bool)), sbHighConverter, SLOT(setEnabled(bool)));
        QObject::connect(cbEnableAutoReopenExternal, SIGNAL(toggled(bool)), udAutoReopenExternalInterval, SLOT(setEnabled(bool)));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Config);
    } // setupUi

    void retranslateUi(QDialog *Config)
    {
        Config->setWindowTitle(QCoreApplication::translate("Config", "Settings", nullptr));
        label->setText(QCoreApplication::translate("Config", "Market title", nullptr));
        btnBuyCatshop->setText(QCoreApplication::translate("Config", "Buy catshop", nullptr));
#if QT_CONFIG(tooltip)
        lbCatshop->setToolTip(QCoreApplication::translate("Config", "Indicates presence of catshop item", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        lbCatshopTimer->setToolTip(QCoreApplication::translate("Config", "Displays time of catshop item will be active", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        lbMoney->setToolTip(QCoreApplication::translate("Config", "Banknotes and money", nullptr));
#endif // QT_CONFIG(tooltip)
        label_13->setText(QCoreApplication::translate("Config", "Selling:", nullptr));
        label_14->setText(QCoreApplication::translate("Config", "Buying:", nullptr));
        label_6->setText(QCoreApplication::translate("Config", "Profit:", nullptr));
        btnProfitDetails->setText(QCoreApplication::translate("Config", "Details", nullptr));
#if QT_CONFIG(tooltip)
        cbLowConverter->setToolTip(QCoreApplication::translate("Config", "Perform banknote to money conversion\n"
"when money is less than number (in millions).", nullptr));
#endif // QT_CONFIG(tooltip)
        cbLowConverter->setText(QCoreApplication::translate("Config", "Convert to money if less than", nullptr));
#if QT_CONFIG(tooltip)
        sbLowConverter->setToolTip(QCoreApplication::translate("Config", "Money, in millions", nullptr));
#endif // QT_CONFIG(tooltip)
        sbLowConverter->setSuffix(QCoreApplication::translate("Config", "0 M", nullptr));
#if QT_CONFIG(tooltip)
        cbHighConverter->setToolTip(QCoreApplication::translate("Config", "Perform money to banknote conversion\n"
"when money is greater than number (in millions).", nullptr));
#endif // QT_CONFIG(tooltip)
        cbHighConverter->setText(QCoreApplication::translate("Config", "Convert to notes if greater than", nullptr));
#if QT_CONFIG(tooltip)
        sbHighConverter->setToolTip(QCoreApplication::translate("Config", "Money, in millions", nullptr));
#endif // QT_CONFIG(tooltip)
        sbHighConverter->setSuffix(QCoreApplication::translate("Config", "0 M", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("Config", "Inventory and Market", nullptr));
        groupBox->setTitle(QCoreApplication::translate("Config", "Market reopen", nullptr));
#if QT_CONFIG(tooltip)
        cbEnableAutoreopenValue->setToolTip(QCoreApplication::translate("Config", "Enabled or disables trigger based on remaining item amount.", nullptr));
#endif // QT_CONFIG(tooltip)
        cbEnableAutoreopenValue->setText(QCoreApplication::translate("Config", "Use count mode: when current market item count less than", nullptr));
#if QT_CONFIG(tooltip)
        udAutoreopenModeSelector->setToolTip(QCoreApplication::translate("Config", "Sets item amount which determines what kind of trigger to use for reopen decision.\n"
"For example, if this is set to 10 and planned amount is greater than 10 then percent trigger will be used,\n"
"otherwise the decision will be made based on remaining items amount", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        udAutoreopenValue->setToolTip(QCoreApplication::translate("Config", "If amount of items remaining in market slot\n"
"will be less than this value market will be reopen\n"
"", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        udAutoreopenPercentage->setToolTip(QCoreApplication::translate("Config", "If change in item amount (market reopen effect)\n"
"will be greater than this percent market will be reopen", nullptr));
#endif // QT_CONFIG(tooltip)
        udAutoreopenPercentage->setSuffix(QCoreApplication::translate("Config", " %", nullptr));
        label_5->setText(QCoreApplication::translate("Config", "Mode level: if number of items greater than this, percent mode will be used, otherwise use number mode", nullptr));
#if QT_CONFIG(tooltip)
        cbEnableAutoreopenPercentage->setToolTip(QCoreApplication::translate("Config", "Enables or disables trigger based on item change percentage.", nullptr));
#endif // QT_CONFIG(tooltip)
        cbEnableAutoreopenPercentage->setText(QCoreApplication::translate("Config", "Use percent mode: when current market items differ from planned count in percents", nullptr));
#if QT_CONFIG(tooltip)
        cbEnableAutoReopen->setToolTip(QCoreApplication::translate("Config", "Enables or disables automatic market reopening based on what of following rules are apllying.\n"
"Market will be closed, reevaluated and opened again", nullptr));
#endif // QT_CONFIG(tooltip)
        cbEnableAutoReopen->setText(QCoreApplication::translate("Config", "Enable automatic market reopening on slot, price, or items items change", nullptr));
        udAutoReopenExternalInterval->setSuffix(QCoreApplication::translate("Config", " s", nullptr));
#if QT_CONFIG(tooltip)
        cbEnableAutoReopenExternal->setToolTip(QCoreApplication::translate("Config", "Enables or disables automatic market reopening based on external configuration source (additional parital configuration file).", nullptr));
#endif // QT_CONFIG(tooltip)
        cbEnableAutoReopenExternal->setText(QCoreApplication::translate("Config", "Reopen market by polling external market configuration source every", nullptr));
        label_7->setText(QCoreApplication::translate("Config", "Wait before market closed", nullptr));
        label_8->setText(QCoreApplication::translate("Config", "+ random of", nullptr));
        udAutoreopenDelayClose->setSuffix(QCoreApplication::translate("Config", " s", nullptr));
        udAutoreopenDelayCloseRnd->setSuffix(QCoreApplication::translate("Config", " s", nullptr));
        udAutoreopenDelayCloseRnd->setPrefix(QCoreApplication::translate("Config", "0 - ", nullptr));
        label_9->setText(QCoreApplication::translate("Config", "Wait before market opened", nullptr));
        udAutoreopenDelayOpen->setSuffix(QCoreApplication::translate("Config", " s", nullptr));
        label_10->setText(QCoreApplication::translate("Config", "+ random of", nullptr));
        udAutoreopenDelayOpenRnd->setSuffix(QCoreApplication::translate("Config", " s", nullptr));
        udAutoreopenDelayOpenRnd->setPrefix(QCoreApplication::translate("Config", "0 - ", nullptr));
#if QT_CONFIG(tooltip)
        cbSavePartialConfig->setToolTip(QCoreApplication::translate("Config", "When enabled, exports market configuration to file which can be used as source to market update and reopen.", nullptr));
#endif // QT_CONFIG(tooltip)
        cbSavePartialConfig->setText(QCoreApplication::translate("Config", "Export ready-for-update partial market configuration", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("Config", "In game", nullptr));
#if QT_CONFIG(tooltip)
        cbOnlineRegistration->setToolTip(QCoreApplication::translate("Config", "Performs 'online registration' on login and once per next day(s)", nullptr));
#endif // QT_CONFIG(tooltip)
        cbOnlineRegistration->setText(QCoreApplication::translate("Config", "Automatic online registration", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("Config", "Settings", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("Config", "Reconnect on disconnection", nullptr));
        label_2->setText(QCoreApplication::translate("Config", "Wait before new connection if connection lost", nullptr));
        udConnectDelayFixed->setSuffix(QCoreApplication::translate("Config", " s", nullptr));
        label_11->setText(QCoreApplication::translate("Config", "+ random of", nullptr));
        udConnectDelayRandom->setSuffix(QCoreApplication::translate("Config", " s", nullptr));
        udConnectDelayRandom->setPrefix(QCoreApplication::translate("Config", "0 - ", nullptr));
#if QT_CONFIG(tooltip)
        cbDelayConnectStart->setToolTip(QCoreApplication::translate("Config", "If enabled, application will wait this time before\n"
"first connection attempt after started", nullptr));
#endif // QT_CONFIG(tooltip)
        cbDelayConnectStart->setText(QCoreApplication::translate("Config", "Apply connection delay also after application start", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("Config", "Proxy", nullptr));
#if QT_CONFIG(tooltip)
        cbEnableProxy->setToolTip(QCoreApplication::translate("Config", "Use proxy server to connect to game server", nullptr));
#endif // QT_CONFIG(tooltip)
        cbEnableProxy->setText(QCoreApplication::translate("Config", "Use proxy server to connect", nullptr));
        label_3->setText(QCoreApplication::translate("Config", "Host (IP)", nullptr));
        label_15->setText(QCoreApplication::translate("Config", "Type", nullptr));
        label_17->setText(QCoreApplication::translate("Config", "Proxy\n"
"authorization", nullptr));
        rbProxyTypeSocks->setText(QCoreApplication::translate("Config", "SOCKS 5", nullptr));
        rbProxyTypeHttp->setText(QCoreApplication::translate("Config", "HTTPS", nullptr));
        label_16->setText(QCoreApplication::translate("Config", "&nbsp;Use format \"host\" or \"host:port\" <br/>where host is hostname or IP address.<br/>&nbsp;By default port is 1080 for SOCKS<br/>and 80 for HTTP proxies.<br/>&nbsp;Only HTTPS proxies supporting CONNECT<br/>method to non-SSL ports can be used.</p>\n"
"<p>Changes will be applied on next connection.</p>", nullptr));
        label_19->setText(QCoreApplication::translate("Config", "Password", nullptr));
        label_18->setText(QCoreApplication::translate("Config", "Username", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("Config", "Connection", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("Config", "Chat", nullptr));
        label_12->setText(QCoreApplication::translate("Config", "Buffer size", nullptr));
        lbChatBufferSize->setText(QCoreApplication::translate("Config", "TextLabel", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("Config", "Event logging", nullptr));
#if QT_CONFIG(tooltip)
        cbLimitHistory->setToolTip(QCoreApplication::translate("Config", "Limit lines in cat history screen", nullptr));
#endif // QT_CONFIG(tooltip)
        cbLimitHistory->setText(QCoreApplication::translate("Config", "Limit history lines to", nullptr));
        cbSaveLog->setText(QCoreApplication::translate("Config", "Save history to file", nullptr));
        cbSplitLog->setText(QCoreApplication::translate("Config", "Create new log file every day", nullptr));
        label_4->setText(QCoreApplication::translate("Config", "Path to log files", nullptr));
#if QT_CONFIG(tooltip)
        btnOpenLogPath->setToolTip(QCoreApplication::translate("Config", "Select folder for log files", nullptr));
#endif // QT_CONFIG(tooltip)
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QCoreApplication::translate("Config", "Interface", nullptr));
        btnGlobalSettings->setText(QCoreApplication::translate("Config", "Application settings", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Config: public Ui_Config {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIG_H
