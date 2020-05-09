#ifndef event_h_
#define event_h_

#include <assert.h>
#include "types.h"
#include "netdata\content.h"


class BaseEvent
{
public:
    enum Type
    {
        EventUnknown,

        // Login
        CharacterCreated,
        CharSelectRequest,
        CharselectCompleted,

        // Market
        MarketItemSold,
        MarketItemPurchased,

        // Commi
        CommiListUpdated,
        CommiShopsUpdateProgress,
        CommiShopsUpdated,

        // Player
        PlayerMarketState,
        PlayerDamaged,
        PlayerKilled,
        PlayerExp,
        PlayerLevelup,
        PlayerEnableResurrect,
        PlayerRevived,
        PlayerPositionNotify,
        LocationChange,

        // Inventory
        InventoryUpdate,
        DropItem,
        ObtainItem,

        // Errors
        ServerError,
        GameError,
        BanMessage,

        // Trade
        TradeRequest,
        TradeStart,
        TradeEnd,
        TradeAddItem,
        TradeRemoveItem,
        TradeConfirm,

        //
        SetLockTime,

        // Chat
        ChatMessage,
        TWBetMessage,

        // World
        BattleMapUpdated,

        StatusAnnounce,

        // Resolve
        PlayerResolved,
        MarketResolved,
        GuildResolved,

        // Battle
        TargetStatus,
        TargetInfo,
    };

    BaseEvent()
        : type_(EventUnknown)
    {}

    BaseEvent(Type t)
        : type_(t)
    {}
    virtual ~BaseEvent() {}

    Type type() const
    {
        return type_;
    }

    virtual BaseEvent *clone() const { assert(0); return NULL; }

private:
    Type type_;
};

template<class C>
class CopyableEvent : public BaseEvent
{
public:
    CopyableEvent(Type t)
        : BaseEvent(t)
    {}

    virtual BaseEvent *clone() const
    {
        return new C(*static_cast<const C *>(this));
    }
};


//

class CharacterCreatedEvent : public CopyableEvent < CharacterCreatedEvent >
{
public:
    CharacterCreatedEvent(DWORD charId, const std::wstring & name)
        : CopyableEvent(CharacterCreated)
        , charId_(charId)
        , name_(name)
    {}

    DWORD               charId() const  { return charId_; }
    const std::wstring  & name() const  { return name_; }

private:
    DWORD           charId_;
    std::wstring    name_;
};

class CharSelectRequestEvent : public CopyableEvent<CharSelectRequestEvent>
{
public:
    CharSelectRequestEvent()
        : CopyableEvent(CharSelectRequest)
    {}
};

class CharselectCompletedEvent : public CopyableEvent<CharselectCompletedEvent>
{
public:
    CharselectCompletedEvent()
        : CopyableEvent(CharselectCompleted)
    {}
};


// Market

// class MarketItemSoldEvent : public BaseEvent
// {
// public:
//     MarketItemSoldEvent
// };

// Player

class PlayerMarketStateEvent : public CopyableEvent<PlayerMarketStateEvent>
{
public:
    PlayerMarketStateEvent(bool isOpened)
        : CopyableEvent(PlayerMarketState)
        , isOpened_(isOpened)
    {}

    bool isOpened() const
    {
        return isOpened_;
    }

private:
    bool    isOpened_;
};

class PlayerKilledEvent : public CopyableEvent<PlayerKilledEvent>
{
public:
    PlayerKilledEvent(DWORD attacker)
        : CopyableEvent(PlayerKilled)
        , attacker_(attacker)
    {}

    DWORD attacker() const
    {
        return attacker_;
    }

private:
    DWORD   attacker_;
};

class PlayerDamagedEvent : public CopyableEvent<PlayerDamagedEvent>
{
public:
    PlayerDamagedEvent(DWORD attacker, int damage)
        : CopyableEvent(PlayerDamaged)
        , attacker_(attacker)
        , damage_(damage)
    {}

    DWORD attacker() const  { return attacker_; }
    int damage() const      { return damage_; }

private:
    DWORD   attacker_;
    int     damage_;
};

