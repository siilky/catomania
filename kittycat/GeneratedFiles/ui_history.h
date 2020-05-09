/********************************************************************************
** Form generated from reading UI file 'history.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HISTORY_H
#define UI_HISTORY_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_History
{
public:
    QVBoxLayout *verticalLayout_5;
    QSplitter *splitter;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_4;
    QTableWidget *tableItems;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLineEdit *leProfit;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *btnReset;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QToolButton *btnOpIn;
    QToolButton *btnOpOut;
    QSpacerItem *horizontalSpacer;
    QTableWidget *tableEvents;

    void setupUi(QDialog *History)
    {
        if (History->objectName().isEmpty())
            History->setObjectName(QString::fromUtf8("History"));
        History->resize(650, 400);
        verticalLayout_5 = new QVBoxLayout(History);
        verticalLayout_5->setSpacing(0);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(5, 5, 5, 5);
        splitter = new QSplitter(History);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        splitter->setChildrenCollapsible(false);
        verticalLayoutWidget_2 = new QWidget(splitter);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayout_4 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_4->setSpacing(4);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        tableItems = new QTableWidget(verticalLayoutWidget_2);
        if (tableItems->columnCount() < 4)
            tableItems->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableItems->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableItems->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableItems->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableItems->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        tableItems->setObjectName(QString::fromUtf8("tableItems"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tableItems->sizePolicy().hasHeightForWidth());
        tableItems->setSizePolicy(sizePolicy);
        QFont font;
        font.setFamily(QString::fromUtf8("Microsoft Sans Serif"));
        font.setPointSize(8);
        tableItems->setFont(font);
        tableItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableItems->setAlternatingRowColors(true);
        tableItems->setSelectionMode(QAbstractItemView::SingleSelection);
        tableItems->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableItems->setIconSize(QSize(20, 20));
        tableItems->setShowGrid(false);
        tableItems->setSortingEnabled(true);
        tableItems->setWordWrap(false);
        tableItems->setCornerButtonEnabled(false);
        tableItems->setColumnCount(4);
        tableItems->horizontalHeader()->setCascadingSectionResizes(true);
        tableItems->horizontalHeader()->setMinimumSectionSize(20);
        tableItems->horizontalHeader()->setHighlightSections(false);
        tableItems->horizontalHeader()->setProperty("showSortIndicator", QVariant(true));
        tableItems->verticalHeader()->setVisible(false);
        tableItems->verticalHeader()->setMinimumSectionSize(21);
        tableItems->verticalHeader()->setDefaultSectionSize(21);
        tableItems->verticalHeader()->setHighlightSections(false);

        verticalLayout_4->addWidget(tableItems);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(5);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(5, 0, 0, 0);
        label = new QLabel(verticalLayoutWidget_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        leProfit = new QLineEdit(verticalLayoutWidget_2);
        leProfit->setObjectName(QString::fromUtf8("leProfit"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(leProfit->sizePolicy().hasHeightForWidth());
        leProfit->setSizePolicy(sizePolicy1);
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        leProfit->setFont(font1);
        leProfit->setFocusPolicy(Qt::NoFocus);
        leProfit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        leProfit->setReadOnly(true);

        horizontalLayout_2->addWidget(leProfit);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        btnReset = new QPushButton(verticalLayoutWidget_2);
        btnReset->setObjectName(QString::fromUtf8("btnReset"));
        btnReset->setFocusPolicy(Qt::NoFocus);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/multiView/cross.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnReset->setIcon(icon);
        btnReset->setAutoDefault(false);
        btnReset->setFlat(true);

        horizontalLayout_2->addWidget(btnReset);


        verticalLayout_4->addLayout(horizontalLayout_2);

        splitter->addWidget(verticalLayoutWidget_2);
        verticalLayoutWidget = new QWidget(splitter);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_2->setSpacing(1);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(1);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        btnOpIn = new QToolButton(verticalLayoutWidget);
        btnOpIn->setObjectName(QString::fromUtf8("btnOpIn"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/history/arrow_left_16.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnOpIn->setIcon(icon1);
        btnOpIn->setCheckable(true);

        horizontalLayout->addWidget(btnOpIn);

        btnOpOut = new QToolButton(verticalLayoutWidget);
        btnOpOut->setObjectName(QString::fromUtf8("btnOpOut"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/history/arrow_right_16.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnOpOut->setIcon(icon2);
        btnOpOut->setCheckable(true);

        horizontalLayout->addWidget(btnOpOut);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(horizontalLayout);

        tableEvents = new QTableWidget(verticalLayoutWidget);
        if (tableEvents->columnCount() < 4)
            tableEvents->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableEvents->setHorizontalHeaderItem(0, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableEvents->setHorizontalHeaderItem(1, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tableEvents->setHorizontalHeaderItem(2, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableEvents->setHorizontalHeaderItem(3, __qtablewidgetitem7);
        tableEvents->setObjectName(QString::fromUtf8("tableEvents"));
        sizePolicy.setHeightForWidth(tableEvents->sizePolicy().hasHeightForWidth());
        tableEvents->setSizePolicy(sizePolicy);
        tableEvents->setFont(font);
        tableEvents->setFocusPolicy(Qt::NoFocus);
        tableEvents->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableEvents->setSelectionMode(QAbstractItemView::NoSelection);
        tableEvents->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableEvents->setShowGrid(false);
        tableEvents->setSortingEnabled(true);
        tableEvents->setWordWrap(false);
        tableEvents->setCornerButtonEnabled(false);
        tableEvents->horizontalHeader()->setMinimumSectionSize(20);
        tableEvents->horizontalHeader()->setStretchLastSection(true);
        tableEvents->verticalHeader()->setVisible(false);
        tableEvents->verticalHeader()->setDefaultSectionSize(21);

        verticalLayout_2->addWidget(tableEvents);

        splitter->addWidget(verticalLayoutWidget);

        verticalLayout_5->addWidget(splitter);


        retranslateUi(History);

        QMetaObject::connectSlotsByName(History);
    } // setupUi

    void retranslateUi(QDialog *History)
    {
        History->setWindowTitle(QCoreApplication::translate("History", "Trade history", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableItems->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("History", "Item", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableItems->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("History", "Stock", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableItems->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("History", "Value", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableItems->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("History", "Profit", nullptr));
        label->setText(QCoreApplication::translate("History", "Total profit", nullptr));
#if QT_CONFIG(tooltip)
        btnReset->setToolTip(QCoreApplication::translate("History", "Reset trades to zero profit on each", nullptr));
#endif // QT_CONFIG(tooltip)
        btnReset->setText(QCoreApplication::translate("History", "Reset", nullptr));
#if QT_CONFIG(tooltip)
        btnOpIn->setToolTip(QCoreApplication::translate("History", "Show buys", nullptr));
#endif // QT_CONFIG(tooltip)
        btnOpIn->setText(QCoreApplication::translate("History", "...", nullptr));
#if QT_CONFIG(tooltip)
        btnOpOut->setToolTip(QCoreApplication::translate("History", "Show sells", nullptr));
#endif // QT_CONFIG(tooltip)
        btnOpOut->setText(QCoreApplication::translate("History", "...", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableEvents->horizontalHeaderItem(0);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("History", "Op", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableEvents->horizontalHeaderItem(1);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("History", "Date", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableEvents->horizontalHeaderItem(2);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("History", "Count", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableEvents->horizontalHeaderItem(3);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("History", "Price", nullptr));
    } // retranslateUi

};

namespace Ui {
    class History: public Ui_History {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HISTORY_H
