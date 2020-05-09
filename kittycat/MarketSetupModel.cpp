
#include "stdafx.h"

#include "MarketSetupModel.h"
#include "util.h"
#include "itempropformatter.h"
#include "qlib/game/iconmanager.h"

//

static bool hasSellOption(const MarketItemSetup &item)
{
    return item.doSell.isTristate() || item.doSell;
}

static bool hasBuyOption(const MarketItemSetup &item)
{
    return item.doBuy.isTristate() || item.doBuy;
}

static bool hasSellAPOption(const MarketItemSetup &item)
{
    return item.sellAutoPrice.isTristate() || item.sellAutoPrice;
}

static bool hasBuyAPOption(const MarketItemSetup &item)
{
    return item.buyAutoPrice.isTristate() || item.buyAutoPrice;
}

//

const int MarketSetupModel::sizeHints[NumColumns_] = { 
    160, // Name = 0,
    45,  // Amount,
    35,  // IsSelling,
    30,  // SellAutoPrice,
    80,  // SellPrice,
    45,  // SellAmount,
    45,  // KeepMin,
    35,  // IsBuying,
    30,  // BuyAutoPrice,
    80,  // BuyPrice,
    45,  // BuyAmount,
    45,  // KeepMax,
    80,  // ApPriceStep,
    45,  // ApMinCount,
};    

MarketSetupModel::MarketSetupModel(QObject * parent)
    : QAbstractTableModel(parent)
    , itemsMovable_(false)
{
}

MarketSetupModel::~MarketSetupModel()
{
}

void MarketSetupModel::setData(const QList<MarketItemSetup> & data)
{
    if (data.isEmpty())
    {
        data_.clear();
        return;
    }

    beginResetModel();
    data_ = data;
    endResetModel();
}

bool MarketSetupModel::insertRows(int row, int count, const QModelIndex &/*parent*/ /*= QModelIndex()*/)
{
    qDebug() << "Add rows" << row << count;
    return false;
}

bool MarketSetupModel::insertRow(int row, const QModelIndex &/*parent*/ /*= QModelIndex()*/)
{
    qDebug() << "Add row" << row;
    return false;
}

bool MarketSetupModel::removeRows(int row, int count, const QModelIndex & /*parent*/ /*= QModelIndex()*/)
{
    qDebug() << "Remove rows" << row << count;
    return false;
}

bool MarketSetupModel::removeRow(int row, const QModelIndex &/*parent*/ /*= QModelIndex()*/)
{
    qDebug() << "Remove row" << row;
    return false;
}

QStringList MarketSetupModel::mimeTypes() const
{
    return QStringList() << "application/x-marketsetupdatalist";
}

QMimeData * MarketSetupModel::mimeData(const QModelIndexList &indexes) const
{
    QStringList types = mimeTypes();
    if (types.isEmpty())
    {
        return 0;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    for (QModelIndexList::ConstIterator it = indexes.begin(); it != indexes.end(); ++it)
    {
        stream << (*it).row();
    }

    QMimeData *mime = new QMimeData();
    mime->setData(types.at(0), data);
    return mime;
}

bool MarketSetupModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    qDebug() << "Drop data" << parent.row() << parent.column() << row << column << data->text();

    // col обычно = 0
    // дроп на пустое место в конце: row=-1 parent=-1
    // дроп на пустое место в начале: row=0 parent=-1
    // дроп между концом и пустым местом row=последний+1 parent=-1
    // дроп между ячеек row=нижний ряд parent=-1
    // дроп на ячейку row=-1 parent=ячейка

    QStringList types = mimeTypes();
    if (types.isEmpty())
    {
        return false;
    }

    int destRow = -1;
    bool insertBefore = false;
    if (parent.isValid())
    {
        destRow = parent.row();
    }
    else
    {
        insertBefore = true;
        destRow = row;
    }

    int srcRow = -1;
    QString format = types.at(0);
    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    if (!stream.atEnd())
    {
        stream >> srcRow;
    }

    if (srcRow < 0
        || srcRow > data_.size()
        || destRow > (data_.size() + 1)
        || (destRow == (data_.size() + 1) && !insertBefore))     // +1 от числа валидный случай
    {
        return false;
    }
    if (srcRow == destRow && action == Qt::MoveAction)  // дроп перед собой (на себя нельзя)
    {
        return true;
    }
    if (destRow == data_.size() && insertBefore)
    {
        // дроп между концом и пустым местом
        destRow = -1;
    }

    moveItems(data_.at(srcRow).slot, destRow >= 0 ? data_.at(destRow).slot : -1, action == Qt::CopyAction, insertBefore);
    return true;
}