class PlayerEnableResurrectEvent : public CopyableEvent<PlayerEnableResurrectEvent>
{
public:
    PlayerEnableResurrectEvent()
        : CopyableEvent(PlayerEnableResurrect)
    {}
};

class PlayerRevivedEvent : public CopyableEvent<PlayerRevivedEvent>
{
public:
    PlayerRevivedEvent(unsigned stage)
        : CopyableEvent(PlayerRevived)
        , stage_(stage)
    {}

    unsigned stage() const
    {
        return stage_;
    }

private:
    unsigned stage_;
};

class PlayerExpEvent : public CopyableEvent<PlayerExpEvent>
{
public:
    PlayerExpEvent(unsigned exp, unsigned soul)
        : CopyableEvent(PlayerExp)
        , exp_(exp)
        , soul_(soul)
    {}

    unsigned exp() const
    {
        return exp_;
    }

    unsigned soul() const
    {
        return soul_;
    }

private:
    unsigned exp_;
    unsigned soul_;
};

class PlayerLevelUpEvent : public CopyableEvent<PlayerLevelUpEvent>
{
public:
    PlayerLevelUpEvent()
        : CopyableEvent(PlayerLevelup)
    {}
};


class PlayerPositionNotifyEvent : public CopyableEvent<PlayerPositionNotifyEvent>
{
public:
    PlayerPositionNotifyEvent(CoordEx position)
        : CopyableEvent(PlayerPositionNotify)
        , position_(position)
    {}

    CoordEx position() const
    {
        return position_;
    }

private:
    CoordEx position_;
};


class LocationChangeEvent : public CopyableEvent < LocationChangeEvent >
{
public:
    LocationChangeEvent(unsigned instanceId)
        : CopyableEvent(LocationChange)
        , instanceId_(instanceId)
    {
    }

    unsigned instanceId() const
    {
        return instanceId_;
    }

private:
    unsigned instanceId_;
};


// Inventory

class InventoryUpdateEvent : public CopyableEvent<InventoryUpdateEvent>
{
public:
    InventoryUpdateEvent(int storageType)
        : CopyableEvent(InventoryUpdate)
        , storageType_(storageType)
    {}

    int storageType() const { return storageType_; }

private:
    int storageType_;
};

class DropItemEvent : public CopyableEvent<DropItemEvent>
{
public:
    DropItemEvent(int storageType, int slot, int count, unsigned itemId)
        : CopyableEvent(DropItem)
        , storageType_(storageType)
        , slot_(slot)
        , itemId_(itemId)
        , count_(count)
    {}

    int storageType() const { return storageType_; }
    int slot() const        { return slot_; }
    DWORD itemId() const    { return itemId_; }
    unsigned count() const  { return count_; }

private:
    int         storageType_;
    int         slot_;
    DWORD       itemId_;
    unsigned    count_;
};

class ObtainItemEvent : public CopyableEvent<ObtainItemEvent>
{
public:
    ObtainItemEvent(int storageType, int slot, int count, unsigned itemId)
        : CopyableEvent(ObtainItem)
        , storageType_(storageType)
        , slot_(slot)
        , itemId_(itemId)
        , count_(count)
    {}

    int storageType() const { return storageType_; }
    int slot() const        { return slot_; }
    DWORD itemId() const    { return itemId_; }
    unsigned count() const  { return count_; }

private:
    int         storageType_;
    int         slot_;
    DWORD       itemId_;
    unsigned    count_;
};

class ItemSoldEvent : public CopyableEvent<ItemSoldEvent>
{
public:
    ItemSoldEvent(DWORD itemId, unsigned count, unsigned price, DWORD buyerId)
        : CopyableEvent(MarketItemSold)
        , itemId_(itemId), count_(count), price_(price), buyerId_(buyerId)
    {}

    DWORD itemId() const    { return itemId_; }
    unsigned count() const  { return count_; }
    unsigned price() const  { return price_; }
    DWORD buyerId() const   { return buyerId_; }

private:
    DWORD       itemId_;
    unsigned    count_;
    unsigned    price_;
    DWORD       buyerId_;
};

