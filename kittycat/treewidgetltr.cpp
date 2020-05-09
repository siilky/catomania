
#include "stdafx.h"

#include "treewidgetltr.h"

QModelIndex TreeWidgetLtr::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    if (cursorAction == QAbstractItemView::MoveNext)
    {
        QModelIndex index = currentIndex();
        if (index.column() < (columnCount() - 1))
        {
            return model()->index(index.row(), index.column() + 1, index.parent());
        }

        setCurrentIndex(model()->index(index.row(), 0, index.parent()));
    }
    else if(cursorAction == QAbstractItemView::MovePrevious)
    {
        QModelIndex index = currentIndex();
        if (index.column() != 0)
        {
            return model()->index(index.row(), index.column() - 1, index.parent());
        }

        setCurrentIndex(model()->index(index.row(), columnCount() - 1, index.parent()));
    }

    return QTreeView::moveCursor(cursorAction, modifiers);
}