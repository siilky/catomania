#ifndef gshop_h_
#define gshop_h_

#define GSHOP_VERSION   1780

namespace GShop
{

struct Category
{
    std::wstring                name;
    std::vector<std::wstring>   subCategories;
};

struct SaleOption
{
    unsigned    price;
    unsigned    expireDate;
    unsigned    duration;
    unsigned    startDate;
    int         controlType;
    unsigned    day;
    unsigned    status;
    unsigned    flags;
};

class Item
{
public:
    Item()
        : itemId(0)
    {}

    std::wstring    name;           // 64 Byte Unicode
    std::wstring    description;    // 1024 Byte Unicode
    std::string     surfacePath;    // 128 Byte GBK

    unsigned itemId;
    int      itemAmount;
    unsigned shopId;
    int      catIndex;
    int      subCatIndex;

    std::vector<SaleOption> saleOptions;    // length = 4

#if GSHOP_VERSION >= 1450
    int giftId;
    int giftAmount;
    int giftDuration;
    int logPrice;
#endif
#if GSHOP_VERSION >= 1520
    unsigned    sellNpcId[8];
#endif
#if GSHOP_VERSION >= 1532
    unsigned    extra1[4];
    unsigned    extra2[2];
#endif
#if GSHOP_VERSION >= 1552
    unsigned    extra3;
#endif
#if GSHOP_VERSION >= 1700
    unsigned    extra4;
    unsigned    extra5;
#endif
#if GSHOP_VERSION >= 1760
    unsigned    extra6;
    unsigned    extra7;
    unsigned    extra8;
    unsigned    extra9;
#endif

    bool isNull() const
    {
        return itemId == 0;
    }
};

class GShop
{
public:
    static GShop * const instance();
    bool load(const std::wstring & filename) const;

    int indexOf(unsigned itemId) const;             // returns -1 if not found
    const Item & item(unsigned itemShopId, size_t * index = 0) const;

    const std::vector<Item> & items() const
    {
        return items_;
    }

    const std::vector<Category> & categories() const
    {
        return categories_;
    }

private:
    enum
    {
        SaleOptions     = 4,
        Categories      = 8,
    };
    GShop(){};

    int                     timestamp_;
    std::vector<Item>       items_;
    std::vector<Category>   categories_;
};

}

#endif