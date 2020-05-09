#ifndef trade_h_
#define trade_h_

#include "netdata/connection.h"
#include "netdata/fragments_server.h"
#include "event.h"

class Trade
{
public:
    Trade();

    void init();
    void bind(std::shared_ptr<Connection> connection);
    void unbind();

    void accept(DWORD tradeId);
    void reject(DWORD tradeId);

    void discard();
    void submit();
    void confirm();

    void addItem(DWORD itemId, int inventorySlot, unsigned count, unsigned money);
    void removeItem(DWORD itemId, int inventorySlot, unsigned count, unsigned money);


    //void tick(Timestamp timestamp);
    
    // signals
    boost::signal<void (BaseEvent *event)>      sEvent;

private:
    static const DWORD INVALID_ID = (DWORD)-1;

    void onSelfInfo(const serverdata::FragmentGiSelfInfo *f);

    void onTradeRequest(const serverdata::FragmentTradeRequest *f);
    void onTradeStartRe(const serverdata::FragmentTradeStartRe *f);
    void onTradeEnd(const serverdata::FragmentTradeEnd *f);
    void onTradeAddGoodsRe(const serverdata::FragmentTradeAddGoodsRe *f);
    void onTradeRemoveGoodsRe(const serverdata::FragmentTradeRemoveGoodsRe *f);
    void onTradeSubmitRe(const serverdata::FragmentTradeSubmitRe *f);
    void onTradeConfirmRe(const serverdata::FragmentTradeConfirmRe *f);


    std::shared_ptr<Connection>   connection_;
    std::vector<Connection::Cookie> cookies_;

    DWORD       myId_;

    // active trade
    DWORD   activeId1_;
    DWORD   activeId2_;
    DWORD   activeId3_;
    DWORD   playerId_;      // partner id
};

#endif