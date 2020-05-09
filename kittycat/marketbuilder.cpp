
#include "stdafx.h"

#include "marketbuilder.h"
#include "game/data/items.h"
#include "MarketItemSetup.h"

#if defined(TEST_MARKETBUILDER) && !defined(_DEBUG)
#error please unset TEST_MARKETBUILDER
#endif

extern elements::ItemListCollection g_elements_;


struct ShortMarketItemEx
{
    int             count;
    ShortMarketItem item;

    ShortMarketItemEx(const ShortMarketItem & i)
        : count(0)
        , item(i)
    {}
    operator ShortMarketItem() const
    {
        ShortMarketItem r(item);
        r.count = count;
        return r;
    }

    bool isFixedItem() const
    {
        return item.count > 0;
    }

    bool isVariableItem() const
    {
        return item.count < 0 && count < abs(item.count);
    }
};

// Для наглядности можно допустить, что расстановка цен предметов это расположение одномерных предметов в заданном
// отрезке (интервале).
// Здесь будеи иметь два типа размещения: фиксированное и плавающее.
// Фиксированное размещение размещает все предметы в интервале таким образом, чтобы они занимали все требуемое или 
// отведенное пространство. Если интервал не может разместить все предметы, то количество предметов пропоруцонально
// уменьшается.
// Для этого "суммарная длина" предметов масштабируется под интервал
// предметы  |---|---|-------|-------|-------|-------|
// интервал  |---------------------|
// результат |---|-------|-------|
// 

static long long placeFixedItems(std::vector<ShortMarketItemEx> & items, long long money)
{
    // returns 'filled' money count

    // get 'total' price

    long long totalCost = 0;

    for (std::vector<ShortMarketItemEx>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        const ShortMarketItemEx & item = *it;

        if (item.isFixedItem())
        {
            totalCost += long long(item.item.count) * item.item.price;
        }
    }

    if (totalCost == 0)
    {
        // no fixed items
        return 0;
    }

    double coeff = 1.;
    if (totalCost - money > 0)
    {
        coeff = double(money) / totalCost;
    }

    // actual placing
    long long realCost = 0;
    for (std::vector<ShortMarketItemEx>::iterator it = items.begin(); it != items.end();)
    {
        ShortMarketItemEx & item = *it;

        if (item.isFixedItem())
        {
            item.count = int(coeff * item.item.count);
            if (item.count > 0)
            {
                realCost += long long(item.count) * item.item.price;
                ++it;
            }
            else
            {
                it = items.erase(it);
            }
        }
        else
        {
            ++it;
        }
    }

    return realCost;
}

// Плавающее размещение пытается разместить предметы с неуказанным числом так, чтобы они занимали примерно равный
// обьем по стоимости вплоть до заполнения всего интервала.
// Этот рекурсивный процесс заканчивается тогда, когда невозможно больше разместить ни одного предмета.

static long long placeVariableItems(std::vector<ShortMarketItemEx> & items, unsigned money)
{
    // returns 'filled' money count

    if (money == 0)
    {
        return 0;
    }

    // calc slot count

    unsigned itemSlots = 0;

    for (std::vector<ShortMarketItemEx>::iterator it = items.begin(); it != items.end(); ++it)
    {
        ShortMarketItemEx & item = *it;
        if (item.isVariableItem())
        {
            if (item.item.price > money)
            {
                // предмет выходит из рекурсии так как его стоимость больше лимита денег
                item.item.count = 0;
            }
            else
            {
                itemSlots ++;
            }
        }
    }

    if (itemSlots == 0)
    {
        // no variable items
        return 0;
    }

    unsigned moneyPerSlot = money / itemSlots;
    long long totalCost = 0;

    // place slots

    for (std::vector<ShortMarketItemEx>::iterator it = items.begin(); it != items.end(); ++it)
    {
        ShortMarketItemEx & item = *it;

        // не обрабатываем только те, для которых наступило ограничение по количеству
        if (item.isVariableItem())
        {
            int count = std::min<int>(std::abs(item.item.count) - item.count, moneyPerSlot / item.item.price);
            if (count != 0)
            {
                totalCost += long long(count) * item.item.price;
                item.count += count;
            }
        }
    }

    if (totalCost != 0)
    {
        totalCost += placeVariableItems(items, money > totalCost ? money - (unsigned)totalCost : 0);
    }
    // else если ни один предмет не может быть размещен

    // предметы с нулевым количеством мы можем почистить только после окончания размещения

    return totalCost;
}

