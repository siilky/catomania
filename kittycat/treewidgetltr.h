#ifndef treeviewltr_h
#define treeviewltr_h

#include <QTreeWidget>
#include <QModelIndex>

class TreeWidgetLtr
    : public QTreeWidget
{
    Q_OBJECT
public:
    TreeWidgetLtr(QWidget * parent = NULL)
        : QTreeWidget(parent)
    {}

    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
};


#endif