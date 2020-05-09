#ifndef fragments_npc_h
#define fragments_npc_h

#include "netdata/fragments.h"
#include "netdata/serialize.h"

namespace clientdata
{
    extern FragmentFactory fragmentNpcInteractFactory;

    //

    class BattleChallengeMapImpl    // this has another name probably
    {
    public:
        enum { ID = 0x001E };

        DWORD   subtype;    // 0x56030000
        DWORD   myId;
        DWORD   unk1;       // 0
        DWORD   unk2;       // 0

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(subtype).l(myId).l(unk1).l(unk2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [BattleChallengeMap]  myId:") << setw(8) << hex << myId
                << _T(" unk1:") << unk1
                << _T(" unk2:") << unk2
                << endl;
        }
    };

    typedef FragmentBaseSpec<BattleChallengeMapImpl>   FragmentBattleChallengeMap;

    
    class RequestTaskImpl    // this has another name probably
    {
    public:
        enum { ID = 0x0007 };

        DWORD   taskId;
        DWORD   unk1;       // 0
        DWORD   unk2;       // 0

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(taskId).lr(unk1).lr(unk2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Request task]  taskId:") << taskId
                << _T(" unk1:") << unk1
                << _T(" unk2:") << unk2
                << endl;
        }
    };

    typedef FragmentBaseSpec<RequestTaskImpl>   FragmentRequestTask;


} // namespace

#endif