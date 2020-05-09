
#include "stdafx.h"

#include "AutoPriceMonitor.h"
#include "catctl.h"
#include "qlib\vmp.h"


static std::tuple<unsigned, unsigned> getOptimalPrices(const MarketItemSetup & item,
                                                       PWCatsPriceHistoryRequest *history,
                                                       const QString & myName);


AutoPriceMonitor::AutoPriceMonitor(QObject *parent)
    : QObject(parent)
{
//     VMProtectBeginVirtualization(__FUNCTION__);
    if (!Vmp::hasOption(Vmp::OptionAutoprice))
    {
        return;
    }

    api_ = new PWCatsApi(this);
    updateTimer_.setInterval(1000 * 60 * 60);   // 1h
    connect(&updateTimer_, &QTimer::timeout, this, &AutoPriceMonitor::updatePrices);
    updateTimer_.start();

//     VMProtectEnd();
}

AutoPriceMonitor::~AutoPriceMonitor()
{
}

AutoPriceMonitor * AutoPriceMonitor::instance()
{
    static AutoPriceMonitor m;
    return &m;
}

void AutoPriceMonitor::addMonitoring(CatCtl *ctl)
{
    if (!monitoring_.contains(ctl))
    {
        monitoring_.append(ctl);
        connect(ctl, &QObject::destroyed, this, &AutoPriceMonitor::catDectroyed);
    }
}

void AutoPriceMonitor::removeMonitoring(CatCtl *ctl)
{
    monitoring_.removeAll(ctl);
}

void AutoPriceMonitor::catDectroyed(QObject *obj)
{
    monitoring_.removeAll(static_cast<CatCtl*>(obj));
}

//

void AutoPriceMonitor::updatePrices()
{
    requestCache_.clear();

    for (CatCtl * cat : monitoring_)
    {
        int serverId = cat->getCurrentServerAsPwcatsIndex();
        HistoryRequestMap & requests = requestCache_[serverId];

        QList<MarketItemSetup>  shopItems = cat->getShopItems();
        for (const MarketItemSetup & item : shopItems)
        {
            if (!(item.doSell && item.sellAutoPrice)
                && !(item.doBuy && item.buyAutoPrice))
            {
                // not tracking item
                continue;
            }

            if (requests.find(item.item.id) == requests.end())
            {
                Request request(serverId, item.item.id);
                connect(request.request.data(), &PWCatsPriceHistoryRequest::finished, this, &AutoPriceMonitor::requestCompleted);
                api_->get(request.request.data());
                requests.insert(item.item.id, request);
            }
            else
            {
                // already in cache
            }
        }
    }
}

void AutoPriceMonitor::requestCompleted()
{
    PWCatsPriceHistoryRequest *r = qobject_cast<PWCatsPriceHistoryRequest*>(sender());
    if (r == 0)
    {
        assert(0);
        return;
    }

    // check if all requests for this server are completed

    int serverId = r->serverId();
    HistoryRequestMap & requests = requestCache_[serverId];

    bool allCompleted = true;
    for (auto it = requests.begin(); it != requests.end(); ++it)
    {
        if (it->request == r)
        {
            // take a decision on this
            it->retryCount++;
            if (!it->request->isOk()
                && !it->isFailed())
            {
                qDebug() << "Retried request" << it->request->itemId() << it->request->serverId();
                api_->get(it->request.data());
                allCompleted = false;
            }
            r = 0;  // текущий обработан

            if (!allCompleted)
            {
                // нет смысла просматривать дальше
                break;
            }
        }
        else
        {
            // take a decision on others
            if (!it->request->isFinished())
            {
                allCompleted = false;
                if (r == 0)
                {
                    break;
                }
            }
        }
    }

    if (allCompleted)
    {
        updateServerCats(serverId);
    }
}

