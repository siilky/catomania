
#include "stdafx.h"

#include "MarketItemSetup.h"
#include "game/data/itemprops.h"
#include "qlib/vmp.h"
#include "persistence2.h"

static Tristate<bool>   s_AutopriceEnabled;

class ByteConversion
{
public:
    JsonValue convert(byte v) const
    {
        return JsonValue((int)v, nullptr);
    }

    byte convertIn(const JsonValue & v) const
    {
        return (char)static_cast<int>(v);
    }
};


MarketItemSetup::MarketItemSetup()
    : slot(-1)
    , doSell(false)
    , sellPrice(0), sellAutoPrice(false), sellCount(0)
    , buyPrice(0), buyAutoPrice(false), buyCount(0)
    , doBuy(false)
    , keepMin(0), keepMax(0)
    , apMinCount(0), apPriceStep(0)
{
}

MarketItemSetup::MarketItemSetup(const InventoryItem & i)
    : item(i)
    , slot(-1)
    , doSell(false)
    , sellPrice(0), sellAutoPrice(false), sellCount(0)
    , buyPrice(0), buyAutoPrice(false), buyCount(0)
    , doBuy(false)
    , keepMin(0), keepMax(0)
    , apMinCount(0), apPriceStep(0)
{
}

MarketItemSetup::MarketItemSetup(const JsonValue & config)
{
    config.get(L"Attrs", attributes, ByteConversion());

    config.get(L"Id", item.id);

    int mode;
    config.get(L"Mode", mode);
    doSell = (mode == Sell || mode == Auto) ? true : false;
    doBuy = (mode == Buy || mode == Auto) ? true : false;
    config.get(L"SellPrice", sellPrice, Tristate<int>(0));
    config.get(L"SellAmount", sellCount, Tristate<int>(0));
    config.get(L"BuyPrice", buyPrice, Tristate<int>(0));
    config.get(L"BuyAmount", buyCount, Tristate<int>(0));

    config.get(L"KeepMin", keepMin, Tristate<int>(0));
    config.get(L"KeepMax", keepMax, Tristate<int>(0));
    config.get(L"Slot", slot);

    // установка через ref() и без значений по умолчанию не обнуляет tristate
    // tristate по умолчанию true
    config.get(L"ApSellAutoPrice", sellAutoPrice, Tristate<bool>(false));
    config.get(L"ApBuyAutoPrice", buyAutoPrice, Tristate<bool>(false));
    config.get(L"ApMinCount", apMinCount, Tristate<int>(0));
    config.get(L"ApPriceStep", apPriceStep, Tristate<int>(0));

    normalize();
}

MarketItemSetup::operator JsonValue() const
{
    JsonValue config;

    std::vector<int> attrs;
    if (item.props)
    {
        barray b = item.props->assemble();
        std::copy(b.begin(), b.end(), back_inserter(attrs));
    }

    config.set(L"Id", item.id);
    config.set(L"Attrs", attrs, json::NoConversion<int>());
    OperationMode mode = (doSell && doBuy) ? Auto
                            : (doSell ? Sell
                                : (doBuy ? Buy
                                    : NotSet));
    config.set(L"Mode", mode);
    config.set(L"Slot", slot);
    config.set(L"SellPrice", sellPrice);
    config.set(L"SellAmount", sellCount);
    config.set(L"BuyPrice", buyPrice);
    config.set(L"BuyAmount", buyCount);
    config.set(L"KeepMin", keepMin);
    config.set(L"KeepMax", keepMax);

    if (s_AutopriceEnabled.isTristate())
    {
        VMProtectBeginVirtualization(__FUNCTION__);
        s_AutopriceEnabled = Vmp::hasOption(Vmp::OptionAutoprice);
        VMProtectEnd();
    }
    if (s_AutopriceEnabled)
    {
        config.set(L"ApSellAutoPrice", sellAutoPrice);
        config.set(L"ApBuyAutoPrice", buyAutoPrice);
        config.set(L"ApMinCount", apMinCount);
        config.set(L"ApPriceStep", apPriceStep);
    }
    return config;
}

MarketItemSetup & MarketItemSetup::operator<<(const MarketItemSetup & r)
{
    item.count  += r.item.count;    // count assumed to be sum

    slot            << r.slot;
    doSell          << r.doSell;
    sellPrice       << r.sellPrice;
    sellAutoPrice   << r.sellAutoPrice;
    sellCount       << r.sellCount;
    doBuy           << r.doBuy;
    buyPrice        << r.buyPrice;
    buyAutoPrice    << r.buyAutoPrice;
    buyCount        << r.buyCount;
    keepMin         << r.keepMin;
    keepMax         << r.keepMax;
    apMinCount      << r.apMinCount;
    apPriceStep     << r.apPriceStep;

    return *this;
}

void MarketItemSetup::normalize()
{
    if (sellCount < 0)  { sellCount = 0; }
    if (sellPrice < 0)  { sellPrice = 0; }
    if (buyCount < 0)   { buyCount = 0; }
    if (buyPrice < 0)   { buyPrice = 0; }

    if (doSell && sellPrice == 0)
    {
        doSell = false;
    }
    if (doBuy && buyPrice == 0)
    {
        doBuy = false;
    }

    if (keepMin < 0)    { keepMin = 0; }
    if (keepMax < 0)    { keepMax = 0; }
    if (apMinCount < 0) { apMinCount = 0; }
}

bool MarketItemSetup::isEqualItem(const MarketItemSetup & r) const
{
    return (item.id == r.item.id
        && item.propsEqualTo(r.item.props));
}

void MarketItemSetup::setFrom(const MarketItemSetup & r)
{
    doSell          = r.doSell;
    sellPrice       = r.sellPrice;
    sellAutoPrice   = r.sellAutoPrice;
    sellCount       = r.sellCount;
    doBuy           = r.doBuy;
    buyPrice        = r.buyPrice;
    buyAutoPrice    = r.buyAutoPrice;
    buyCount        = r.buyCount;
    keepMin         = r.keepMin;
    keepMax         = r.keepMax;
    apMinCount      = r.apMinCount;
    apPriceStep     = r.apPriceStep;
}