class ItemPurchasedEvent : public CopyableEvent<ItemPurchasedEvent>
{
public:
    ItemPurchasedEvent(DWORD itemId, unsigned count, unsigned price, unsigned tradeSlot)
        : CopyableEvent(MarketItemPurchased)
        , itemId_(itemId), count_(count), price_(price), tradeSlot_(tradeSlot)
    {}

    DWORD itemId() const        { return itemId_; }
    unsigned count() const      { return count_; }
    unsigned price() const      { return price_; }
    unsigned tradeSlot() const  { return tradeSlot_; }

private:
    DWORD       itemId_;
    unsigned    count_;
    unsigned    price_;
    unsigned    tradeSlot_;
};

//

class CommiListUpdatedEvent : public CopyableEvent<CommiListUpdatedEvent>
{
public:
    CommiListUpdatedEvent()
        : CopyableEvent(CommiListUpdated)
    {}
};

class CommiShopsUpdatedEvent : public CopyableEvent<CommiShopsUpdatedEvent>
{
public:
    CommiShopsUpdatedEvent()
        : CopyableEvent(CommiShopsUpdated)
    {}
};

class CommiShopsUpdateProgressEvent : public CopyableEvent<CommiShopsUpdateProgressEvent>
{
public:
    CommiShopsUpdateProgressEvent(unsigned value, unsigned maxValue)
        : CopyableEvent(CommiShopsUpdateProgress)
        , value_(value)
        , maxValue_(maxValue)
    {}

    unsigned value() const      { return value_; }
    unsigned maxValue() const   { return maxValue_; }

private:
    unsigned value_;
    unsigned maxValue_;
};

//

class ServerErrorEvent : public CopyableEvent<ServerErrorEvent>
{
public:
    ServerErrorEvent(unsigned id, const std::wstring & baseMessage, const std::wstring & message)
        : CopyableEvent(ServerError)
        , id_(id)
        , baseMessage_(baseMessage)
        , message_(message)
    {}

    unsigned id() const                         { return id_; }
    const std::wstring & baseMessage() const    { return baseMessage_; }
    const std::wstring & message() const        { return message_; }

private:
    unsigned        id_;
    std::wstring    baseMessage_;
    std::wstring    message_;
};

class GameErrorEvent : public CopyableEvent<GameErrorEvent>
{
public:
    GameErrorEvent(const std::wstring & message)
        : CopyableEvent(GameError)
        , message_(message)
    {}

    const std::wstring & message() const    { return message_; }

private:
    std::wstring    message_;
};

class BanMessageEvent : public CopyableEvent<BanMessageEvent>
{
public:
    BanMessageEvent(const std::wstring & message, DWORD banTs, DWORD secsRemaining)
        : CopyableEvent(BanMessage)
        , message_(message)
        , banTs_(banTs)
        , secsRemaining_(secsRemaining)
    {}

    const std::wstring & message() const    { return message_; }
    DWORD banTs() const                     { return banTs_; }
    DWORD secsRemaining() const             { return secsRemaining_; }

private:
    std::wstring message_;
    DWORD banTs_;
    DWORD secsRemaining_;
};


class TradeRequestEvent : public CopyableEvent<TradeRequestEvent>
{
public:
    TradeRequestEvent(DWORD playerId, DWORD tradeId1)
        : CopyableEvent(TradeRequest)
        , playerId_(playerId)
        , tradeId1_(tradeId1)
    {}

    DWORD playerId() const      { return playerId_; }
    DWORD tradeId() const       { return tradeId1_; }

private:
    DWORD playerId_;
    DWORD tradeId1_;
};

class TradeStartEvent : public CopyableEvent<TradeStartEvent>
{
public:
    TradeStartEvent(bool success, DWORD playerId)
        : CopyableEvent(TradeStart)
        , success_(success)
        , playerId_(playerId)
    {}

    bool  success() const    { return success_; }
    DWORD playerId() const  { return playerId_; }

private:
    bool success_;
    DWORD playerId_;
};

