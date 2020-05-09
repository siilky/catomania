
#include "stdafx.h"

#include <QMimeData>
#include <QVector>

#include "catcontainer.h"

CatContainer::CatContainer(const JsonValue & config, CatGroup *parent)
    : TreeItem(parent)
    , config_(config)
    , cat_(new CatCtl(config))
    , catView_(new CatView(cat_))
{
    std::wstring tag;
    config.get(L"Tag", tag);
    tag_ = QString::fromStdWString(tag);
}

CatContainer::CatContainer(const CatContainer &r, CatGroup *parent)
    : TreeItem(parent)
    , config_(r.config_)
    , tag_(r.tag_)
    , cat_(r.cat_)
    , catView_(r.catView_)
{
}

CatContainer::operator JsonValue &()
{
    config_.set(L"Tag", tag_.toStdWString());
    return config_;
}

int CatContainer::columnCount() const
{
    return 1;
}

int CatContainer::row() const
{
    if (parent_ != NULL)
    {
        CatGroup *cg = static_cast<CatGroup*>(parent_);
        for (int i = 0; i < cg->cats_.count(); i++)
        {
            if (*cg->cats_[i] == *this)
            {
                return i;
            }
        }
    }
    return -1;
}

QVariant CatContainer::data(int column, int role) const
{
    if (column != 0)
    {
        return QVariant();
    }

    switch (role)
    {
        case Qt::DecorationRole:
        {
            if ( ! catView_)
            {
                return QVariant();
            }

            CatView::CatState state = catView_->state();
            switch (state)
            {
                case CatView::Offline:      return QIcon(":/catView/door_16.png");
                case CatView::Online:       return QIcon(":/catView/door_open_16.png");
                case CatView::Locked:       return QIcon(":/catView/lock_16.png");
                case CatView::MarketOpened: return QIcon(":/catView/cat.png");
                case CatView::Dead:         return QIcon(":/catView/pirate_flag_16.png");
                case CatView::Waiting:      return QIcon(":/catView/time.png");
            }
            break;
        }

        case Qt::DisplayRole:
        case Qt::EditRole:
        {
            if ( ! tag_.isEmpty())
            {
                return tag_;
            }

            QString acc = catView_->currentAccount();
            return acc.isEmpty() ? "[...]" : acc;
        }
    }

    return QVariant();
}

bool CatContainer::setData(int column, const QVariant &value)
{
    if (column == 0)
    {
        tag_ = value.toString();
        return true;
    }
    return false;
}

CatGroup::CatGroup()
    : TreeItem(NULL)
{
}

CatGroup::CatGroup(const CatGroup & r)
    : TreeItem(NULL)
    , tag_(r.tag_)
{
    foreach(CatContainer *c, r.cats_)
    {
        cats_.push_back(new CatContainer(*c, this));
    }
}

CatGroup::CatGroup(const JsonValue & config)
    : TreeItem(NULL)
{
    std::wstring tag;
    config.get(L"Tag", tag);
    tag_ = QString::fromStdWString(tag);

    class CatConverter
    {
    public:
        CatConverter(CatGroup * group)
            : group_(group)
        {}

        CatContainer * convertIn(const JsonValue & v) const
        {
            return new CatContainer(v, group_);
        }

    private:
        CatGroup * group_;
    };

    std::vector<CatContainer *> cats;
    config.get(L"Cats", cats, CatConverter(this));
    cats_ = QVector<CatContainer *>::fromStdVector(cats).toList();
}

CatGroup::~CatGroup()
{
    qDeleteAll(cats_);
}

CatGroup::operator JsonValue()
{
    JsonValue config;
    config.set(L"Tag", tag_.toStdWString());

    config.set(L"Cats", cats_.toVector().toStdVector(), json::TransparentPtrConversion<CatContainer>());

    return config;
}

int CatGroup::rowCount() const
{
    return cats_.count();
}

TreeItem * CatGroup::child(int number)
{
    if (number < 0 || number >= cats_.count())
    {
        return NULL;
    }

    return cats_[number];
}

int CatGroup::columnCount() const
{
    return 1;
}

QVariant CatGroup::data(int column, int role) const
{
    if (column != 0)
    {
        return QVariant();
    }

    switch(role)
    {
        case Qt::DecorationRole:
            return QIcon(":/multiView/reseller_programm.png");

        case Qt::DisplayRole:
        case Qt::EditRole:
            return tag_.isEmpty() ? "..." : tag_;
    }

    return QVariant();
}

