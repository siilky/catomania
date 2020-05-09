#include "StdAfx.h"

#include "historydb.h"


static const quint16 magic_ = 0xD1CE;
static const quint16 version_ = 0x0011;

// Obsolete types

struct HistoryItem00
{
    enum { Version = 0x0010 };

    enum Operation
    {
        OperationSell,
        OperationBuy,
    };

    HistoryItem00()
    {
    }
    HistoryItem00(quint32 t, Operation op, unsigned c, unsigned p)
        : time(t), operation(op), count(c), pricePerItem(p)
        , isActual(true)
    {
    }

    quint32     time;
    Operation   operation;
    unsigned    count;
    unsigned    pricePerItem;

    // для покупки означает, что продана еще не вся запись ()
    // для продажи - что участвует в формировании профита (профит неактивных был снят) 
    bool        isActual;
};

inline QDataStream & operator<<(QDataStream & stream, const HistoryItem00 & item)
{
    return stream << item.time << item.operation << item.count << item.pricePerItem << item.isActual;
}

inline QDataStream & operator>>(QDataStream & stream, HistoryItem00 & item)
{
    return stream >> item.time >> (int&) item.operation >> item.count >> item.pricePerItem >> item.isActual;
}

typedef QList<HistoryItem00>                      ItemHistory00;
typedef QMap<quint32 /*itemId*/, ItemHistory00>   TradeHistory00;

HistoryItem::HistoryItem(const HistoryItem00 & r)
    : time(r.time), operation((Operation)r.operation), count(r.count), pricePerItem(r.pricePerItem)
    , isActual(r.isActual)
    , actualCount(0)
{
}

//

HistoryDb::HistoryDb()
    : file_(0)
{
}

HistoryDb::~HistoryDb()
{
    close();
}

bool HistoryDb::open(const QString & name)
{
    if (file_)
    {
        delete file_;
        file_ = 0;
    }

    items_.clear();
    QScopedPointer<SafeFile> file(new SafeFile(name)); // will be deleted on failure and file_ kept zero

    QByteArray data;
    if (!file->read(data))
    {
        return false;
    }

    if (data.isNull())
    {
        // no file yet, its ok just treat all as empty
        file_ = file.take();
        return true;
    }

    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_5);

    quint16 m, v;
    stream >> m;
    if (m != magic_)
    {
        qWarning() << "History file magic is wrong:" << name;
        return false;
    }

    stream >> v;
    if (v > version_)
    {
        qWarning() << "History file version is too high:" << name;
        return false;
    }

    qint32 itemsNeed;
    stream >> itemsNeed;

    if (v != version_)
    {
        if (v == HistoryItem00::Version)
        {
            QMap<quint32 /*itemId*/, ItemHistory00> items;
            stream >> items;
            for (auto it = items.constBegin(); it != items.constEnd(); ++it)
            {
                const ItemHistory00 & i00 = it.value();

                ItemHistory i;
                std::copy(i00.begin(), i00.end(), std::back_inserter(i));
                items_.insert(it.key(), i);
            }
        }
        else
        {
            qWarning() << "History version file is unsupported:" << v;
            return false;
        }
    }
    else
    {
        stream >> items_;
    }

    if (itemsNeed != items_.size())
    {
        qWarning() << "History corrupted, records do not match:" << itemsNeed << "with" << items_.size();
        items_.clear();
        return false;
    }

    file_ = file.take();
    return true;
}

void HistoryDb::close()
{
    if (file_)
    {
        save();
    }
    delete file_;
    file_ = 0;
}

const HistoryDb::ItemHistory & HistoryDb::get(quint32 itemId) const
{
    auto it = items_.constFind(itemId);
    if (it == items_.end())
    {
        static const ItemHistory cs;
        return cs;
    }

    return *it;
}

const HistoryDb::TradeHistory & HistoryDb::get() const
{
    return items_;
}

