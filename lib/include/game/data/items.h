#ifndef items_h_
#define items_h_

#include "game/data/itemprops.h"


class InventoryItem
{
public:
    static const DWORD INVALID_ID = (DWORD)-1;

    InventoryItem()
        : id(INVALID_ID)
        , count(0)
        , timeLimit(0)
    {}

    InventoryItem(DWORD itemId, unsigned itemCount)
        : id(itemId)
        , count(itemCount)
        , timeLimit(0)
    {}

    bool isEmpty() const
    {
        return id == INVALID_ID;
    }

    bool propsEqualTo(const std::shared_ptr<ItemProps> & r) const
    {
        if (props && r)
        {
            return *props == *r;
        }
        return (! props && ! r);
    }

    void setCount(unsigned amount)
    {
        count = amount;
        if (count == 0)
        {
            id = INVALID_ID;
        }
        else
        {
            assert(!isEmpty());
        }
    }

    void decrease(unsigned amount)
    {
        assert(!isEmpty());

        count -= (count > amount ? amount : count);
        if (count == 0)
        {
            id = INVALID_ID;
        }
    }

    void increase(unsigned amount)
    {
        assert(!isEmpty());
        count += amount;
    }

    unsigned            id;
    mutable unsigned    count;
    unsigned            timeLimit;
    std::wstring        name;
    std::string         icon;       // GBK
    std::shared_ptr<ItemProps>    props;
};


class MarketItem : public InventoryItem
{
public:
    MarketItem()
        : price(0)
    {}

    unsigned int    price;
    int             slot;           // slot < 0 is buying item
};


class CommiShop
{
public:
    CommiShop()
        : state(StateNew)
    {
    }

    unsigned    id;
    unsigned    time;
    unsigned    type;

    std::vector<MarketItem> sellList;
    std::vector<MarketItem> buyList;

    enum
    {
        StateNew,
        StateRequested,
        StateReady,
    }
    state;
};



#endif