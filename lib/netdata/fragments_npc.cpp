
#include "stdafx.h"

#include "common.h"
#include "utils.h"
#include "netdata/fragments_npc.h"

namespace clientdata
{
    bool identifyFragmentNpcInteract(const barray & dataStream, barray::const_iterator & curPos, unsigned int *id)
    {
        size_t bytesLeft = dataStream.end() - curPos;
        if (bytesLeft < 8)
        {
            return false;
        }
    
        *id = getDWORD_r(curPos);
        bytesLeft -= 4;

        unsigned length = getDWORD_r(curPos);
        bytesLeft -= 4;
        if (bytesLeft < length)
        {
            return false;
        }

        return true;
    }


    static const fragmentCollection_t client_fragments[] =
    {
        COLLECTION_ELEMENT(BattleChallengeMap),
        COLLECTION_ELEMENT(RequestTask),
        
        COLLECTION_END
    };

    FragmentFactory fragmentNpcInteractFactory(identifyFragmentNpcInteract, client_fragments, fragment_static_ctor<FragmentBase>);

} // namespace
