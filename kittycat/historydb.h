#ifndef historydb_H
#define historydb_H

#include "qlib\safefile.hpp"

struct HistoryItem00;

struct HistoryItem
{
    enum Operation
    {
        OperationSell,
        OperationBuy,
    };

    HistoryItem()
    {}
    HistoryItem(quint32 t, Operation op, unsigned c, unsigned p) 
        :  time(t), operation(op), count(c), pricePerItem(p)
        , isActual(true)
        , actualCount(0)
    { }

    HistoryItem(const HistoryItem00 &);

    quint32     time;
    Operation   operation;
    unsigned    count;
    unsigned    pricePerItem;

    // ��� ������� ��������, ��� ������� ��� �� ��� ������ ()
    // ��� ������� - ��� ��������� � ������������ ������� (������ ���������� ��� ����) 
    bool        isActual;
    unsigned    actualCount;    // ��� ��������� ������ ���������� ���������� �����, ������� ����� ��������� (������� �� �������� � ���������� ������� �����)
};

inline QDataStream & operator<<(QDataStream & stream, const HistoryItem & item)
{
    return stream << item.time << item.operation << item.count << item.pricePerItem << item.isActual << item.actualCount;
}

inline QDataStream & operator>>(QDataStream & stream, HistoryItem & item)
{
    return stream >> item.time >> (int&) item.operation >> item.count >> item.pricePerItem >> item.isActual >> item.actualCount;
}

//

class HistoryDb
{
public:
    typedef QList<HistoryItem>                      ItemHistory;    // ordered by time ascending
    typedef QMap<quint32 /*itemId*/, ItemHistory>   TradeHistory;

    HistoryDb();
    ~HistoryDb();

    bool open(const QString & name);
    void close();

    const ItemHistory & get(quint32 itemId) const;
    const TradeHistory & get() const;

    bool append(quint32 itemId, const HistoryItem & item);
    void resetProfit();
//     void truncate(quint32 itemId, quint32 time);        // delete all records before time

private:
    bool save();

    SafeFile    *file_;

    TradeHistory    items_;
};


// stock - �������� ����� ��������� �� ��������� (��������� - ���������)
// value - �������� ��������� ����� �� �������� LIFO
// profit - ���������� ��������� �� �������� ��� ��������� (LIFO)
void getIndicators(const HistoryDb::ItemHistory & history, int & stock, qint64 & value, qint64 & profit);

// qint64 getProfit(const HistoryDb::ItemHistory & history);
// qint64 getProfit(const HistoryDb::TradeHistory & history);

#if defined(QT_DEBUG)
void historyDbTest();
#endif

#endif