QList<MarketItemSetup> MarketSetupModel::getData() const
{
    return data_;
}

void MarketSetupModel::setItemsMovable(bool isMovable)
{
    itemsMovable_ = isMovable;
}

int MarketSetupModel::columnWidth(int column)
{
    if (column < NumColumns_)
    {
        return sizeHints[column];
    }

    return -1;
}

//

QVariant MarketSetupModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                case Name:          return tr("Item");
                case Amount:        return tr("Amount");
                case IsSelling:     return tr("Sell");
                case SellAutoPrice: return tr("AP");
                case SellPrice:     return tr("Sell\nprice");
                case SellAmount:    return tr("Sell\namount");
                case KeepMin:       return tr("Keep\nmin");
                case IsBuying:      return tr("Buy");
                case BuyAutoPrice:  return tr("AP");
                case BuyPrice:      return tr("Buy\nprice");
                case BuyAmount:     return tr("Buy\nAmount");
                case KeepMax:       return tr("Keep\nmax");
                case ApPriceStep:   return tr("AP Price\nstep");
                case ApMinCount:    return tr("Min\ncount");
                default:            return QVariant();
            }

        case Qt::TextAlignmentRole:
            return Qt::AlignHCenter;
            break;

        default:
            break;
    }

    return QVariant();
}