class TradeEndEvent : public CopyableEvent<TradeEndEvent>
{
public:
    TradeEndEvent(bool success)
        : CopyableEvent(TradeEnd)
        , success_(success)
    {}

    bool success() const    { return success_; }

private:
    bool success_;
};

class TradeAddItemEvent : public CopyableEvent<TradeAddItemEvent>
{
public:
    TradeAddItemEvent( int status, bool isSelf, DWORD itemId, const barray & attrs
                     , unsigned inventorySlot, unsigned count, unsigned money)
        : CopyableEvent(TradeAddItem)
        , status_(status)
        , isSelf_(isSelf)
        , itemId_(itemId)
        , attrs_(attrs)
        , inventorySlot_(inventorySlot)
        , count_(count)
        , money_(money)
    {}

    int        status() const           { return status_; }    // 0 is ok
    bool       isSelf() const           { return isSelf_; }
    DWORD      itemId() const           { return itemId_; }
    const barray & attrs() const        { return attrs_; }
    unsigned   inventorySlot() const    { return inventorySlot_; }
    unsigned   count() const            { return count_; }
    unsigned   money() const            { return money_; }

private:
    int      status_;
    bool     isSelf_;
    DWORD    itemId_;
    barray   attrs_;
    unsigned inventorySlot_;
    unsigned count_;
    unsigned money_;
};

class TradeRemoveItemEvent : public CopyableEvent<TradeRemoveItemEvent>
{
public:
    TradeRemoveItemEvent( int status, bool isSelf, DWORD itemId, const barray & attrs
                        , unsigned inventorySlot, unsigned count, unsigned money)
        : CopyableEvent(TradeRemoveItem)
        , status_(status)
        , isSelf_(isSelf)
        , itemId_(itemId)
        , attrs_(attrs)
        , inventorySlot_(inventorySlot)
        , count_(count)
        , money_(money)
    {}

    int      status() const         { return status_; }    // 0 is ok
    bool     isSelf() const         { return isSelf_; }
    DWORD    itemId() const         { return itemId_; }
    const barray & attrs() const    { return attrs_; }
    unsigned inventorySlot() const  { return inventorySlot_; }
    unsigned count() const          { return count_; }
    unsigned money() const          { return money_; }

private:
    int      status_;
    bool     isSelf_;
    DWORD    itemId_;
    barray   attrs_;
    unsigned inventorySlot_;
    unsigned count_;
    unsigned money_;
};

class TradeConfirmEvent : public CopyableEvent<TradeConfirmEvent>
{
public:
    enum ConfirmStage
    {
        None,
        Stage1,
        Stage2,
    };

    TradeConfirmEvent(bool isSelf, TradeConfirmEvent::ConfirmStage stage)
        : CopyableEvent(TradeConfirm)
        , isSelf_(isSelf)
        , stage_(stage)
    {}

    bool       isSelf() const   { return isSelf_; }
    ConfirmStage stage() const  { return stage_; }

private:
    bool isSelf_;
    ConfirmStage stage_;
};


class SetLockTimeEvent : public CopyableEvent<SetLockTimeEvent>
{
public:
    SetLockTimeEvent(bool isOk, unsigned seconds)
        : CopyableEvent(SetLockTime)
        , isOk_(isOk)
        , seconds_(seconds)
    {}

    bool isOk() const           { return isOk_; }
    unsigned seconds() const    { return seconds_; }

private:
    bool        isOk_;
    unsigned    seconds_;
};

//

class ChatMessageEvent : public CopyableEvent<ChatMessageEvent>
{
public:
    enum ChatType
    {
        Local,
        World,
        Horn,
        Clan,
        Party,
        Private,
        Trade,
        GMAnnounce,
        Unknown,
    };

    ChatMessageEvent(ChatType type, DWORD charId, const std::wstring & charName, const std::wstring & message)
        : CopyableEvent(ChatMessage)
        , type_(type)
        , charId_(charId)
        , charName_(charName)
        , message_(message)
    {}