bool CatGroup::setData(int column, const QVariant &value)
{
    if (column == 0)
    {
        tag_ = value.toString();
        return true;
    }
    return false;
}

bool CatGroup::insertChildren(int position, int count)
{
    if (position < 0 || position > cats_.size())
    {
        return false;
    }

    for (int row = 0; row < count; ++row)
    {
        cats_.insert(position, new CatContainer(JsonValue(), this));
    }
    return true;
}

bool CatGroup::removeChildren(int position, int count)
{
    if (position < 0 || position > cats_.size())
    {
        return false;
    }

    while (position < cats_.count() && count > 0)
    {
        CatContainer *cat = cats_.takeAt(position);
        delete cat;
        count--;
    }
    return true;
}

//

CatTreeModel::~CatTreeModel()
{
    qDeleteAll(groups_);
}

void CatTreeModel::setCats(const QList<CatGroup*> & cats)
{
    beginResetModel();
//     qDeleteAll(groups_);
//     groups_.clear();
    groups_ = cats;
    endResetModel();
    /*
    if ( ! cats.isEmpty())
    {
        beginInsertRows(QModelIndex(), 0, cats.count());
        endInsertRows();

    }
    */
}

void CatTreeModel::loadItem(const QString & filename, int row, const QModelIndex & parent)
{
    JsonValue jscfg;
    if (jscfg.loadFrom(filename.toStdWString())
        && jscfg.isObject())
    {
        beginInsertRows(parent, row, row);

        TreeItem *parentItem = getItem(parent);
        if (parentItem != NULL)
        {
            CatGroup *cg = dynamic_cast<CatGroup*>(parentItem);
            if (cg != NULL)
            {
                if (jscfg.contains(L"Client"))
                {
                    // old-style configuration
                    cg->cats_.insert(row, new CatContainer(jscfg.get(L"Client"), cg));
                }
                else
                {
                    cg->cats_.insert(row, new CatContainer(jscfg, cg));
                }
            }
        }
        else
        {
            // no parent, top-level
            // TBD groups_.insert(position, new CatGroup());
        }

        endInsertRows();
    }
}

bool CatTreeModel::canBeSaved(const QModelIndex & item)
{
    TreeItem * i = getItem(item);
    if (i != NULL)
    {
        CatContainer *cc = dynamic_cast<CatContainer *>(i);
        return cc != NULL;
    }
    return false;
}

void CatTreeModel::saveItem(const QString & filename, const QModelIndex & item)
{
    TreeItem * i = getItem(item);
    if (i != NULL)
    {
        CatContainer *cc = dynamic_cast<CatContainer *>(i);
        if (cc != NULL)
        {
            cc->operator JsonValue &().saveTo(filename.toStdWString());
        }
    }
}

//

QModelIndex CatTreeModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (parent.isValid() && parent.column() != 0)
    {
        return QModelIndex();
    }

    TreeItem *parentItem = getItem(parent);
    if (parentItem != NULL)
    {
        TreeItem *childItem = parentItem->child(row);
        if (childItem != NULL)
        {
            return createIndex(row, column, childItem);
        }
    }
    else
    {
        // no parent, top-level
        if (row < groups_.count())
        {
            return createIndex(row, column, groups_[row]);
        }
    }
    return QModelIndex();
}

QModelIndex CatTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    TreeItem *item = getItem(index);
    if (item != NULL)
    {
        TreeItem *parentItem = item->parent();
        if (parentItem != NULL)
        {
            // parent of not top-level item
            int pos = groups_.indexOf(static_cast<CatGroup*>(parentItem));
            return createIndex(pos, 0, parentItem);
        }
    }
    return QModelIndex();
}

int CatTreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    TreeItem *parentItem = getItem(parent);
    if (parentItem != NULL)
    {
        return parentItem->rowCount();
    }
    else
    {
        // top-level
        return groups_.count();
    }
}

int CatTreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    TreeItem *parentItem = getItem(parent);
    if (parentItem != NULL)
    {
        if (parentItem->rowCount() > 0)
        {
            return parentItem->child(0)->columnCount();
        }
    }
    else
    {
        // top-level
        if (groups_.count() > 0)
        {
            return groups_[0]->columnCount();
        }
    }

    return 0;
}

