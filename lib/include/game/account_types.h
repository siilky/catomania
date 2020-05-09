#ifndef account_types_h
#define account_types_h

namespace Account
{
    static const uint32_t   INVALID_ID = (uint32_t)-1;
    static const unsigned   CHARLIST_SIZE = 16;


    class CharInfo
    {
    public:
        CharInfo()
            : id(INVALID_ID)
        {}
        CharInfo(uint32_t id_, const std::wstring & name_, int status)
            : id(id_)
            , name(name_)
            , status(status)
        {}

        uint32_t        id;
        std::wstring    name;
        int             status;     // CharacterRecord::CharacterStatus
    };

    typedef std::vector<CharInfo>    CharList;
}


#endif
 