QVariant MarketSetupModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) return Qt::ItemIsEnabled;
    assert(index.row() < data_.count());

    const MarketItemSetup & item = data_[index.row()];
    const int column = index.column();

    switch (role)
    {
        case Qt::EditRole:
            switch(column)
            {
                case Name:          
                    return QString::fromStdWString(item.item.name);
                case Amount:        
                    return item.item.count > 0 ?            QString::number(item.item.count) : QVariant();
                case SellPrice:     
                    return item.sellPrice.isTristate() ?    QVariant()
                            : (item.sellPrice > 0 ?         item.sellPrice.value() : QVariant());
                case SellAmount:    
                    return item.sellCount.isTristate() ?    QVariant()
                            : (item.sellCount > 0 ?         item.sellCount.value() : QVariant());
                case KeepMin:       
                    return item.keepMin.isTristate() ?      QVariant()
                            : (item.keepMin > 0 ?           item.keepMin.value() : QVariant());
                case BuyPrice:      
                    return item.buyPrice.isTristate() ?     QVariant()
                            : (item.buyPrice > 0 ?          item.buyPrice.value() : QVariant());
                case BuyAmount:     
                    return item.buyCount.isTristate() ?     QVariant()
                            : (item.buyCount > 0 ?          item.buyCount.value() : QVariant());
                case KeepMax:       
                    return item.keepMax.isTristate() ?      QVariant()
                            : (item.keepMax > 0 ?           item.keepMax.value() : QVariant());
                case ApPriceStep:
                    return item.apPriceStep.isTristate() ?  QVariant()
                        : (item.apPriceStep != 0 ?          item.apPriceStep.value() : QVariant());
                case ApMinCount:
                    return item.apMinCount.isTristate() ?   QVariant()
                        : (item.apMinCount > 0 ?            item.apMinCount.value() : QVariant());
            }
            break;

        case Qt::DisplayRole:
            switch(column)
            {
                case Name:          
                    return item.item.name.empty() ?         spacedNumber(item.item.id) : QString::fromStdWString(item.item.name);
                case Amount:        
                    return item.item.count > 0 ?            item.item.count : QVariant();
                case SellPrice:     
                    return item.sellPrice.isTristate() ?    QChar(0x2260)
                            : (item.sellPrice > 0 ?         item.sellPrice.value() : QVariant());
                case SellAmount:    
                    return item.sellCount.isTristate() ?    QChar(0x2260)
                            : (item.sellCount > 0 ?         item.sellCount.value() : QVariant());
                case KeepMin:       
                    return item.keepMin.isTristate() ?      QChar(0x2260)
                            : (item.keepMin > 0 ?           item.keepMin.value() : QVariant());
                case BuyPrice:      
                    return item.buyPrice.isTristate() ?     QChar(0x2260)
                            : (item.buyPrice > 0 ?          item.buyPrice.value() : QVariant());
                case BuyAmount:     
                    return item.buyCount.isTristate() ?     QChar(0x2260)
                            : (item.buyCount > 0 ?          item.buyCount.value() : QVariant());
                case KeepMax:       
                    return item.keepMax.isTristate() ?      QChar(0x2260)
                            : (item.keepMax > 0 ?           item.keepMax.value() : QVariant());
                case ApPriceStep:
                    return item.apPriceStep.isTristate() ?  QChar(0x2260)
                        : (item.apPriceStep != 0 ?          item.apPriceStep.value() : QVariant());
                case ApMinCount:
                    return item.apMinCount.isTristate() ?   QChar(0x2260)
                        : (item.apMinCount > 0 ?            item.apMinCount.value() : QVariant());
            }
            break;

        case Qt::CheckStateRole:
            if (column == IsSelling)
            {
                return item.doSell.isTristate() ? Qt::PartiallyChecked
                    : (item.doSell ? Qt::Checked : Qt::Unchecked);
            }
            else if (column == SellAutoPrice)
            {
                return item.sellAutoPrice.isTristate() ? Qt::PartiallyChecked
                    : (item.sellAutoPrice ? Qt::Checked : Qt::Unchecked);
            }
            else if (column == IsBuying)
            {
                return item.doBuy.isTristate() ? Qt::PartiallyChecked
                    : (item.doBuy ? Qt::Checked : Qt::Unchecked);
            }
            else if (column == BuyAutoPrice)
            {
                return item.buyAutoPrice.isTristate() ? Qt::PartiallyChecked
                    : (item.buyAutoPrice ? Qt::Checked : Qt::Unchecked);
            }
            break;

        case Qt::TextAlignmentRole:
            switch (column)
            {
                case Name:
                    return int(Qt::AlignLeft | Qt::AlignVCenter);
                case IsSelling:
                case SellAutoPrice:
                case IsBuying:
                case BuyAutoPrice:
                    return Qt::AlignCenter;
                default:
                    return int(Qt::AlignRight | Qt::AlignVCenter);
            }
            break;

        case Qt::BackgroundRole:
        {
            QPalette palette;
            switch (column)
            {
                case SellPrice:
                case SellAmount:
                    // раскраска ошибочных ячеек
                    if ( ! item.doSell.isTristate() && ! item.doBuy.isTristate()
                        && item.doSell && item.doBuy
                        && item.sellPrice > 0
                        && item.buyPrice > 0
                        && item.sellPrice < item.buyPrice)
                    {
                        return QColor(0xFFC0C0);
                    }
                    // no break
                case KeepMin:
                    // раскраска неактивных (так же для двух колонок выше)
                    if (!hasSellOption(item))
                    {
                        return palette.color(QPalette::Midlight);
                    }
                    break;

                case BuyPrice:
                case BuyAmount:
                    // раскраска ошибочных ячеек
                    if (!item.doSell.isTristate() && !item.doBuy.isTristate()
                        && item.doSell && item.doBuy
                        && item.sellPrice > 0
                        && item.buyPrice > 0
                        && item.sellPrice < item.buyPrice)
                    {
                        return QColor(0xFFC0C0);
                    }
                    // no break
                case KeepMax:
                    // раскраска неактивных (так же для двух колонок выше)
                    if (!hasBuyOption(item))
                    {
                        return palette.color(QPalette::Midlight);
                    }
                    break;

                case ApPriceStep:
                case ApMinCount:
                    if (!(hasSellOption(item) && hasSellAPOption(item))
                        && !(hasBuyOption(item) && hasBuyAPOption(item)))
                    {
                        return palette.color(QPalette::Midlight);
                    }
                    break;
            }
            break;
        }

        case Qt::ToolTipRole:
            if (column == Name)
            {
                QString text;
                if (item.slot >= 0)
                {
                    text += tr("Inventory slot: %1").arg(item.slot);
                }

                if (item.item.props)
                {
                    if (!text.isEmpty())
                    {
                        text.append("\n");
                    }
                    text += itemPropsToString(QString::fromStdWString(item.item.name), item.item.props.get());
                }

                if (item.item.timeLimit != 0)
                {
                    int diff = item.item.timeLimit - QDateTime::currentDateTimeUtc().toTime_t();
                    if (diff > 0)
                    {
                        if (!text.isEmpty())
                        {
                            text += "<br>";
                        }

                        int days = diff / (60 * 60 * 24);   diff -= days * (60 * 60 * 24);
                        int hours = diff / (60 * 60);       diff -= hours * (60 * 60);
                        int minutes = diff / 60;
                        text += tr("Time left: %1d %2:%3")
                            .arg(days)
                            .arg(hours, 2, 10, QChar('0'))
                            .arg(minutes, 2, 10, QChar('0'));
                    }
                }
                return text;
            }
            break;

        case Qt::DecorationRole:
            if (column == Name)
            {
                // return QIcon so itemview will resize it to row height
                QImage image;
                if ( ! item.item.icon.empty())
                {
                    QTextCodec *tc = QTextCodec::codecForName("GBK");
                    QString icon = tc->toUnicode(item.item.icon.c_str(), item.item.icon.size());
                    image = IconManager::instance()->getIconByPath(icon);
                }

                if ( ! image.isNull())
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

        case MarketItemRole:
            return QVariant::fromValue((void*)&item);
    }

    return QVariant();
}