QVariant CatTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    switch(role)
    {
        case Qt::DecorationRole:
        case Qt::DisplayRole:
        case Qt::EditRole:
        {
            TreeItem *item = getItem(index);
            if (item != 0)
            {
                return item->data(index.column(), role);
            }
            break;
        }

        case CatItemRole:
        {
            TreeItem *item = getItem(index);
            if (item != 0)
            {
                return QVariant::fromValue((void*)item);
            }
            break;
        }
    }

    return QVariant();
}

QVariant CatTreeModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/ /*= Qt::DisplayRole*/) const
{
    return QVariant();
}

bool CatTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
    {
        return false;
    }

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
    {
        emit dataChanged(index, index);
    }

    return result;
}

Qt::ItemFlags CatTreeModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled 
        | ((index.isValid() && index.parent().isValid()) ? 0 : Qt::ItemIsDropEnabled)
        | /*((index.isValid() && index.parent().isValid()) ? */Qt::ItemIsSelectable/* : 0)*/;
}

Qt::DropActions CatTreeModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

bool CatTreeModel::insertRows(int position, int rows, const QModelIndex &parent /*= QModelIndex()*/)
{
    beginInsertRows(parent, position, position + rows - 1);

    bool success = false;
    TreeItem *parentItem = getItem(parent);
    if (parentItem != NULL)
    {
        success = parentItem->insertChildren(position, rows);
    }
    else
    {
        // no parent, top-level
        groups_.insert(position, new CatGroup());
    }

    endInsertRows();
    return success;
}

bool CatTreeModel::removeRows(int position, int rows, const QModelIndex &parent /*= QModelIndex()*/)
{
    if (position < 0)
    {
        return false;
    }

    beginRemoveRows(parent, position, position + rows - 1);

    bool success = true;
    TreeItem *parentItem = getItem(parent);
    if (parentItem != NULL)
    {
        success = parentItem->removeChildren(position, rows);
    }
    else
    {
        // no parent, top-level
        while (position < groups_.count() && rows > 0)
        {
            CatGroup *cg = groups_.takeAt(position);
            delete cg;
            rows--;
        }
    }

    endRemoveRows();
    return success;
}

TreeItem *CatTreeModel::getItem(const QModelIndex & index) const
{
    if (index.isValid())
    {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        return item;
    }
    else
    {
        // top-level items
        return NULL;
    }
}

QStringList CatTreeModel::mimeTypes() const
{
    return QStringList()
        << "application/x-catmodeldatalist" 
        << "text/uri-list";
}

QMimeData * CatTreeModel::mimeData(const QModelIndexList & indexes) const
{
    if (indexes.count() <= 0)
    {
        return NULL;
    }

    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    for (QModelIndexList::ConstIterator it = indexes.begin(); it != indexes.end(); ++it)
    {
        TreeItem *item = static_cast<TreeItem*>(it->internalPointer());
        TreeItem *parent= static_cast<TreeItem*>(it->parent().internalPointer());
        if (parent == NULL)
        {
            // group level
            CatGroup *cg = static_cast<CatGroup*>(item);
            stream << false << groups_.indexOf(cg);
        }
        else
        {
            // cat level
            CatContainer *cc = static_cast<CatContainer*>(item);
            CatGroup *cg = static_cast<CatGroup*>(parent);
            stream << true << groups_.indexOf(cg) << cc->row();
        }
    }

    QString format = mimeTypes().at(0);
    QMimeData *data = new QMimeData();
    data->setData(format, encoded);
    return data;
}

