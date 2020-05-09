#ifndef world_types_h_
#define world_types_h_

#include "netdata/content.h"

//     typedef struct
//     {
//         DWORD           guildId;
//         Coord3D         pos;
//     } CharInfo;

struct PlayerRecord
{
    CoordEx         pos;
    CharAttrs       attrs;
    std::wstring    marketName;
};

typedef std::map< DWORD, PlayerRecord >   PlayerList;

struct NpcRecord
{
    NpcRecord()
        : type((DWORD)-1)
    {}

    DWORD           type;
    CoordEx         pos;
};

typedef std::map< DWORD /*wid*/, NpcRecord >   NpcList;

struct BattleZone
{
    BattleZone()
        : zoneLv(0)
        , owningGuild(0)
        , attackingGuild(0)
        , chargeBack(0)
        , id1(0)
        , id2(0)
        , battleTime(0)
        , battleTime2(0)
    {}

    short           zoneLv; 
    DWORD           owningGuild;        // = 0 if no guild
    std::wstring    owningGuildName;
    DWORD           attackingGuild;     // = 0 if no guild
    std::wstring    attackingGuildName;
    unsigned        chargeBack;
    unsigned        id1;
    unsigned        id2;
    DWORD           battleTime;
    DWORD           battleTime2;
};

typedef std::map<int /*zone number*/, BattleZone>   BattleMap;



#endif
