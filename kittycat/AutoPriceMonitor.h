#ifndef AUTOPRICEMONITOR_H
#define AUTOPRICEMONITOR_H

#include <QObject>
#include <QTimer>

#include "PWCatsApi.h"

class CatCtl;
class MarketItemSetup;

class AutoPriceMonitor : public QObject
{
    Q_OBJECT
public:
    AutoPriceMonitor(QObject *parent = 0);
    ~AutoPriceMonitor();

    // немного костылей на время, пока CalCtl не умеет принимать дополнительные обьекты в конструктор
    static AutoPriceMonitor * instance();

public slots:
    void addMonitoring(CatCtl *ctl);
    void removeMonitoring(CatCtl *ctl);

private slots:
    void catDectroyed(QObject *obj);
    void updatePrices();
    void requestCompleted();

private:
    struct Request
    {
        Request(int serverId, unsigned itemId)
            : request(new PWCatsPriceHistoryRequest(serverId, itemId))
            , retryCount(0)
        { }

        bool isFailed() const
        {
            return retryCount > MaxRetries;
        }

        QSharedPointer<PWCatsPriceHistoryRequest>   request;
        int retryCount;

        static const int MaxRetries = 3;
    };

    typedef QMap<unsigned /*itemId*/, Request /*request*/>   HistoryRequestMap;

    void updateServerCats(int serverId);

    QTimer  updateTimer_;

    QList<CatCtl *>  monitoring_;
    PWCatsApi       *api_;

    QMap<int /*server*/, HistoryRequestMap> requestCache_;
};


#if defined(_DEBUG)
void testRemoveProfit();
#endif

#endif
