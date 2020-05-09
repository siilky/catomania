#include "stdafx.h"

#include "uicommi.h"
#include "ui_commi.h"

#include "catctl.h"
#include "itempropformatter.h"
#include "util.h"
#include "qlib\game\iconmanager.h"


CommiModel::CommiModel(QSharedPointer<CatCtl> ctl, QObject *parent)
    : QAbstractItemModel(parent)
    , ctl_(ctl)
{
    connect(ctl_.data(), &CatCtl::gameEvent, this, &CommiModel::onGameEvent);
}

CommiModel::~CommiModel()
{
}

void CommiModel::setData(const std::vector<CommiShop> & data)
{
    beginResetModel();

    data_ = data;

    auto compByPrice = [](const MarketItem & r, const MarketItem & l)
        -> bool { return r.price < l.price; };
    for (auto it : data)
    {
        std::sort(it.sellList.begin(), it.sellList.end(), compByPrice);
        std::sort(it.buyList.begin(), it.buyList.end(), compByPrice);
    }

    endResetModel();
}

QModelIndex CommiModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (row < 0 || column < 0)
    {
        return QModelIndex();
    }

    if (!parent.isValid())
    {
        // list level
        //if (column == ShopName) проверка здесь нужна не очень, потому что хедер пробегает по всем колонкам
        {
            if (row >= 0 && unsigned(row) < data_.size())
            {
                return createIndex(row, column);
            }
        }
    }
    else
    {
        // shop level
        auto it = posOf(parent.row());
        if (it != data_.end())
        {
            const CommiShop & shop = *it;
            unsigned items = (shop.sellList.size() + shop.buyList.size());
            assert(items == 0 || (row >= 0 && items > 0 && unsigned(row) < items));
            if (row >= 0 && items > 0 && unsigned(row) < items)
            {
                return createIndex(row, column, parent.row() + 1);
            }
        }
    }
    return QModelIndex();
}

QModelIndex CommiModel::parent(const QModelIndex &index) const
{
    assert(index.isValid());

    unsigned parentRow = index.internalId();
    if (parentRow != 0)
    {
        // list level
        auto parent = posOf(parentRow - 1);
        if (parent != data_.end())
        {
            int row = parent - data_.begin();
            if (row >= 0)
            {
                return createIndex(row, 0);
            }
        }
    }
    return QModelIndex();
}

QVariant CommiModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation != Qt::Horizontal)
    {
        return QVariant();
    }

    switch (role)
    {
        case Qt::DisplayRole:
            switch (section)
            {
                //case ItemName:
                case ShopName:      return tr("Name");
                case SellPrice:     return tr("Sell price");
                case Amount:        return tr("Count");
                case BuyPrice:      return tr("Buy price");
            }
            break;
    }
    return QVariant();
}

QVariant CommiModel::data(const QModelIndex & index, int role /*= Qt::DisplayRole*/) const
{
    assert(index.isValid());

    const int column = index.column();
    const int row = index.row();

    unsigned parentRow = index.internalId();
    if (parentRow == 0)
    {
        // list level

        if (column != ShopName)
        {
            return QVariant();
        }

        assert(row >= 0 && (unsigned) row < data_.size());

        switch (role)
        {
            case Qt::DisplayRole:
            {
                auto it = posOf(row);
                if (it != data_.end())
                {
                    QString name = ctl_->resolvePlayerName(it->id);
                    return name.isNull() ? spacedNumber(it->id) : name;
                }
            }
            break;

            case Qt::TextAlignmentRole:
                switch (column)
                {
                    case ShopName:
                        return int(Qt::AlignLeft | Qt::AlignVCenter);
                    default:
                        return int(Qt::AlignRight | Qt::AlignVCenter);
                }
                break;
        }
    }
    else
    {
        // shop level
        // buyList
        // sellList

        auto it = posOf(parentRow - 1);
        if (it == data_.end())
        {
            return QVariant();
        }

        const CommiShop & shop = *it;

        assert(row >= 0 && unsigned(row) < shop.sellList.size() + shop.buyList.size());
        bool isSelling = row >= int(shop.buyList.size());
        const MarketItem & item = isSelling ? shop.sellList.at(row - shop.buyList.size())
            : shop.buyList.at(row);

        switch (role)
        {
            case Qt::DisplayRole:
                switch (column)
                {
                    case ItemName:
                        return item.name.empty() ? spacedNumber(item.id) : QString::fromStdWString(item.name);
                    case SellPrice:
                        return isSelling ? spacedNumber(item.price) : QVariant();
                    case Amount:
                        return spacedNumber(item.count);
                    case BuyPrice:
                        return isSelling ? QVariant() : spacedNumber(item.price);
                }
                break;

            case Qt::TextColorRole:
                switch (column)
                {
                    case SellPrice: return QColor("blue");
                    case BuyPrice:  return QColor("darkred");
                }
                break;

            case Qt::ToolTipRole:
                if (column == ItemName)
                {
                    if (item.props)
                    {
                        QString text = itemPropsToString(QString::fromStdWString(item.name), item.props.get());
                        return text;
                    }
                }
                break;

            case Qt::TextAlignmentRole:
                switch (column)
                {
                    case ItemName:
                        return int(Qt::AlignLeft | Qt::AlignVCenter);
                    default:
                        return int(Qt::AlignRight | Qt::AlignVCenter);
                }
                break;

            case Qt::DecorationRole:
                if (column == ItemName)
                {
                    // return QIcon so itemview will resize it to row height
                    QImage image;
                    if (!item.icon.empty())
                    {
                        QTextCodec *tc = QTextCodec::codecForName("GBK");
                        QString icon = tc->toUnicode(item.icon.c_str(), item.icon.size());
                        image = IconManager::instance()->getIconByPath(icon);
                    }

                    if (!image.isNull())
                    {
                        return QIcon(QPixmap::fromImage(image));
                    }
                    else
                    {
                        static QPixmap px(32, 32);
                        static bool pxInit = false;
                        if (!pxInit)
                        {
                            px.fill(Qt::transparent);
                            pxInit = true;
                        }
                        return QIcon(px);
                    }
                }
                break;
        }
    }
    return QVariant();
}

