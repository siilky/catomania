#include "stdafx.h"

#include "boost/signal.hpp"

#include "common.h"
#include "log.h"
#include "game/gtime.h"
#include "netdata/fragments_server.h"
#include "netdata/fragments_client.h"
#include "game/player.h"

//#define FTRACE() Log("->")
#define FTRACE()


Player::Player(std::shared_ptr<GTimer> gtimer)
    : gtimer_(gtimer)
{
    init();
}

Player::~Player()
{
    unbind();
}

void Player::init()
{
    availability_ = NO_INFO;
    myId_ = INVALID_ID;
    cooldowns_.clear();
    effects_ = NO_EFFECT;
    targetId_ = INVALID_ID;
    lockExpires_ = 0;
    lockTime_ = 0;
    lockState_ = LockStateUnknown;
    isFlying_ = false;
    moveSeqNum_ = 0;
    instanceId_ = (DWORD) -1;
    position_ = CoordEx();
    registrationDays_.clear();
}

void Player::bind(std::shared_ptr<Connection> connection)
{
    if (connection_.get() != NULL)
    {
        unbind();
    }

    connection_ = connection;

    HANDLE_S_GI(cookies_, connection_, Player, MyStatus);
    HANDLE_S_GI(cookies_, connection_, Player, SelfInfo);
    HANDLE_S_GI(cookies_, connection_, Player, SelfGetProperty);
    HANDLE_S_GI(cookies_, connection_, Player, NotifyPos);
    HANDLE_S_GI(cookies_, connection_, Player, SelfTraceCurPos);
    HANDLE_S_GI(cookies_, connection_, Player, ProtectionInfo);
    HANDLE_S_GI(cookies_, connection_, Player, ReceiveExp);
    HANDLE_S_GI(cookies_, connection_, Player, LevelUp);
    
    HANDLE_S_GI(cookies_, connection_, Player, ObjectTakeoff);
    HANDLE_S_GI(cookies_, connection_, Player, ObjectLanding);

    HANDLE_S_GI(cookies_, connection_, Player, SetCooldown);
    HANDLE_S_GI(cookies_, connection_, Player, BeAttacked);
    HANDLE_S_GI(cookies_, connection_, Player, BeSkillAttacked);
    HANDLE_S_GI(cookies_, connection_, Player, BeHurt);
    HANDLE_S_GI(cookies_, connection_, Player, BeKilled);
    HANDLE_S_GI(cookies_, connection_, Player, EnableResurrectState);
    HANDLE_S_GI(cookies_, connection_, Player, PlayerRevival);
    HANDLE_S_GI(cookies_, connection_, Player, PlayerInfo);
    HANDLE_S_GI(cookies_, connection_, Player, NpcInfo);
    HANDLE_S_GI(cookies_, connection_, Player, PlayerSelectTarget);
    HANDLE_S_GI(cookies_, connection_, Player, RemoveObject4);
    HANDLE_S_GI(cookies_, connection_, Player, Unselect);

    HANDLE_S_GI(cookies_, connection_, Player, SelfOpenMarket);
    HANDLE_S_GI(cookies_, connection_, Player, PlayerCancelMarket);

    HANDLE_C_GI(cookies_, connection_, Player, Move);
    HANDLE_C_GI(cookies_, connection_, Player, MoveStop);

    HANDLE_S(cookies_, connection_, Player, SetLockTimeRe);
    HANDLE_S_GI(cookies_, connection_, Player, OnlineRegistrationInfo);
}

void Player::unbind()
{
    if (connection_.get() != NULL)
    {
        connection_->unbindHandlers(cookies_);
        connection_.reset();
    }

    init();
}

DWORD Player::getMyId() const
{
    return myId_;
}

Player::Availability Player::getAvailability() const
{
    return (Availability)availability_;
}

const Player::PlayerInfo & Player::getBasicInfo() const
{
    return charInfo_;
}

const Player::StatInfo & Player::getStatInfo() const
{
    return statInfo_;
}

unsigned Player::instanceId() const
{
    return instanceId_;
}

const CoordEx & Player::getPosition() const
{
    return position_;
}

const CharAttrs & Player::getAttrs() const
{
    return attrs_;
}

bool Player::getCooldown(DWORD id, std::pair<Timestamp, Timestamp> & cooldown) const
{
    Cooldown::const_iterator it = cooldowns_.find(id);
    if (it != cooldowns_.end())
    {
        cooldown = it->second;
        return true;
    }
    return false;
}

bool Player::getCooldown(DWORD id) const
{
    return cooldowns_.find(id) != cooldowns_.end();
}

