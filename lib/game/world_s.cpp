#include "stdafx.h"

#include "boost/signal.hpp"

#include "common.h"
#include "log.h"
#include "game/ai/task.h"
#include "game/gtime.h"
#include "event.h"
#include "game/world_s.h"
#include "netdata/fragments_npc.h"


#define SERVER_HOOK(hooks, connection, fragment, handler)   (hooks.push_back(connection->bindServerHandler(     \
                                                                std::function<void (##fragment *)>            \
                                                                    (std::bind(handler, this, _1))) ))

#define CLIENT_HOOK(hooks, connection, fragment, handler)   (hooks.push_back(connection->bindClientHandler(     \
                                                                std::function<void (##fragment *)>            \
                                                                    (std::bind(handler, this, _1))) ))


World_s::World_s(std::shared_ptr<GTimer> timer)
    : gtimer_(timer)
{
    init();

    taskRoot_ \
        = task<TLoop>
        (
            (
                task<TDelay>(500)
                & task<TEvent>(std::bind(&World_s::onGuildResolveTick, this))
            )
            && (
                task<TDelay>(400)
                & task<TEvent>(std::bind(&World_s::onPlayerNameResolveTick, this))
            )
            && (
                task<TDelay>(300)
                & task<TEvent>(std::bind(&World_s::onPlayerMarketResolveTick, this))
            )
        );
}

World_s::~World_s()
{
    unbind();
}

void World_s::init()
{
    instanceId_ = (DWORD)-1;
    myId_       = (DWORD)-1;

    playerDB_.clear();
    battleMap_.clear();
    battleMapNeedResolve_ = false;
    guildNameCache_.clear();
    playerNameCache_.clear();
    playerNameResolveQ_.clear();
    guildResolveQ_.clear();
    playerMarketResolveQ_.clear();
}

void World_s::bind(std::shared_ptr<Connection> connection)
{
    if (connection_.get() != NULL)
    {
        unbind();
    }

    connection_ = connection;

    CLIENT_HOOK(registrations_, connection, const clientdata::FragmentEnterWorld, &World_s::onEnterGame);
    // TBD macros
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGetFactionBaseInfoRe, &World_s::onFactionBaseInfoRe);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentPlayerBaseInfoRe, &World_s::onPlayerBaseInfo);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentPlayerLogout, &World_s::onExit);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiCharacters, &World_s::onGiCharacters);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiObjectsAppear, &World_s::onObjectsAppear);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiRemoveObject1, &World_s::onGiObjectDisappear1);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiRemoveObject2, &World_s::onGiObjectDisappear2);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiRemoveObject3, &World_s::onGiObjectDisappear3);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiRemoveObject4, &World_s::onGiObjectDisappear4);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiOOSList, &World_s::onGiObjectsDisappear);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiPlayerEnter1, &World_s::onGiPlayerEnter1);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiPlayerEnter2, &World_s::onGiPlayerEnter2);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiNpcEnterSlice, &World_s::onGiNpcEnterSlice);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiNpcEnterWorld, &World_s::onGiNpcEnterWorld);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiNpcDead, &World_s::onGiNpcDead);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiNotifyPos, &World_s::onGiMapChange);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiServerConfig, &World_s::onGiServerConfig);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiObjectMove, &World_s::onGiObjectMove);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiObjectStopMove, &World_s::onGiObjectStopMove);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiServerTimestamp, &World_s::onServerTimestamp);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiPlayerOpenMarket, &World_s::onGiPlayerOpenMarket);
    //SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiPlayerSuccessMarket, &World_s::onGiPlayerSuccessMarket);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiPlayerCancelMarket, &World_s::onGiPlayerCancelMarket);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentGiPlayerMarketName, &World_s::onGiPlayerMarketName);

    SERVER_HOOK(registrations_, connection, const serverdata::FragmentBattleGetMapRe, &World_s::onBattleGetMap);
    SERVER_HOOK(registrations_, connection, const serverdata::FragmentBattleChallengeMapRe, &World_s::onBattleChallengeMapRe);

    SERVER_HOOK(registrations_, connection, const serverdata::FragmentStatusAnnounce, &World_s::onStatusAnnounce);
}

