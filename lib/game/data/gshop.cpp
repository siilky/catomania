
#include "stdafx.h"

#include "game/data/gshop.h"
#include "game/data/filereader.h"
#include "log.h"


GShop::GShop * const GShop::GShop::instance()
{
    static GShop gshop;
    return & gshop;
}

bool GShop::GShop::load(const std::wstring & filename) const
{
    FileReader fr;
    if ( ! fr.open(filename))
    {
        Log("Error opening gshop file");
        return false;
    }

    GShop * gshop = const_cast<GShop *>(instance());

    gshop->timestamp_ = fr.readDword();

    unsigned count = fr.readDword();
    if (count > 10000)  // some good reasonable value
    {
        return false;
    }

    gshop->items_.resize(count);
    unsigned index = 0;
    while (fr && index < count)
    {
        Item & item = gshop->items_[index];

        item.shopId         = fr.readDword();
        item.catIndex       = fr.readDword();
        item.subCatIndex    = fr.readDword();
        item.surfacePath    = fr.readString(128);
        item.itemId         = fr.readDword();
        item.itemAmount     = fr.readDword();

        item.saleOptions.resize(4);
        for (int i = 0; i < SaleOptions; i++)
        {
            SaleOption & option = item.saleOptions[i];

            option.price        = fr.readDword();
            option.expireDate   = fr.readDword();
            option.duration     = fr.readDword();
            option.startDate    = fr.readDword();
            option.controlType  = fr.readDword();
            option.day          = fr.readDword();
            option.status       = fr.readDword();
            option.flags        = fr.readDword();
        }

    #if GSHOP_VERSION >= 1532
        item.extra1[0]      = fr.readDword();
        item.extra1[1]      = fr.readDword();
        item.extra1[2]      = fr.readDword();
        item.extra1[3]      = fr.readDword();
    #endif
        item.description    = fr.readWstring(512);
        item.name           = fr.readWstring(32);
    #if GSHOP_VERSION >= 1532
        item.extra2[0] = fr.readDword();
        item.extra2[1] = fr.readDword();
    #endif
    #if GSHOP_VERSION >= 1450
        item.giftId         = fr.readDword();
        item.giftAmount     = fr.readDword();
        item.giftDuration   = fr.readDword();
        item.logPrice       = fr.readDword();
    #endif
    #if GSHOP_VERSION >= 1520
        item.sellNpcId[0]   = fr.readDword();
        item.sellNpcId[1]   = fr.readDword();
        item.sellNpcId[2]   = fr.readDword();
        item.sellNpcId[3]   = fr.readDword();
        item.sellNpcId[4]   = fr.readDword();
        item.sellNpcId[5]   = fr.readDword();
        item.sellNpcId[6]   = fr.readDword();
        item.sellNpcId[7]   = fr.readDword();
    #endif
    #if GSHOP_VERSION >= 1552
        item.extra3 = fr.readDword();
    #endif

        index ++;
    }

    gshop->categories_.resize(Categories);
    for (unsigned i = 0; fr && i < gshop->categories_.size(); i++)
    {
        Category & cat = gshop->categories_[i];

        cat.name = fr.readWstring(64);

        unsigned subCategories = fr.readDword();
        if (subCategories > 1000)     // some good reasonable value
        {
            return false;
        }

        cat.subCategories.resize(subCategories);
        for (unsigned n = 0; fr && n < subCategories; n++)
        {
            cat.subCategories[n] = fr.readWstring(64);
        }
    }

    return fr;
}

int GShop::GShop::indexOf(unsigned itemId) const
{
    for (unsigned i = 0; i < items_.size(); i++)
    {
        if (items_[i].itemId == itemId)
        {
            return i;
        }
    }
    return -1;
}

const GShop::Item & GShop::GShop::item(unsigned itemShopId, size_t * index) const
{
    for (unsigned i = 0; i < items_.size(); i++)
    {
        if (items_[i].shopId == itemShopId)
        {
            if (index != 0)
            {
                *index = i;
            }
            return items_[i];
        }
    }
    static const Item nullItem;
    return nullItem;
}