bool MarketSetupModel::setData(const QModelIndex & pos, const QVariant & value, int role)
{
    if (!pos.isValid()) return false;
    assert(pos.row() < data_.count());

    if (role != Qt::EditRole
        && role != Qt::CheckStateRole)
    {
        return false;
    }

    const int column = pos.column();
    const int row = pos.row();

    MarketItemSetup & item = data_[row];

    if (role == Qt::CheckStateRole)
    {
        if (value.type() == QVariant::Int)
        {
            int state = value.toInt();
            switch (column)
            {
                case IsSelling:
                    item.doSell = state == Qt::Checked;
                    emit dataChanged(index(row, SellPrice), index(row, KeepMin));
                    return true;

                case SellAutoPrice:
                    item.sellAutoPrice = state == Qt::Checked;
                    // update row to end
                    emit dataChanged(index(row, SellAutoPrice), index(row, NumColumns_ - 1));
                    return true;

                case IsBuying:
                    item.doBuy = state == Qt::Checked;
                    emit dataChanged(index(row, BuyPrice), index(row, KeepMax));
                    return true;

                case BuyAutoPrice:
                    item.buyAutoPrice = state == Qt::Checked;
                    // update row to end
                    emit dataChanged(index(row, BuyAutoPrice), index(row, NumColumns_ - 1));
                    return true;
            }
        }
    }
    else
    {
        switch (column)
        {
            case SellPrice:
            case SellAmount:
            case KeepMin:
            case BuyPrice:
            case BuyAmount:
            case KeepMax:
            case ApMinCount:
            case ApPriceStep:
            {
                int num;
                bool isOk = false;
                if (value.type() == QVariant::String)
                {
                    // раньше нвдо было делать когда числа были строками
                    QString str = value.toString().remove(' ').trimmed();
                    //unsigned num = str.isEmpty() ? -1 : str.toInt();
                    num = str.toInt(&isOk);
                }
                else if (value.type() == QVariant::Int || value.type() == QVariant::UInt)
                {
                    num = value.toInt();
                    isOk = true;
                }
                else
                {
                    break;
                }

                switch (pos.column())
                {
                    // empty string does not override tristate values
                    case SellPrice:     if (isOk || !item.sellPrice.isTristate())   item.sellPrice   = num; break;
                    case SellAmount:    if (isOk || !item.sellCount.isTristate())   item.sellCount   = num; break;
                    case KeepMin:       if (isOk || !item.keepMin.isTristate())     item.keepMin     = num; break;
                    case BuyPrice:      if (isOk || !item.buyPrice.isTristate())    item.buyPrice    = num; break;
                    case BuyAmount:     if (isOk || !item.buyCount.isTristate())    item.buyCount    = num; break;
                    case KeepMax:       if (isOk || !item.keepMax.isTristate())     item.keepMax     = num; break;
                    case ApPriceStep:   if (isOk || !item.apPriceStep.isTristate()) item.apPriceStep = num; break;
                    case ApMinCount:    if (isOk || !item.apMinCount.isTristate())  item.apMinCount  = num; break;
                }

                emit dataChanged(pos, pos);
                return true;
            }

            case Name:
            case Amount:
            default:
                break;
        }
    }

    return false;
}