void World_s::unbind()
{
    if (connection_.get() != NULL)
    {
        connection_->unbindHandlers(registrations_);
        connection_.reset();
    }

    init();
}

void World_s::tick(Timestamp ts)
{
    taskRoot_->tick(ts);
}

const PlayerList & World_s::getPlayerList() const
{
    return playerDB_;
}

const NpcList & World_s::getNpcList() const
{
    return npcDB_;
}

const BattleMap & World_s::getBattleMap() const
{
    return battleMap_;
}

void World_s::updateBattleMap()
{
    if (!connection_)
    {
        return;
    }

    clientdata::FragmentBattleGetMap f;
    f.myId = myId_;
    f.unk = 0;
    connection_->send(&f);
}

void World_s::updateChallengeBattleMap()
{
    if (!connection_)
    {
        return;
    }

    clientdata::FragmentBattleChallengeMap *cmap = new clientdata::FragmentBattleChallengeMap();
    cmap->subtype = 0x56030000;
    cmap->myId = myId_;
    cmap->unk1 = 0;
    cmap->unk2 = 0;

    clientdata::FragmentGiNpcInteract *npcint = new clientdata::FragmentGiNpcInteract(cmap);
    clientdata::FragmentGameinfoSet fg(npcint);
    connection_->send(&fg);
}

const std::wstring & World_s::getPlayerName( DWORD charId )
{
    NameList::const_iterator it = playerNameCache_.find(charId);
    if (it != playerNameCache_.end())
    {
        return it->second;
    }
    else
    {
        static const std::wstring empty(L"");
        return empty;
    }
}

const std::wstring & World_s::getGuildName( DWORD guildId )
{
    GuildList::const_iterator it = guildNameCache_.find(guildId);
    if (it != guildNameCache_.end())
    {
        return it->second.name;
    }
    else
    {
        static const std::wstring empty(L"");
        return empty;
    }
}

std::tuple<bool /*success*/, bool /*isDelayed*/, std::wstring /*name*/> World_s::resolvePlayer(DWORD playerId)
{
    assert(playerId != 0);
    if (playerId == 0)
    {
        return std::make_tuple(false, false, std::wstring());
    }

    NameList::const_iterator it = playerNameCache_.find(playerId);
    if (it != playerNameCache_.end())
    {
        std::wstring playerName = it->second;
        return std::make_tuple(true, false, playerName);
    }
    else
    {
        // add request only if it's not queued yet
        if (playerNameResolveQ_.find(playerId) == playerNameResolveQ_.end())
        {
            playerNameResolveQ_.insert(std::make_pair(playerId, ResolveElement()));
        }
        return std::make_tuple(true, true, std::wstring());
    }
}

std::tuple<bool /*success*/, DWORD /*Id*/> World_s::resolvePlayer(const std::wstring & name)
{
    if (!name.empty())
    {
        for (auto it : playerNameCache_)
        {
            if (it.second == name)
            {
                return std::make_tuple(true, it.first);
            }
        }
    }
    return std::make_tuple(false, 0);
}

std::tuple<bool /*success*/, bool /*isDelayed*/, std::wstring /*name*/> World_s::resolveMarket(DWORD playerId)
{
    PlayerList::iterator it = playerDB_.find(playerId);
    if (it != playerDB_.end())
    {
        PlayerRecord & player = it->second;
        if ( ! player.attrs.isCat())
        {
            Log("Attempt to resolve market name of non-market player %08X", playerId);
            return std::make_tuple(false, false, std::wstring());
        }

        if ( ! player.marketName.empty())
        {
            std::wstring marketName = player.marketName;
            return std::make_tuple(true, false, marketName);
        }

        // add request only if it's not queued yet
        if (playerMarketResolveQ_.find(playerId) == playerMarketResolveQ_.end())
        {
            playerMarketResolveQ_.insert(std::make_pair(playerId, ResolveElement()));
        }
        return std::make_tuple(true, true, std::wstring());
    }

    return std::make_tuple(false, false, std::wstring());
}