    ChatType type() const                   { return type_; }
    DWORD charId() const                    { return charId_; }
    const std::wstring & charName() const   { return charName_; }
    const std::wstring & message() const    { return message_; }

private:
    ChatType        type_;
    DWORD           charId_;
    std::wstring    charName_;
    std::wstring    message_;
};

class TWBetMessageEvent : public CopyableEvent<TWBetMessageEvent>
{
public:
    TWBetMessageEvent(int zone, std::wstring clan, Timestamp closingTime)
        : CopyableEvent(TWBetMessage)
        , zone_(zone)
        , clan_(clan)
        , closingTime_(closingTime)
    {}

    int zone() const                    { return zone_; }
    const std::wstring & clan() const   { return clan_; }
    Timestamp closingTime() const       { return closingTime_; }

private:
    int             zone_;
    std::wstring    clan_;
    Timestamp       closingTime_;
};


class BattleMapUpdatedEvent : public CopyableEvent<BattleMapUpdatedEvent>
{
public:
    BattleMapUpdatedEvent()
        : CopyableEvent(BattleMapUpdated)
    {}
};


class StatusAnnounceEvent : public CopyableEvent<StatusAnnounceEvent>
{
public:
    StatusAnnounceEvent(const std::wstring & name)
        : CopyableEvent(StatusAnnounce)
        , name_(name)
    {}

    const std::wstring & name() const { return name_; }

private:
    std::wstring    name_;
};


class PlayerResolvedEvent : public CopyableEvent<PlayerResolvedEvent>
{
public:
    PlayerResolvedEvent(DWORD gId, const std::wstring & name)
        : CopyableEvent(PlayerResolved)
        , gId_(gId)
        , name_(name)
    {}

    DWORD gId() const                   { return gId_; }
    const std::wstring & name() const   { return name_; }

private:
    DWORD           gId_;
    std::wstring    name_;
};


class MarketResolvedEvent : public CopyableEvent<MarketResolvedEvent>
{
public:
    MarketResolvedEvent(DWORD gId, std::wstring name)
        : CopyableEvent(MarketResolved)
        , gId_(gId)
        , name_(name)
    {}

    DWORD gId() const                   { return gId_; }
    const std::wstring & name() const   { return name_; }

private:
    DWORD           gId_;
    std::wstring    name_;
};

class GuildResolvedEvent : public CopyableEvent<GuildResolvedEvent>
{
public:
    GuildResolvedEvent(DWORD gId, std::wstring name)
        : CopyableEvent(GuildResolved)
        , gId_(gId)
        , name_(name)
    {}

    DWORD gId() const                   { return gId_; }
    const std::wstring & name() const   { return name_; }

private:
    DWORD           gId_;
    std::wstring    name_;
};
    

class TargetStatusEvent : public CopyableEvent < TargetStatusEvent >
{
public:
    TargetStatusEvent(bool isPresent, unsigned wId = 0)
        : CopyableEvent(TargetStatus)
        , isPresent_(isPresent)
        , wId_(wId)
    {}

    bool     isPresent() const  { return isPresent_; }
    unsigned wId() const        { return wId_; }

private:
    bool     isPresent_;
    unsigned wId_;
};

class TargetInfoEvent : public CopyableEvent < TargetInfoEvent >
{
public:
    TargetInfoEvent(unsigned lvl, unsigned hp, unsigned maxHp, unsigned mp = 0, unsigned maxMp = 0)
        : CopyableEvent(TargetInfo)
        , lvl_(lvl)
        , hp_(hp)
        , maxHp_(maxHp)
        , mp_(mp)
        , maxMp_(maxMp)
    {}

    unsigned lvl() const    { return lvl_; }
    unsigned hp() const     { return hp_; }
    unsigned maxHp() const  { return maxHp_; }
    unsigned mp() const     { return mp_; }
    unsigned maxMp() const  { return maxMp_; }

private:
    unsigned    lvl_;
    unsigned    hp_, maxHp_;
    unsigned    mp_, maxMp_;
};


#endif