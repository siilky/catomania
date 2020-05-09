
#include "stdafx.h"

#include "types.h"
#include "utils.h"
#include "netdata/fragments_giclient.h"
#include "netdata/fragments_npc.h"
#include "netdata/serialize.h"


namespace clientdata
{
    static const fragmentCollection_t fragments[] =
    {
        COLLECTION_ELEMENT_GI(Move),
        COLLECTION_ELEMENT_GI(Exit),
        COLLECTION_ELEMENT_GI(SelectTarget),
        COLLECTION_ELEMENT_GI(Attack),
        COLLECTION_ELEMENT_GI(ResurrectRespawn),
        COLLECTION_ELEMENT_GI(TakeItem),
        COLLECTION_ELEMENT_GI(MoveStop),
        COLLECTION_ELEMENT_GI(CancelTarget),
        COLLECTION_ELEMENT_GI(GetSelfItemInfo),
        COLLECTION_ELEMENT_GI(InventoryItemSwap),
        COLLECTION_ELEMENT_GI(InventoryItemMove),
        // 10
        COLLECTION_ELEMENT_GI(InventoryToEquipSwap),
        COLLECTION_ELEMENT_GI(GetCharStatus),
        COLLECTION_ELEMENT_GI(GetEquip),
        COLLECTION_ELEMENT_GI(OpenNpc),
        COLLECTION_ELEMENT_GI(NpcCommand),
        COLLECTION_ELEMENT_GI(NpcInteract),
        COLLECTION_ELEMENT_GI(GetFullInfo),
        COLLECTION_ELEMENT_GI(UseItem),
        // 30
        COLLECTION_ELEMENT_GI(Emotion),
        COLLECTION_ELEMENT_GI(ActivateTask),
        COLLECTION_ELEMENT_GI(AttackSkill),
        COLLECTION_ELEMENT_GI(GetCharAttrs),
        // 40
        COLLECTION_ELEMENT_GI(OpenMarket),
        COLLECTION_ELEMENT_GI(CancelMarket),
        COLLECTION_ELEMENT_GI(GetShopName),
        // 50
        COLLECTION_ELEMENT_GI(MarketSkill),
        COLLECTION_ELEMENT_GI(PlayerRevive),
        COLLECTION_ELEMENT_GI(PurchaseShopItem),
        //
        COLLECTION_ELEMENT_GI(ExchangeNotes),
        COLLECTION_ELEMENT_GI(OnlineRegistration),
        COLLECTION_ELEMENT_GI(GetRegistrationInfo),

        COLLECTION_END
    };    

    FragmentFactory fragmentGiFactory(identifyFragmentGameinfo, fragments, fragment_static_ctor<FragmentGameinfo>);

    //

    FragmentGiNpcInteract::~FragmentGiNpcInteract()
    {
        delete nested;
    }

    bool FragmentGiNpcInteract::parse()
    {
        checkOk();

        auto pos = payload_.begin();
        nested = fragmentNpcInteractFactory.create(payload_, pos);
        if (nested && nested->isOk())
        {
            nested->parse();
            return nested->isOk();
        }
        return false;
    }

    barray FragmentGiNpcInteract::assemble()
    {
        if (nested == 0)
        {
            return barray();
        }

        barray nestedData = nested->assemble();

        payload_.clear();
        payload_.resize(8);
        SerializerOut out(payload_);
        DWORD id = nested->getId();
        DWORD size = nestedData.size();
        out.lr(id).lr(size);
        payload_.insert(payload_.end(), nestedData.begin(), nestedData.end());

        return FragmentGameinfo::assemble(ID);
    }

    void FragmentGiNpcInteract::print(tostream & stream) const
    {
        #ifdef FRAGMENT_PRINTABLE
            checkParsed();

            stream << _T("  [25] Npc Interact  ");

            if (nested)
            {
                stream << _T("[") << hex << nested->getId() << _T("]") << endl
                    << nested;
            }
            stream << endl;

            dumpBytes(stream, payload_.begin(), payload_.end());
            stream << endl;
        #else
                (void)stream;
        #endif
    }

} // namespace