bool World_s::resolveGuild(DWORD guildId , std::wstring & guildName)
{
    GuildList::const_iterator it = guildNameCache_.find(guildId);
    if (it != guildNameCache_.end())
    {
        guildName = it->second.name;
        return true;
    }
    else
    {
        // add request only if it's not queued yet
        if (guildResolveQ_.find(guildId) == guildResolveQ_.end())
        {
            guildResolveQ_.insert(std::make_pair(guildId, ResolveElement()));
        }
        return false;
    }
}

//

void World_s::onEnterGame(const clientdata::FragmentEnterWorld * f)
{
    playerDB_.clear();
    guildNameCache_.clear();
    playerNameCache_.clear();

    myId_ = f->charId;
}

void World_s::onFactionBaseInfoRe(const serverdata::FragmentGetFactionBaseInfoRe *f)
{
    //Log("%08X is %s", f->guildId, wstrToStr(f->guildName).c_str());

    GuildList::iterator it = guildNameCache_.find(f->guildId);
    if (it == guildNameCache_.end())
    {
        // add new
        GuildRecord newGuild;
        newGuild.name = f->guildName;
        guildNameCache_[f->guildId] = newGuild;
    }
    else
    {
        // update existing
        it->second.name = f->guildName;
    }

    std::map<DWORD, ResolveElement>::iterator rit = guildResolveQ_.find(f->guildId);
    if (rit != guildResolveQ_.end())
    {
        guildResolveQ_.erase(rit);

        GuildResolvedEvent e(f->guildId, f->guildName);
        sEvent(&e);
    }

    // fill in battlemap guild names
    if (battleMapNeedResolve_)
    {
        bool allResolved = true;
        for (BattleMap::iterator bmap = battleMap_.begin(); bmap != battleMap_.end(); ++bmap)
        {
            BattleZone & z = bmap->second;

            if (z.owningGuild != 0
                && z.owningGuildName.empty()
                && z.owningGuild == f->guildId)
            {
                z.owningGuildName = f->guildName;
            }
            if (z.attackingGuild != 0
                && z.attackingGuildName.empty()
                && z.attackingGuild == f->guildId)
            {
                z.attackingGuildName = f->guildName;
            }

            allResolved &= 
                (z.owningGuild != 0 ? !z.owningGuildName.empty() : true)
                && (z.attackingGuild != 0 ? !z.attackingGuildName.empty() : true);
        }

        if (allResolved)
        {
            battleMapNeedResolve_ = false;

            BattleMapUpdatedEvent e;
            sEvent(&e);
        }
    }
}

void World_s::onExit(const serverdata::FragmentPlayerLogout * /*f*/)
{
    playerDB_.clear();
    guildNameCache_.clear();        // we can relog to other server
    playerNameCache_.clear();
}

void World_s::onPlayerBaseInfo(const serverdata::FragmentPlayerBaseInfoRe *f)
{
    //Log("%08X is %ls", f->id, f->charName.c_str());

    playerNameCache_[f->id] = f->charName;

    std::map<DWORD, ResolveElement>::iterator rit = playerNameResolveQ_.find(f->id);
    if (rit != playerNameResolveQ_.end())
    {
        playerNameResolveQ_.erase(rit);

        PlayerResolvedEvent e(f->id, f->charName);
        sEvent(&e);
    }
}

void World_s::onGiCharacters(const serverdata::FragmentGiCharacters *f)
{
    //Log("Characters: %i", f->characters.size());

    for (size_t i = 0; i < f->characters.size(); i++)
    {
//        const struct serverdata::Character & c = f->characters[i];
//
//         #ifdef _DEBUG
//             std::pair< bool, DWORD >    guild = c.attrs.getGuild();
//             Log(": %08X  Guild %08X", c.id, guild.first ? guild.second : 0);
//         #endif

        addPlayer(f->characters[i]);
    }
}

