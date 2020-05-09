#ifndef game_h
#define game_h


#include "netdata/fragments_server.h"
#include "game/player.h"
#include "game/world_s.h"
#include "game/inventory.h"
#include "game/account.h"
#include "game/chat.h"
#include "game/gtime.h"
#include "game/data/itemprops.h"
#include "game/trade.h"
#include "game/anticheat.h"
#include "game/gametask.h"
#include "game/comissionshop.h"

class Connection;

class Game
{
public:
    Game();
    ~Game();

    // signals

    boost::signal<void (BaseEvent *event)>      sBaseEvent;


    Account::Account &  getAccount()    { return account_; }
    Player &            getPlayer()     { return player_; }
    World_s &           getWorld()      { return world_; }
    Inventory &         getInventory()  { return inventory_; }
    Chat &              getChat()       { return chat_; }
    Trade &             getTrade()      { return trade_; }
    Comissionshop &     getCommi()      { return commi_; }
    GameTask &          gameTask()      { return gameTask_; }

    // Connection

    void init();
    void bind(std::shared_ptr<Connection> connection);
    void unbind();

    void setElements(const elements::ItemListCollection & items);
    void enableGac(bool enable = true)
    {
        gacEnabled_ = enable;
    }

    std::shared_ptr<ItemPropsFactory> getPropFactory() const  { return itemPropFactory_; }

    // Game error code

    const int getLastGameError() const
    {
        // errorCode = -1 in case of no error
        return lastGiError_;
    }
    void clearLastGameError();

    int getLastServerError() const
    {
        // errorCode = -1 in case of no error
        return lastServerError_;
    }

    //

    void tick(Timestamp timestamp);

    std::shared_ptr<GTimer> timer() const
    {
        return gtimer_;
    }

    // Task API

    std::shared_ptr<Connection> getConnection() const         { return connection_; }

private:
    void onBaseEvent(BaseEvent *event);

    void clearLastServerError();

    std::shared_ptr<Connection> connection_;

    std::shared_ptr<GTimer>             gtimer_;
    std::shared_ptr<ItemPropsFactory>   itemPropFactory_;

    Account::Account    account_;
    Player              player_;
    World_s             world_;
    Inventory           inventory_;
    Chat                chat_;
    Trade               trade_;
    AntiCheat           gac_;
    Comissionshop       commi_;
    GameTask            gameTask_;

    int                 lastGiError_;
    int                 lastServerError_;

    std::vector<Connection::Cookie> hooks_;

    bool                gacEnabled_;
    // handlers
    void onErrorMsg(const serverdata::FragmentGiErrorMsg *f);
    void onErrorInfo(const serverdata::FragmentErrorInfo *f);
    void onSelectRoleRe(const serverdata::FragmentSelectRoleRe *f);
    void onBannedMessage(const serverdata::FragmentBannedMessage *f);
};

#endif
