/********************************************************************************
** Form generated from reading UI file 'trade.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRADE_H
#define UI_TRADE_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Trade
{
public:
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLabel *lbPlayerName;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QLabel *lbInventoryMoney;
    QToolButton *btnSetMoney;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_5;
    QLabel *lbMyMoney;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_6;
    QLabel *lbPartnerMoney;
    QTableWidget *twPartnerItems;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer_2;
    QToolButton *btnAddOne;
    QToolButton *btnAddAll;
    QToolButton *btnAddAllExOne;
    QSpacerItem *verticalSpacer_3;
    QToolButton *btnRemoveOne;
    QToolButton *btnRemoveAll;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnConfirm;
    QPushButton *btnAccept;
    QCheckBox *checkBox;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_3;
    QLabel *label_2;
    QTableWidget *twInventory;
    QTableWidget *twMyItems;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *btnSetMax;
    QLineEdit *eMoney;
    QHBoxLayout *horizontalLayout_6;
    QLabel *lbConfirm;
    QLabel *lbAccept;
    QLabel *lbError;

    void setupUi(QDialog *Trade)
    {
        if (Trade->objectName().isEmpty())
            Trade->setObjectName(QString::fromUtf8("Trade"));
        Trade->resize(700, 420);
        Trade->setStyleSheet(QString::fromUtf8("QTableView\n"
"{\n"
"	outline: 0;\n"
"}"));
        Trade->setSizeGripEnabled(true);
        verticalLayout_2 = new QVBoxLayout(Trade);
        verticalLayout_2->setSpacing(5);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(4, 4, 4, 5);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setHorizontalSpacing(4);
        gridLayout->setVerticalSpacing(5);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        lbPlayerName = new QLabel(Trade);
        lbPlayerName->setObjectName(QString::fromUtf8("lbPlayerName"));
        QFont font;
        font.setBold(true);
        font.setUnderline(true);
        font.setWeight(75);
        lbPlayerName->setFont(font);
        lbPlayerName->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(lbPlayerName, 0, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(5);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(Trade);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setMinimumSize(QSize(23, 0));
        label->setPixmap(QPixmap(QString::fromUtf8(":/catView/coins.png")));
        label->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(label);

        lbInventoryMoney = new QLabel(Trade);
        lbInventoryMoney->setObjectName(QString::fromUtf8("lbInventoryMoney"));
        lbInventoryMoney->setMinimumSize(QSize(0, 20));
        lbInventoryMoney->setFrameShape(QFrame::Panel);
        lbInventoryMoney->setFrameShadow(QFrame::Sunken);
        lbInventoryMoney->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(lbInventoryMoney);


        gridLayout->addLayout(horizontalLayout_3, 2, 4, 1, 1);

        btnSetMoney = new QToolButton(Trade);
        btnSetMoney->setObjectName(QString::fromUtf8("btnSetMoney"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/trade/prevprev.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSetMoney->setIcon(icon);

        gridLayout->addWidget(btnSetMoney, 3, 3, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(5);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_5 = new QLabel(Trade);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy);
        label_5->setMinimumSize(QSize(23, 0));
        label_5->setPixmap(QPixmap(QString::fromUtf8(":/catView/coins.png")));
        label_5->setAlignment(Qt::AlignCenter);

        horizontalLayout_4->addWidget(label_5);

        lbMyMoney = new QLabel(Trade);
        lbMyMoney->setObjectName(QString::fromUtf8("lbMyMoney"));
        lbMyMoney->setFrameShape(QFrame::Panel);
        lbMyMoney->setFrameShadow(QFrame::Sunken);
        lbMyMoney->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(lbMyMoney);


        gridLayout->addLayout(horizontalLayout_4, 3, 2, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(5);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_6 = new QLabel(Trade);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);
        label_6->setMinimumSize(QSize(23, 0));
        label_6->setPixmap(QPixmap(QString::fromUtf8(":/catView/coins.png")));
        label_6->setAlignment(Qt::AlignCenter);

        horizontalLayout_5->addWidget(label_6);

        lbPartnerMoney = new QLabel(Trade);
        lbPartnerMoney->setObjectName(QString::fromUtf8("lbPartnerMoney"));
        lbPartnerMoney->setFrameShape(QFrame::Panel);
        lbPartnerMoney->setFrameShadow(QFrame::Sunken);
        lbPartnerMoney->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(lbPartnerMoney);


        gridLayout->addLayout(horizontalLayout_5, 3, 0, 1, 1);

        twPartnerItems = new QTableWidget(Trade);
        if (twPartnerItems->columnCount() < 2)
            twPartnerItems->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
        twPartnerItems->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        __qtablewidgetitem1->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
        twPartnerItems->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        twPartnerItems->setObjectName(QString::fromUtf8("twPartnerItems"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Microsoft Sans Serif"));
        font1.setPointSize(8);
        twPartnerItems->setFont(font1);
        twPartnerItems->setFrameShape(QFrame::Panel);
        twPartnerItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
        twPartnerItems->setSelectionMode(QAbstractItemView::NoSelection);
        twPartnerItems->setIconSize(QSize(20, 20));
        twPartnerItems->setShowGrid(false);
        twPartnerItems->setWordWrap(false);
        twPartnerItems->horizontalHeader()->setMinimumSectionSize(40);
        twPartnerItems->horizontalHeader()->setDefaultSectionSize(45);
        twPartnerItems->horizontalHeader()->setHighlightSections(false);
        twPartnerItems->horizontalHeader()->setStretchLastSection(true);
        twPartnerItems->verticalHeader()->setVisible(false);
        twPartnerItems->verticalHeader()->setMinimumSectionSize(21);
        twPartnerItems->verticalHeader()->setDefaultSectionSize(21);

        gridLayout->addWidget(twPartnerItems, 1, 0, 2, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(4);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalSpacer_2 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        btnAddOne = new QToolButton(Trade);
        btnAddOne->setObjectName(QString::fromUtf8("btnAddOne"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/trade/previous.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnAddOne->setIcon(icon1);

        verticalLayout->addWidget(btnAddOne, 0, Qt::AlignHCenter);

        btnAddAll = new QToolButton(Trade);
        btnAddAll->setObjectName(QString::fromUtf8("btnAddAll"));
        btnAddAll->setIcon(icon);

        verticalLayout->addWidget(btnAddAll, 0, Qt::AlignHCenter);

        btnAddAllExOne = new QToolButton(Trade);
        btnAddAllExOne->setObjectName(QString::fromUtf8("btnAddAllExOne"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/trade/previous_1.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnAddAllExOne->setIcon(icon2);

        verticalLayout->addWidget(btnAddAllExOne);

        verticalSpacer_3 = new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_3);

        btnRemoveOne = new QToolButton(Trade);
        btnRemoveOne->setObjectName(QString::fromUtf8("btnRemoveOne"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/trade/next.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnRemoveOne->setIcon(icon3);

        verticalLayout->addWidget(btnRemoveOne, 0, Qt::AlignHCenter);

        btnRemoveAll = new QToolButton(Trade);
        btnRemoveAll->setObjectName(QString::fromUtf8("btnRemoveAll"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/trade/nextnext.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnRemoveAll->setIcon(icon4);

        verticalLayout->addWidget(btnRemoveAll, 0, Qt::AlignHCenter);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        gridLayout->addLayout(verticalLayout, 1, 3, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(15);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(10, 0, 0, 0);
        btnConfirm = new QPushButton(Trade);
        btnConfirm->setObjectName(QString::fromUtf8("btnConfirm"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(btnConfirm->sizePolicy().hasHeightForWidth());
        btnConfirm->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(btnConfirm);

        btnAccept = new QPushButton(Trade);
        btnAccept->setObjectName(QString::fromUtf8("btnAccept"));
        btnAccept->setEnabled(false);
        sizePolicy1.setHeightForWidth(btnAccept->sizePolicy().hasHeightForWidth());
        btnAccept->setSizePolicy(sizePolicy1);
        btnAccept->setCheckable(false);
        btnAccept->setChecked(false);

        horizontalLayout->addWidget(btnAccept);

        checkBox = new QCheckBox(Trade);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setEnabled(false);
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(checkBox->sizePolicy().hasHeightForWidth());
        checkBox->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(checkBox);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        gridLayout->addLayout(horizontalLayout, 4, 2, 1, 3);

        label_3 = new QLabel(Trade);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font);
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 0, 4, 1, 1);

        label_2 = new QLabel(Trade);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);
        label_2->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_2, 0, 2, 1, 1);

        twInventory = new QTableWidget(Trade);
        if (twInventory->columnCount() < 2)
            twInventory->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        __qtablewidgetitem2->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
        twInventory->setHorizontalHeaderItem(0, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        __qtablewidgetitem3->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
        twInventory->setHorizontalHeaderItem(1, __qtablewidgetitem3);
        twInventory->setObjectName(QString::fromUtf8("twInventory"));
        twInventory->setFont(font1);
        twInventory->setFrameShape(QFrame::Panel);
        twInventory->setEditTriggers(QAbstractItemView::NoEditTriggers);
        twInventory->setSelectionBehavior(QAbstractItemView::SelectRows);
        twInventory->setIconSize(QSize(20, 20));
        twInventory->setShowGrid(false);
        twInventory->setWordWrap(false);
        twInventory->horizontalHeader()->setMinimumSectionSize(40);
        twInventory->horizontalHeader()->setDefaultSectionSize(45);
        twInventory->horizontalHeader()->setHighlightSections(false);
        twInventory->horizontalHeader()->setStretchLastSection(true);
        twInventory->verticalHeader()->setVisible(false);
        twInventory->verticalHeader()->setMinimumSectionSize(21);
        twInventory->verticalHeader()->setDefaultSectionSize(21);

        gridLayout->addWidget(twInventory, 1, 4, 1, 1);

        twMyItems = new QTableWidget(Trade);
        if (twMyItems->columnCount() < 2)
            twMyItems->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        __qtablewidgetitem4->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
        twMyItems->setHorizontalHeaderItem(0, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        __qtablewidgetitem5->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
        twMyItems->setHorizontalHeaderItem(1, __qtablewidgetitem5);
        twMyItems->setObjectName(QString::fromUtf8("twMyItems"));
        twMyItems->setFont(font1);
        twMyItems->setFrameShape(QFrame::Panel);
        twMyItems->setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::CurrentChanged|QAbstractItemView::EditKeyPressed|QAbstractItemView::SelectedClicked);
        twMyItems->setSelectionBehavior(QAbstractItemView::SelectRows);
        twMyItems->setIconSize(QSize(20, 20));
        twMyItems->setShowGrid(false);
        twMyItems->setWordWrap(false);
        twMyItems->horizontalHeader()->setMinimumSectionSize(40);
        twMyItems->horizontalHeader()->setDefaultSectionSize(45);
        twMyItems->horizontalHeader()->setHighlightSections(false);
        twMyItems->horizontalHeader()->setStretchLastSection(true);
        twMyItems->verticalHeader()->setVisible(false);
        twMyItems->verticalHeader()->setMinimumSectionSize(21);
        twMyItems->verticalHeader()->setDefaultSectionSize(21);

        gridLayout->addWidget(twMyItems, 1, 2, 2, 1);

        label_4 = new QLabel(Trade);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setPixmap(QPixmap(QString::fromUtf8(":/trade/arrow.png")));

        gridLayout->addWidget(label_4, 1, 1, 2, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(5);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        btnSetMax = new QToolButton(Trade);
        btnSetMax->setObjectName(QString::fromUtf8("btnSetMax"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/trade/coins_in_hand.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSetMax->setIcon(icon5);

        horizontalLayout_2->addWidget(btnSetMax);

        eMoney = new QLineEdit(Trade);
        eMoney->setObjectName(QString::fromUtf8("eMoney"));
        eMoney->setMaxLength(16);
        eMoney->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(eMoney);


        gridLayout->addLayout(horizontalLayout_2, 3, 4, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(0);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalLayout_6->setContentsMargins(5, 0, 5, 0);
        lbConfirm = new QLabel(Trade);
        lbConfirm->setObjectName(QString::fromUtf8("lbConfirm"));
        lbConfirm->setMaximumSize(QSize(75, 16777215));
        lbConfirm->setFrameShape(QFrame::Panel);
        lbConfirm->setFrameShadow(QFrame::Raised);
        lbConfirm->setAlignment(Qt::AlignCenter);
        lbConfirm->setMargin(4);

        horizontalLayout_6->addWidget(lbConfirm);

        lbAccept = new QLabel(Trade);
        lbAccept->setObjectName(QString::fromUtf8("lbAccept"));
        lbAccept->setEnabled(false);
        lbAccept->setMaximumSize(QSize(75, 16777215));
        lbAccept->setFrameShape(QFrame::Panel);
        lbAccept->setFrameShadow(QFrame::Raised);
        lbAccept->setAlignment(Qt::AlignCenter);
        lbAccept->setMargin(4);

        horizontalLayout_6->addWidget(lbAccept);


        gridLayout->addLayout(horizontalLayout_6, 4, 0, 1, 1);


        verticalLayout_2->addLayout(gridLayout);

        lbError = new QLabel(Trade);
        lbError->setObjectName(QString::fromUtf8("lbError"));
        lbError->setStyleSheet(QString::fromUtf8("color: red;"));
        lbError->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(lbError);

        QWidget::setTabOrder(btnAddOne, btnAddAll);
        QWidget::setTabOrder(btnAddAll, btnRemoveOne);
        QWidget::setTabOrder(btnRemoveOne, btnRemoveAll);
        QWidget::setTabOrder(btnRemoveAll, twInventory);
        QWidget::setTabOrder(twInventory, twMyItems);
        QWidget::setTabOrder(twMyItems, twPartnerItems);

        retranslateUi(Trade);

        QMetaObject::connectSlotsByName(Trade);
    } // setupUi

    void retranslateUi(QDialog *Trade)
    {
        Trade->setWindowTitle(QCoreApplication::translate("Trade", "Trade", nullptr));
#if QT_CONFIG(tooltip)
        btnSetMoney->setToolTip(QCoreApplication::translate("Trade", "Add money to trade", nullptr));
#endif // QT_CONFIG(tooltip)
        QTableWidgetItem *___qtablewidgetitem = twPartnerItems->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("Trade", "Count", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = twPartnerItems->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("Trade", "Name", nullptr));
#if QT_CONFIG(tooltip)
        btnAddOne->setToolTip(QCoreApplication::translate("Trade", "Add one selected item", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        btnAddAll->setToolTip(QCoreApplication::translate("Trade", "Add all selected items", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        btnAddAllExOne->setToolTip(QCoreApplication::translate("Trade", "Add all selected items except one", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        btnRemoveOne->setToolTip(QCoreApplication::translate("Trade", "Remove one selected item", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        btnRemoveAll->setToolTip(QCoreApplication::translate("Trade", "Remove all selected items", nullptr));
#endif // QT_CONFIG(tooltip)
        btnConfirm->setText(QCoreApplication::translate("Trade", "Confirm", nullptr));
        btnAccept->setText(QCoreApplication::translate("Trade", "Accept", nullptr));
        checkBox->setText(QCoreApplication::translate("Trade", "Auto accept", nullptr));
        label_3->setText(QCoreApplication::translate("Trade", "Inventory", nullptr));
        label_2->setText(QCoreApplication::translate("Trade", "You", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = twInventory->horizontalHeaderItem(0);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("Trade", "Count", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = twInventory->horizontalHeaderItem(1);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("Trade", "Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = twMyItems->horizontalHeaderItem(0);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("Trade", "Count", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = twMyItems->horizontalHeaderItem(1);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("Trade", "Name", nullptr));
#if QT_CONFIG(tooltip)
        btnSetMax->setToolTip(QCoreApplication::translate("Trade", "Set to maximum", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        eMoney->setToolTip(QCoreApplication::translate("Trade", "Money to trade", nullptr));
#endif // QT_CONFIG(tooltip)
    } // retranslateUi

};

namespace Ui {
    class Trade: public Ui_Trade {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRADE_H