void World_s::onObjectsAppear(const serverdata::FragmentGiObjectsAppear *f)
{
    //Log("Objects: %i", f->characters.size());

    for (size_t i = 0; i < f->objects.size(); i++)
    {
        addNpc(f->objects[i].id, f->objects[i].position, f->objects[i].type1);
    }
}

void World_s::onGiObjectDisappear1(const serverdata::FragmentGiRemoveObject1 *f)
{
    objectDisappear(f->id);
}

void World_s::onGiObjectDisappear2(const serverdata::FragmentGiRemoveObject2 *f)
{
    objectDisappear(f->id);
}

void World_s::onGiObjectDisappear3(const serverdata::FragmentGiRemoveObject3 *f)
{
    objectDisappear(f->id);
}

void World_s::onGiObjectDisappear4(const serverdata::FragmentGiRemoveObject4 *f)
{
    objectDisappear(f->id);
}

void World_s::onGiObjectsDisappear(const serverdata::FragmentGiOOSList *f)
{
    //Log("Objects: %i", f->objIds.size());

    for (unsigned i = 0; i < f->objIds.size(); i++)
    {
        //Log(": %08X", f->objIds[i]);
        objectDisappear(f->objIds[i]);
    }
}

void World_s::onGiPlayerEnter1(const serverdata::FragmentGiPlayerEnter1 *f)
{
    addPlayer(f->character);
}

void World_s::onGiPlayerEnter2(const serverdata::FragmentGiPlayerEnter2 *f)
{
    addPlayer(f->character);
}

void World_s::onGiNpcEnterSlice(const serverdata::FragmentGiNpcEnterSlice *f)
{
    // TBD angle
    addNpc(f->id, f->pos, f->type);
}

void World_s::onGiNpcEnterWorld(const serverdata::FragmentGiNpcEnterWorld *f)
{
    // TBD angle
    addNpc(f->id, f->pos, f->type);
}

void World_s::onGiNpcDead(const serverdata::FragmentGiNpcDead *f)
{
    //Log("Npc dead: %08X", f->id);
    npcDB_.erase(f->id);
}

void World_s::onGiMapChange(const serverdata::FragmentGiNotifyPos *f)
{
    Log("MapChange: %u : %.1f %.1f %.1f", f->instanceId, f->pos.x(), f->pos.y(), f->pos.z());

//     if (fragment->instanceId != world->instanceId_)
//     {
//         world->notify(WORLD_NOTIFY_MAPCHANGE, 0);
//     }
//     world->myInfo_.pos = fragment->pos;
//     world->notify(WORLD_NOTIFY_UPDATEMYPOS, world->myId_);
//     world->unlock();
}

void World_s::onGiServerConfig(const serverdata::FragmentGiServerConfig *f)
{
    Log("ServerConfig: %u", f->instanceId);

    instanceId_ = f->instanceId;
}

void World_s::onGiMyInfo(const serverdata::FragmentGiSelfInfo *f)
{
    //Log("%08X  %.1f %.1f %.1f", f->id, f->pos.x(), f->pos.y(), f->pos.z());

    PlayerRecord & charInfo = playerDB_[f->id];

    charInfo.pos = f->position;
    charInfo.attrs = f->attrs;
}

void World_s::onGiObjectMove(const serverdata::FragmentGiObjectMove *f)
{
    // this doesn't indicates my own moves
    objectMove(f->id, f->pos);
}

void World_s::onGiObjectStopMove(const serverdata::FragmentGiObjectStopMove *f)
{
    // this doesn't indicates my moves
    objectMove(f->id, f->pos);
}

void World_s::onServerTimestamp(const serverdata::FragmentGiServerTimestamp *f)
{
    Log("serverTime: %08X (%u)  serverTimeTz : %i", f->timestamp, f->timestamp, int(f->tz_min));

    Timestamp gt = gtimer_->serverTime();
    Timestamp newGt = GTime::unixToTs(f->timestamp);
    if (newGt > gt)
    {
        Log("Updating gameTime with: %llu", newGt);
        gtimer_->serverSync(newGt, int(f->tz_min));
    }
}

