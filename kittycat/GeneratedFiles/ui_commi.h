/********************************************************************************
** Form generated from reading UI file 'commi.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMMI_H
#define UI_COMMI_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Commi
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QLineEdit *leFilterItems;
    QSpacerItem *horizontalSpacer_2;
    QTreeView *treeView;
    QHBoxLayout *horizontalLayout_3;
    QLabel *lbLastUpdated;
    QProgressBar *progressBar;
    QPushButton *btnUpdate;
    QPushButton *btnSaveMargins;

    void setupUi(QWidget *Commi)
    {
        if (Commi->objectName().isEmpty())
            Commi->setObjectName(QString::fromUtf8("Commi"));
        Commi->resize(506, 518);
        verticalLayout = new QVBoxLayout(Commi);
        verticalLayout->setSpacing(5);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(4, 3, 4, 3);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(Commi);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setPixmap(QPixmap(QString::fromUtf8(":/commi/filter.png")));

        horizontalLayout_2->addWidget(label_3);

        leFilterItems = new QLineEdit(Commi);
        leFilterItems->setObjectName(QString::fromUtf8("leFilterItems"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(leFilterItems->sizePolicy().hasHeightForWidth());
        leFilterItems->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(leFilterItems);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_2);

        treeView = new QTreeView(Commi);
        treeView->setObjectName(QString::fromUtf8("treeView"));
        QFont font;
        font.setFamily(QString::fromUtf8("Microsoft Sans Serif"));
        font.setPointSize(8);
        treeView->setFont(font);
        treeView->setFocusPolicy(Qt::NoFocus);
        treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeView->setAlternatingRowColors(true);
        treeView->setSelectionMode(QAbstractItemView::NoSelection);
        treeView->setIconSize(QSize(20, 20));
        treeView->setTextElideMode(Qt::ElideNone);
        treeView->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        treeView->setIndentation(10);
        treeView->setSortingEnabled(true);

        verticalLayout->addWidget(treeView);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(10);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        lbLastUpdated = new QLabel(Commi);
        lbLastUpdated->setObjectName(QString::fromUtf8("lbLastUpdated"));
        lbLastUpdated->setMinimumSize(QSize(150, 0));

        horizontalLayout_3->addWidget(lbLastUpdated);

        progressBar = new QProgressBar(Commi);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(progressBar->sizePolicy().hasHeightForWidth());
        progressBar->setSizePolicy(sizePolicy2);
        progressBar->setMaximumSize(QSize(16777215, 18));
        progressBar->setValue(24);
        progressBar->setAlignment(Qt::AlignCenter);
        progressBar->setTextVisible(true);

        horizontalLayout_3->addWidget(progressBar);

        btnUpdate = new QPushButton(Commi);
        btnUpdate->setObjectName(QString::fromUtf8("btnUpdate"));
        sizePolicy1.setHeightForWidth(btnUpdate->sizePolicy().hasHeightForWidth());
        btnUpdate->setSizePolicy(sizePolicy1);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/commi/update.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnUpdate->setIcon(icon);

        horizontalLayout_3->addWidget(btnUpdate, 0, Qt::AlignRight);

        btnSaveMargins = new QPushButton(Commi);
        btnSaveMargins->setObjectName(QString::fromUtf8("btnSaveMargins"));
        sizePolicy1.setHeightForWidth(btnSaveMargins->sizePolicy().hasHeightForWidth());
        btnSaveMargins->setSizePolicy(sizePolicy1);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/multiView/disk.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSaveMargins->setIcon(icon1);

        horizontalLayout_3->addWidget(btnSaveMargins);

        horizontalLayout_3->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout_3);

        QWidget::setTabOrder(leFilterItems, treeView);
        QWidget::setTabOrder(treeView, btnUpdate);

        retranslateUi(Commi);

        QMetaObject::connectSlotsByName(Commi);
    } // setupUi

    void retranslateUi(QWidget *Commi)
    {
        Commi->setWindowTitle(QCoreApplication::translate("Commi", "Comissioner", nullptr));
        label_3->setText(QString());
#if QT_CONFIG(tooltip)
        leFilterItems->setToolTip(QCoreApplication::translate("Commi", "Item filter", nullptr));
#endif // QT_CONFIG(tooltip)
        progressBar->setFormat(QCoreApplication::translate("Commi", "%v / %m", nullptr));
#if QT_CONFIG(tooltip)
        btnUpdate->setToolTip(QCoreApplication::translate("Commi", "Update all items and info", nullptr));
#endif // QT_CONFIG(tooltip)
        btnUpdate->setText(QCoreApplication::translate("Commi", "Update", nullptr));
        btnSaveMargins->setText(QCoreApplication::translate("Commi", "Save margins", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Commi: public Ui_Commi {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMMI_H
