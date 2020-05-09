#ifndef account_h
#define account_h

#include "netdata/connection.h"
#include "netdata/fragments_server.h"
#include "netdata/fragments_client.h"
#include "account_types.h"

class BaseEvent;

namespace Account
{
    class Account
    {
    public:
        Account();
        ~Account();

        void bind(std::shared_ptr<Connection> connection);
        void unbind();

        void init();

        bool getAccountId(DWORD & accountId) const;
        CharList getCharList() const;
        bool selectChar(const std::wstring & name);

        // return false if character is not selected, otherwise reference to a selected character
        bool getCurrentChar(CharInfo & charinfo) const;

        void setIngameCharId(DWORD charId);
        void setIngameCharName(const std::wstring & charName);

        void getIpInfo(DWORD & lastLoginTs, DWORD & lastIp, DWORD & currentIp) const;

        void createCharacter(const std::wstring & name);

        //

        boost::signal<void(BaseEvent *event)>   sEvent;

    private:
        void enterWorld();

        void onLoginIpInfo(const serverdata::FragmentLoginIpInfo *f);
        void onAccInfo(const serverdata::FragmentAccInfo *f);
        void onCreateRoleRe(const serverdata::FragmentCreateRoleRe *f);
        void onRoleListRe(const serverdata::FragmentRoleListRe *f);
        void onSelectRole(const clientdata::FragmentSelectRole *f);
        void onSelectRoleRe(const serverdata::FragmentSelectRoleRe *f);
        void onPlayerLogout(const serverdata::FragmentPlayerLogout *f);   // imagine charselect via client

        std::shared_ptr<Connection>   connection_;
        std::vector<Connection::Cookie> hooks_;

        DWORD       lastLoginTs_;
        DWORD       lastIp_;
        DWORD       currentIp_;

        DWORD       accountId_;
        CharList    charList_;
        unsigned    currentChar_;
        unsigned    lastCharSelection_;
        bool        autoEnterWorld_;
    };

}


#endif
 