void World_s::onGiPlayerOpenMarket(const serverdata::FragmentGiPlayerOpenMarket *f)
{
    if (f->id == myId_)
    {
        return;
    }

    PlayerRecord & charInfo = playerDB_[f->id];
    charInfo.attrs.setCat(true);
    charInfo.marketName = f->name;

    // can't store empty names
    if (charInfo.marketName.empty())
    {
        charInfo.marketName = L" ";
    }
}

// void World_s::onGiPlayerSuccessMarket(const serverdata::FragmentGiPlayerSuccessMarket *f)
// {
//     if (f->id == myId_)
//     {
//         return;
//     }
// 
//     PlayerRecord & charInfo = playerDB_[f->id];
//     charInfo.attrs.setCat(false);
//     charInfo.marketName.clear();
// }

void World_s::onGiPlayerCancelMarket(const serverdata::FragmentGiPlayerCancelMarket *f)
{
    if (f->id == myId_)
    {
        return;
    }

    PlayerRecord & charInfo = playerDB_[f->id];
    charInfo.attrs.setCat(false);
    charInfo.marketName.clear();
}

void World_s::onGiPlayerMarketName(const serverdata::FragmentGiPlayerMarketName *f)
{
    PlayerRecord & charInfo = playerDB_[f->id];
    charInfo.marketName = f->name;

    // can't store empty names
    if (charInfo.marketName.empty())
    {
        charInfo.marketName = L" ";
    }

    std::map<DWORD, ResolveElement>::iterator it = playerMarketResolveQ_.find(f->id);
    if (it != playerMarketResolveQ_.end())
    {
        playerMarketResolveQ_.erase(it);

        MarketResolvedEvent e(f->id, f->name);
        sEvent(&e);
    }
}

void World_s::onBattleGetMap(const serverdata::FragmentBattleGetMapRe *f)
{
    bool guildsResolved = true;

    for (size_t i = 0; i < f->zones.size(); i++)
    {
        const serverdata::FragmentBattleGetMapRe::Zone & fz = f->zones[i];

        BattleZone & z = battleMap_[fz.zoneNumber];

        z.zoneLv         = fz.zoneLevel;
        z.owningGuild    = fz.owningClan;
        z.attackingGuild = fz.attackingClan;
        z.battleTime     = fz.battleTime;

        guildsResolved &= (z.owningGuild != 0 ? resolveGuild(z.owningGuild, z.owningGuildName) : true);
        guildsResolved &= (z.attackingGuild != 0 ? resolveGuild(z.attackingGuild, z.attackingGuildName) : true);
    }

    if (guildsResolved)
    {
        BattleMapUpdatedEvent e;
        sEvent(&e);
    }
    else
    {
        battleMapNeedResolve_ = true;
    }
}

void World_s::onBattleChallengeMapRe(const serverdata::FragmentBattleChallengeMapRe *f)
{
    for (size_t i = 0; i < f->zones.size(); i++)
    {
        const serverdata::FragmentBattleChallengeMapRe::Zone & fz = f->zones.at(i);

        BattleZone & z = battleMap_[fz.zoneNumber];

        z.chargeBack   = fz.chargeback;
        z.battleTime2  = fz.timestamp;
        z.id1          = fz.id1;
        z.id2          = fz.id2;
    }

    BattleMapUpdatedEvent e;
    sEvent(&e);
}

void World_s::onStatusAnnounce(const serverdata::FragmentStatusAnnounce * f)
{
    // tbd parameters
    StatusAnnounceEvent e(f->name);
    sEvent(&e);
}

//

void World_s::objectDisappear(DWORD id)
{
    //Log("Remove Obj: %08X", id);
    if (isNpc(id))
    {
        npcDB_.erase(id);
    }
    else
    {
        playerDB_.erase(id);
    }
} 

