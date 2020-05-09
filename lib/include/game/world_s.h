#ifndef world_s_h
#define world_s_h

#include "game/world_types.h"
#include "netdata/content.h"
#include "netdata/fragments_server.h"
#include "netdata/fragments_client.h"
#include "netdata/connection.h"
#include "gtime.h"

class ITask;

class World_s
{
public:
    World_s(std::shared_ptr<GTimer> gtimer);
    ~World_s();

    void init();
    void bind(std::shared_ptr<Connection> connection);
    void unbind();

    void tick(Timestamp ts);

    //
    // World access
    //

    // returns true if charinfo exists, false otherwise
    //bool getCharInfo(DWORD id, CharInfo & info);

    const PlayerList & getPlayerList() const;
    const NpcList & getNpcList() const;
    const BattleMap & getBattleMap() const;

    void updateBattleMap();
    void updateChallengeBattleMap();    // performed via NPC interacton

    // statically gets name or empty string if name missing
    const std::wstring & getPlayerName(DWORD charId);
    const std::wstring & getGuildName(DWORD guildId);

    // resolve name: 
    // true: resolved and name returned or delayed set
    // false and call resolve signal function later
    std::tuple<bool /*success*/, bool /*isDelayed*/, std::wstring /*name*/> resolvePlayer(DWORD playerId);
    std::tuple<bool /*success*/, bool /*isDelayed*/, std::wstring /*name*/> resolveMarket(DWORD playerId);

    bool resolveGuild(DWORD guildId, std::wstring & guildName);

    // resolve ID: 
    std::tuple<bool /*success*/, DWORD /*Id*/> resolvePlayer(const std::wstring & name);

    // signals

    boost::signal<void (BaseEvent *event)>                      sEvent;

private:
    enum { GUILD_REQUEST_MAX  = 50 };
    enum { PLAYER_REQUEST_MAX = 50 };
    enum { REQUEST_TIMEOUT    = 10000 };

    struct GuildRecord
    {
        wstring    name;
    };

    struct ResolveElement
    {
        ResolveElement()
            : requested(false)
            , requestedTs(0)
        {}

        bool        requested;
        Timestamp   requestedTs;
    };

    typedef std::map< DWORD, GuildRecord >  GuildList;
    typedef std::map< DWORD, std::wstring > NameList;

    // Data

    std::shared_ptr<GTimer>   gtimer_;

    DWORD           instanceId_;
    DWORD           myId_;

    BattleMap       battleMap_;
    bool            battleMapNeedResolve_;

    PlayerList      playerDB_;
    NpcList         npcDB_;
    GuildList       guildNameCache_;
    NameList        playerNameCache_;

    std::map<DWORD, ResolveElement>    guildResolveQ_;
    std::map<DWORD, ResolveElement>    playerNameResolveQ_;
    std::map<DWORD, ResolveElement>    playerMarketResolveQ_;

    // TBD bool            playerAutoResolve_;

    std::shared_ptr<ITask>    taskRoot_;

    // connection

    std::shared_ptr<Connection>   connection_;
    std::vector<Connection::Cookie> registrations_;

    //

    void onEnterGame(const clientdata::FragmentEnterWorld *f);
    void onExit(const serverdata::FragmentPlayerLogout *f);
    void onFactionBaseInfoRe(const serverdata::FragmentGetFactionBaseInfoRe *f);
    void onPlayerBaseInfo(const serverdata::FragmentPlayerBaseInfoRe *f);
    void onGiCharacters(const serverdata::FragmentGiCharacters *f);
    void onObjectsAppear(const serverdata::FragmentGiObjectsAppear *f);

    void onGiObjectsDisappear(const serverdata::FragmentGiOOSList *f);
    void onGiObjectDisappear1(const serverdata::FragmentGiRemoveObject1 *f);
    void onGiObjectDisappear2(const serverdata::FragmentGiRemoveObject2 *f);
    void onGiObjectDisappear3(const serverdata::FragmentGiRemoveObject3 *f);
    void onGiObjectDisappear4(const serverdata::FragmentGiRemoveObject4 *f);
    void onGiPlayerEnter1(const serverdata::FragmentGiPlayerEnter1 *f);
    void onGiPlayerEnter2(const serverdata::FragmentGiPlayerEnter2 *f);
    void onGiNpcEnterSlice(const serverdata::FragmentGiNpcEnterSlice *f);
    void onGiNpcEnterWorld(const serverdata::FragmentGiNpcEnterWorld *f);
    void onGiNpcDead(const serverdata::FragmentGiNpcDead *f);
    void onGiMapChange(const serverdata::FragmentGiNotifyPos *f);
    void onGiServerConfig(const serverdata::FragmentGiServerConfig *f);
    void onGiMyInfo(const serverdata::FragmentGiSelfInfo *f);
    void onGiObjectMove(const serverdata::FragmentGiObjectMove *f);
    void onGiObjectStopMove(const serverdata::FragmentGiObjectStopMove *f);
   
    void onServerTimestamp(const serverdata::FragmentGiServerTimestamp *f);

    void onGiPlayerOpenMarket(const serverdata::FragmentGiPlayerOpenMarket *f);
    //void onGiPlayerSuccessMarket(const serverdata::FragmentGiPlayerSuccessMarket *f);
    void onGiPlayerCancelMarket(const serverdata::FragmentGiPlayerCancelMarket *f);
    void onGiPlayerMarketName(const serverdata::FragmentGiPlayerMarketName *f);

    //

    void objectDisappear(DWORD id);
    void addPlayer(const serverdata::Character & c);
    void addNpc(DWORD id, const CoordEx & pos, DWORD type);
    void objectMove(DWORD id, const Coord3D pos);

    void onBattleGetMap(const serverdata::FragmentBattleGetMapRe *f);
    void onBattleChallengeMapRe(const serverdata::FragmentBattleChallengeMapRe *f);

    void onStatusAnnounce(const serverdata::FragmentStatusAnnounce * f);

    //

    inline bool isNpc(DWORD id) const
    {
        return (id & CHARACTER_NPC_FLAG) != 0;
    }

    void onGuildResolveTick();
    void onPlayerNameResolveTick();
    void onPlayerMarketResolveTick();
};

#endif