std::vector<ShortMarketItem> buildMarketList( const std::vector<InventoryItem> & bag_
                                            , const std::vector<MarketItemSetup> & marketSetup
                                            , unsigned moneyLimitOnSell
                                            , unsigned moneyLimitOnBuy 
                                            , unsigned maxSlots
#if defined(TEST_MARKETBUILDER)
                                            , int stack
#endif
                                            )
{
    std::vector<ShortMarketItem> sellItems, buyItems;
    std::vector<InventoryItem> bag(bag_);

    unsigned buySlotCount = 0;
    unsigned sellSlotCount = 0;

    for (size_t i = 0; i < marketSetup.size(); ++i)
    {
        const MarketItemSetup & setupItem = marketSetup.at(i);
        setupItem.item.count = 0;      // reset so missing item will be skipped

        // pick a match from bag, skip item if there's none
        auto itemMatch = [](const InventoryItem & i, const MarketItemSetup & is) -> bool
        {
            return i.id != InventoryItem::INVALID_ID
                && i.id == is.item.id
                && i.propsEqualTo(is.item.props);
        };
        
        if (setupItem.slot >= 0
            && setupItem.slot < int(bag.size())
            && itemMatch(bag[setupItem.slot], setupItem))
        {
            // если предмет попадает в слот, то юзаем его

            setupItem.item.count = bag[setupItem.slot].count;
            bag.at(setupItem.slot).id = InventoryItem::INVALID_ID;         // do not reuse item after
        }
        else
        {
            // иначе находим подходящий итем последовательным просмотром
            
            for (int slot = 0; slot < (int)bag.size(); slot++)
            {
                const InventoryItem & inventoryItem = bag.at(slot);

                if (itemMatch(inventoryItem, setupItem))
                {
                    // можем реюзать предмет, только если для его слота нет соответствующей записи в сетапе

                    if (!std::any_of(marketSetup.begin(), marketSetup.end(),
                                     [&inventoryItem, slot, itemMatch](const MarketItemSetup & i)
                                    { return slot == i.slot && itemMatch(inventoryItem, i); }
                                    ))
                    {
                        setupItem.item.count = inventoryItem.count;
                        const_cast<decltype(setupItem.slot)&>(setupItem.slot) = slot;
                        bag[setupItem.slot].id = InventoryItem::INVALID_ID; // do not reuse item after
                        break;
                    }
                }
            }
        }

        if (setupItem.item.count == 0)
        {
            Log("Could not match market item with inventory: %i", setupItem.item.id);
            continue;
        }

#if !defined(TEST_MARKETBUILDER)
        int stack = -1;
        elements::Item item;
        if (g_elements_.getItem(setupItem.item.id, item))
        {
            stack = item["Stack"];
        }
#endif

        // buy stage

        if (setupItem.doBuy && (maxSlots == 0 || buySlotCount < maxSlots))
        {
            if (setupItem.keepMax <= 0
                || (unsigned(setupItem.keepMax) > setupItem.item.count))
            {
                if (setupItem.buyPrice > 0)
                {
                    // count 
                    // >0   - certain count is set
                    // == 0 - buy as much as possible
                    // < 0  - buy as much as possible but no more than count
                    // 
                    // stack       -  -  -    S    S           -           S             S                         
                    // keepMax     -  -  K    -    -           K           K             K                         
                    // buyCount    -  C  -    -    C           C           -             C                         
                    //                                                                  
                    // count       0  С  K-I  S-I  min(C,S-I)  min(C,K-I)  min(K-I,S-I)  min(C,K-I,S-I)            
                    // при наличии одинаковых предметов в инвентаре они складываются в первый по порядку
                    // 
                    int stackV = stack > 0 ? (stack == 1 ? 1 : (stack - setupItem.item.count)) : std::numeric_limits<int>::max();  // allow buying single-stack items
                    int keepMaxV = setupItem.keepMax > 0 ? (setupItem.keepMax - setupItem.item.count) : std::numeric_limits<int>::max();
                    int buyCountV = setupItem.buyCount > 0 ? setupItem.buyCount : std::numeric_limits<int>::max();

                    int count = 0;
                    if (stack > 0 || setupItem.keepMax > 0 || setupItem.buyCount > 0)
                    {
                        count = std::min(stackV, std::min(keepMaxV, buyCountV));
                    }
                    else
                    {
                        count = std::numeric_limits<int>::max();
                    }

                    if (setupItem.buyCount == 0)    // если изначально количество не было задано, индицируем это отрицательным количеством для второго прохода
                    {
                        count = -count;
                    }

                    ShortMarketItem  mi;
                    mi.itemId = setupItem.item.id;
                    mi.count  = count;
                    mi.price  = setupItem.buyPrice;
                    mi.slot   = -1;

                    buyItems.push_back(mi);

                    if (maxSlots > 0)
                    {
                        ++buySlotCount;
                    }
                }
                else
                {
                    Log("Item skipped at %i: buyPrice <= 0", i);
                }
            }
            else
            {
                Log("Item skipped at %i: itemsAvailable >= keepMax", i);
            }
        }

        // sell stage

        if (setupItem.doSell && (maxSlots == 0 || sellSlotCount < maxSlots)
            && (setupItem.keepMin <= 0
                || (unsigned(setupItem.keepMin) < setupItem.item.count)))
        {
            if (setupItem.sellPrice > 0)
            {
                // sellCount 
                // >0   - certain count is set
                // < 0  - sell as much as possible but no more than count
                        
                int sellCount = setupItem.sellCount > 0 ? setupItem.sellCount : setupItem.item.count;
                sellCount = setupItem.keepMin > 0 ?
                    std::min(sellCount, int(setupItem.item.count) - setupItem.keepMin)
                    : std::min(sellCount, int(setupItem.item.count));
                if (setupItem.sellCount < 0)
                {
                    sellCount = -sellCount;
                }

                // not applying stacking constraint because item count cannot be greater than stack

                if (sellCount != 0)
                {
                    ShortMarketItem  mi;
                    mi.itemId = setupItem.item.id;
                    mi.count = sellCount;
                    mi.price = setupItem.sellPrice;
                    mi.slot  = setupItem.slot;

                    sellItems.push_back(mi);

                    if (maxSlots > 0)
                    {
                        ++sellSlotCount;
                    }
                }
            }
            else
            {
                Log("Item skipped at %i: sellPrice <= 0", i);
            }
        }
    }

    // контроль достижения лимитов по деньгам на покупке и продаже

    std::vector<ShortMarketItem> result;

    if (moneyLimitOnSell > 0)
    {
        std::vector<ShortMarketItemEx> items;
        std::copy(sellItems.begin(), sellItems.end(), std::back_inserter(items));

        long long placed = placeFixedItems(items, moneyLimitOnSell);
        placeVariableItems(items, moneyLimitOnSell > placed ? moneyLimitOnSell - (unsigned)placed : 0);

        // предметы с нулевым количеством мы можем почистить только после окончания переменного размещения
        for (std::vector<ShortMarketItemEx>::const_iterator it = items.begin(); it != items.end(); ++it)
        {
            if (it->count != 0)
            {
                result.push_back(*it);
            }
        }
    }
    else
    {
        result = sellItems;
        // on sell stage some of items would have negative count
        for (std::vector<ShortMarketItem>::iterator it = result.begin(); it != result.end(); ++it)
        {
            it->count = abs(it->count);
        }
    }

    // buy items
    {   
        std::vector<ShortMarketItemEx> items;
        std::copy(buyItems.begin(), buyItems.end(), std::back_inserter(items));

        long long placed = placeFixedItems(items, moneyLimitOnBuy);
        placeVariableItems(items, moneyLimitOnBuy > placed ? moneyLimitOnBuy - (unsigned)placed : 0);

        // предметы с нулевым количеством мы можем почистить только после окончания переменного размещения
        for (std::vector<ShortMarketItemEx>::const_iterator it = items.begin(); it != items.end(); ++it)
        {
            if (it->count != 0)
            {
                result.push_back(*it);
            }
        }
    }

    return result;
}

