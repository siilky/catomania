#ifndef chat_h
#define chat_h

#include "netdata/connection.h"
#include "netdata/fragments_server.h"
#include "netdata/fragments_client.h"
#include "event.h"


class Chat
{
public:
    static const DWORD INVALID_ID = (DWORD)-1;

    enum ChatType
    {
        ChatTypePublic,
        ChatTypeGroup,
    };

    Chat();
    ~Chat();

    void init();
    void bind(std::shared_ptr<Connection> connection);
    void unbind();

    void setMyName(const std::wstring & name);

    // signals

    boost::signal<void (BaseEvent *event)>  sEvent;

    //

    void sendPrivateMessage(const std::wstring & playerName, DWORD playerId, const std::wstring & text);
    void sendChatMessage(ChatType type, const std::wstring & text);
    void sendGuildMessage(const std::wstring & text);

private:
    void onEnterWorld(const clientdata::FragmentEnterWorld * f);
    void onWorldChat(const serverdata::FragmentWorldChat * f);
    void onChatMessage(const serverdata::FragmentChatMessage *f);
    void onPrivateChat(const serverdata::FragmentPrivateChat * f);
    void onFactionChat(const serverdata::FragmentFactionChat * f);
    std::shared_ptr<Connection>   connection_;
    std::vector<Connection::Cookie> cookies_;

    DWORD           myId_;      // from onEnterWorld
    std::wstring    myName_;
};

#endif
