#include "stdafx.h"

#include <boost/archive/codecvt_null.hpp>
#include <boost/archive/detail/auto_link_warchive.hpp>

#include "types.h"
#include "utils.h"
#include "netdata/serialize.h"
#include "game/data/elements.h"
#include "game/data/itemprops.h"

//

void ItemPropsFactory::addSet(const ItemPropClass & c)
{
    if ( ! c.itemIds.empty())
    {
        itemClasses_.push_back(c);
    }
}

ItemProps * ItemPropsFactory::create(DWORD itemId, const barray & data)
{
    if (data.empty())
    {
        // buying items have no properties
        return 0;
    }

    for (std::vector<ItemPropClass>::const_iterator it = itemClasses_.begin();
        it != itemClasses_.end(); ++it)
    {
        if (it->itemIds.find(itemId) != it->itemIds.end())
        {
            return it->ctor(data);
        }
    }
    return 0;
}

//

#if defined(DEBUG)

void itemPropTest(const ItemPropClass & wpClass)
{
    ItemPropsFactory    propFactory_;
    ItemPropsDecoder    propDecoder_(L"item_desc.txt", L"item_ext_prop.txt", L"item_ext_prop_map.txt");

    propFactory_.addSet(wpClass);

    byte t1[] = {
        0x55, 0x00, 0x04, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0xF6, 0x13, 0x00, 0x00,
        0xE0, 0x2E, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF5, 0x62, 0x00, 0x00,
        0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x02, 0x00, 0x00, 0x93, 0x02, 0x00, 0x00,
        0xD0, 0x02, 0x00, 0x00, 0xCE, 0x03, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xE9, 0x18, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x8E, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x34, 0x25, 0x00, 0x00, 0x36, 0x01, 0x00, 0x00,
        0xCC, 0x25, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x88, 0xA4, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
        0xE3, 0x46, 0x00, 0x00, 0xA6, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00
    };
    barray t(t1, t1 + sizeof(t1));
    ItemProps *ip = propFactory_.create(0x65c8, t);
    wstring dc = propDecoder_.decode(ip);
    delete ip;

    byte t2[] = {
        0x5A, 0x00, 0xFF, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8A, 0x00, 0x9A, 0x29, 0x00, 0x00,
        0xB0, 0x36, 0x00, 0x00, 0x24, 0x00, 0x04, 0x0A, 0x18, 0x04, 0x41, 0x04, 0x41, 0x04, 0x38, 0x04, 0x3D, 0x04,
        0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x8F, 0x02, 0x00, 0x00, 0x8F, 0x02, 0x00, 0x00, 0x8F, 0x02, 0x00, 0x00, 0x8F, 0x02, 0x00, 0x00,
        0x8F, 0x02, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0xE0, 0x18, 0x00, 0x00, 0xE0, 0x18, 0x00, 0x00,
        0xE0, 0x18, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x38, 0x21, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0xB5, 0x25, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xBB, 0x25, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x7D, 0xA4, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x7D, 0xA4, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00,
        0x7D, 0xA4, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x1E, 0x47, 0x00, 0x00, 0x6B, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00,
    };

    barray tt(t2, t2 + sizeof(t2));
    ip = propFactory_.create(0x3a78, tt);
    dc = propDecoder_.decode(ip);
    delete ip;
}
#endif