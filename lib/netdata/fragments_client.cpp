
#include "stdafx.h"

#include "common.h"
#include "utils.h"
#include "netdata/fragments_client.h"

namespace clientdata
{
    static const fragmentCollection_t client_fragments[] =
    {
        COLLECTION_ELEMENT(Array),
        COLLECTION_ELEMENT(GameinfoSet),
        COLLECTION_ELEMENT(SelectRole),
        COLLECTION_ELEMENT(EnterWorld),
        COLLECTION_ELEMENT(ChatMessage),
        // 50
        COLLECTION_ELEMENT(RoleList),
        COLLECTION_ELEMENT(CreatRole),
        COLLECTION_ELEMENT(PlayerBaseInfo),
        // 60
        COLLECTION_ELEMENT(PlayerBriefInfo),
        COLLECTION_ELEMENT(PrivateChat),
        COLLECTION_ELEMENT(GetFriends),
        COLLECTION_ELEMENT(SetLockTime),
        COLLECTION_ELEMENT(BattleGetMap),
        // 3xx
        COLLECTION_ELEMENT(ComissionShopListRequest),
        COLLECTION_ELEMENT(ComissionShopRequest),

        COLLECTION_ELEMENT(TradeResponse),
        COLLECTION_ELEMENT(TradeAddGoods),
        COLLECTION_ELEMENT(TradeRemoveGoods),
        COLLECTION_ELEMENT(TradeConfirm),
        COLLECTION_ELEMENT(TradeSubmit),
        COLLECTION_ELEMENT(TradeDiscard),
        //
        COLLECTION_ELEMENT(FactionChat),
        COLLECTION_ELEMENT(GetFactionBaseInfo),

        COLLECTION_ELEMENT(ACReport),
        COLLECTION_ELEMENT(ACWhoami),
        COLLECTION_ELEMENT(ACProtoStat),
        COLLECTION_ELEMENT(ACStatusAnnounce),
        COLLECTION_ELEMENT(ACReportCheater),
        COLLECTION_ELEMENT(ACTriggerQuestion),
        COLLECTION_ELEMENT(ACQuestion),
        COLLECTION_ELEMENT(ACAnswer),


        COLLECTION_END
    };

    FragmentFactory fragmentFactory(identifyFragment, client_fragments, fragment_static_ctor<Fragment>);

} // namespace
