#ifndef inventory_types_h
#define inventory_types_h
    
enum InventoryStorageType
{
    NO_INFO             = 0x00,
    EQUIPMENT_INFO      = 0x01,
    BAG_INFO            = 0x02,

    ALL_INFO            = 0x03,

    TRASHBOX_INFO       = 0x04,
    TRASHBOXRES_INFO    = 0x08,

    MONEY_INFO          = 0x10,
    MARKET_INFO         = 0x20,     // market info availability means market is currently open

    GOLD_INFO           = 0x40,

//         STYLE_INFO      = 0x04,
//         QUEST_INFO      = 0x08,
};

#endif