#ifndef MarketSetupModel_h__
#define MarketSetupModel_h__

#include <QAbstractTableModel>
#include <QStyledItemDelegate>

#include "MarketItemSetup.h"


class MarketSetupModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Columns
    {
        Name = 0,
        Amount,
        IsSelling,
        SellAutoPrice,
        SellPrice,
        SellAmount,
        KeepMin,
        IsBuying,
        BuyAutoPrice,
        BuyPrice,
        BuyAmount,
        KeepMax,
        ApPriceStep,
        ApMinCount,

        NumColumns_
    };

    enum
    {
        MarketItemRole  = Qt::UserRole + 1,
    };

    const static int sizeHints[NumColumns_];

    MarketSetupModel(QObject *parent = 0);
    virtual ~MarketSetupModel();

    void setData(const QList<MarketItemSetup> & data);
    QList<MarketItemSetup> getData() const;
    void setItemsMovable(bool isMovable);

    int columnWidth(int column);

    //

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool insertRow(int row, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    QStringList mimeTypes() const;
    QMimeData * mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex & parent);

    virtual Qt::ItemFlags flags(const QModelIndex & index) const;
    virtual int	rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int	columnCount(const QModelIndex & parent = QModelIndex()) const;

    Qt::DropActions supportedDropActions() const;

signals:
    void moveItems(unsigned srcSlot, int dstSlot, bool copyItems, bool insertBefore);

private:
    QList<MarketItemSetup>  data_;
    bool                    itemsMovable_;
};


class CenteredItemDelegate : public QStyledItemDelegate
{
public:
    CenteredItemDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {}

    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

class RenderLinkDelegate : public QStyledItemDelegate
{
public:
    RenderLinkDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {}

    virtual void initStyleOption(QStyleOptionViewItem * option, const QModelIndex & index) const;
};

class SpacedNumberDelegate : public QStyledItemDelegate
{
public:
    SpacedNumberDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {}

    virtual QString displayText(const QVariant &value, const QLocale & locale) const;
};

#endif
