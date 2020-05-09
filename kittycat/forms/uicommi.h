#ifndef UICOMMI_H
#define UICOMMI_H

#include <QDialog>
#include <QSortFilterProxyModel>

#include "qlib\game\gamebase.h"
#include "game\data\items.h"

class CatCtl;

namespace Ui { class Commi; }

class CommiModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    CommiModel(QSharedPointer<CatCtl> ctl, QObject *parent);
    virtual ~CommiModel();

    void setData(const std::vector<CommiShop> & data);

    enum Columns
    {
        // Commi level
        ShopName = 0,
        NumColumns0_,

        // Cat level
        ItemName = 0,
        SellPrice,
        Amount,
        BuyPrice,
        NumColumns1_,
    };

    //

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    //virtual Qt::ItemFlags flags(const QModelIndex & index) const;
    virtual int	rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int	columnCount(const QModelIndex & parent = QModelIndex()) const;

private slots:
    void onGameEvent(const qlib::GameEvent & event);

private:
    std::vector<CommiShop>::const_iterator posOf(int row) const;

    std::vector<CommiShop>  data_;
    QSharedPointer<CatCtl>  ctl_;
};

class CommiFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    CommiFilterModel(QObject *parent);
    virtual ~CommiFilterModel();

public slots: 
    void setFilterFixedString(const QString &pattern);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    QString filterString_;
};


class CommiView : public QDialog
{
    Q_OBJECT
public:
    CommiView(QSharedPointer<CatCtl> ctl, QWidget *parent = 0);
    ~CommiView(); 

private slots:
    void on_btnUpdate_clicked();
    void on_btnSaveMargins_clicked();
    void on_leFilterItems_textChanged(const QString & text);

    void onGameEvent(const qlib::GameEvent & event);

private:
    void showLastUpdated();

    QScopedPointer<Ui::Commi>   ui_;
    CommiModel                  *commiModel_;
    CommiFilterModel            *commiFilter_;

    QSharedPointer<CatCtl>      ctl_;
};

#endif