bool Player::isAlive() const
{
    return ! attrs_.isDead();
}

Player::Effects Player::getEffects() const
{
    return effects_;
}

void Player::setEffects(Effects eff)
{
    effects_ = eff;
}

bool Player::isFlying() const
{
    return isFlying_;
}

unsigned Player::getMoveCounter() const
{
    return moveSeqNum_;
}

unsigned Player::getMoveType() const
{
    return isFlying_ ? MoveTypeMountFly : MoveTypeRun;;
}

DWORD Player::getTarget() const
{
    return targetId_;
}

const std::vector<bool> & Player::onlineRegistrationDays() const
{
    return registrationDays_;
}

void Player::onlineRegistration()
{
    if ( ! connection_)
    {
        return;
    }

    clientdata::FragmentGiOnlineRegistration *f = new clientdata::FragmentGiOnlineRegistration();
    clientdata::FragmentGameinfoSet fg(f);
    connection_->send(&fg);
}

void Player::convertNotes(bool toMoney /*false: to notes*/, int count)
{
    clientdata::FragmentGiExchangeNotes *f = new clientdata::FragmentGiExchangeNotes();
    f->direction = toMoney ? 1 : 0;
    f->count = count;

    clientdata::FragmentGameinfoSet fg(f);
    connection_->send(&fg);
}