int CommiModel::rowCount(const QModelIndex & parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
    {
        // list level
        return data_.size();
    }
    else
    {
        // item level
        if (parent.parent().isValid())
        {
            return 0;
        }

        // shop level
        // parent does not have id

        auto it = posOf(parent.row());
        if (it != data_.end())
        {
            int rowCount = it->sellList.size() + it->buyList.size();
            return rowCount;
        }
        else
        {
            return 0;
        }
    }
}

int CommiModel::columnCount(const QModelIndex & /*parent*/ /*= QModelIndex()*/) const
{
    return NumColumns1_;
}

void CommiModel::onGameEvent(const qlib::GameEvent & event)
{
    switch (event->type())
    {
        case BaseEvent::PlayerResolved:
        {
            PlayerResolvedEvent *e = static_cast<PlayerResolvedEvent*>(event.data());
            unsigned id = e->gId();
            auto position = std::find_if(data_.begin(), data_.end()
                                        , [id](const CommiShop & shop)
                                            { return id == shop.id; });
            if (position != data_.end())
            {
                int row = position - data_.begin();
                assert(row >= 0);
                QModelIndex index = createIndex(row, 0);
                emit dataChanged(index, index);
            }

            break;
        }
    }
}

std::vector<CommiShop>::const_iterator CommiModel::posOf(int row) const
{
    if (row < 0)
    {
        return data_.end();
    }
    return data_.begin() + row;
}

//

CommiFilterModel::CommiFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

CommiFilterModel::~CommiFilterModel()
{
}

void CommiFilterModel::setFilterFixedString(const QString &pattern)
{
    filterString_ = pattern;
    invalidateFilter();
}

bool CommiFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (filterString_.isEmpty())
    {
        return true;
    }

    QModelIndex index = sourceModel()->index(sourceRow, CommiModel::Columns::ItemName, sourceParent);

    if (!sourceParent.isValid())    // list level
    {
        // if any of children matches the filter, then current index matches the filter as well
        int nb = sourceModel()->rowCount(index);
        for (int i = 0; i < nb; ++i)
        {
            if (filterAcceptsRow(i, index))
            {
                return true;
            }
        }
    }
    else    // item level
    {
        QVariant data = sourceModel()->data(index);
        if (data.type() == QVariant::String)
        {
            return data.toString().contains(filterString_, Qt::CaseInsensitive);
        }
    }

    return false;
}

//

CommiView::CommiView(QSharedPointer<CatCtl> ctl, QWidget *parent)
    : QDialog(parent)
    , ui_(new Ui::Commi)
    , commiModel_(new CommiModel(ctl, this))
    , commiFilter_(new CommiFilterModel(this))
    , ctl_(ctl)
{
    ui_->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui_->progressBar->hide();

    commiFilter_->setSourceModel(commiModel_);

    ui_->treeView->setModel(commiFilter_);
    ui_->treeView->setColumnWidth(CommiModel::ItemName, 200);
    ui_->treeView->setColumnWidth(CommiModel::SellPrice, 80);
    ui_->treeView->setColumnWidth(CommiModel::Amount, 45);
    ui_->treeView->setColumnWidth(CommiModel::BuyPrice, 80);
    
    ui_->treeView->header()->setFont(ui_->treeView->font());
    ui_->treeView->sortByColumn(-1);

    commiModel_->setData(ctl_->getCommi());
    showLastUpdated();

    ui_->btnSaveMargins->setEnabled(false);

    /*std::vector<CommiShop> test;
    CommiShop s1;
    MarketItem m1;
    m1.id = 0x11;
    m1.name = L"Item1";
    MarketItem m2;
    m2.id = 0x12;
    m2.name = L"Item2";

    s1.id = 0x222;
    s1.sellList.push_back(m1);
    s1.buyList.push_back(m2);
    test.push_back(s1);
    s1.id = 0x333;
    test.push_back(s1);
    s1.id = 0x002;
    test.push_back(s1);
    commiModel_->setData(test);*/

    connect(ctl_.data(), &CatCtl::gameEvent, this, &CommiView::onGameEvent);
}