Qt::ItemFlags MarketSetupModel::flags(const QModelIndex & index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;           // enable free-space drop
    assert(index.row() < data_.count());

    Qt::ItemFlags flags = Qt::ItemIsEnabled;

    const MarketItemSetup & item = data_.at(index.row());

    switch(index.column())
    {
        case Name:
            flags |= Qt::ItemIsSelectable;
            if (itemsMovable_)
            {
                flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
            }
            break;

        case Amount:
            // no change
            break;

        case IsSelling:
        case IsBuying:
            flags |= Qt::ItemIsUserCheckable;
            break;

        case SellAutoPrice:
            if (hasSellOption(item))
            {
                flags |= Qt::ItemIsUserCheckable;
            }
            else
            {
                flags &= ~Qt::ItemIsEnabled;
            }
            break;

        case BuyAutoPrice:
            if (hasBuyOption(item))
            {
                flags |= Qt::ItemIsUserCheckable;
            }
            else
            {
                flags &= ~Qt::ItemIsEnabled;
            }
            break;

        case SellPrice:
        case SellAmount:
        case KeepMin:
            if (hasSellOption(item))
            {
                flags |= Qt::ItemIsEditable;
            }
            else
            {
                flags &= ~Qt::ItemIsEnabled;
            }
            break;

        case BuyPrice:
        case BuyAmount:
        case KeepMax:
            if (hasBuyOption(item))
            {
                flags |= Qt::ItemIsEditable;
            }
            else
            {
                flags &= ~Qt::ItemIsEnabled;
            }
            break;

        case ApPriceStep:
        case ApMinCount:
            if ((hasSellOption(item) && hasSellAPOption(item))
                || (hasBuyOption(item) && hasBuyAPOption(item)))
            {
                flags |= Qt::ItemIsEditable;
            }
            else
            {
                flags &= ~Qt::ItemIsEnabled;
            }
            break;
    }

    return flags;
}

int MarketSetupModel::rowCount(const QModelIndex & /*parent*/) const
{
    return data_.count();
}

int MarketSetupModel::columnCount(const QModelIndex & /*parent*/) const
{
    return NumColumns_;
}

Qt::DropActions MarketSetupModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

//

void CenteredItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QStyleOptionViewItem viewOption(option);

    if (viewOption.state & QStyle::State_HasFocus)
        viewOption.state = viewOption.state ^ QStyle::State_HasFocus;

    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

    QRect newRect = QStyle::alignedRect( option.direction
                                        , Qt::AlignCenter
                                        , QSize(option.decorationSize.width() + 5, option.decorationSize.height())
                                        , QRect( option.rect.x() + textMargin
                                                , option.rect.y()
                                                , option.rect.width() - (2 * textMargin)
                                                , option.rect.height()));
    viewOption.rect = newRect;

    QStyledItemDelegate::paint(painter, viewOption, index);
}

bool CenteredItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);
    if (model == NULL || event == NULL)
    {
        return false;
    }

    // make sure that the item is checkable
    Qt::ItemFlags flags = model->flags(index);
    if (!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled))
        return false;

    // make sure that we have a check state
    QVariant value = index.data(Qt::CheckStateRole);
    if (!value.isValid())
        return false;

    // make sure that we have the right event type
    if (event->type() == QEvent::MouseButtonRelease)
    {
        const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        QRect checkRect = QStyle::alignedRect( option.direction
                                             , Qt::AlignCenter
                                             , option.decorationSize
                                             , QRect( option.rect.x() + (2 * textMargin)
                                                    , option.rect.y()
                                                    , option.rect.width() - (2 * textMargin)
                                                    , option.rect.height()));
        if (!checkRect.contains(static_cast<QMouseEvent*>(event)->pos()))
            return false;
    }
    else if (event->type() == QEvent::KeyPress)
    {
        if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space
            && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked ? Qt::Unchecked : Qt::Checked);

    return model->setData(index, state, Qt::CheckStateRole);
}

void RenderLinkDelegate::initStyleOption(QStyleOptionViewItem * option, const QModelIndex & index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    if (option->state & QStyle::State_MouseOver)
    {
        option->font.setUnderline(true);
        option->palette.setColor(QPalette::Text, option->palette.link().color());
    }
}

QString SpacedNumberDelegate::displayText(const QVariant &value, const QLocale & locale) const
{
    switch (value.type())
    {
        case QVariant::Int:
            return spacedNumber(value.toInt());
        case QVariant::UInt:
            return spacedNumber(value.toUInt());
        case QVariant::LongLong:
            return spacedNumber(value.toLongLong());
        default:
            return QStyledItemDelegate::displayText(value, locale);
            break;
    }
}
