#ifndef player_h
#define player_h

#include "game/gtime.h"
#include "netdata/connection.h"
#include "netdata/fragments_server.h"
#include "netdata/fragments_client.h"
#include "netdata/fragments_giclient.h"
#include "event.h"


// Interface class to be used in controllers
class Player
{
public:
    static const DWORD INVALID_ID = (DWORD)-1;

    enum Effects
    {
        NO_EFFECT               = 0,
        PHYSIC_DEF_ENCHANT      = 0x0400,   // физ соски
        MAGIC_DEF_ENCHANT       = 0x0800,   // маг соски
        CRITICAL                = 0x1000,   // крит прошел
    };

    enum Availability
    {
        NO_INFO     = 0x00,
        BASIC_INFO  = 0x01,
        STAT_INFO   = 0x02,
        POS_INFO    = 0x04,
        ATTR_INFO   = 0x08,
    };

    enum LockState
    {
        LockStateUnknown = 0,
        LockStateOn     = 1,
        LockStateOff    = 2,
    };

    class BasicInfo
    {
    public:
        unsigned    lvl;

        unsigned    hp, maxHp;
        unsigned    mp, maxMp;

        unsigned getHpPercent() const { return maxHp != 0 ? (hp * 100) / maxHp : 0; }
        unsigned getMpPercent() const { return maxMp != 0 ? (mp * 100) / maxMp : 0; }
    };

    class PlayerInfo
    {
    public:
        BasicInfo   basic;

        unsigned    exp;
        unsigned    soul;

        unsigned    fury;
        unsigned    maxFury;

        bool        inBattle;

        unsigned getHpPercent() const   { return basic.getHpPercent(); }
        unsigned getMpPercent() const   { return basic.getMpPercent(); }
    };

    class StatInfo
    {
    public:
        unsigned    agis, strs, ints, vits;

        unsigned    hit, flee;
        float       crit;

        float       walkSpeed, flySpeed;

        // etc
    };


    Player(std::shared_ptr<GTimer> gtimer);
    ~Player();

    void init();
    void bind(std::shared_ptr<Connection> connection);
    void unbind();

    // signals
    boost::signal<void (BaseEvent *event)>  sEvent;

    //

    DWORD getMyId() const;

    Availability getAvailability() const;

    const PlayerInfo & getBasicInfo() const;
    const StatInfo & getStatInfo() const;
    const CharAttrs & getAttrs() const;

    unsigned    instanceId() const;
    const CoordEx & getPosition() const;

    // return true if skill is in cooldown state, otherwise false
    // returns timestamp of cooldown begin and ens, if found
    bool getCooldown(DWORD id, std::pair<Timestamp, Timestamp> & cooldown) const;
    bool getCooldown(DWORD id) const;

    bool        isAlive() const;        // returns true if player is alive and false otherwise

    Effects     getEffects() const;
    void        setEffects(Effects eff);

    LockState   lockState(Timestamp *remaining = 0) const;
    unsigned    lockTime() const;
    void        setLockTime(unsigned seconds);

    bool        isFlying() const;
    unsigned    getMoveCounter() const;
    unsigned    getMoveType() const;

    DWORD       getTarget() const;

    const std::vector<bool> & onlineRegistrationDays() const;

    //

    void onlineRegistration();
    void convertNotes(bool toMoney /*false: to notes*/, int count);

    //

    void tick(Timestamp timestamp);

private:
    typedef std::map< DWORD, std::pair<Timestamp, Timestamp> >      Cooldown;   // ID, (begin, end)

    void setEffect(WORD effects);
    void setPosition(const CoordEx & position);

    void onMyStatus(const serverdata::FragmentGiMyStatus *f);
    void onSelfInfo(const serverdata::FragmentGiSelfInfo *f);
    void onSelfGetProperty(const serverdata::FragmentGiSelfGetProperty *f);
    void onNotifyPos(const serverdata::FragmentGiNotifyPos *f);
    void onSelfTraceCurPos(const serverdata::FragmentGiSelfTraceCurPos *f);
    void onProtectionInfo(const serverdata::FragmentGiProtectionInfo *f);

    void onReceiveExp(const serverdata::FragmentGiReceiveExp *f);
    void onLevelUp(const serverdata::FragmentGiLevelUp *f);

    // battle
    void onBeAttacked(const serverdata::FragmentGiBeAttacked *f);
    void onBeSkillAttacked(const serverdata::FragmentGiBeSkillAttacked *f);
    void onBeHurt(const serverdata::FragmentGiBeHurt *f);
    void onBeKilled(const serverdata::FragmentGiBeKilled *f);
    void onPlayerRevival(const serverdata::FragmentGiPlayerRevival *f);
    void onEnableResurrectState(const serverdata::FragmentGiEnableResurrectState *f);
    void onSetCooldown(const serverdata::FragmentGiSetCooldown *f);

    // targeting
    void onPlayerInfo(const serverdata::FragmentGiPlayerInfo *f);
    void onNpcInfo(const serverdata::FragmentGiNpcInfo *f);
    void onPlayerSelectTarget(const serverdata::FragmentGiPlayerSelectTarget *f);
    void onRemoveObject4(const serverdata::FragmentGiRemoveObject4 *f); // TBD 1, 2, 3
    void onUnselect(const serverdata::FragmentGiUnselect *f);

    void onObjectTakeoff(const serverdata::FragmentGiObjectTakeoff *f);
    void onObjectLanding(const serverdata::FragmentGiObjectLanding *f);

    //
    void onSelfOpenMarket(const serverdata::FragmentGiSelfOpenMarket *f);
    void onPlayerCancelMarket(const serverdata::FragmentGiPlayerCancelMarket *f);

    void onSetLockTimeRe(const serverdata::FragmentSetLockTimeRe *f);
    void onOnlineRegistrationInfo(const serverdata::FragmentGiOnlineRegistrationInfo *f);

    // Client'

    void onEnterWorld(const clientdata::FragmentEnterWorld *f);
    void onMove(const clientdata::FragmentGiMove *f);
    void onMoveStop(const clientdata::FragmentGiMoveStop *f);

    //

    std::shared_ptr<GTimer>       gtimer_;

    std::shared_ptr<Connection>   connection_;
    std::vector<Connection::Cookie> cookies_;

    DWORD       myId_;
    int         availability_;
    PlayerInfo  charInfo_;
    StatInfo    statInfo_;

    unsigned    instanceId_;
    CoordEx     position_;
    WORD        moveSeqNum_;

    CharAttrs   attrs_;

    Cooldown    cooldowns_;
    Effects     effects_;

    DWORD       targetId_;
    BasicInfo   targetinfo_;

    LockState   lockState_;
    Timestamp   lockExpires_;
    unsigned    lockTime_;

    bool        isFlying_;

    std::vector<bool>   registrationDays_;
};


// Conditions

// struct PlayerBaseInfoCond
// {
//     typedef 
//     PlayerBaseInfoCond()
//     bool operator ()(std::shared_ptr<Game> game) const
//     {
//     }
// }

#endif