bool HistoryDb::append(quint32 itemId, const HistoryItem & item)
{
    HistoryDb::ItemHistory & history = items_[itemId];
    history.append(item);

    // переактуализируем позиции покупки
    
    unsigned buyCount = 0;
    auto iLookback = history.begin();
    for (auto iCurrent = history.constBegin(); iCurrent != history.constEnd(); ++iCurrent)
    {
        const HistoryItem & i = *iCurrent;

        if (i.operation == HistoryItem::OperationSell)
        {
            unsigned count = i.count;

            do
            {
                while (iLookback->operation != HistoryItem::OperationBuy
                       && iLookback != iCurrent)
                {
                    ++iLookback;
                }
                if (iLookback == iCurrent)
                {
                    // no more buy items, skip this
                    break;
                }

                HistoryItem & buy = *iLookback;
                if (buyCount == 0)
                {
                    buyCount = buy.count;
                }

                if (buyCount <= count)
                {
                    buy.isActual = false;
                    count -= buyCount;
                    buyCount = 0;
                    ++iLookback;
                }
                else
                {
                    buyCount -= count;
                    count = 0;
                }
            } while (count > 0);
        }
    }

    return save();
}

void HistoryDb::resetProfit()
{
    for (auto it = items_.begin(); it != items_.end(); ++it)
    {
        HistoryDb::ItemHistory & history = it.value();

        unsigned lbCount = 0;

        do
        {
            // ищем опорную закупку. Пока не нашли, все продажи удаляются.
            while (history.size() > 0
                   && history.at(0).operation != HistoryItem::OperationBuy)
            {
                history.removeFirst();
            }
            if (history.size() < 2)
            {
                // переходим к следующему предмету
                break;
            }

            const HistoryItem & buy = history.at(0);
            lbCount = buy.count;

            // подбираем продажи в эту покупку. Если набирается на всю сумму, то их потом удалим
            
            for (int idxSell = 1; idxSell < history.size() && lbCount > 0; idxSell++)
            {
                HistoryItem & sell = history[idxSell];

                // покупки игнорируем на этом этаме
                if (sell.operation != HistoryItem::OperationSell)
                {
                    continue;
                }

                unsigned count = qMin(lbCount, (sell.actualCount != 0 ? sell.actualCount : sell.count));
                if (count <= lbCount)
                {
                    sell.isActual = false;
                }
                lbCount -= count;
            }

            if (lbCount == 0)
            {
                // если закупка целиком продана, то удалим опорную закупку и все связанные с ней продажи, и начнем заново

                unsigned rmCount = buy.count;
                int idxSell = 1;
                while (idxSell < history.size()
                        && rmCount > 0)
                {
                    HistoryItem & sell = history[idxSell];

                    if (sell.operation != HistoryItem::OperationSell)
                    {
                        idxSell ++;
                        continue;
                    }

                    unsigned & sellCount = (sell.actualCount != 0 ? sell.actualCount : sell.count);

                    unsigned count = qMin(rmCount, sellCount);
                    rmCount -= count;

                    if (count != sellCount)
                    {
                        // частичная продажа
                        sell.actualCount = sellCount - count;
                        if (sell.actualCount == 0)
                        {
                            // частично полностью продали
                            history.removeAt(idxSell);
                        }
                        else
                        {
                            idxSell ++;
                        }
                    }
                    else
                    {
                        history.removeAt(idxSell);
                    }
                }

                history.removeFirst();
            }
        } while (lbCount == 0);
    }

    save();
}

// void HistoryDb::truncate(quint32 itemId, quint32 time)
// {
//     // With QMap, the items are always sorted by key.
//     auto iHistory = items_.find(itemId);
//     if (iHistory == items_.end())
//     {
//         return;
//     }
// 
//     auto iUpper = iHistory->upperBound(time);
//     for (auto it = iHistory->begin(); it != iUpper;)
//     {
//         it = iHistory->erase(it);
//     }
// }

//