void AutoPriceMonitor::updateServerCats(int serverId)
{
    HistoryRequestMap & requests = requestCache_[serverId];
    for (CatCtl * cat : monitoring_)
    {
        int server = cat->getCurrentServerAsPwcatsIndex();
        if (server != serverId)
        {
            continue;
        }

        // get my name
        QString myName;

        CatCtl::AccountSelection current;
        QList<CatCtl::Account> accounts = cat->getAccounts(current);
        if (current.accountIndex >= 0 && current.accountIndex < accounts.size()
            && current.characterIndex >= 0 && current.characterIndex < accounts.at(current.accountIndex).characters.size())
        {
            myName = accounts.at(current.accountIndex).characters.at(current.characterIndex);
        }
        assert(!myName.isEmpty());

        QList<MarketItemSetup>  shopItems = cat->getShopItems();
        for (MarketItemSetup & item : shopItems)
        {
            if (!(item.doSell && item.sellAutoPrice)
                && !(item.doBuy && item.buyAutoPrice))
            {
                // not tracking item
                continue;
            }

            auto ir = requests.find(item.item.id);
            if (ir != requests.end())
            {
                if (!ir->isFailed())
                {
                    unsigned sellPrice, buyPrice;
                    std::tie(sellPrice, buyPrice) = getOptimalPrices(item, ir->request.data(), myName);
                    unsigned stockPrice = cat->getStockBuyPrice(item.item.id);

                    if (sellPrice > 0 
                        && sellPrice <= buyPrice)
                    {
                        cat->error(tr("%1 price(s) wont be changed to suspicious (%2 %3)")
                                   .arg(QString::fromStdWString(item.item.name))
                                   .arg(sellPrice)
                                   .arg(buyPrice));
                    }
                    else if (stockPrice > 0 
                             && sellPrice > 0
                             && sellPrice < stockPrice)
                    {
                        cat->error(tr("%1 price(s) wont be changed to undersell stock (%2 %3 stock %4)")
                                   .arg(QString::fromStdWString(item.item.name))
                                   .arg(sellPrice)
                                   .arg(buyPrice)
                                   .arg(stockPrice)
                                   );
                    }
                    else
                    {
                        QString notice;
                        if ((item.doSell
                             && sellPrice > 0
                             && sellPrice != unsigned(item.sellPrice)))
                        {
                            notice += tr(" sell price %1 to %2").arg(item.sellPrice).arg(sellPrice);
                            item.sellPrice = sellPrice;
                        }
                        if ((item.doBuy
                             && buyPrice > 0
                             && buyPrice != unsigned(item.buyPrice)))
                        {
                            notice += tr(" buy price %1 to %2").arg(item.buyPrice).arg(buyPrice);
                            item.buyPrice = buyPrice;
                        }
                        if (!notice.isNull())
                        {
                            notice = tr("%1 changed:%2")
                                .arg(QString::fromStdWString(item.item.name))
                                .arg(notice);
                            cat->message(notice);
                        }
                    }
                }
                else
                {
                    cat->error(tr("Failed to get price data of %1").arg(QString::fromStdWString(item.item.name)));
                }
            }
        }

        cat->setShopItems(shopItems);
    }
}

//

