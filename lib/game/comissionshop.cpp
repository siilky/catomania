#include "stdafx.h"

#include <chrono>

#include "boost/signal.hpp"

#include "game\comissionshop.h"
#include "game\account_types.h"
#include "log.h"


Comissionshop::Comissionshop()
    : inUpdate_(false)
    , lastUpdated_(0)
{
}


Comissionshop::~Comissionshop()
{
    unbind();
}

void Comissionshop::init()
{
    shopList_.clear();
    lastUpdated_ = 0;
    myId_ = Account::INVALID_ID;
    inUpdate_ = false;
}

void Comissionshop::bind(std::shared_ptr<Connection> connection)
{
    if (connection_.get() != NULL)
    {
        unbind();
    }

    connection_ = connection;

    HANDLE_C(cookies_, connection_, Comissionshop, EnterWorld);

    HANDLE_S(cookies_, connection_, Comissionshop, ComissionShop);
    HANDLE_S(cookies_, connection_, Comissionshop, ComissionShopList);
}

void Comissionshop::unbind()
{
    if (connection_.get() != NULL)
    {
        connection_->unbindHandlers(cookies_);
        connection_.reset();
    }

    init();
}

void Comissionshop::setElements(const elements::ItemListCollection & items)
{
    allItems_ = items;
}

void Comissionshop::setPropFactory(std::shared_ptr<ItemPropsFactory> propFactory)
{
    propFactory_ = propFactory;
}

void Comissionshop::update(bool trackProgress)
{
    if (!connection_ || inUpdate_)
    {
        return;
    }

    clientdata::FragmentComissionShopListRequest f;
    f.id = myId_;
    f.filter = 0xFF;
    f.unk1 = 0;

    connection_->send(&f);

    trackProgress_ = trackProgress;
    lastProgress_ = 0;
    inUpdate_ = true;
}

//

void Comissionshop::onEnterWorld(const clientdata::FragmentEnterWorld * f)
{
    myId_ = f->charId;
}

void Comissionshop::onComissionShopList(const serverdata::FragmentComissionShopList * f)
{
    if (!inUpdate_)
    {
        // спонтанные евенты, если юзер из клиента заходит например
        return;
    }

    shopList_.clear();

    for (const auto shop : f->shops)
    {
        CommiShop s;
        s.id = shop.id;
        s.time = shop.time;
        s.type = shop.type;

        shopList_.push_back(s);
    }

    {
        CommiListUpdatedEvent e;
        sEvent(&e);
    }

    if (!shopList_.empty())
    {
        updateShopListItem();
    }
    else
    {
        // means complete
        updateCompleted();
    }
}

void Comissionshop::updateCompleted()
{
    inUpdate_ = false;
    lastUpdated_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    CommiShopsUpdatedEvent e;
    sEvent(&e);
}

void Comissionshop::onComissionShop(const serverdata::FragmentComissionShop * f)
{
    if (!inUpdate_)
    {
        // спонтанные евенты, если юзер из клиента заходит например
        return;
    }

    unsigned id = f->id;
    auto iShop = std::find_if(shopList_.begin(), shopList_.end()
                              , [id](const CommiShop & shop)
                                { return id == shop.id; });
    if (iShop == shopList_.end())
    {
        Log("Commi shop record is missing for %08X", f->id);
        assert(0);
        return;
    }

    iShop->sellList.clear();
    iShop->buyList.clear();

    for (const auto & item : f->sellItems)
    {
        MarketItem m;
        m.id = item.id;
        m.slot = item.slot;
        m.count = item.count;
        m.price = item.price;
        fillProperty(m);
        if (!item.attrs.empty() && propFactory_)
        {
            m.props.reset(propFactory_->create(item.id, item.attrs));
        }

        iShop->sellList.push_back(m);
    }
    for (const auto & item : f->buyItems)
    {
        MarketItem m;
        m.id = item.id;
        m.slot = item.slot;
        m.count = item.count;
        m.price = item.price;
        fillProperty(m);
        if (!item.attrs.empty() && propFactory_)
        {
            m.props.reset(propFactory_->create(item.id, item.attrs));
        }

        iShop->buyList.push_back(m);
    }

    iShop->state = CommiShop::StateReady;

    unsigned filled = 0;
    for (const auto shop : shopList_)
    {
        if (shop.state == CommiShop::StateReady)
        {
            filled++;
        }
    }

    if (trackProgress_)
    {
        unsigned progress = (100 * filled) / shopList_.size();
        if (progress != lastProgress_)
        {
            lastProgress_ = progress;
            CommiShopsUpdateProgressEvent e(progress, 100);
            sEvent(&e);
        }
    }

    if (filled == shopList_.size())
    {
        updateCompleted();
    }
    else
    {
        updateShopListItem();
    }
}

//

void Comissionshop::updateShopListItem()
{
    if (!connection_)
    {
        return;
    }

    assert(myId_ != Account::INVALID_ID);

    auto it = shopList_.begin();
    int count = 0;
    do {
        while (it != shopList_.end() 
               && it->state != CommiShop::StateNew) ++it;
        if (it == shopList_.end())
        {
            break;
        }

        clientdata::FragmentComissionShopRequest f;
        f.id = myId_;
        f.shopId = it->id;
        connection_->send(&f);

        it->state = CommiShop::StateRequested;
    } while (++count < 1/*3*/);
}

void Comissionshop::fillProperty(MarketItem & item)
{
    elements::Item eItem;
    if (allItems_.getItem(item.id, eItem))
    {
        item.name = eItem["Name"];
        item.icon = eItem["file_icon"];
    }
    else
    {
        item.name = std::to_wstring(item.id);
    }

}