void getMarketSums( const std::vector<MarketItemSetup> & marketSetup
                  , unsigned moneyLimitOnSell
                  , unsigned moneyLimitOnBuy
                  , unsigned maxSlots
                  , long long & sellSum
                  , long long & buySum
                  , unsigned & sellCount
                  , unsigned & buyCount
)
{
    // build fake bag

    std::vector<InventoryItem> bag;
    for (size_t i = 0; i < marketSetup.size(); ++i)
    {
        const MarketItemSetup & si = marketSetup.at(i);

        InventoryItem item(si.item);
        if (si.slot >= int(bag.size()))
        {
            bag.resize(si.slot + 1);
        }
        if (si.slot >= 0)
        {
            // negative slot is 'virtual'
            bag[si.slot] = item;
        }
    }

    sellSum = 0;
    sellCount = 0;
    buySum = 0;
    buyCount = 0;

    std::vector<ShortMarketItem> built = buildMarketList(bag, marketSetup, moneyLimitOnSell, moneyLimitOnBuy, maxSlots);

    for (size_t i = 0; i < built.size(); ++i)
    {
        const ShortMarketItem & item = built[i];

        if (item.slot >= 0)  // sell or buy
        {
            sellSum += long long(item.price) * item.count;
            sellCount ++;
        }
        else
        {
            buySum += long long(item.price) * item.count;
            buyCount ++;
        }
    }
}

