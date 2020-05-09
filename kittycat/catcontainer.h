#ifndef catcontainer_h_
#define catcontainer_h_

#include <QAbstractItemModel>
#include <QSharedPointer>

#include "persistence2.h"
#include "catctl.h"
#include "forms\uicatview.h"

// Про конфиг:
// Поскольку изменение родительского контейнера инвалидирует позиции на детей - мы не можем
// отдавать в обьекты котов постоянные ссылки на ноды конфига. Таким образом, конфиг на каждого кота 
// бежит в независимой ноде. Будучи единожды прочитанным, родительский конфиг не изменяет своего состояния 
// из котов; изменения должны сбрасываться на диск перезаписью всего массива.
// Т.е. конфиг является только входными и выходными данными для конструкторов.

class TreeItem
{
public:
    TreeItem(TreeItem *parent)
        : parent_(parent)
    {}

    virtual int rowCount() const        { return 0; }
    virtual int row() const             { return -1; };         // return item's row in parent context
    virtual TreeItem *parent() const    { return parent_; };
    virtual TreeItem *child(int /*number*/) { return NULL; };

    virtual int columnCount() const = 0;
    virtual QVariant data(int /*column*/, int role) const = 0;
    virtual bool setData(int /*column*/, const QVariant &/*value*/) { return false; }

    virtual bool insertChildren(int /*position*/, int /*count*/)    { return false; }
    virtual bool removeChildren(int /*position*/, int /*count*/)    { return false; }

    //

    virtual QSharedPointer<CatView> catView() const
    {
        return QSharedPointer<CatView>();
    }

    virtual QList< QSharedPointer<CatCtl> > catList() const
    {
        return QList< QSharedPointer<CatCtl> >();
    }

protected:
    TreeItem *parent_;
};

//

class CatGroup;

class CatContainer : public TreeItem
{
public:
    CatContainer(const CatContainer &r, CatGroup *parent);  // performs move operation

    // config 
    CatContainer(const JsonValue & config, CatGroup *parent);
    operator JsonValue &();

    // tree
    virtual int row() const;
    virtual int columnCount() const;

    virtual QVariant data(int column, int role) const;
    virtual bool setData(int column, const QVariant &value);

    //

    virtual QSharedPointer<CatView> catView() const
    {
        return catView_;
    }

    virtual QList< QSharedPointer<CatCtl> > catList() const
    {
        return QList< QSharedPointer<CatCtl> >() << cat_;
    }

    //  

    bool operator==(const CatContainer &r)
    {
        return catView_ == r.catView_;
    }

private:
    friend class CatGroup;
    friend class CatTreeModel;

    CatContainer(const CatContainer &);

    JsonValue   config_;
    QString     tag_;

    QSharedPointer<CatCtl>  cat_;
    QSharedPointer<CatView> catView_;
};


class CatGroup : public TreeItem
{
public:
    CatGroup();
    CatGroup(const CatGroup & r);
    ~CatGroup();

    // config
    CatGroup(const JsonValue & val);
    operator JsonValue();

    // tree
    virtual int rowCount() const;
    virtual TreeItem *child(int number);

    virtual int columnCount() const;
    virtual QVariant data(int column, int role) const;
    virtual bool setData(int column, const QVariant &value);

    virtual bool insertChildren(int position, int count);
    virtual bool removeChildren(int position, int count);

    //

    virtual QSharedPointer<CatView> catView() const
    {
        return QSharedPointer<CatView>();
    }

    virtual QList< QSharedPointer<CatCtl> > catList() const
    {
        QList< QSharedPointer<CatCtl> > result;
        for (int i = 0; i < cats_.count(); i++)
        {
            result << cats_[i]->catList();
        }
        return result;
    }


private:
    friend class CatContainer;
    friend class CatTreeModel;

    CatContainer *take(int index)
    {
        return cats_.takeAt(index);
    }

    void put(int index, CatContainer *cat)
    {
        cats_.insert(index, cat);
    }

    QString                 tag_;
    QList<CatContainer*>    cats_;
};


class CatTreeModel : public QAbstractItemModel
{
public:
    enum
    {
        CatItemRole   = Qt::UserRole + 1,   // data() returns pointer to TreeItem
    };

    CatTreeModel(QObject * parent = NULL)
        : QAbstractItemModel(parent)
    {}
    ~CatTreeModel();

    void setCats(const QList<CatGroup*> & cats);
    const QList<CatGroup*> & cats() const
    {
        return groups_;
    }

    void loadItem(const QString & filename, int row, const QModelIndex & parent);

    bool canBeSaved(const QModelIndex & item);
    void saveItem(const QString & filename, const QModelIndex & item);

    // model
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual Qt::DropActions supportedDropActions() const;

    virtual bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());

    virtual QStringList mimeTypes() const;
    virtual QMimeData *	mimeData(const QModelIndexList & indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex & parent);

public slots:
    void itemUpdate(CatView * w);

private:
    TreeItem *CatTreeModel::getItem(const QModelIndex &index) const;

    QList<CatGroup*> groups_;
};

#endif