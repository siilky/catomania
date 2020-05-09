/********************************************************************************
** Form generated from reading UI file 'multiview.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MULTIVIEW_H
#define UI_MULTIVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MultiView
{
public:
    QVBoxLayout *verticalLayout_2;
    QSplitter *splitter;
    QWidget *catBox;
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *btnAddCat;
    QSpacerItem *horizontalSpacer;
    QToolButton *btnGlobalSettings;
    QFrame *line;
    QToolButton *btnLeft;
    QTreeView *treeView;

    void setupUi(QWidget *MultiView)
    {
        if (MultiView->objectName().isEmpty())
            MultiView->setObjectName(QString::fromUtf8("MultiView"));
        MultiView->resize(700, 264);
        verticalLayout_2 = new QVBoxLayout(MultiView);
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        splitter = new QSplitter(MultiView);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setStyleSheet(QString::fromUtf8("QSplitter::handle {\n"
"    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, \n"
"	stop:0 rgba(255, 255, 255, 0), \n"
"	stop:0.5 rgba(200, 200, 200, 255), \n"
"	stop:0.7 rgba(101, 104, 113, 235), \n"
"	stop:0.9 rgba(255, 255, 255, 0));\n"
"}\n"
""));
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(5);
        splitter->setChildrenCollapsible(true);
        catBox = new QWidget(splitter);
        catBox->setObjectName(QString::fromUtf8("catBox"));
        verticalLayout = new QVBoxLayout(catBox);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        frame = new QFrame(catBox);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::Box);
        frame->setFrameShadow(QFrame::Sunken);
        horizontalLayout_2 = new QHBoxLayout(frame);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(1, 1, 1, 1);
        btnAddCat = new QToolButton(frame);
        btnAddCat->setObjectName(QString::fromUtf8("btnAddCat"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/catView/cat.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnAddCat->setIcon(icon);
        btnAddCat->setPopupMode(QToolButton::MenuButtonPopup);
        btnAddCat->setAutoRaise(true);

        horizontalLayout_2->addWidget(btnAddCat);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        btnGlobalSettings = new QToolButton(frame);
        btnGlobalSettings->setObjectName(QString::fromUtf8("btnGlobalSettings"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/catView/widgets16.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnGlobalSettings->setIcon(icon1);
        btnGlobalSettings->setAutoRaise(true);

        horizontalLayout_2->addWidget(btnGlobalSettings);

        line = new QFrame(frame);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShadow(QFrame::Raised);
        line->setFrameShape(QFrame::VLine);

        horizontalLayout_2->addWidget(line);

        btnLeft = new QToolButton(frame);
        btnLeft->setObjectName(QString::fromUtf8("btnLeft"));
        btnLeft->setIconSize(QSize(5, 16));
        btnLeft->setAutoRaise(true);
        btnLeft->setArrowType(Qt::LeftArrow);

        horizontalLayout_2->addWidget(btnLeft);


        verticalLayout->addWidget(frame);

        treeView = new QTreeView(catBox);
        treeView->setObjectName(QString::fromUtf8("treeView"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(treeView->sizePolicy().hasHeightForWidth());
        treeView->setSizePolicy(sizePolicy);
        QFont font;
        font.setFamily(QString::fromUtf8("Microsoft Sans Serif"));
        font.setPointSize(8);
        treeView->setFont(font);
        treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        treeView->setFrameShape(QFrame::NoFrame);
        treeView->setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::SelectedClicked);
        treeView->setDragEnabled(true);
        treeView->setDragDropMode(QAbstractItemView::DragDrop);
        treeView->setDefaultDropAction(Qt::MoveAction);
        treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
        treeView->setTextElideMode(Qt::ElideNone);
        treeView->setIndentation(9);
        treeView->setAnimated(true);
        treeView->setHeaderHidden(true);

        verticalLayout->addWidget(treeView);

        splitter->addWidget(catBox);

        verticalLayout_2->addWidget(splitter);


        retranslateUi(MultiView);

        QMetaObject::connectSlotsByName(MultiView);
    } // setupUi

    void retranslateUi(QWidget *MultiView)
    {
        MultiView->setWindowTitle(QCoreApplication::translate("MultiView", "Cats", nullptr));
#if QT_CONFIG(tooltip)
        btnAddCat->setToolTip(QCoreApplication::translate("MultiView", "Add ...", nullptr));
#endif // QT_CONFIG(tooltip)
    } // retranslateUi

};

namespace Ui {
    class MultiView: public Ui_MultiView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MULTIVIEW_H