#if defined(TEST_MARKETBUILDER)

void testBuildMarketList()
{
    std::vector<InventoryItem> bag;
    InventoryItem i1(1, 10);
    InventoryItem i2(2, 10);
    InventoryItem i21(21, 10);
    InventoryItem i3(3, 10);
    InventoryItem i31(31, 10);
    InventoryItem i4(4, 10);
    InventoryItem i41(41, 10);
    InventoryItem i42(42, 10);
    InventoryItem i5(5, 10);
    InventoryItem i50(5, 1);
    bag.push_back(i1);      // 0
    bag.push_back(i2);      // 1
    bag.push_back(i21);     // 2 
    bag.push_back(i3);      // 3
    bag.push_back(i31);     // 4
    bag.push_back(i4);      // 5
    bag.push_back(i41);     // 6
    bag.push_back(i42);     // 7
    //bag.push_back(i5);    // 
    bag.push_back(i50);     // 8

    // Testing single cases for buy stage:
    // 
    // stack       -  -  -    S    S           -           S             S                         
    // keepMax     -  -  K    -    -           K           K             K                         
    // buyCount    -  C  -    -    C           C           -             C                         
    //                                                                  
    // count       0  С  K-I  S-I  min(C,S-I)  min(C,K-I)  min(K-I,S-I)  min(C,K-I,S-I)            
    // 

    unsigned money = 100000;

    {
        std::vector<MarketItemSetup> marketSetup;
        int stack = -1;

        MarketItemSetup s1;
        s1.slot     = 0;
        s1.item.id  = 1;
        s1.doBuy    = true;
        s1.buyPrice = 1000;
        marketSetup.push_back(s1);

        // C
        MarketItemSetup s2;
        s2.slot     = 1;
        s2.item.id  = 2;
        s2.doBuy    = true;
        s2.buyPrice = 1000;
        s2.buyCount = 5;
        marketSetup.push_back(s2);

        // K
        MarketItemSetup s3;
        s3.slot     = 1;
        s3.item.id  = 3;
        s3.doBuy    = true;
        s3.buyPrice = 1000;
        s3.keepMax  = 15;
        marketSetup.push_back(s3);

        // KC1
        MarketItemSetup s4;
        s4.slot     = 1;
        s4.item.id  = 4;
        s4.doBuy    = true;
        s4.buyPrice = 1000;
        s4.buyCount = 5;
        s4.keepMax  = 20;       // have 10
        marketSetup.push_back(s4);
        // KC2
        MarketItemSetup s41;
        s41.slot     = 1;
        s41.item.id   = 41;
        s41.doBuy    = true;
        s41.buyPrice = 1000;
        s41.buyCount = 20;
        s41.keepMax  = 20;       // have 10
        marketSetup.push_back(s41);

        std::vector<ShortMarketItem> items = buildMarketList(bag, marketSetup, InventoryMoneyLimit - money, money, CatshopMaxSlotsDefault, stack);
        assert(items.size() == 5);

        assert(items[0].slot   == -1);  // buying
        assert(items[0].itemId == 1);
        assert(items[0].price  == (unsigned)s1.buyPrice);
        assert(items[0].count  == 75 /*(int)money / s1.buyPrice*/);

        assert(items[1].slot   == -1);  // buying
        assert(items[1].itemId == 2);
        assert(items[1].price  == (unsigned)s2.buyPrice);
        assert(items[1].count  == s2.buyCount);

        assert(items[2].slot   == -1);  // buying
        assert(items[2].itemId == 3);
        assert(items[2].price  == (unsigned)s3.buyPrice);
        assert(items[2].count  == s3.keepMax - (int)bag[2].count);

        assert(items[3].slot   == -1);  // buying
        assert(items[3].itemId == 4);
        assert(items[3].price  == (unsigned)s3.buyPrice);
        assert(items[3].count  == 5);

        assert(items[4].slot   == -1);  // buying
        assert(items[4].itemId == 41);
        assert(items[4].price  == (unsigned)s3.buyPrice);
        assert(items[4].count  == 10);
    }
    {
        std::vector<MarketItemSetup> marketSetup;
        int stack = 25;

        MarketItemSetup s1;
        s1.slot     = 0;
        s1.item.id  = 1;
        s1.doBuy    = true;
        s1.buyPrice = 1000;
        marketSetup.push_back(s1);

        // C1
        MarketItemSetup s2;
        s2.slot     = 1;
        s2.item.id  = 2;
        s2.doBuy    = true;
        s2.buyPrice = 1000;
        s2.buyCount = 5;
        marketSetup.push_back(s2);
        // C2
        MarketItemSetup s21;
        s21.slot     = 1;
        s21.item.id  = 21;
        s21.doBuy    = true;
        s21.buyPrice = 1000;
        s21.buyCount = 30;
        marketSetup.push_back(s21);

        // K1
        MarketItemSetup s3;
        s3.slot     = 1;
        s3.item.id  = 3;
        s3.doBuy    = true;
        s3.buyPrice = 1000;
        s3.keepMax  = 20;
        marketSetup.push_back(s3);
        // K2
        MarketItemSetup s31;
        s31.slot     = 1;
        s31.item.id  = 31;
        s31.doBuy    = true;
        s31.buyPrice = 1000;
        s31.keepMax  = 30;
        marketSetup.push_back(s31);

        // KC1
        MarketItemSetup s4;
        s4.slot     = 1;
        s4.item.id  = 4;
        s4.doBuy    = true;
        s4.buyPrice = 1000;
        s4.buyCount = 5;
        s4.keepMax  = 20;       // have 10
        marketSetup.push_back(s4);
        // KC2
        MarketItemSetup s41;
        s41.slot     = 1;
        s41.item.id  = 41;
        s41.doBuy    = true;
        s41.buyPrice = 1000;
        s41.buyCount = 20;
        s41.keepMax  = 20;       // have 10
        marketSetup.push_back(s41);
        // KC3
        MarketItemSetup s42;
        s42.slot     = 1;
        s42.item.id  = 42;
        s42.doBuy    = true;
        s42.buyPrice = 1000;
        s42.buyCount = 20;
        s42.keepMax  = 30;       // have 10
        marketSetup.push_back(s42);

        std::vector<ShortMarketItem> items = buildMarketList(bag, marketSetup, InventoryMoneyLimit - money, money, CatshopMaxSlotsDefault, stack);
        assert(items.size() == 8);

        assert(items[0].slot   == -1);  // buying
        assert(items[0].itemId == 1);
        assert(items[0].price  == (unsigned)s1.buyPrice);
        assert(items[0].count  == stack - (int)bag[0].count);

        assert(items[1].slot   == -1);  // buying
        assert(items[1].itemId == 2);
        assert(items[1].price  == (unsigned)s2.buyPrice);
        assert(items[1].count  == s2.buyCount);
        assert(items[2].slot   == -1);  // buying
        assert(items[2].itemId == 21);
        assert(items[2].price  == (unsigned)s21.buyPrice);
        assert(items[2].count  == stack - (int)bag[2].count);

        assert(items[3].slot   == -1);  // buying
        assert(items[3].itemId == 3);
        assert(items[3].price  == (unsigned)s3.buyPrice);
        assert(items[3].count  == s3.keepMax - (int)bag[3].count);
        assert(items[4].slot   == -1);  // buying
        assert(items[4].itemId == 31);
        assert(items[4].price  == (unsigned)s31.buyPrice);
        assert(items[4].count  == stack - (int)bag[3].count);

        assert(items[5].slot   == -1);  // buying
        assert(items[5].itemId == 4);
        assert(items[5].price  == (unsigned)s4.buyPrice);
        assert(items[5].count  == s4.buyCount);
        assert(items[6].slot   == -1);  // buying
        assert(items[6].itemId == 41);
        assert(items[6].price  == (unsigned)s3.buyPrice);
        assert(items[6].count  == s41.keepMax - (int)bag[6].count);
        assert(items[7].slot   == -1);  // buying
        assert(items[7].itemId == 42);
        assert(items[7].price  == (unsigned)s3.buyPrice);
        assert(items[7].count  == stack - (int)bag[7].count);
    }

    // fixed items scaling, variable placement and both removal
    {
        std::vector<MarketItemSetup> marketSetup;
        int stack = 25;

        MarketItemSetup s1;
        s1.slot     = 0;
        s1.item.id  = 1;
        s1.doBuy    = true;
        s1.buyPrice = 50000;
        s1.buyCount = 5;
        marketSetup.push_back(s1);

        MarketItemSetup s2;
        s2.slot     = 1;
        s2.item.id  = 2;
        s2.doBuy    = true;
        s2.buyPrice = 10000;
        s2.buyCount = 10;
        marketSetup.push_back(s2);
        MarketItemSetup s21;
        s21.slot     = 1;
        s21.item.id  = 21;
        s21.doBuy    = true;
        s21.buyPrice = 110000;
        s21.buyCount = 1;
        marketSetup.push_back(s21);

        MarketItemSetup s3;
        s3.slot     = 1;
        s3.item.id  = 3;
        s3.doBuy    = true;
        s3.buyPrice = 1000;
        marketSetup.push_back(s3);
        MarketItemSetup s31;
        s31.slot     = 1;
        s31.item.id  = 31;
        s31.doBuy    = true;
        s31.buyPrice = 60000;
        marketSetup.push_back(s31);

        MarketItemSetup s4;
        s4.slot     = 1;
        s4.item.id  = 4;
        s4.doBuy    = true;
        s4.buyPrice = 5000;
        marketSetup.push_back(s4);

        std::vector<ShortMarketItem> items = buildMarketList(bag, marketSetup, InventoryMoneyLimit - money, money, CatshopMaxSlotsDefault, stack);
        assert(items.size() == 4);

        assert(items[0].slot   == -1);  // buying
        assert(items[0].itemId == 1);
        assert(items[0].price  == (unsigned)s1.buyPrice);
        assert(items[0].count  == 1);

        assert(items[1].slot   == -1);  // buying
        assert(items[1].itemId == 2);
        assert(items[1].price  == (unsigned)s2.buyPrice);
        assert(items[1].count  == 2);

        assert(items[2].slot   == -1);  // buying
        assert(items[2].itemId == 3);
        assert(items[2].price  == (unsigned)s3.buyPrice);
        assert(items[2].count  == 15);

        assert(items[3].slot   == -1);  // buying
        assert(items[3].itemId == 4);
        assert(items[3].price  == (unsigned)s4.buyPrice);
        assert(items[3].count  == 3);
    }

    // Sell ops scaling and slot matching
    {
        std::vector<MarketItemSetup> marketSetup;
        int stack = 25;

        MarketItemSetup s1;
        s1.slot      = 0;
        s1.item.id   = 1;
        s1.doSell    = true;
        s1.sellPrice = 50000000;
        s1.sellCount = 5;
        marketSetup.push_back(s1);

        MarketItemSetup s2;
        s2.slot      = 1;
        s2.item.id   = 2;
        s2.doSell    = true;
        s2.sellPrice = 10000000;
        s2.sellCount = 15;
        marketSetup.push_back(s2);
        MarketItemSetup s21;
        s21.slot      = 1;
        s21.item.id   = 21;
        s21.doSell    = true;
        s21.sellPrice = 150000000;
        s21.sellCount = 1;
        marketSetup.push_back(s21);

        MarketItemSetup s3;
        s3.slot      = 1;
        s3.item.id   = 3;
        s3.doSell    = true;
        s3.sellPrice = 1000000;
        marketSetup.push_back(s3);
        MarketItemSetup s31;
        s31.slot      = 1;
        s31.item.id   = 31;
        s31.doSell    = true;
        s31.sellPrice = 60000000;
        marketSetup.push_back(s31);

        MarketItemSetup s4;
        s4.slot      = 1;
        s4.item.id   = 4;
        s4.doSell    = true;
        s4.sellPrice = 10000000;
        marketSetup.push_back(s4);

        std::vector<ShortMarketItem> items = buildMarketList(bag, marketSetup, InventoryMoneyLimit - money, money, CatshopMaxSlotsDefault, stack);
        assert(items.size() == 4);
//         assert(items[0].slot   == 0);
//         assert(items[0].itemId == 1);
//         assert(items[0].price  == (unsigned)s1.sellPrice);
//         assert(items[0].count  == 1);
// 
//         assert(items[1].slot   == 1);
//         assert(items[1].itemId == 2);
//         assert(items[1].price  == (unsigned)s2.sellPrice);
//         assert(items[1].count  == 3);
// 
//         assert(items[2].slot   == 3);
//         assert(items[2].itemId == 3);
//         assert(items[2].price  == (unsigned)s3.sellPrice);
//         assert(items[2].count  == 10);
// 
//         assert(items[3].slot   == 5);
//         assert(items[3].itemId == 4);
//         assert(items[3].price  == (unsigned)s4.sellPrice);
//         assert(items[3].count  == 10);

        assert(items[0].slot == 1);
        assert(items[0].itemId == 2);
        assert(items[0].price == (unsigned) s2.sellPrice);
        assert(items[0].count == 1);

        assert(items[1].slot == 3);
        assert(items[1].itemId == 3);
        assert(items[1].price == (unsigned) s3.sellPrice);
        assert(items[1].count == 1);

        assert(items[2].slot == 4);
        assert(items[2].itemId == 31);
        assert(items[2].price == (unsigned) s31.sellPrice);
        assert(items[2].count == 1);

        assert(items[3].slot == 5);
        assert(items[3].itemId == 4);
        assert(items[3].price == (unsigned) s4.sellPrice);
        assert(items[3].count == 1);
    }

    // sell ops additional
    {
        std::vector<MarketItemSetup> marketSetup;
        int stack = 25;

        // если в инвентаре отсутствует s5 то мы должны заматчить s51 (тот же ид) по слоту и не выставлять его на продажу т к у него ограничение min1
        MarketItemSetup s5;
        s5.slot = 1;
        s5.item.id = 5;
        s5.doSell = true;
        s5.sellPrice = 100000;
        marketSetup.push_back(s5);
        MarketItemSetup s51;
        s51.slot = 8;
        s51.item.id = 5;
        s51.doSell = true;
        s51.sellPrice = 100000;
        s51.keepMin = 1;
        marketSetup.push_back(s51);

        std::vector<ShortMarketItem> items = buildMarketList(bag, marketSetup, InventoryMoneyLimit - money, money, CatshopMaxSlotsDefault, stack);
        assert(items.size() == 0);
    }
}

#endif

