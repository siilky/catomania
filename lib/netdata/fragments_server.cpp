#include "stdafx.h"

#include "netdata/fragments_server.h"

namespace serverdata
{
    static const fragmentCollection_t fragments[] =
    {
        COLLECTION_ELEMENT(Array),
        COLLECTION_ELEMENT(ArrayPacked),
        COLLECTION_ELEMENT(GameinfoSet),
        COLLECTION_ELEMENT(AccInfo),
        COLLECTION_ELEMENT(ErrorInfo),
        COLLECTION_ELEMENT(StatusAnnounce),
        COLLECTION_ELEMENT(PlayerLogout),
        COLLECTION_ELEMENT(SelectRoleRe),
        // 50
        COLLECTION_ELEMENT(ChatMessage),
        COLLECTION_ELEMENT(CreateRoleRe),
        COLLECTION_ELEMENT(RoleListRe),
        COLLECTION_ELEMENT(Keepalive),
        COLLECTION_ELEMENT(PlayerBaseInfoRe),
        // 60
        COLLECTION_ELEMENT(PrivateChat),
        COLLECTION_ELEMENT(BannedMessage),
        COLLECTION_ELEMENT(WorldChat),
        COLLECTION_ELEMENT(LoginIpInfo),
        COLLECTION_ELEMENT(GetFriendsRe),
        COLLECTION_ELEMENT(SetLockTimeRe),
        COLLECTION_ELEMENT(BattleGetMapRe),
        COLLECTION_ELEMENT(BattleChallengeMapRe),
        //
        COLLECTION_ELEMENT(ComissionShop),
        COLLECTION_ELEMENT(ComissionShopList),

        COLLECTION_ELEMENT(TradeStartRe),
        COLLECTION_ELEMENT(TradeRequest),
        COLLECTION_ELEMENT(TradeAddGoodsRe),
        COLLECTION_ELEMENT(TradeRemoveGoodsRe),
        COLLECTION_ELEMENT(TradeSubmitRe),
        COLLECTION_ELEMENT(TradeConfirmRe),
        COLLECTION_ELEMENT(TradeDiscardRe),
        COLLECTION_ELEMENT(TradeEnd),
        // 1200
        COLLECTION_ELEMENT(FactionChat),
        COLLECTION_ELEMENT(GetFactionBaseInfoRe),

        COLLECTION_ELEMENT(ACWhoami),
        COLLECTION_ELEMENT(ACRemoteCode),
        COLLECTION_ELEMENT(ACProtoStat),
        COLLECTION_ELEMENT(ACStatusAnnounce),
        COLLECTION_ELEMENT(ACReportCheater),
        COLLECTION_ELEMENT(ACTriggerQuestion),
        COLLECTION_ELEMENT(ACQuestion),
        COLLECTION_ELEMENT(ACAnswer),

        COLLECTION_END
    };

    FragmentFactory fragmentFactory(identifyFragment, fragments, fragment_static_ctor<Fragment>);


} // namespace