void World_s::addPlayer(const serverdata::Character & c)
{
    //Log("Player: %08X", f->character.id);

    PlayerRecord & charInfo = playerDB_[c.id];

    charInfo.pos = c.position;
    charInfo.attrs = c.attrs;
}

void World_s::addNpc(DWORD id, const CoordEx & pos, DWORD type)
{
    //Log("Add NPC: %08X", id);

    NpcRecord & npc = npcDB_[id];

    npc.type = type;
    npc.pos  = pos;
}

void World_s::objectMove(DWORD id, const Coord3D pos)
{
    if (isNpc(id))
    {
        npcDB_[id].pos = pos;
    }
    else
    {
        playerDB_[id].pos = pos;
    }
}

void World_s::onGuildResolveTick()
{
    if (guildResolveQ_.empty() || !connection_)
    {
        return;
    }

    clientdata::FragmentGetFactionBaseInfo f;
    f.myId = myId_;
    f.unk = 0;

    Timestamp now = gtimer_->time();
    std::map<DWORD, ResolveElement>::iterator it = guildResolveQ_.begin();

    while (f.guilds.size() < GUILD_REQUEST_MAX
            && it != guildResolveQ_.end())
    {
        if (it->second.requested
            && (it->second.requestedTs + REQUEST_TIMEOUT) > now)
        {
            ++it;
            continue;
        }

        f.guilds.push_back(it->first);

        it->second.requested = true;
        it->second.requestedTs = now;

        ++it;
    }

    if ( ! f.guilds.empty())
    {
        Log("Sending %i guild resolve requests", f.guilds.size());
        connection_->send(&f);
    }
}

void World_s::onPlayerNameResolveTick()
{
    if (playerNameResolveQ_.empty() || !connection_)
    {
        return;
    }

    clientdata::FragmentPlayerBaseInfo f;
    f.myId = myId_;
    f.unk = 0;
    f.unk2 = 1;

    Timestamp now = gtimer_->time();
    std::map<DWORD, ResolveElement>::iterator it = playerNameResolveQ_.begin();
    
    while (f.characters.size() < PLAYER_REQUEST_MAX
            && it != playerNameResolveQ_.end())
    {
        if (it->second.requested
            && (it->second.requestedTs + REQUEST_TIMEOUT) > now)
        {
            ++it;
            continue;
        }

        // OOS resolve works here

        f.characters.push_back(it->first);

        it->second.requested = true;
        it->second.requestedTs = now;

        ++it;
    }

    if ( ! f.characters.empty())
    {
        Log("Sending %i player resolve requests", f.characters.size());
        connection_->send(&f);
    }
}

void World_s::onPlayerMarketResolveTick()
{
    if (playerMarketResolveQ_.empty() || !connection_)
    {
        return;
    }

    Timestamp now = gtimer_->time();
    clientdata::FragmentGiGetShopName *f = new clientdata::FragmentGiGetShopName();

    std::map<DWORD, ResolveElement>::iterator it = playerMarketResolveQ_.begin();

    while (f->shops.size() < PLAYER_REQUEST_MAX
        && it != playerMarketResolveQ_.end())
    {
        if (it->second.requested
            && (it->second.requestedTs + REQUEST_TIMEOUT) > now)
        {
            ++it;
            continue;
        }

        // player may go out of sight so resolve woildn't work
        auto player = playerDB_.find(it->first);
        if (player == playerDB_.end()
            || !player->second.attrs.isCat())
        {
            Log("Canceling market resolve as it's OOS/not cat: %08X", it->first);
            it = playerMarketResolveQ_.erase(it);
            //assert(0);
            continue;
        }

        f->shops.push_back(it->first);

        it->second.requested = true;
        it->second.requestedTs = now;

        ++it;
    }

    if ( ! f->shops.empty())
    {
        Log("Sending %i market resolve requests", f->shops.size());
        clientdata::FragmentGameinfoSet s(f);
        connection_->send(&s);
    }
    else
    {
        delete f;
    }
}
