#ifndef marketbuilder_h_
#define marketbuilder_h_

//define TEST_MARKETBUILDER 1
#undef TEST_MARKETBUILDER

class InventoryItem;
class MarketItemSetup;

struct ShortMarketItem
{
    unsigned    itemId;
    int         slot;       // set to -1 for buying
    int         count;
    unsigned    price;
};

enum
{
    CatshopMaxSlotsDefault = 12,
    CatshopMaxCharsDefault = 13,
    InventoryMoneyLimit         = 1000000000,
    InventoryMoneyLimitCatshop  = std::numeric_limits<unsigned int>().max(),
};


std::vector<ShortMarketItem> buildMarketList( const std::vector<InventoryItem> & bag_
                                            , const std::vector<MarketItemSetup> & marketSetup
                                            , unsigned moneyLimitOnSell = InventoryMoneyLimit     // 0 is not set - no limit
                                            , unsigned moneyLimitOnBuy = InventoryMoneyLimit
                                            , unsigned maxSlots = CatshopMaxSlotsDefault
#if defined(TEST_MARKETBUILDER)
                                            , int stack = 0
#endif
                                            );


void getMarketSums( const std::vector<MarketItemSetup> & marketSetup
                  , unsigned moneyLimitOnSell   // 0 is not set - no limit
                  , unsigned moneyLimitOnBuy
                  , unsigned maxSlots
                  , long long & sellSum
                  , long long & buySum
                  , unsigned & sellCount
                  , unsigned & buyCount
                  );

#if defined(TEST_MARKETBUILDER)
void testBuildMarketList();
#endif

#endif