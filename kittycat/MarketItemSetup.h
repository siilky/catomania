#ifndef mconfig_h_
#define mconfig_h_

#include "game/data/items.h"
#include "tristate.h"

class ItemProps;
class JsonValue;

class MarketItemSetup
{
public:
    InventoryItem   item;
    std::vector<unsigned char>  attributes; // used for deserialization

    // setup
    Tristate<int>   slot;                   // inventory slot for inventory item
    Tristate<bool>  doSell;
    Tristate<bool>  sellAutoPrice;          // использовать автомат выставления цен
    Tristate<int>   sellPrice;
    Tristate<int>   sellCount;
    Tristate<bool>  doBuy;
    Tristate<bool>  buyAutoPrice;           // использовать автомат выставления цен
    Tristate<int>   buyPrice;
    Tristate<int>   buyCount;
    Tristate<int>   keepMin;
    Tristate<int>   keepMax;

    // Autoprice setup
    Tristate<int>           apMinCount;     // если обьем торговли по оптимальной цене меньше, то смотреть следующий (0 - disable)
    Tristate<int>           apPriceStep;    // шаг изменения цены - перебивать цену на шаг (0 - выставлять такую же)  может быть отрицательным

    MarketItemSetup();
    MarketItemSetup(const InventoryItem & item);
    MarketItemSetup(const JsonValue & config);
    operator JsonValue() const;
    MarketItemSetup & operator<<(const MarketItemSetup & r);

    void normalize();
    bool isEqualItem(const MarketItemSetup & r) const;
    void setFrom(const MarketItemSetup & r);

private:
    enum OperationMode
    {
        NotSet,
        Sell,
        Buy,
        Auto,
    };
};

#endif