bool HistoryDb::save()
{
    if (!file_)
    {
        return false;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_5);

    stream << magic_ << version_ << items_.size() << items_;

    return file_->write(data);
}

//

void getIndicators(const HistoryDb::ItemHistory & history, int & stock, qint64 & value, qint64 & profit)
{
    stock = 0;
    value = 0;
    profit = 0;

    QList<HistoryItem> lookback;

    for (auto iCurrent = history.constBegin(); iCurrent != history.constEnd(); ++iCurrent)
    {
        const HistoryItem & item = *iCurrent;

        if (item.operation == HistoryItem::OperationBuy)
        {
            // закупки увеличивают сток и стоимость
            stock += item.count;
            value += qint64(item.count) * item.pricePerItem;

            lookback.append(item);
        }
        else
        {
            // продажа уменьшает сток, стоимость по lookback и формирует профит
            unsigned itemCount = (item.actualCount != 0 ? item.actualCount : item.count);
            while (!lookback.isEmpty() && itemCount > 0)
            {
                HistoryItem & lb = lookback[0];
                unsigned count = qMin(itemCount, lb.count);

                stock -= count;

                qint64 v = qint64(count) * item.pricePerItem;
                qint64 b = qint64(count) * lb.pricePerItem;
                value -= b;

                if (item.isActual)
                {
                    profit += (v - b);
                }

                itemCount -= count;
                lb.count -= count;
                if (lb.count == 0)
                {
                    // all sold from this record
                    lookback.takeFirst();
                }
            }
        }
    }
}

// qint64 getProfit(const HistoryDb::ItemHistory & history)
// {
//     qint64 profit = 0;
// 
//     for (auto it = history.begin(); it != history.end(); ++it)
//     {
//         qint64 value = qint64(it->count) * it->pricePerItem;
//         profit += (it->operation == HistoryItem::OperationBuy ? -value : value);
//     }
//     return profit;
// }
// 
// qint64 getProfit(const HistoryDb::TradeHistory & history)
// {
//     qint64 profit = 0;
//
//     for (auto it = history.begin(); it != history.end(); ++it)
//     {
//         profit += getProfit(it.value());
//     }
//     return profit;
// }

#if defined(QT_DEBUG)

void historyDbTest()
{
    HistoryDb db;
    assert(db.open("DBx01"));

    uint time = QDateTime::currentDateTime().toTime_t();
    quint32 id = 41072;

//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationBuy, 65, 155600));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationSell, 1, 193000));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationBuy, 70, 155600));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationBuy, 3, 155600));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationBuy, 10, 155600));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationBuy, 5, 155600));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationBuy, 10, 161200));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationSell, 30, 193000));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationSell, 30, 193000));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationSell, 92, 193000));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationSell, 10, 193000));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationBuy, 12, 161200));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationBuy, 10, 161200));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationBuy, 10, 162020));
//     db.append(id, HistoryItem(time, HistoryItem::Operation::OperationSell, 20/*22*/, 212000));

     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationBuy, 30, 2000));
     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationBuy, 50, 2500));
//     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationBuy, 100, 3000));
     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationSell, 10, 3500));
     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationSell, 15, 3500));
     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationSell, 30, 3500));
     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationSell, 10, 3500));
//     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationSell, 20, 4000));
//     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationSell, 20, 4000));
//     db.append(id, HistoryItem(time - qrand(), HistoryItem::Operation::OperationSell, 30, 4000));
// 
//     db.append(02, HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));
//     db.append(02, HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));
//     db.append(02, HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));
//     db.append(02, HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));
//     db.append(02, HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));
//     db.append(qrand(), HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));
//     db.append(qrand(), HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));
//     db.append(qrand(), HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));
//     db.append(qrand(), HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));
//     db.append(qrand(), HistoryItem(qrand(), HistoryItem::Operation(qrand() & 1), qrand(), qrand()));

//    db.truncate(02, 0x2000);
}

#endif
