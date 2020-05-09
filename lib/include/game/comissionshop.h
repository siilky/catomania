#pragma once
#ifndef commissionshop_h
#define commissionshop_h

#include "netdata/connection.h"
#include "netdata/fragments_client.h"
#include "netdata/fragments_server.h"
#include "event.h"
#include "game/data/items.h"


class Comissionshop
{
public:
//     class CommiItem
//     {
//     public:
//         unsigned    id;
// 
//     };

    Comissionshop();
    ~Comissionshop();

    void init();
    void bind(std::shared_ptr<Connection> connection);
    void unbind();

    void setElements(const elements::ItemListCollection & items);
    void setPropFactory(std::shared_ptr<ItemPropsFactory> propFactory);

    void update(bool trackProgress);
    void updateShop(unsigned id);

    time_t lastUpdated() const
    {
        return lastUpdated_;
    }

    //const CommiShop                             shop() const;
    const std::vector<CommiShop> & shopList() const
    {
        return shopList_;
    }

    // signals
    boost::signal<void(BaseEvent *event)>  sEvent;

private:
    void onEnterWorld(const clientdata::FragmentEnterWorld * f);

    void onComissionShopList(const serverdata::FragmentComissionShopList * f);

    void updateCompleted();

    void onComissionShop(const serverdata::FragmentComissionShop * f);

    void updateShopListItem();
    void fillProperty(MarketItem & item);

    std::shared_ptr<Connection>   connection_;
    std::vector<Connection::Cookie> cookies_;

    elements::ItemListCollection        allItems_;
    std::shared_ptr<ItemPropsFactory> propFactory_;

    DWORD           myId_;      // from onEnterWorld

    std::vector<CommiShop>  shopList_;
    bool            inUpdate_;
    bool            trackProgress_;
    unsigned        lastProgress_;
    time_t          lastUpdated_;
};

#endif
