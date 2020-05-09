
#include "stdafx.h"

#include "common.h"
#include "utils.h"
#include "netdata/fragments_giserver.h"
#include "netdata/content.h"

namespace serverdata
{
    static const fragmentCollection_t fragments[] =
    {
        COLLECTION_ELEMENT_GI(Characters),
        COLLECTION_ELEMENT_GI(SelfInfo),
        COLLECTION_ELEMENT_GI(ObjectsAppear),
        COLLECTION_ELEMENT_GI(Items),
        COLLECTION_ELEMENT_GI(NpcEnterSlice),
        // 10
        COLLECTION_ELEMENT_GI(NpcEnterWorld),
        COLLECTION_ELEMENT_GI(RemoveObject1),
        COLLECTION_ELEMENT_GI(RemoveObject2),
        COLLECTION_ELEMENT_GI(RemoveObject3),
        COLLECTION_ELEMENT_GI(RemoveObject4),
        COLLECTION_ELEMENT_GI(NotifyPos),
        COLLECTION_ELEMENT_GI(PlayerEnter1),
        COLLECTION_ELEMENT_GI(PlayerEnter2),
        COLLECTION_ELEMENT_GI(ObjectMove),
        COLLECTION_ELEMENT_GI(MatterEnterWorld),
        COLLECTION_ELEMENT_GI(NpcDead),
        COLLECTION_ELEMENT_GI(SelfAttackResul),
        COLLECTION_ELEMENT_GI(ErrorMsg),
        COLLECTION_ELEMENT_GI(BeAttacked),
        COLLECTION_ELEMENT_GI(BeKilled),
        COLLECTION_ELEMENT_GI(PlayerRevival),
        COLLECTION_ELEMENT_GI(PickupMoney),
        COLLECTION_ELEMENT_GI(PlayerPickupItem),
        // 20
        COLLECTION_ELEMENT_GI(PlayerInfo),
        COLLECTION_ELEMENT_GI(NpcInfo),
        COLLECTION_ELEMENT_GI(OOSList),
        COLLECTION_ELEMENT_GI(ObjectStopMove),
        COLLECTION_ELEMENT_GI(ReceiveExp),
        COLLECTION_ELEMENT_GI(LevelUp),
        COLLECTION_ELEMENT_GI(MyStatus),
        COLLECTION_ELEMENT_GI(Unselect),
        COLLECTION_ELEMENT_GI(SelfItemInfo),
        COLLECTION_ELEMENT_GI(SelfItemEmptyInfo),
        COLLECTION_ELEMENT_GI(ExchangeInventoryItem),
        COLLECTION_ELEMENT_GI(MoveInventoryItem),
        COLLECTION_ELEMENT_GI(PlayerDropItem),
        COLLECTION_ELEMENT_GI(ExchangeEquipmentItem),
        // 30
        COLLECTION_ELEMENT_GI(EquipItem),
        COLLECTION_ELEMENT_GI(MoveEquipmentItem),
        COLLECTION_ELEMENT_GI(SelfInventoryDetail),
        COLLECTION_ELEMENT_GI(SelfGetProperty),
        COLLECTION_ELEMENT_GI(PlayerSelectTarget),
        // 40
        COLLECTION_ELEMENT_GI(SendEquipmentInfo),
        COLLECTION_ELEMENT_GI(NpcGreeting),
        COLLECTION_ELEMENT_GI(PlayerPurchaseItem),
        COLLECTION_ELEMENT_GI(ItemToMoney),
        COLLECTION_ELEMENT_GI(SpendMoney),
        // 50
        COLLECTION_ELEMENT_GI(GetOwnMoney),
        COLLECTION_ELEMENT_GI(ObjectCastSkill),
        COLLECTION_ELEMENT_GI(SkillData),
        COLLECTION_ELEMENT_GI(PlayerUseItem),
        // 60
        COLLECTION_ELEMENT_GI(ObjectTakeoff),
        COLLECTION_ELEMENT_GI(ObjectLanding),
        COLLECTION_ELEMENT_GI(PlayerObtainItem),
        COLLECTION_ELEMENT_GI(ProduceOnce),
        COLLECTION_ELEMENT_GI(TaskVarData),
        // 70
        COLLECTION_ELEMENT_GI(DoEmotion),
        COLLECTION_ELEMENT_GI(ServerTimestamp),
        COLLECTION_ELEMENT_GI(ObjectAttackResult),
        COLLECTION_ELEMENT_GI(BeHurt),
        COLLECTION_ELEMENT_GI(PlayerGatherStart),
        COLLECTION_ELEMENT_GI(PlayerGatherStop),
        // 80
        COLLECTION_ELEMENT_GI(ExchangeTrashboxItem),
        COLLECTION_ELEMENT_GI(MoveTrashboxItem),
        COLLECTION_ELEMENT_GI(ExchangeTrashboxInventory),
        COLLECTION_ELEMENT_GI(InventoryItemToTrash),
        COLLECTION_ELEMENT_GI(TrashItemToInventory),
        COLLECTION_ELEMENT_GI(EnchantResult),
        COLLECTION_ELEMENT_GI(SelfSkillAttackResult),
        COLLECTION_ELEMENT_GI(ObjectSkillAttackResult),
        // 90
        COLLECTION_ELEMENT_GI(BeSkillAttacked),
        COLLECTION_ELEMENT_GI(MatterPickup),
        COLLECTION_ELEMENT_GI(TaskDeliverItem),
        COLLECTION_ELEMENT_GI(PlayerOpenMarket),
        // A0
        COLLECTION_ELEMENT_GI(SelfOpenMarket),
        COLLECTION_ELEMENT_GI(PlayerCancelMarket),
        COLLECTION_ELEMENT_GI(PlayerMarketInfo),
        COLLECTION_ELEMENT_GI(PlayerSuccessMarket),
        COLLECTION_ELEMENT_GI(PlayerMarketName),
        // B0
        COLLECTION_ELEMENT_GI(SelfTraceCurPos),
        COLLECTION_ELEMENT_GI(PlayerWaypointList),
        COLLECTION_ELEMENT_GI(UnlockInventorySlot),
        COLLECTION_ELEMENT_GI(PersonalMarketAvailable),
        COLLECTION_ELEMENT_GI(TradeAwayItem),
        // C0
        COLLECTION_ELEMENT_GI(EnableResurrectState),
        COLLECTION_ELEMENT_GI(SetCooldown),
        COLLECTION_ELEMENT_GI(ServerConfig),
        COLLECTION_ELEMENT_GI(PlayerRushMode),
        // F0
        COLLECTION_ELEMENT_GI(PlayerCash),
        // 100
        COLLECTION_ELEMENT_GI(ProtectionInfo),
        COLLECTION_ELEMENT_GI(OnlineRegistrationInfo),
        COLLECTION_END
    };    

    FragmentFactory fragmentGiFactory(identifyFragmentGameinfo, fragments, fragment_static_ctor<FragmentGameinfo>);


} // namespace
