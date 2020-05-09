/********************************************************************************
** Form generated from reading UI file 'prices.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRICES_H
#define UI_PRICES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "QtCharts"

QT_BEGIN_NAMESPACE

class Ui_Prices
{
public:
    QHBoxLayout *horizontalLayout;
    QStackedWidget *stackedWidget;
    QWidget *pageLoading;
    QVBoxLayout *verticalLayout;
    QLabel *lbStatus;
    QWidget *pageData;
    QHBoxLayout *horizontalLayout_2;
    QTableWidget *tableWidget;
    QChartView *chart;

    void setupUi(QDialog *Prices)
    {
        if (Prices->objectName().isEmpty())
            Prices->setObjectName(QString::fromUtf8("Prices"));
        Prices->resize(620, 300);
        horizontalLayout = new QHBoxLayout(Prices);
        horizontalLayout->setSpacing(5);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(3, 5, 3, 3);
        stackedWidget = new QStackedWidget(Prices);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        pageLoading = new QWidget();
        pageLoading->setObjectName(QString::fromUtf8("pageLoading"));
        verticalLayout = new QVBoxLayout(pageLoading);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        lbStatus = new QLabel(pageLoading);
        lbStatus->setObjectName(QString::fromUtf8("lbStatus"));
        QFont font;
        font.setPointSize(10);
        lbStatus->setFont(font);
        lbStatus->setAlignment(Qt::AlignCenter);
        lbStatus->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);

        verticalLayout->addWidget(lbStatus);

        stackedWidget->addWidget(pageLoading);
        pageData = new QWidget();
        pageData->setObjectName(QString::fromUtf8("pageData"));
        horizontalLayout_2 = new QHBoxLayout(pageData);
        horizontalLayout_2->setSpacing(5);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        tableWidget = new QTableWidget(pageData);
        if (tableWidget->columnCount() < 4)
            tableWidget->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        __qtablewidgetitem1->setTextAlignment(Qt::AlignCenter);
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        __qtablewidgetitem2->setTextAlignment(Qt::AlignCenter);
        tableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        __qtablewidgetitem3->setTextAlignment(Qt::AlignCenter);
        tableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
        tableWidget->setTextElideMode(Qt::ElideNone);
        tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableWidget->setShowGrid(false);
        tableWidget->setCornerButtonEnabled(false);
        tableWidget->horizontalHeader()->setMinimumSectionSize(20);
        tableWidget->horizontalHeader()->setDefaultSectionSize(80);
        tableWidget->horizontalHeader()->setHighlightSections(false);
        tableWidget->verticalHeader()->setVisible(false);
        tableWidget->verticalHeader()->setMinimumSectionSize(14);
        tableWidget->verticalHeader()->setDefaultSectionSize(14);

        horizontalLayout_2->addWidget(tableWidget);

        chart = new QChartView(pageData);
        chart->setObjectName(QString::fromUtf8("chart"));

        horizontalLayout_2->addWidget(chart);

        horizontalLayout_2->setStretch(0, 1);
        horizontalLayout_2->setStretch(1, 1);
        stackedWidget->addWidget(pageData);

        horizontalLayout->addWidget(stackedWidget);


        retranslateUi(Prices);

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(Prices);
    } // setupUi

    void retranslateUi(QDialog *Prices)
    {
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("Prices", "Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("Prices", "Sell", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("Prices", "Count", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("Prices", "Buy", nullptr));
        Q_UNUSED(Prices);
    } // retranslateUi

};

namespace Ui {
    class Prices: public Ui_Prices {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRICES_H