bool CatTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    QStringList types = mimeTypes();
    if (!data
        || types.isEmpty()
        || (data->hasFormat(types.at(0)) && action != Qt::MoveAction)
        || (data->hasFormat(types.at(1)) && action != Qt::CopyAction))
    {
        return false;
    }

    if (column == -1)
        column = 0;

    // move to
    // - empty space: parent=-1 row=-1
    // - on group: parent=group row=-1
    // - below group: parent=-1 row=nextGroup
    // - above group: parent=-1 row=group
    // - on cat: parent=cat row=-1
    // - between cats: parent=group row=cat

    if (data->hasFormat(types.at(0)))
    {
        QByteArray encoded = data->data(types.at(0));
        QDataStream stream(&encoded, QIODevice::ReadOnly);

        bool isCat;
        int fromGroup;
        stream >> isCat >> fromGroup;

        if (isCat)
        {
            // moving cat to other group or within group

            int fromRow;
            stream >> fromRow;
            QModelIndex fromIdx = index(fromRow, 0, index(fromGroup, 0, QModelIndex()));

            QModelIndex toGroupIdx = parent;
            int toRow = 0;
            if (parent.parent().isValid())              // directly to cat
            {
                toRow = parent.row();
                toGroupIdx = parent.parent();

                if (toRow > fromRow && toGroupIdx.row() >= fromGroup)
                {
                    toRow++;
                }
            }
            else if (parent.isValid() && row >= 0)       // between cats; row < 0 : directly to groups
            {
                toRow = row;
            }
            else if (parent.isValid() && row < 0)       // directly on group
            {
                TreeItem *item = (TreeItem *)parent.internalPointer();
                toRow = item->rowCount();
            }
            else if (row < 0)   //  && ! parent.isValid() // to empty space
            {
                if ( ! groups_.isEmpty())
                {
                    toGroupIdx = index(groups_.count() - 1, 0, QModelIndex());
                    toRow = groups_[toGroupIdx.row()]->rowCount();
                }
            }
            else    // row >= 0 && !parent.isValid()    // group level between groups
            {
                if ( ! groups_.isEmpty())
                {
                    toGroupIdx = index(row > 0 ? row - 1 : row, 0, QModelIndex());
                    toRow = groups_[toGroupIdx.row()]->rowCount();
                }
            }

            beginInsertRows(toGroupIdx, toRow, toRow);
            groups_[toGroupIdx.row()]->put(toRow, new CatContainer(*groups_[fromGroup]->cats_[fromRow], groups_[toGroupIdx.row()]));
            endInsertRows();
        }
        else
        {
            // moving/reordering groups

            int toRow;
            if (parent.parent().isValid())      // directly to cat
            {
                toRow = parent.parent().row();
            }
            else if (parent.isValid())  // row >= 0 : between cats; row < 0 : directly on group
            {
                toRow = parent.row();
            }
            else if (row < 0)   //  && ! parent.isValid()   // to empty space
            {
                toRow = groups_.count();
            }
            else    // row >= 0 && !parent.isValid()        // group level between groups
            {
                toRow = row;
            }

            // 'down' direction should increment index by 1
            if (toRow > fromGroup
                && !(row >= 0 && !parent.isValid()))
            {
                toRow++;
            }

            beginInsertRows(QModelIndex(), toRow, toRow);
            groups_.insert(toRow, new CatGroup(*groups_[fromGroup]));
            endInsertRows();
        }
    }
    else if (data->hasFormat(types.at(1)))
    {
//         QByteArray encoded = data->data(types.at(1));
//         QString filename = QString::fromWCharArray((wchar_t*)encoded.data(), encoded.count() / sizeof(wchar_t)); 

        QModelIndex toGroupIdx = parent;
        int toRow = 0;
        if (parent.parent().isValid())              // directly to cat
        {
            toRow = parent.row();
            toGroupIdx = parent.parent();
        }
        else if (parent.isValid() && row >= 0)      // between cats; row < 0 : directly to groups
        {
            toRow = row;
        }
        else if (parent.isValid() && row < 0)       // directly on group
        {
            TreeItem *item = (TreeItem *)parent.internalPointer();
            toRow = item->rowCount();
        }
        else if (row < 0)   //  && ! parent.isValid()   // to empty space
        {
            if ( ! groups_.isEmpty())
            {
                toGroupIdx = index(groups_.count() - 1, 0, QModelIndex());
                toRow = groups_[toGroupIdx.row()]->rowCount();
            }
        }
        else    // row >= 0 && !parent.isValid()    // group level between groups
        {
            if ( ! groups_.isEmpty())
            {
                toGroupIdx = index(row > 0 ? row - 1 : row, 0, QModelIndex());
                toRow = groups_[toGroupIdx.row()]->rowCount();
            }
        }

        QList<QUrl> urlList = data->urls();
        for (int i = 0; i < urlList.size(); ++i)
        {
            loadItem(urlList[i].toLocalFile(), toRow++, toGroupIdx);
        }
    }

    return true;
}

void CatTreeModel::itemUpdate(CatView * w)
{
    // TBD should be a tree-like

    for (int i = 0; i < groups_.count(); i++)
    {
        // QModelIndex index = groups_[i]->indexOf(w);
        QModelIndex index;
        const QList<CatContainer*> & cats_ = groups_.at(i)->cats_;
        for (int c = 0; c < cats_.count(); c++)
        {
            if (cats_[c]->catView() == w)
            {
                index = createIndex(c, 0, cats_[c]);
                break;
            }
        }

        if (index.isValid())
        {
            emit dataChanged(index, index);
            break;
        }
    }
}