CommiView::~CommiView()
{
}

//

void CommiView::on_btnUpdate_clicked()
{
    ui_->progressBar->show();
    ui_->progressBar->setValue(0);

    ui_->treeView->setSortingEnabled(false);

    ctl_->updateCommi(true);
    ui_->btnUpdate->setEnabled(false);
    ui_->btnSaveMargins->setEnabled(false);
}

void CommiView::on_btnSaveMargins_clicked()
{
    auto now = QDateTime::currentDateTime();
    QString ts = now.toString("yyyy-MM-ddTHH.mm.ss");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save margins"),
                                                    QCoreApplication::instance()->applicationDirPath() + '/' + ts,
                                                    tr("Text (*.txt)"));
    if (fileName.isEmpty())
    {
        return;
    }

    struct ItemStat
    {
        bool        buyPriceSet = false;
        unsigned    buyPrice;
        bool        sellPriceSet = false;
        unsigned    sellPrice;
    };

    std::map<unsigned, ItemStat>    stats;

    auto commi = ctl_->getCommi();
    for (const auto shop : commi)
    {
        for (const auto item : shop.sellList)
        {
            auto & itemStat = stats[item.id];

            itemStat.sellPrice = itemStat.sellPriceSet ? std::min(itemStat.sellPrice, item.price) : item.price;
            itemStat.sellPriceSet = true;
        }

        for (const auto item : shop.buyList)
        {
            auto & itemStat = stats[item.id];

            itemStat.buyPrice = itemStat.buyPriceSet ? std::max(itemStat.buyPrice, item.price) : item.price;
            itemStat.buyPriceSet = true;
        }
    }
    QFile fw(fileName);
    if (!fw.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, "Error", "Failed to create file: " + fw.errorString());
        return;
    }

    int currentServer;
    auto servers = ctl_->getServers(currentServer);
    QString server = (servers.isEmpty() || currentServer >= servers.size()) ? "" : servers[currentServer];

    fw.write(QString("%1,%2,%3\n")
             .arg(server)
             .arg(now.date().day())
             .arg(now.time().toString("HH.mm.ss"))
             .toUtf8()
    );

    for (const auto itemStat : stats)
    {
        fw.write(QString("%1,%2,%3\n")
                 .arg(itemStat.first)
                 .arg(itemStat.second.sellPriceSet ? QString::number(itemStat.second.sellPrice) : "-")
                 .arg(itemStat.second.buyPriceSet ? QString::number(itemStat.second.buyPrice) : "-")
                 .toUtf8());
    }
}

void CommiView::on_leFilterItems_textChanged(const QString & text)
{
    commiFilter_->setFilterFixedString(text);
    if (text.isEmpty())
    {
        ui_->treeView->collapseAll();
    }
    else
    {
        ui_->treeView->expandToDepth(1);
    }
}

void CommiView::onGameEvent(const qlib::GameEvent & ev)
{
    BaseEvent *event = ev.data();

    switch (ev->type())
    {
        case BaseEvent::CommiListUpdated:
            commiModel_->setData(ctl_->getCommi());
            break;

        case BaseEvent::CommiShopsUpdateProgress:
        {
            const CommiShopsUpdateProgressEvent *e = static_cast<const CommiShopsUpdateProgressEvent *>(event);
            if (ui_->progressBar->isHidden())
            {
                ui_->progressBar->show();
            }
            if (ui_->btnUpdate->isEnabled())
            {
                ui_->btnUpdate->setEnabled(false);
            }
            ui_->progressBar->setValue(e->value());
            ui_->progressBar->setMaximum(e->maxValue());

            break;
        }

        case BaseEvent::CommiShopsUpdated:
        {
            ui_->progressBar->hide();
            commiModel_->setData(ctl_->getCommi());
            ui_->treeView->setSortingEnabled(true);

            ui_->btnUpdate->setEnabled(true);
            ui_->btnSaveMargins->setEnabled(true);
            showLastUpdated();

            break;
        }
    }
}

void CommiView::showLastUpdated()
{
    time_t lu = ctl_->commiLastUpdated();
    QString text = tr("Last updated: %1");
    ui_->lbLastUpdated->setText(lu == 0 ? text.arg(tr("never"))
                                : text.arg(QDateTime::fromTime_t(unsigned int(lu)).toString()));
}