void Player::tick(Timestamp timestamp)
{
    for (Cooldown::iterator it = cooldowns_.begin(); it != cooldowns_.end();)
    {
        if (it->second.second <= timestamp)
        {
            cooldowns_.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

Player::LockState Player::lockState(Timestamp *remaining /*= 0*/) const
{
    if (remaining != 0)
    {
        Timestamp now = gtimer_->serverTime();
        if (now == 0)
        {
            return LockStateUnknown;    // avoid displaying negative numbers
        }
        *remaining = now < lockExpires_ ? lockExpires_ - now : 0;
    }

    return lockState_;
}

unsigned Player::lockTime() const
{
    return lockTime_;
}

void Player::setLockTime(unsigned seconds)
{
    if (!connection_) return;

    clientdata::FragmentSetLockTime f;
    f.myId = myId_;
    f.seconds = seconds;
    f.unk1 = f.unk2 = 0;

    connection_->send(&f);
}

//

void Player::setEffect(WORD effects)
{
    if ((effects & PHYSIC_DEF_ENCHANT) != 0)
    {
        Log("Phys enchant reset");
        effects_ = (Effects)(effects_ & ~PHYSIC_DEF_ENCHANT);
    }
    else if ((effects & MAGIC_DEF_ENCHANT) != 0)
    {
        Log("Mag enchant reset");
        effects_ = (Effects)(effects_ & ~MAGIC_DEF_ENCHANT);
    }

    Log("Effect: %08X  actual %08X", effects, effects_);
}

void Player::setPosition(const CoordEx & position)
{
    availability_ |= POS_INFO;
    if ( ! position_.equalsTo(position))
    {
        position_ = position;
        PlayerPositionNotifyEvent e(position);
        sEvent(&e);
    }
}

void Player::onMyStatus(const serverdata::FragmentGiMyStatus *f)
{
    FTRACE();

    charInfo_.basic.lvl = f->lvl;
    charInfo_.basic.hp = f->hp;
    charInfo_.basic.maxHp = f->maxHp;
    charInfo_.basic.mp = f->mp;
    charInfo_.basic.maxMp = f->maxMp;
    charInfo_.exp = f->exp;
    charInfo_.soul = f->soul;
    charInfo_.fury = f->fury;
    charInfo_.maxFury = f->maxfury;
    charInfo_.inBattle = f->inBattle != 0;

//    isAlive_ = basicInfo_.hp != 0;

    availability_ |= BASIC_INFO;
}

void Player::onSelfInfo(const serverdata::FragmentGiSelfInfo *f)
{
    FTRACE();

    if (myId_ != INVALID_ID)
    {
        assert(myId_ == f->id);
    }
    else
    {
        myId_ = f->id;
    }

    setPosition(f->position);

    attrs_ = f->attrs;
    availability_ |= ATTR_INFO | POS_INFO;

    // TBD need a better solution
    if (attrs_.isDead())
    {
        PlayerKilledEvent e(INVALID_ID);
        sEvent(&e);
    }

    charInfo_.exp = f->exp;
    charInfo_.soul = f->soul;
}

void Player::onSelfGetProperty(const serverdata::FragmentGiSelfGetProperty *f)
{
    FTRACE();

    statInfo_.agis = f->agi_points;
    statInfo_.strs = f->str_points;
    statInfo_.vits = f->vit_points;
    statInfo_.ints = f->int_points;

    statInfo_.flySpeed = f->flySpeed;
    statInfo_.walkSpeed = f->walkSpeed;

    statInfo_.hit = f->hit;
    statInfo_.flee = f->flee;
    statInfo_.crit = f->crit;

    availability_ |= STAT_INFO;
}

void Player::onNotifyPos(const serverdata::FragmentGiNotifyPos *f)
{
    FTRACE();
    if (instanceId_ != f->instanceId)
    {
        instanceId_ = f->instanceId;
        LocationChangeEvent e(instanceId_);
        sEvent(&e);
    }

    setPosition(CoordEx(f->pos, position_.angle));
}

void Player::onSelfTraceCurPos(const serverdata::FragmentGiSelfTraceCurPos *f)
{
    FTRACE();

    Log("SelfTraceCurPos: [%.2f %.2f %.2f]"
        , f->pos.x()
        , f->pos.y()
        , f->pos.z());

    setPosition(f->pos);
    moveSeqNum_ = f->seqNum;

    availability_ |= POS_INFO;
}

void Player::onProtectionInfo(const serverdata::FragmentGiProtectionInfo *f)
{
    FTRACE();

#if PW_SERVER_VERSION >= 1640
    if (f->lock == serverdata::FragmentGiProtectionInfo::LockStateOn
        || f->lock == serverdata::FragmentGiProtectionInfo::LockStateOnV1)
    {
        lockState_ = LockStateOn;
    }
    else if (f->lock == serverdata::FragmentGiProtectionInfo::LockStateOff)
    {
        lockState_ = LockStateOff;
    }
    else
    {
        assert(0);
    }
#else
    lockState_ = f->lock != 0 ? LockStateOn : LockStateOff; 
#endif

    lockExpires_ = GTime::unixToTs(f->unlockTS);
    lockTime_ = f->lockTime;

    Log("LockGT: %llu  Lock:%i", lockExpires_, unsigned(f->lock));
    /*
    DWORD ts;
    if (f->lock)
    {
        ts = f->unlockTS - f->lockTime;
    }
    else
    {
        ts = f->unlockTS;
    }

    Timestamp gt = gtimer_->serverTime();
    Timestamp newGt = GTime::unixToTs(ts);

    if ((newGt - gt) > 3600 || (newGt - gt) < -3600)
    {
        Log("Updating gameTime with: %llu", newGt);
        gtimer_->serverSync(newGt);
    }*/
}

void Player::onReceiveExp(const serverdata::FragmentGiReceiveExp *f)
{
    charInfo_.exp += f->exp;
    charInfo_.soul += f->soul;

    PlayerExpEvent e(f->exp, f->soul);
    sEvent(&e);
}

void Player::onLevelUp(const serverdata::FragmentGiLevelUp *f)
{
    if (f->id != myId_)
    {
        return;
    }

    charInfo_.basic.lvl++;

    PlayerLevelUpEvent e;
    sEvent(&e);
}

void Player::onSetCooldown(const serverdata::FragmentGiSetCooldown *f)
{
    Timestamp now = gtimer_->time();
    cooldowns_[f->id] = std::make_pair(now, now + f->time);
}

void Player::onBeAttacked(const serverdata::FragmentGiBeAttacked *f)
{
    setEffect(f->effects);

    PlayerDamagedEvent e(f->attackerId, f->damage);
    sEvent(&e);
}

void Player::onBeSkillAttacked(const serverdata::FragmentGiBeSkillAttacked *f)
{
    setEffect(f->effects);

    PlayerDamagedEvent e(f->attackerId, f->damage);
    sEvent(&e);
}

void Player::onBeHurt(const serverdata::FragmentGiBeHurt *f)
{
    PlayerDamagedEvent e(f->attackerId, f->damage);
    sEvent(&e);
}

void Player::onBeKilled(const serverdata::FragmentGiBeKilled *f)
{
    attrs_.setDead(true);
    setEffects(NO_EFFECT);

    PlayerKilledEvent e(f->attackerId);
    sEvent(&e);
}

void Player::onEnableResurrectState(const serverdata::FragmentGiEnableResurrectState * /*f*/)
{
    PlayerEnableResurrectEvent e;
    sEvent(&e);

    // following TBD
}

void Player::onPlayerRevival(const serverdata::FragmentGiPlayerRevival *f)
{
    if (myId_ != INVALID_ID
        && myId_ == f->id)
    {
        if (f->stage == 0 || f->stage == 2)
        {
            attrs_.setDead(false);
        }
        PlayerRevivedEvent e(f->stage);
        sEvent(&e);

        setPosition(CoordEx(f->pos, position_.angle));
    }
}

void Player::onPlayerInfo(const serverdata::FragmentGiPlayerInfo *f)
{
    if (targetId_ != INVALID_ID
        && targetId_ != f->id)
    {
        return;
    }

    targetinfo_.lvl     = f->lvl;
    targetinfo_.hp      = f->hp;
    targetinfo_.maxHp   = f->maxHp;
    targetinfo_.mp      = f->mp;
    targetinfo_.maxMp   = f->maxMp;
           
    if (targetId_ == INVALID_ID)
    {
        // we don't have a target notification yet but already have the info
        targetId_ = f->id;
    }
    else
    {
        TargetInfoEvent e(targetinfo_.lvl, targetinfo_.hp, targetinfo_.maxHp, targetinfo_.mp, targetinfo_.maxMp);
        sEvent(&e);
    }
}

void Player::onNpcInfo(const serverdata::FragmentGiNpcInfo *f)
{
    if (targetId_ != INVALID_ID
        && targetId_ != f->id)
    {
        return;
    }

    targetinfo_.lvl     = 0;
    targetinfo_.hp      = f->hp;
    targetinfo_.maxHp   = f->maxHp;
    targetinfo_.mp      = 0;
    targetinfo_.maxMp   = 0;

    if (targetId_ == INVALID_ID)
    {
        // we don't have a target notification yet but already have the info
        targetId_ = f->id;
    }
    else
    {
        // no mp
        TargetInfoEvent e(targetinfo_.lvl, targetinfo_.hp, targetinfo_.maxHp);
        sEvent(&e);
    }
}

void Player::onPlayerSelectTarget(const serverdata::FragmentGiPlayerSelectTarget *f)
{
    Log("Selected: %08X", f->id);

    targetId_ = f->id;
    TargetStatusEvent e(true, targetId_);
    sEvent(&e);
}

void Player::onRemoveObject4(const serverdata::FragmentGiRemoveObject4 * f)
{
    Log("RemoveObject4: %08X", f->id);
    targetId_ = INVALID_ID;
    TargetStatusEvent e(false);
    sEvent(&e);
}

void Player::onUnselect(const serverdata::FragmentGiUnselect * /*f*/)
{
    targetId_ = INVALID_ID;
    TargetStatusEvent e(false);
    sEvent(&e);
}

void Player::onObjectTakeoff(const serverdata::FragmentGiObjectTakeoff *f)
{
    if (f->id == myId_)
    {
        isFlying_ = true;
    }
}

void Player::onObjectLanding(const serverdata::FragmentGiObjectLanding *f)
{
    if (f->id == myId_)
    {
        isFlying_ = false;
    }
}

void Player::onSelfOpenMarket(const serverdata::FragmentGiSelfOpenMarket * /*f*/)
{
    FTRACE();

    attrs_.setCat(true);
    PlayerMarketStateEvent event(true);
    sEvent(&event);
}

void Player::onPlayerCancelMarket(const serverdata::FragmentGiPlayerCancelMarket *f)
{
    FTRACE();

    if (f->id == myId_)
    {
        attrs_.setCat(false);
        PlayerMarketStateEvent event(false);
        sEvent(&event);
    }
}

void Player::onSetLockTimeRe(const serverdata::FragmentSetLockTimeRe *f)
{
    if (f->status == 0)
    {
        lockTime_ = f->seconds;
    }

    SetLockTimeEvent e(f->status == 0, f->seconds);
    sEvent(&e);
}

void Player::onOnlineRegistrationInfo(const serverdata::FragmentGiOnlineRegistrationInfo *f)
{
    registrationDays_.clear();

    for (size_t i = 0; i < 32; i++)
    {
        // bytes are bit reversed
        //int off = 8 * (1 + i/8) - i%8 - 1;
        registrationDays_.push_back((f->daysMask & (1 << i)) != 0);
    }
    // TBD check TS
}

void Player::onEnterWorld(const clientdata::FragmentEnterWorld * /*f*/)
{
    FTRACE();
    availability_ = NO_INFO;
}

void Player::onMove(const clientdata::FragmentGiMove *f)
{
    setPosition(CoordEx(f->posTo, position_.angle));   // questionable
    moveSeqNum_ = f->seqNum + 1;
}

void Player::onMoveStop(const clientdata::FragmentGiMoveStop *f)
{
    setPosition(CoordEx(f->pos, f->orientation));
    moveSeqNum_ = f->seqNum + 1;
}