// фильтруем старые и наши позиции
static void filterUnneeded(std::vector<PWCatsPriceHistoryRequest::PriceElement> & list, const QString & myName)
{
    QDateTime now = QDateTime::currentDateTime();
    unsigned lowestTs = now.addSecs(-(60 * 60 * 12)).toTime_t();

    for (auto it = list.begin(); it != list.end();)
    {
        if (it->name == myName
            || (it->ts > 0 && it->ts < lowestTs))
        {
            it = list.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// выбирает элемент из списка по минимальносу количеству (список должен быть отсортирован в нужном порядке)
static unsigned getOptimalOfList(const std::vector<PWCatsPriceHistoryRequest::PriceElement> & list, int minCount)
{
    if (minCount < 0)
    {
        minCount = 0;
    }
    if (list.size() <= 3)
    {
        // минимальное количестко позиций на рынке, меньше которого опасно маневрировать
        return 0;
    }
    for (const auto & entry : list)
    {
        if (entry.count >= unsigned(minCount))
        {
            return entry.price;
        }
    }
    return 0;
}

static void markFilterList(std::vector<PWCatsPriceHistoryRequest::PriceElement> & filterList
                           , const std::vector<PWCatsPriceHistoryRequest::PriceElement> & refList
                           , std::function<bool(const PWCatsPriceHistoryRequest::PriceElement &, const PWCatsPriceHistoryRequest::PriceElement &)> comp)
{
    if (filterList.empty()
        || refList.empty())
    {
        return;
    }

    // находим ближайшую позицию мз референс листа, и помечаем эту для удаления, если расстояние до него меньше трех по своему листу
    // листы отсортированы по порядку элементов 
    //
    //  O   O   O   0    O   O   O           O
    // X   XX  X   X    O   X   X   .....   X
    //  X  O   XX  X    O    X  X           X
    // O   O   O   XX   O   X    X           X
    // O       O   O     X   X   X          X
    //             O    O   O   O           X
    //             O                         X
    //                                      X
    //                                      X
    //                                       X
    //                                      O

    auto iList = filterList.begin();
    do
    {
        auto iRef = refList.begin();

        // находим ближайший реф
        while (iRef != refList.end()
               && !comp(*iList, *iRef))
        {
            ++iRef;
        };

        // нашли меньший в рефлисте. Нам надо взять предыдущий и посчитать длину в нашем
        if (iRef == refList.begin())
        {
            // нет больших элементов
            break;
        }

        --iRef;
        auto iMy = iList;
        while (iMy != filterList.end()
               && comp(*iRef, *iMy))
        {
            ++iMy;
        }

        if ((iMy - iList) < 3)
        {
            iList->count = 0;
            ++iList;
            // удаляем этот и переходим к следующему
        }
        else
        {
            // дальше не исчем, считая что в этом списке профита не осталось
            break;;
        }
    } while (iList != filterList.end());
}

static void removeMarkedItems(std::vector<PWCatsPriceHistoryRequest::PriceElement> & list)
{
    for (auto i = list.begin(); i != list.end();)
    {
        if (i->count == 0)
        {
            i = list.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

static std::tuple<unsigned, unsigned> getOptimalPrices(const MarketItemSetup & item,
                                                       PWCatsPriceHistoryRequest * history,
                                                       const QString & myName)
{
    assert(item.sellAutoPrice || item.buyAutoPrice);

    // фильтр профита работает до всех прочих проверок

    std::vector<PWCatsPriceHistoryRequest::PriceElement> sellList { history->sellList };
    sellList.insert(sellList.end(), history->comSellList.begin(), history->comSellList.end());
    std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList = history->buyList;
    buyList.insert(buyList.end(), history->comBuyList.begin(), history->comBuyList.end());
    filterUnneeded(sellList, myName);
    filterUnneeded(buyList, myName);

    auto lessPrice =
        [](const PWCatsPriceHistoryRequest::PriceElement & l, const PWCatsPriceHistoryRequest::PriceElement & r)
    {
        return l.price < r.price;
    };
    auto greaterPrice =
        [](const PWCatsPriceHistoryRequest::PriceElement & l, const PWCatsPriceHistoryRequest::PriceElement & r)
    {
        return l.price > r.price;
    };

    // сортируем по возрастанию - продажа должна быть самой дешевой
    std::sort(sellList.begin(), sellList.end(), lessPrice);
    // сортируем по убыванию - покупка должна быть самой дорогой
    std::sort(buyList.begin(), buyList.end(), greaterPrice);

    // filter Profit
    markFilterList(sellList, buyList, greaterPrice);
    markFilterList(buyList, sellList, lessPrice);
    removeMarkedItems(sellList);
    removeMarkedItems(buyList);

    unsigned sellPrice = 0, buyPrice = 0;

    if (sellPrice == 0 && item.sellAutoPrice)
    {
        sellPrice = getOptimalOfList(sellList, item.apMinCount);
    }

    if (buyPrice == 0 && item.buyAutoPrice)
    {
        buyPrice = getOptimalOfList(buyList, item.apMinCount);
    }

    // добавляем шаг в зависимости от настройки

    if (item.apPriceStep != 0)
    {
        // тут надо осторожно с unsigbed и signed

        int sellP = sellPrice;
        if (sellP > 0
            && (item.apPriceStep > 0 && sellP > item.apPriceStep
                || item.apPriceStep < 0))
        {
            // продаем дешевле
            sellP -= item.apPriceStep;
            if (sellP > 0)
            {
                sellPrice = sellP;
            }
        }

        int buyP = buyPrice;
        if (buyP > 0
            && (item.apPriceStep > 0
                || (item.apPriceStep < 0 && buyP > -item.apPriceStep)))
        {
            // покупаем дороже
            buyP += item.apPriceStep;
            if (buyP > 0)
            {
                buyPrice = buyP;
            }
        }
    }

    return std::tuple<unsigned, unsigned>(sellPrice, buyPrice);
}

#if defined(_DEBUG)
static void testLists(std::vector<PWCatsPriceHistoryRequest::PriceElement> & sellList
                     , std::vector<PWCatsPriceHistoryRequest::PriceElement> & buyList
                     )
{
    auto lessPrice =
        [](const PWCatsPriceHistoryRequest::PriceElement & l, const PWCatsPriceHistoryRequest::PriceElement & r)
    {
        return l.price < r.price;
    };
    auto greaterPrice =
        [](const PWCatsPriceHistoryRequest::PriceElement & l, const PWCatsPriceHistoryRequest::PriceElement & r)
    {
        return l.price > r.price;
    };

    markFilterList(sellList, buyList, greaterPrice);
    markFilterList(buyList, sellList, lessPrice);
    removeMarkedItems(sellList);
    removeMarkedItems(buyList);
}

void testRemoveProfit()
{
    const std::vector<PWCatsPriceHistoryRequest::PriceElement> sellList_
    {
        { 100, 10 },
        { 110, 10 },
        { 120, 10 },
        { 130, 10 },
        { 140, 10 },
        { 150, 10 },
    };
    const size_t sellSize = sellList_.size();

    {
        std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList
        {
            { 90, 10 },
            { 80, 10 },
            { 70, 10 },
        };
        auto sellList(sellList_);
        testLists(sellList, buyList);
        assert(sellList.size() == sellSize);
        assert(buyList.size() == 3);
    }
    {
        // первый равный 
        std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList
        {
            { 100, 10 },
            { 80, 10 },
            { 70, 10 },
        };
        auto sellList(sellList_);
        testLists(sellList, buyList);
        assert(sellList.size() == sellSize - 1 && sellList[0].price == 110);
        assert(buyList.size() == 2 && buyList[0].price == 80);
    }
    {
        // первый между
        std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList
        {
            { 105, 10 },
            { 80, 10 },
            { 70, 10 },
        };
        auto sellList(sellList_);
        testLists(sellList, buyList);
        assert(sellList.size() == sellSize - 1 && sellList[0].price == 110);
        assert(buyList.size() == 2 && buyList[0].price == 80);
    }
    {
        // второй
        std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList
        {
            { 110, 10 },
            { 80, 10 },
            { 70, 10 },
        };
        auto sellList(sellList_);
        testLists(sellList, buyList);
        assert(sellList.size() == sellSize - 2 && sellList[0].price == 120);
        assert(buyList.size() == 2 && buyList[0].price == 80);
    }
    {
        // третий равный
        std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList
        {
            { 120, 10 },
            { 80, 10 },
            { 70, 10 },
        };
        auto sellList(sellList_);
        testLists(sellList, buyList);
        assert(sellList.size() == sellSize - 3 && sellList[0].price == 130);
        assert(buyList.size() == 2 && buyList[0].price == 80);
    }
    {
        // третий
        std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList
        {
            { 125, 10 },
            { 80, 10 },
            { 70, 10 },
        };
        auto sellList(sellList_);
        testLists(sellList, buyList);
        assert(sellList.size() == sellSize && sellList[0].price == 100);
        assert(buyList.size() == 2 && buyList[0].price == 80);
    }
    {
        // четвертый равный
        std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList
        {
            { 130, 10 },
            { 80, 10 },
            { 70, 10 },
        };
        auto sellList(sellList_);
        testLists(sellList, buyList);
        assert(sellList.size() == sellSize && sellList[0].price == 100);
        assert(buyList.size() == 2 && buyList[0].price == 80);
    }
    {
        // два
        std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList
        {
            { 115, 10 },
            { 105, 10 },
            { 80, 10 },
            { 70, 10 },
        };
        auto sellList(sellList_);
        testLists(sellList, buyList);
        assert(sellList.size() == sellSize - 2 && sellList[0].price == 120);
        assert(buyList.size() == 2 && buyList[0].price == 80);
    }
    {
        // два два
        std::vector<PWCatsPriceHistoryRequest::PriceElement> buyList
        {
            { 135, 10 },
            { 115, 10 },
            { 80, 10 },
            { 70, 10 },
        };
        auto sellList(sellList_);
        testLists(sellList, buyList);
        assert(sellList.size() == sellSize - 4 && sellList[0].price == 140);
        assert(buyList.size() == 2 && buyList[0].price == 80);
    }
}
#endif